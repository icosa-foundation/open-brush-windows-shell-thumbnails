#include <cstddef>
#include <cstdint>
#include <cwchar>
#include <type_traits>

#include <Windows.h>
#include <shlobj.h>

#include "TiltThumbProvider.hpp"
#include <Config.hpp>
#include <Globals.hpp>
#include <ThumbnailProviderClassFactory.hpp>

extern "C" IMAGE_DOS_HEADER __ImageBase;

std::int32_t __stdcall DllMain(
	HINSTANCE hDLL, std::uint32_t Reason, void* Reserved
)
{
	switch( Reason )
	{
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hDLL);
		break;
	}
	}

	return true;
}

/// Registry
struct RegistryEntry
{
	HKEY           Root;
	const wchar_t* KeyName;
	const wchar_t* KeyValue;
	DWORD          ValueType;
	const wchar_t* Data;
};

/// Registers this DLL as a COM server
extern "C" HRESULT __stdcall DllRegisterServer()
{
	WCHAR ModulePath[MAX_PATH];
	GetModuleFileNameW(
		reinterpret_cast<HMODULE>(&__ImageBase), ModulePath, MAX_PATH
	);

	const RegistryEntry Registry[] = {
		// clang-format off
		// Register Tilt Handler
		{HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\" TiltThumbHandlerCLSID,                     nullptr,           REG_SZ, TiltThumbHandlerName},
		{HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\" TiltThumbHandlerCLSID L"\\InProcServer32", nullptr,           REG_SZ, ModulePath},
		{HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\" TiltThumbHandlerCLSID L"\\InProcServer32", L"ThreadingModel", REG_SZ, L"Apartment"},
		{HKEY_CURRENT_USER, L"Software\\Classes\\" TiltThumbHandlerExtension L"\\ShellEx\\" IThumbnailProviderCLSID, nullptr, REG_SZ, TiltThumbHandlerCLSID},
		// clang-format on
	};

	// Set all the appropriate registery entries
	for( std::size_t i = 0; i < std::extent_v<decltype(Registry)>; i++ )
	{
		HKEY                 CurKey;
		const RegistryEntry& CurReg = Registry[i];
		RegCreateKeyExW(
			CurReg.Root, CurReg.KeyName, 0, nullptr, REG_OPTION_NON_VOLATILE,
			KEY_SET_VALUE, nullptr, &CurKey, nullptr
		);
		RegSetValueExW(
			CurKey, CurReg.KeyValue, 0, CurReg.ValueType,
			reinterpret_cast<const unsigned char*>(CurReg.Data),
			static_cast<std::uint32_t>(
				(std::wcslen(CurReg.Data) + 1) * sizeof(wchar_t)
			)
		);
		RegCloseKey(CurKey);
	}

	// Further configure the Tilt thumbnail-handler
	{

		HKEY CurKey;
		RegCreateKeyExW(
			HKEY_CURRENT_USER,
			L"Software\\Classes\\CLSID\\" TiltThumbHandlerCLSID, 0, nullptr,
			REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &CurKey, nullptr
		);

		// Don't run this thumbnail handler in a separate process
		DWORD DisableProcessIsolation = 1;
		RegSetValueExW(
			CurKey, L"DisableProcessIsolation", 0, REG_DWORD,
			reinterpret_cast<const unsigned char*>(&DisableProcessIsolation),
			sizeof(DWORD)
		);
		RegCloseKey(CurKey);

		// Use the Photo-Border for this thumbnail-handler
		RegCreateKeyExW(
			HKEY_CURRENT_USER, L"Software\\Classes\\" TiltThumbHandlerExtension,
			0, nullptr, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &CurKey,
			nullptr
		);
		DWORD Treatment = 2;
		RegSetValueExW(
			CurKey, L"Treatment", 0, REG_DWORD,
			reinterpret_cast<const unsigned char*>(&Treatment), sizeof(DWORD)
		);
		RegCloseKey(CurKey);
	}

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
	return S_OK;
}

/// Unregisters this DLL as a COM server
extern "C" HRESULT __stdcall DllUnregisterServer()
{
	const wchar_t* RegistryFolders[] = {
		L"Software\\Classes\\CLSID\\" TiltThumbHandlerCLSID,
		L"Software\\Classes\\" TiltThumbHandlerExtension,
	};

	for( std::size_t i = 0; i < std::extent_v<decltype(RegistryFolders)>; i++ )
	{
		RegDeleteTreeW(HKEY_CURRENT_USER, RegistryFolders[i]);
	}
	return S_OK;
}

extern "C" HRESULT __stdcall DllCanUnloadNow()
{
	return Globals::ReferenceGet() ? S_FALSE : S_OK;
}

extern "C" HRESULT __stdcall DllGetClassObject(
	const IID& rclsid, const IID& riid, void** ppv
)
{
	if( ppv == nullptr )
	{
		return E_INVALIDARG;
	}

	IID TiltThumbHandlerIID;
	IIDFromString(TiltThumbHandlerCLSID, &TiltThumbHandlerIID);

	IClassFactory* ClassFactory = nullptr;
	if( IsEqualCLSID(TiltThumbHandlerIID, rclsid) )
	{
		ClassFactory = new ThumbnailProviderClassFactory<TiltThumbProvider>();
	}
	else
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	if( ClassFactory == nullptr )
	{
		return E_OUTOFMEMORY;
	}
	const HRESULT Result = ClassFactory->QueryInterface(riid, ppv);
	ClassFactory->Release();
	return Result;
}
