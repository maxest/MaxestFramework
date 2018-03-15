#include "common.hlsl"
#include "../../../data/gpu/samplers.hlsl"


Texture2D<float4> backBufferDepthStencilTexture: register(t0);
Texture2D<float4> shadowMapTexture: register(t1);
Texture2D<float4> shadowMapTexture_x4: register(t2);
Texture2D<float4> ignTexture: register(t3); // interleaved gradient noise


cbuffer ConstantBuffer: register(b0)
{
	float4x4 cameraViewProjTransformInversed_lightViewProjTransform;
	float4x4 lightProjTransformInversed;
}


struct PS_INPUT
{
	float4 position: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};


float4 main(PS_INPUT input): SV_Target
{
	float gradientNoise = TwoPi * InterleavedGradientNoise(input.position.xy);
	
	float depthSample = backBufferDepthStencilTexture.Sample(pointClampSampler, input.texCoord).r;
	float4 position_cameraNDC = float4(input.texCoord, depthSample, 1.0f);
	position_cameraNDC.y = 1.0f - position_cameraNDC.y;
	position_cameraNDC.xy = 2.0f*position_cameraNDC.xy - 1.0f;

	float4 position_shadowMapNDC = mul(cameraViewProjTransformInversed_lightViewProjTransform, position_cameraNDC);
	position_shadowMapNDC /= position_shadowMapNDC.w;
	float4 position_shadowMapView = mul(lightProjTransformInversed, position_shadowMapNDC);

	float2 shadowMapUV = position_shadowMapNDC.xy;
	shadowMapUV.y *= -1.0f;
	shadowMapUV = 0.5f*shadowMapUV + 0.5f;
	float z_shadowMapView = -position_shadowMapView.z - 2.0f;
	
//	gradientNoise = ignTexture.SampleLevel(linearWrapSampler, shadowMapUV, 0).x;

#if defined(USE_ORIGINAL_SHADOW_MAP)
	return Penumbra(gradientNoise, shadowMapTexture, shadowMapUV, z_shadowMapView, SAMPLES_COUNT);
#elif defined(USE_DOWNSAMPLED_SHADOW_MAP)
	return Penumbra(gradientNoise, shadowMapTexture_x4, shadowMapUV, z_shadowMapView, SAMPLES_COUNT);
#endif
}
