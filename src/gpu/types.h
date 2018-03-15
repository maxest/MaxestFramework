#pragma once


#include "../math/vector.h"

#include <d3d11_1.h>


namespace NMaxestFramework { namespace NGPU
{
	struct STexture
	{
		enum class EType { Read2D, ReadWrite2D, RenderTarget, DepthStencilTarget, Unknown };

		EType type;
		int width, height;
		int mipmapsCount;
		int samplesCount;
		DXGI_FORMAT format;
		uint32 tag;

		ID3D11Texture2D* texture;
		ID3D11RenderTargetView* rtv;
		ID3D11UnorderedAccessView* uav;
		ID3D11DepthStencilView* dsv;
		ID3D11ShaderResourceView* srv;

		NMath::SVector2 PixelSize() const
		{
			return NMath::VectorCustom(1.0f / (float)width, 1.0f / (float)height);
		}
	};

	struct SBuffer
	{
		enum class EType { Vertex, Index, Constant, Read, ReadWrite };

		EType type;
		int size;
		uint32 tag;

		ID3D11Buffer* buffer;
		ID3D11UnorderedAccessView* uav;
		ID3D11ShaderResourceView* srv;	
	};

	enum class ESamplerFilter { Point, Linear, Anisotropic };
	enum class ESamplerAddressing { Wrap, Clamp, BorderGrey };
	enum class ESamplerComparisonFunction { None, Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always };

	typedef STexture TRenderTarget;
	typedef STexture TDepthStencilTarget;
} }
