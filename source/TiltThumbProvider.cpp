#include <TiltThumbProvider.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <Shlwapi.h>
#include <stb_image.h>
#include <stb_image_resize2.h>

#include <Globals.hpp>

namespace
{
#pragma pack(push, 1)
	struct TiltFileHeader
	{
		std::uint32_t Sentinel;      // 'tilT' = 0x546C6974
		std::uint16_t HeaderSize;    // Size of this header (usually 16)
		std::uint16_t HeaderVersion; // Version (usually 1)
		std::uint32_t Unused1;
		std::uint32_t Unused2;
	};

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
#pragma pack(pop)

	constexpr std::uint32_t TiltFileSentinel = 0x546C6974; // 'tilT'
	constexpr std::uint32_t ZipLocalFileHeaderSignature = 0x04034B50;
	constexpr std::uint16_t ZipCompressionStore   = 0;
	constexpr std::uint16_t ZipCompressionDeflate = 8;


	std::vector<std::byte> ReadFileBytes(const wchar_t* path)
	{
		std::vector<std::byte> data;
		HANDLE file = CreateFileW(
			path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, nullptr
		);
		if( file == INVALID_HANDLE_VALUE )
		{
			return data;
		}

		LARGE_INTEGER fileSize = {};
		if( GetFileSizeEx(file, &fileSize) == 0 || fileSize.QuadPart <= 0
			|| fileSize.QuadPart > static_cast<LONGLONG>(0x7FFFFFFF) )
		{
			CloseHandle(file);
			return data;
		}

		data.resize(static_cast<std::size_t>(fileSize.QuadPart));
		DWORD bytesRead = 0;
		if( ReadFile(
			file, data.data(), static_cast<DWORD>(data.size()), &bytesRead,
			nullptr
		) == 0
			|| bytesRead != data.size() )
		{
			data.clear();
		}

		CloseHandle(file);
		return data;
	}

	bool GetOverlayPath(wchar_t* outPath, std::size_t outPathLen)
	{
		if( outPath == nullptr || outPathLen == 0 )
		{
			return false;
		}

		HMODULE module = nullptr;
		if( GetModuleHandleExW(
				GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
					| GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				reinterpret_cast<LPCWSTR>(&GetOverlayPath), &module
			) == 0 )
		{
			return false;
		}

		wchar_t modulePath[MAX_PATH] = {};
		if( GetModuleFileNameW(module, modulePath, MAX_PATH) == 0 )
		{
			return false;
		}

		if( PathRemoveFileSpecW(modulePath) == FALSE )
		{
			return false;
		}

		return PathCombineW(outPath, modulePath, L"overlay-icon.png") != nullptr;
	}

	struct OverlayCache
	{
		std::vector<std::uint8_t> Rgba;
		int                       Width  = 0;
		int                       Height = 0;
		bool                      Available = false;
	};

	OverlayCache LoadOverlay()
	{
		OverlayCache cache;
		wchar_t overlayPath[MAX_PATH] = {};
		if( !GetOverlayPath(overlayPath, MAX_PATH) )
		{
			return cache;
		}

		const auto bytes = ReadFileBytes(overlayPath);
		if( bytes.empty() )
		{
			return cache;
		}

		int width = 0;
		int height = 0;
		int channels = 0;
		stbi_uc* decoded = stbi_load_from_memory(
			reinterpret_cast<const stbi_uc*>(bytes.data()),
			static_cast<int>(bytes.size()), &width, &height, &channels, 4
		);
		if( decoded == nullptr || width <= 0 || height <= 0 )
		{
			return cache;
		}

		cache.Rgba.assign(decoded, decoded + (width * height * 4));
		stbi_image_free(decoded);
		cache.Width = width;
		cache.Height = height;
		cache.Available = true;
		return cache;
	}

	const OverlayCache& GetOverlayCache()
	{
		static OverlayCache cache;
		static std::once_flag loadOnce;
		std::call_once(loadOnce, []() { cache = LoadOverlay(); });
		return cache;
	}

