#include "common.hlsl"
#include "../../../data/gpu/samplers.hlsl"


Texture2D<float4> backBufferDepthStencilTexture: register(t0);
Texture2D<float4> shadowMapTexture: register(t1);
Texture2D<float4> shadowMapTexture_x4: register(t2);
Texture2D<float4> shadowMapMinBlurredTexture: register(t3);
Texture2D<float4> penumbraTexture: register(t4);


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


float2 CheckerResPixelCoordToFullResUV(int2 pixelCoord)
{
	pixelCoord.x *= 2;
	if (pixelCoord.y % 2 == 1)
		pixelCoord.x += 1;
	return ((float2)pixelCoord + float2(0.5f, 0.5f)) * screenPixelSize;
}


float4 main(PS_INPUT input): SV_Target
{
	int2 pixelCoord = (int2)input.position.xy;
#ifndef USE_CHECKER
	float2 uv = input.texCoord;
	float gradientNoise = TwoPi * InterleavedGradientNoise(uv*screenSize);
#else
	float2 uv = CheckerResPixelCoordToFullResUV(pixelCoord);
	float gradientNoise = TwoPi * InterleavedGradientNoise(uv*screenSize * float2(1.0f, 4.0f));
#endif

	//

	float depthSample = backBufferDepthStencilTexture.Sample(pointClampSampler, uv).r;
	float4 position_cameraNDC = float4(uv, depthSample, 1.0f);
	position_cameraNDC.y = 1.0f - position_cameraNDC.y;
	position_cameraNDC.xy = 2.0f*position_cameraNDC.xy - 1.0f;

	float4 position_shadowMapNDC = mul(cameraViewProjTransformInversed_lightViewProjTransform, position_cameraNDC);
	position_shadowMapNDC /= position_shadowMapNDC.w;
	float4 position_shadowMapView = mul(lightProjTransformInversed, position_shadowMapNDC);

	float2 shadowMapUV = position_shadowMapNDC.xy;
	shadowMapUV.y *= -1.0f;
	shadowMapUV = 0.5f*shadowMapUV + 0.5f;
	float z_shadowMapView = -position_shadowMapView.z - 2.0f;

	//

	float penumbra = 1.0f;
#ifdef USE_PENUMBRA
	#if defined(USE_PENUMBRA_MASK)
		penumbra = penumbraTexture.SampleLevel(linearClampSampler, uv, 0).x;
	#elif defined(USE_PENUMBRA_REGULAR)
		#if defined(USE_PENUMBRA_ORIGINAL_SHADOW_MAP)
			penumbra = Penumbra(gradientNoise, shadowMapTexture, shadowMapUV, z_shadowMapView, PENUMBRA_SAMPLES_COUNT);
		#elif defined(USE_PENUMBRA_DOWNSAMPLED_SHADOW_MAP)
			penumbra = Penumbra(gradientNoise, shadowMapTexture_x4, shadowMapUV, z_shadowMapView, PENUMBRA_SAMPLES_COUNT);
		#endif
	#elif defined(USE_MIN_FILTER)
		float shadowMapMinBlurred = shadowMapMinBlurredTexture.SampleLevel(linearClampSampler, shadowMapUV, 0).x;
		penumbra = AvgBlockersDepthToPenumbra(z_shadowMapView, shadowMapMinBlurred);
	#endif
#endif

	float shadow = 0.0f;
	for (int i = 0; i < 16; i++)
	{
		float2 sampleUV = VogelDiskOffset(i, 16, gradientNoise);

		sampleUV = shadowMapUV + shadowFilterMaxSize*penumbra*sampleUV;
		shadow += shadowMapTexture.SampleCmp(linearClampComparisonSampler, sampleUV, z_shadowMapView).x;
	}
	shadow /= 16.0f;
	
	//
	
	return saturate(shadow);
}
