#pragma once

#include <atomic>
#include <cstddef>
#include <span>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Propsys.h>
#include <thumbcache.h>

class TiltThumbProvider : public IThumbnailProvider, IInitializeWithFile
{
public:
	TiltThumbProvider();
	virtual ~TiltThumbProvider();

	// IUnknown
	virtual HRESULT _stdcall QueryInterface(
		const IID& riid, void** ppvObject
	) override;
	virtual ULONG __stdcall AddRef() throw() override;
	virtual ULONG __stdcall Release() throw() override;

	// IInitializeWithFile
	virtual HRESULT _stdcall Initialize(
		LPCWSTR pszFilePath, DWORD grfMode
	) throw() override;

	// IThumbnailProvider
	virtual HRESULT _stdcall GetThumbnail(
		UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha
	) throw() override;

private:
	static std::vector<std::byte>
		FindThumbnailPng(std::span<const std::byte> Bytes);

	std::atomic<std::size_t> ReferenceCount;

	std::vector<std::byte> FileData;
};