	void BlendOverlay(
		std::uint8_t* baseRgba, int baseWidth, int baseHeight,
		const std::uint8_t* overlayRgba, int overlayWidth, int overlayHeight,
		int dstX, int dstY
	)
	{
		for( int y = 0; y < overlayHeight; ++y )
		{
			if( dstY + y < 0 || dstY + y >= baseHeight )
			{
				continue;
			}
			for( int x = 0; x < overlayWidth; ++x )
			{
				if( dstX + x < 0 || dstX + x >= baseWidth )
				{
					continue;
				}

				const int srcIndex = (y * overlayWidth + x) * 4;
				const int dstIndex = ((dstY + y) * baseWidth + (dstX + x)) * 4;

				const std::uint8_t srcA = overlayRgba[srcIndex + 3];
				if( srcA == 0 )
				{
					continue;
				}

				const std::uint8_t invA = static_cast<std::uint8_t>(255 - srcA);

				for( int c = 0; c < 3; ++c )
				{
					const std::uint8_t srcC = overlayRgba[srcIndex + c];
					const std::uint8_t dstC = baseRgba[dstIndex + c];
					baseRgba[dstIndex + c] = static_cast<std::uint8_t>(
						(srcC * srcA + dstC * invA + 127) / 255
					);
				}

				const std::uint8_t dstA = baseRgba[dstIndex + 3];
				baseRgba[dstIndex + 3] = static_cast<std::uint8_t>(
					(srcA + (dstA * invA + 127) / 255)
				);
			}
		}
	}
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
	std::fprintf(stderr, "[TILT] FindThumbnailPng: File size=%zu bytes\n", Bytes.size());

	// .tilt files have a header before the ZIP data
	// Read the header to determine where ZIP data starts
	if( Bytes.size() < sizeof(TiltFileHeader) )
	{
		std::fprintf(stderr, "[TILT] ERROR: File too small for .tilt header (need %zu bytes)\n", sizeof(TiltFileHeader));
		return {};
	}

	const auto* TiltHeader = reinterpret_cast<const TiltFileHeader*>(Bytes.data());

	if( TiltHeader->Sentinel != TiltFileSentinel )
	{
		std::fprintf(stderr, "[TILT] ERROR: Invalid .tilt sentinel (expected 0x%08X, got 0x%08X)\n",
			TiltFileSentinel, TiltHeader->Sentinel);
		return {};
	}

	std::fprintf(stderr, "[TILT] Valid .tilt header: version=%u, headerSize=%u\n",
		TiltHeader->HeaderVersion, TiltHeader->HeaderSize);

	// Skip the .tilt header (using the size from the header itself)
	std::size_t Offset = TiltHeader->HeaderSize;
	int FileCount = 0;

	if( Offset > Bytes.size() )
	{
		std::fprintf(stderr, "[TILT] ERROR: Header size %u exceeds file size\n", TiltHeader->HeaderSize);
		return {};
	}

	std::fprintf(stderr, "[TILT] Starting ZIP parse at offset %zu\n", Offset);

	// Debug: show first 32 bytes at ZIP offset
	std::fprintf(stderr, "[TILT] First 32 bytes at offset %zu: ", Offset);
	for( std::size_t i = 0; i < 32 && (Offset + i) < Bytes.size(); ++i )
	{
		std::fprintf(stderr, "%02X ", static_cast<unsigned char>(Bytes[Offset + i]));
	}
	std::fprintf(stderr, "\n");

