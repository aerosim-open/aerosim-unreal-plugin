#include "Render/ImageUtil.h"
#include "Actors/CameraSensor.h"
#include "RHISurfaceDataConversion.h"
#include "ImageWriteQueue.h"
#include "HighResScreenshot.h"
#include "RHIGPUReadback.h"

template <typename F>
class ScopedCallback
{
	F fn;

public:
	constexpr ScopedCallback(F&& fn)
		: fn(fn) {}

	~ScopedCallback() { fn(); }
};

namespace ImageUtil
{
	bool DecodePixelsByFormat(
		const void* PixelData,
		int32 SourcePitch,
		FIntPoint Extent,
		EPixelFormat Format,
		FReadSurfaceDataFlags Flags,
		TArrayView<FLinearColor> Out)
	{
		SourcePitch *= GPixelFormats[Format].BlockBytes;
		auto OutPixelCount = Extent.X * Extent.Y;
		switch (Format)
		{
			case PF_G16:
			case PF_R16_UINT:
			case PF_R16_SINT:
				// Shadow maps
				ConvertRawR16DataToFLinearColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			case PF_R8G8B8A8:
				ConvertRawR8G8B8A8DataToFLinearColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			case PF_B8G8R8A8:
				ConvertRawB8G8R8A8DataToFLinearColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			case PF_A2B10G10R10:
				ConvertRawA2B10G10R10DataToFLinearColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			case PF_FloatRGBA:
			case PF_R16G16B16A16_UNORM:
			case PF_R16G16B16A16_SNORM:
				ConvertRawR16G16B16A16FDataToFLinearColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags);
				break;
			case PF_FloatR11G11B10:
				ConvertRawRR11G11B10DataToFLinearColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			case PF_A32B32G32R32F:
				ConvertRawR32G32B32A32DataToFLinearColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags);
				break;
			case PF_A16B16G16R16:
				ConvertRawR16G16B16A16DataToFLinearColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			case PF_G16R16:
				ConvertRawR16G16DataToFLinearColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			case PF_X24_G8: // Depth Stencil
				ConvertRawR24G8DataToFLinearColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags);
				break;
			case PF_R32_FLOAT: // Depth Stencil
				ConvertRawR32DataToFLinearColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags);
				break;
			case PF_R16G16B16A16_UINT:
			case PF_R16G16B16A16_SINT:
				ConvertRawR16G16B16A16DataToFLinearColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			default:
				UE_LOG(LogAerosimConnector, Warning, TEXT("Unsupported format %llu"), (unsigned long long)Format);
				return false;
		}
		return true;
	}

	bool DecodePixelsByFormat(
		const void* PixelData,
		int32 SourcePitch,
		FIntPoint Extent,
		EPixelFormat Format,
		FReadSurfaceDataFlags Flags,
		TArrayView<FColor> Out)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(ImageUtil::DecodePixelsByFormat);
		SourcePitch *= GPixelFormats[Format].BlockBytes;
		auto OutPixelCount = Extent.X * Extent.Y;
		switch (Format)
		{
			case PF_G16:
			case PF_R16_UINT:
			case PF_R16_SINT:
				// Shadow maps
				ConvertRawR16DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			case PF_R8G8B8A8:
				ConvertRawR8G8B8A8DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			case PF_B8G8R8A8:
				ConvertRawB8G8R8A8DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			case PF_A2B10G10R10:
				ConvertRawR10G10B10A2DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			case PF_FloatRGBA:
			case PF_R16G16B16A16_UNORM:
			case PF_R16G16B16A16_SNORM:
				ConvertRawR16G16B16A16FDataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags.GetLinearToGamma());
				break;
			case PF_FloatR11G11B10:
				ConvertRawR11G11B10DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags.GetLinearToGamma());
				break;
			case PF_A32B32G32R32F:
				ConvertRawR32G32B32A32DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags.GetLinearToGamma());
				break;
			case PF_A16B16G16R16:
				ConvertRawR16G16B16A16DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			case PF_G16R16:
				ConvertRawR16G16DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			case PF_DepthStencil: // Depth / Stencil
				ConvertRawD32S8DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags);
				break;
			case PF_X24_G8: // Depth / Stencil
				ConvertRawR24G8DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData(), Flags);
				break;
			case PF_R32_FLOAT: // Depth
				for (uint32 Y = 0; Y < (uint32)Extent.Y; Y++)
				{
					auto SrcPtr = (float*)((uint8*)PixelData + Y * SourcePitch);
					auto DestPtr = Out.GetData() + Y * Extent.X;
					for (uint32 X = 0; X < (uint32)Extent.X; X++)
					{
						*DestPtr = FLinearColor(SrcPtr[0], 0.f, 0.f, 1.f).QuantizeRound();
						++SrcPtr;
						++DestPtr;
					}
				}
				break;
			case PF_R16G16B16A16_UINT:
			case PF_R16G16B16A16_SINT:
				ConvertRawR16G16B16A16DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			case PF_G8:
				ConvertRawR8DataToFColor(Extent.X, Extent.Y, (uint8*)PixelData, SourcePitch, Out.GetData());
				break;
			default:
				UE_LOG(LogAerosimConnector, Warning, TEXT("Unsupported format %llu"), (unsigned long long)Format);
				return false;
		}
		return true;
	}

	bool ReadImageData(
		UTextureRenderTarget2D& RenderTarget,
		TArray<FColor>& Out)
	{
		check(IsInGameThread());
		auto Resource = RenderTarget.GameThread_GetRenderTargetResource();
		FReadSurfaceDataFlags ReadFlags(RCM_UNorm);
		ReadFlags.SetLinearToGamma(true);
		return Resource->ReadPixels(Out, ReadFlags);
	}

	bool ReadImageData(
		UTextureRenderTarget2D& RenderTarget,
		TArray64<FColor>& Out)
	{
		auto Resource = RenderTarget.GameThread_GetRenderTargetResource();
		FReadSurfaceDataFlags ReadFlags(RCM_UNorm);
		ReadFlags.SetLinearToGamma(true);
		Out.SetNum(RenderTarget.GetSurfaceWidth() * RenderTarget.GetSurfaceHeight());
		return Resource->ReadPixelsPtr(Out.GetData(), ReadFlags);
	}

	TUniquePtr<TImagePixelData<FColor>> ReadImageData(
		UTextureRenderTarget2D& RenderTarget)
	{
		const auto Size = FIntPoint(
			RenderTarget.GetSurfaceWidth(),
			RenderTarget.GetSurfaceHeight());
		auto PixelData = MakeUnique<TImagePixelData<FColor>>(Size);
		ReadImageData(RenderTarget, PixelData->Pixels);
		return TUniquePtr<TImagePixelData<FColor>>();
	}

	TFuture<bool> SaveImageData(
		UTextureRenderTarget2D& RenderTarget,
		const FStringView& Path)
	{
		return SaveImageData(ReadImageData(RenderTarget), Path);
	}

	TFuture<bool> SaveImageData(
		TUniquePtr<TImagePixelData<FColor>> Data,
		const FStringView& Path)
	{
		auto& HighResScreenshotConfig = GetHighResScreenshotConfig();
		auto ImageTask = MakeUnique<FImageWriteTask>();
		ImageTask->PixelData = MoveTemp(Data);
		ImageTask->Filename = Path;
		ImageTask->Format = EImageFormat::PNG;
		ImageTask->CompressionQuality = (int32)EImageCompressionQuality::Default;
		ImageTask->bOverwriteFile = true;
		ImageTask->PixelPreProcessors.Add(TAsyncAlphaWrite<FColor>(255));
		return HighResScreenshotConfig.ImageWriteQueue->Enqueue(MoveTemp(ImageTask));
	}

	struct ReadImageDataContext
	{
		EPixelFormat Format;
		FIntPoint Size;
		ReadImageDataAsyncCallback Callback;
		TUniquePtr<FRHIGPUTextureReadback> Readback;
	};

	static void ReadImageDataBegin(
		ReadImageDataContext& Self,
		UTextureRenderTarget2D& RenderTarget,
		ReadImageDataAsyncCallback&& Callback)
	{
		static thread_local auto RenderQueryPool =
			RHICreateRenderQueryPool(RQT_AbsoluteTime);

		auto& CmdList = FRHICommandListImmediate::Get();
		auto Resource = static_cast<FTextureRenderTarget2DResource*>(
			RenderTarget.GetResource());
		auto Texture = Resource->GetRenderTargetTexture();
		if (Texture == nullptr)
			return;
		Self.Callback = std::move(Callback);
		Self.Readback = MakeUnique<FRHIGPUTextureReadback>(
			TEXT("ReadImageData-Readback"));
		Self.Size = Texture->GetSizeXY();
		Self.Format = Texture->GetFormat();
		auto ResolveRect = FResolveRect();
		Self.Readback->EnqueueCopy(CmdList, Texture, ResolveRect);

		auto Query = RenderQueryPool->AllocateQuery();
		CmdList.EndRenderQuery(Query.GetQuery());
		CmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
		uint64 DeltaTime;
		RHIGetRenderQueryResult(Query.GetQuery(), DeltaTime, true);
		Query.ReleaseQuery();
	}

	static void ReadImageDataEnd(
		ReadImageDataContext& Self)
	{
		int32 RowPitch, BufferHeight;
		auto MappedPtr = Self.Readback->Lock(RowPitch, &BufferHeight);
		if (MappedPtr != nullptr)
		{
			ScopedCallback Unlock = [&] { Self.Readback->Unlock(); };
			Self.Callback(MappedPtr, RowPitch, BufferHeight, Self.Format, Self.Size);
		}
	}

	static void ReadImageDataEndAsync(
		ReadImageDataContext&& Self)
	{
		AsyncTask(
			ENamedThreads::HighTaskPriority, [Self = std::move(Self)]() mutable {
				while (!Self.Readback->IsReady())
					std::this_thread::yield();
				ReadImageDataEnd(Self);
			});
	}

	static void ReadImageDataAsyncCommand(
		UTextureRenderTarget2D& RenderTarget,
		ReadImageDataAsyncCallback&& Callback)
	{
		ReadImageDataContext Context = {};
		ReadImageDataBegin(Context, RenderTarget, std::move(Callback));
		ReadImageDataEndAsync(std::move(Context));
	}

	bool ReadImageDataAsync(
		UTextureRenderTarget2D& RenderTarget,
		ReadImageDataAsyncCallback&& Callback)
	{
		if (IsInRenderingThread())
		{
			ReadImageDataAsyncCommand(
				RenderTarget,
				std::move(Callback));
		}
		else
		{
			ENQUEUE_RENDER_COMMAND(ReadImageDataAsyncCmd)([&RenderTarget, Callback = std::move(Callback)](auto& CmdList) mutable {
				ReadImageDataContext Context = {};
				ReadImageDataBegin(Context, RenderTarget, std::move(Callback));
				ReadImageDataEnd(Context);
			});
		}
		return true;
	}

	bool ReadSensorImageDataAsync(
		ACameraSensor* Sensor,
		ReadImageDataAsyncCallback&& Callback)
	{
		TArray<FColor> Pixels;
		auto RenderTarget = Sensor->GetRenderTarget();
		if (RenderTarget == nullptr)
			return false;
		return ReadImageDataAsync(
			*RenderTarget,
			std::move(Callback));
	}

	bool ReadImageDataAsyncFColor(
		UTextureRenderTarget2D& RenderTarget,
		ReadImageDataAsyncCallbackFColor&& Callback)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(ImageUtil::ReadImageDataAsyncFColor);
		return ReadImageDataAsync(RenderTarget, [Callback = std::move(Callback)](const void* Mapping, size_t RowPitch, size_t BufferHeight, EPixelFormat Format, FIntPoint Size) -> bool {
			FReadSurfaceDataFlags Flags;
			TArray<FColor> Pixels;
			Pixels.SetNum(Size.X * Size.Y);
			if (!DecodePixelsByFormat(Mapping, RowPitch, Size, Format, Flags, Pixels))
				return false;
			return Callback(Pixels, Size);
		});
	}

	bool ReadSensorImageDataAsyncFColor(
		ACameraSensor* Sensor,
		ReadImageDataAsyncCallbackFColor&& Callback)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(ImageUtil::ReadSensorImageDataAsyncFColor);
		auto RenderTarget = Sensor->GetRenderTarget();
		if (RenderTarget == nullptr)
			return false;
		return ReadImageDataAsyncFColor(*RenderTarget, std::move(Callback));
	}

	bool ReadImageDataAsyncFLinearColor(
		UTextureRenderTarget2D& RenderTarget,
		ReadImageDataAsyncCallbackFLinearColor&& Callback)
	{
		return ReadImageDataAsync(RenderTarget, [Callback = std::move(Callback)](const void* Mapping, size_t RowPitch, size_t BufferHeight, EPixelFormat Format, FIntPoint Size) -> bool {
			FReadSurfaceDataFlags Flags;
			TArray<FLinearColor> Pixels;
			Pixels.SetNum(Size.X * Size.Y);
			if (!DecodePixelsByFormat(Mapping, RowPitch, Size, Format, Flags, Pixels))
				return false;
			return Callback(Pixels, Size);
		});
	}

	bool ReadSensorImageDataAsyncFLinearColor(
		ACameraSensor* Sensor,
		ReadImageDataAsyncCallbackFLinearColor&& Callback)
	{
		auto RenderTarget = Sensor->GetRenderTarget();
		if (RenderTarget == nullptr)
			return false;
		return ReadImageDataAsyncFLinearColor(*RenderTarget, std::move(Callback));
	}

	bool ReadSensorDataAsyncRaw(
		ACameraSensor* Sensor,
		ReadImageDataAsyncCallbackRaw&& Callback)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(ImageUtil::ReadSensorImageDataAsyncRaw);
		auto RenderTarget = Sensor->GetRenderTarget();
		if (RenderTarget == nullptr)
			return false;
		return ReadImageDataAsyncRaw(*RenderTarget, std::move(Callback));
	}

	bool ReadImageDataAsyncRaw(
		UTextureRenderTarget2D& RenderTarget,
		ReadImageDataAsyncCallbackRaw&& Callback)
	{
		return ReadImageDataAsync(RenderTarget, [Callback = std::move(Callback)](const void* Mapping, size_t RowPitch, size_t BufferHeight, EPixelFormat Format, FIntPoint Size) -> bool {
			TRACE_CPUPROFILER_EVENT_SCOPE(ImageUtils::ReadImageDataAsync);
			
			uint32 Height = Size.Y;
			uint32 Width = Size.X;
			const uint32 DstPitch = Width * sizeof(FColor);
			const uint32 SourcePitch = RowPitch * GPixelFormats[Format].BlockBytes;

			// If source & dest pitch matches, don't operate so far.
			if (DstPitch == SourcePitch)
			{
				return Callback(Mapping, Size);
			}
			else
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(ImageUtils::ReadImageDataAsync::PitchMismatch);
				// Need to copy row wise since the Pitch does not match the Width.
				// Pitch --> Number of bytes per image row.
				TArray<FColor> Pixels;
				Pixels.SetNumUninitialized(Size.X * Size.Y);
				uint8* In = (uint8*)Mapping;
				FColor* Out = Pixels.GetData();
				// Check if Source pitch is bigger than Dst Pitch to avoid rise conditions while copying. Source Pitch should be equal or less than DestPitch. 
				// Pitch means number of bytes per image row according to Unreal, this code is based on ConvertRawB8G8R8A8DataToFColor function from engine can be found on RHISurfaceDataConversion.h
				check(SourcePitch > DstPitch);

				ParallelFor(Height, [&](int32 Y)
				{
					FColor* SrcPtr = (FColor*)(In + Y * SourcePitch);
					FColor* DestPtr = Out + Y * Width;
					FMemory::Memcpy(DestPtr, SrcPtr, DstPitch);
				});

				return Callback((void*)Out, Size);
			}
		});
	}

} // namespace ImageUtil
