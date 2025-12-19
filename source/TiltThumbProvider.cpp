#include <TiltThumbProvider.hpp>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>
#include <compressapi.h>
#include <stb_image.h>
#include <stb_image_resize2.h>

#include <Globals.hpp>

namespace
{
	struct ZipLocalFileHeader
	{
		std::uint32_t Signature;
		std::uint16_t VersionNeeded;
		std::uint16_t BitFlag;
		std::uint16_t CompressionMethod;
		std::uint16_t ModTime;
		std::uint16_t ModDate;
		std::uint32_t Crc32;
		std::uint32_t CompressedSize;
		std::uint32_t UncompressedSize;
		std::uint16_t FileNameLength;
		std::uint16_t ExtraFieldLength;
	};

	constexpr std::uint32_t ZipLocalFileHeaderSignature = 0x04034B50;
	constexpr std::uint16_t ZipCompressionStore   = 0;
	constexpr std::uint16_t ZipCompressionDeflate = 8;
} // namespace

TiltThumbProvider::TiltThumbProvider() : ReferenceCount(1)
{
	Globals::ReferenceAdd();
}

TiltThumbProvider::~TiltThumbProvider()
{
	Globals::ReferenceRelease();
}

std::vector<std::byte>
	TiltThumbProvider::FindThumbnailPng(std::span<const std::byte> Bytes)
{
	std::size_t Offset = 0;
	while( Offset + sizeof(ZipLocalFileHeader) <= Bytes.size() )
	{
		const auto* Header = reinterpret_cast<const ZipLocalFileHeader*>(
			Bytes.data() + Offset
		);

		if( Header->Signature != ZipLocalFileHeaderSignature )
		{
			break;
		}

		const std::size_t FileNameOffset = Offset + sizeof(ZipLocalFileHeader);
		if( FileNameOffset > Bytes.size() )
		{
			break;
		}
		const std::size_t ExtraFieldOffset
			= FileNameOffset + Header->FileNameLength;
		if( ExtraFieldOffset > Bytes.size() )
		{
			break;
		}
		const std::size_t FileDataOffset
			= ExtraFieldOffset + Header->ExtraFieldLength;
		if( FileDataOffset > Bytes.size() )
		{
			break;
		}

		const std::span<const std::byte> FileNameBytes(
			Bytes.data() + FileNameOffset, Header->FileNameLength
		);
		const std::string FileName(
			reinterpret_cast<const char*>(FileNameBytes.data()),
			FileNameBytes.size()
		);

		const std::span<const std::byte> FileBytes(
			Bytes.data() + FileDataOffset, Header->CompressedSize
		);

		if( _stricmp(FileName.c_str(), "thumbnail.png") == 0 )
		{
			if( Header->CompressionMethod == ZipCompressionStore )
			{
				return std::vector<std::byte>(FileBytes.begin(), FileBytes.end());
			}
			else if( Header->CompressionMethod == ZipCompressionDeflate )
			{
				std::vector<std::byte> Decompressed(
					static_cast<std::size_t>(Header->UncompressedSize)
				);
				COMPRESSOR_HANDLE Decompressor = nullptr;
				if( CreateDecompressor(
						COMPRESS_ALGORITHM_MSZIP, nullptr, &Decompressor
					)
					== FALSE )
				{
					return {};
				}

				size_t DecompressedSize = Decompressed.size();
				const BOOL Success      = Decompress(
					Decompressor, FileBytes.data(), FileBytes.size(),
					Decompressed.data(), Decompressed.size(), &DecompressedSize
				);
				CloseCompressor(Decompressor);
				if( Success == FALSE )
				{
					return {};
				}

				Decompressed.resize(DecompressedSize);
				return Decompressed;
			}
		}

		Offset = FileDataOffset + Header->CompressedSize;
	}

	return {};
}
HRESULT TiltThumbProvider::QueryInterface(const IID& riid, void** ppvObject)
{
	static const QITAB InterfaceTable[] = {
		QITABENT(TiltThumbProvider, IInitializeWithFile),
		QITABENT(TiltThumbProvider, IThumbnailProvider),
		{nullptr},
	};
	return QISearch(this, InterfaceTable, riid, ppvObject);
}

ULONG TiltThumbProvider::AddRef() throw()
{
	return static_cast<std::uint32_t>(++ReferenceCount);
}

ULONG TiltThumbProvider::Release() throw()
{
	const std::size_t NewReferenceCount = --ReferenceCount;
	if( NewReferenceCount == 0 )
	{
		delete this;
	}
	return static_cast<std::uint32_t>(NewReferenceCount);
}