	while( Offset + sizeof(ZipLocalFileHeader) <= Bytes.size() )
	{
		const auto* Header = reinterpret_cast<const ZipLocalFileHeader*>(
			Bytes.data() + Offset
		);

		if( Header->Signature != ZipLocalFileHeaderSignature )
		{
			std::fprintf(stderr, "[TILT] Invalid ZIP signature at offset %zu (expected 0x%08X, got 0x%08X)\n",
				Offset, ZipLocalFileHeaderSignature, Header->Signature);
			break;
		}
		FileCount++;

		std::fprintf(stderr, "[TILT] ZIP Header at offset %zu:\n", Offset);
		std::fprintf(stderr, "  Signature: 0x%08X\n", Header->Signature);
		std::fprintf(stderr, "  VersionNeeded: %u\n", Header->VersionNeeded);
		std::fprintf(stderr, "  BitFlag: 0x%04X\n", Header->BitFlag);
		std::fprintf(stderr, "  CompressionMethod: %u\n", Header->CompressionMethod);
		std::fprintf(stderr, "  FileNameLength: %u\n", Header->FileNameLength);
		std::fprintf(stderr, "  ExtraFieldLength: %u\n", Header->ExtraFieldLength);
		std::fprintf(stderr, "  CompressedSize: %u\n", Header->CompressedSize);
		std::fprintf(stderr, "  UncompressedSize: %u\n", Header->UncompressedSize);

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

		std::fprintf(stderr, "[TILT] File #%d: '%s' (len=%u, compressed=%u, uncompressed=%u, compression=%u)\n",
			FileCount, FileName.c_str(), Header->FileNameLength,
			Header->CompressedSize, Header->UncompressedSize, Header->CompressionMethod);

		const std::span<const std::byte> FileBytes(
			Bytes.data() + FileDataOffset, Header->CompressedSize
		);

		if( _stricmp(FileName.c_str(), "thumbnail.png") == 0 )
		{
			std::fprintf(stderr, "[TILT] Found thumbnail.png, compression=%d, compressed=%u, uncompressed=%u\n",
				Header->CompressionMethod, Header->CompressedSize, Header->UncompressedSize);

			if( Header->CompressionMethod == ZipCompressionStore )
			{
				std::fprintf(stderr, "[TILT] Using stored (uncompressed) data\n");
				return std::vector<std::byte>(FileBytes.begin(), FileBytes.end());
			}
			else if( Header->CompressionMethod == ZipCompressionDeflate )
			{
				std::fprintf(stderr, "[TILT] Decompressing with DEFLATE\n");
				int   OutLen = 0;
				auto  ZlibDecompressed = stbi_zlib_decode_malloc(
                    reinterpret_cast<const char*>(FileBytes.data()),
                    static_cast<int>(FileBytes.size()), &OutLen
                );
				if( ZlibDecompressed == nullptr || OutLen <= 0 )
				{
					std::fprintf(stderr, "[TILT] ERROR: Decompression failed, OutLen=%d\n", OutLen);
					return {};
				}

				std::fprintf(stderr, "[TILT] Decompression succeeded, OutLen=%d\n", OutLen);

				std::vector<std::byte> Decompressed(
					reinterpret_cast<std::byte*>(ZlibDecompressed),
					reinterpret_cast<std::byte*>(ZlibDecompressed) + OutLen
				);
				std::free(ZlibDecompressed);
				return Decompressed;
			}
			else
			{
				std::fprintf(stderr, "[TILT] ERROR: Unsupported compression method: %d\n", Header->CompressionMethod);
			}
		}

		Offset = FileDataOffset + Header->CompressedSize;
	}

	std::fprintf(stderr, "[TILT] Scanned %d files, thumbnail.png not found\n", FileCount);
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
	std::fprintf(stderr, "[TILT] GetThumbnail called\n");
	if( FileData.empty() )
	{
		std::fprintf(stderr, "[TILT] ERROR: FileData is empty\n");
		return E_FAIL;
	}

	std::fprintf(stderr, "[TILT] Searching for thumbnail.png in ZIP\n");
	const auto ThumbnailBytes = FindThumbnailPng(FileData);
	if( ThumbnailBytes.empty() )
	{
		std::fprintf(stderr, "[TILT] ERROR: FindThumbnailPng returned empty\n");
		return E_FAIL;
	}

	int      Width        = 0;
	int      Height       = 0;
	int      Channels     = 0;
	using StbImagePtr
		= std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;

	std::fprintf(stderr, "[TILT] Decoding PNG, size=%zu bytes\n", ThumbnailBytes.size());

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
		std::fprintf(stderr, "[TILT] ERROR: PNG decode failed or invalid dimensions (w=%d, h=%d)\n", Width, Height);
		return E_FAIL;
	}

	std::fprintf(stderr, "[TILT] PNG decoded: %dx%d, channels=%d\n", Width, Height, Channels);

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



	// Overlay Open Brush logo in the bottom-right corner.
	const auto& overlay = GetOverlayCache();
	if( overlay.Available )
	{
		const int maxOverlayW = (std::max)(1, Width / 4);
		const int maxOverlayH = (std::max)(1, Height / 4);

		const double scale = (std::min)(
			static_cast<double>(maxOverlayW) / overlay.Width,
			static_cast<double>(maxOverlayH) / overlay.Height
		);

		const int overlayW = static_cast<int>(overlay.Width * scale);
		const int overlayH = static_cast<int>(overlay.Height * scale);

		if( overlayW >= 4 && overlayH >= 4 )
		{
			std::vector<std::uint8_t> overlayResized(
				static_cast<std::size_t>(overlayW) * overlayH * 4
			);
			stbir_resize_uint8_linear(
				overlay.Rgba.data(), overlay.Width, overlay.Height, 0,
				overlayResized.data(), overlayW, overlayH, 0, STBIR_RGBA
			);

			const int padding = (std::max)(2, (std::min)(Width, Height) / 32);
			const int dstX = (std::max)(0, Width - overlayW - padding);
			const int dstY = (std::max)(0, Height - overlayH - padding);

			BlendOverlay(
				reinterpret_cast<std::uint8_t*>(PixelData.data()), Width, Height,
				overlayResized.data(), overlayW, overlayH, dstX, dstY
			);
		}
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
