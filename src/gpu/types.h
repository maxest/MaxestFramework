#pragma once
#ifdef MAXEST_FRAMEWORK_WINDOWS


#include "../math/vector.h"

#include <d3d11_1.h>


namespace NMaxestFramework { namespace NGPU
{
	struct STexture
	{
		enum class EType { Read2D, ReadWrite2D, RenderTarget, DepthStencilTarget, Unknown };

		EType type;
		int width, height, depth;
		int mipmapsCount;
		int arraysCount;
		int samplesCount;
		DXGI_FORMAT format;
		uint32 tag;

		ID3D11Resource* texture;
		ID3D11RenderTargetView* rtv;
		ID3D11UnorderedAccessView* uav;
		ID3D11DepthStencilView* dsv;
		ID3D11ShaderResourceView* srv;

		void Zero()
		{
			type = EType::Unknown;
			texture = nullptr;
			rtv = nullptr;
			uav = nullptr;
			dsv = nullptr;
			srv = nullptr;
		}

		NMath::SVector2 PixelSize() const
		{
			return NMath::VectorCustom(1.0f / (float)width, 1.0f / (float)height);
		}
	};

	struct SBuffer
	{
		enum class EType { Vertex, Index, Constant, Read, ReadWrite, Unknown };

		EType type;
		int size;
		uint32 tag;

		ID3D11Buffer* buffer;
		ID3D11UnorderedAccessView* uav;
		ID3D11ShaderResourceView* srv;

		void Zero()
		{
			type = EType::Unknown;
			buffer = nullptr;
			uav = nullptr;
			srv = nullptr;
		}
	};

	enum class ESamplerFilter { Point, Linear, Anisotropic };
	enum class ESamplerAddressing { Wrap, Clamp, BorderGrey };
	enum class ESamplerComparisonFunction { None, Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always };

	typedef STexture TRenderTarget;
	typedef STexture TDepthStencilTarget;
} }


#endif