HRESULT TiltThumbProvider::GetThumbnail(
	UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha
) throw()
{
	if( FileData.empty() )
	{
		return E_FAIL;
	}

	const auto ThumbnailBytes = FindThumbnailPng(FileData);
	if( ThumbnailBytes.empty() )
	{
		return E_FAIL;
	}

	int      Width        = 0;
	int      Height       = 0;
	int      Channels     = 0;
	using StbImagePtr
		= std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;

	StbImagePtr DecodedPng(
		stbi_load_from_memory(
			reinterpret_cast<const stbi_uc*>(ThumbnailBytes.data()),
			static_cast<int>(ThumbnailBytes.size()), &Width, &Height, &Channels,
			4
		),
		&stbi_image_free
	);
	if( DecodedPng == nullptr || Width <= 0 || Height <= 0 )
	{
		return E_FAIL;
	}

	const std::size_t PixelCount = static_cast<std::size_t>(Width) * Height;
	std::vector<std::uint32_t>    PixelData(
		reinterpret_cast<std::uint32_t*>(DecodedPng.get()),
		reinterpret_cast<std::uint32_t*>(DecodedPng.get()) + PixelCount
	);
	if( PixelData.empty() )
	{
		return E_FAIL;
	}

	if( static_cast<std::uint32_t>(cx) < static_cast<std::uint32_t>(Width)
		|| static_cast<std::uint32_t>(cx)
			< static_cast<std::uint32_t>(Height) )
	{
		const double Scale
			= (std::min)(cx / static_cast<double>(Width),
						 cx / static_cast<double>(Height));

		const std::uint32_t NewWidth
			= static_cast<std::uint32_t>(Width * Scale);
		const std::uint32_t NewHeight
			= static_cast<std::uint32_t>(Height * Scale);

		if( !NewWidth || !NewHeight )
		{
			return E_FAIL;
		}

		std::vector<std::uint32_t> Resized(NewWidth * NewHeight);

		stbir_resize_uint8_linear(
			reinterpret_cast<const std::uint8_t*>(PixelData.data()), Width,
			Height, 0, reinterpret_cast<std::uint8_t*>(Resized.data()),
			NewWidth, NewHeight, 0, STBIR_RGBA
		);

		Width     = static_cast<int>(NewWidth);
		Height    = static_cast<int>(NewHeight);
		PixelData = std::move(Resized);
	}

	// RGBA to ABGR
	const std::span<std::uint32_t> ThumbnailPixels(
		PixelData.data(), static_cast<std::size_t>(Width) * Height
	);
	for( std::uint32_t& Pixel32 : ThumbnailPixels )
	{
		const std::uint32_t Alpha = (Pixel32 >> 24) & 0xFF;
		const std::uint32_t Red   = (Pixel32 >> 16) & 0xFF;
		const std::uint32_t Green = (Pixel32 >> 8) & 0xFF;
		const std::uint32_t Blue  = (Pixel32 >> 0) & 0xFF;

		Pixel32 = (Alpha << 24) | // AA
				  (Blue << 16) |  // BB
				  (Green << 8) |  // GG
				  (Red << 0);     // RR
	}

	const HBITMAP Bitmap = CreateBitmap(
		static_cast<int>(Width), static_cast<int>(Height), 1, 32,
		PixelData.data()
	);

	if( Bitmap == nullptr )
	{
		return E_FAIL;
	}

	*phbmp    = Bitmap;
	*pdwAlpha = WTSAT_ARGB;
	return S_OK;
}

HRESULT
TiltThumbProvider::Initialize(LPCWSTR pszFilePath, DWORD grfMode) throw()
{
	// Ensure the extension matches
	if( pszFilePath == nullptr
		|| PathMatchSpecW(pszFilePath, L"*.tilt") == FALSE )
	{
		return E_INVALIDARG;
	}

	HANDLE FileHandle = CreateFileW(
		pszFilePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr
	);
	if( FileHandle == INVALID_HANDLE_VALUE )
	{
		return E_FAIL;
	}

	LARGE_INTEGER FileSize = {};
	if( GetFileSizeEx(FileHandle, &FileSize) == 0 || FileSize.QuadPart <= 0 )
	{
		CloseHandle(FileHandle);
		return E_FAIL;
	}

	FileData.resize(static_cast<std::size_t>(FileSize.QuadPart));
	DWORD BytesRead = 0;
	if( ReadFile(
		FileHandle, FileData.data(), static_cast<DWORD>(FileData.size()),
		&BytesRead, nullptr
	) == 0 || BytesRead != FileData.size() )
	{
		FileData.clear();
		CloseHandle(FileHandle);
		return E_FAIL;
	}

	CloseHandle(FileHandle);

	return S_OK;
}
