#ifndef SAMPLERS_HLSL
#define SAMPLERS_HLSL


SamplerState pointClampSampler: register(s0);
SamplerState linearClampSampler: register(s1);
SamplerState anisotropicWrapSampler: register(s2);
SamplerComparisonState pointClampComparisonSampler: register(s3);
SamplerComparisonState linearClampComparisonSampler: register(s4);
SamplerComparisonState anisotropicClampComparisonSampler: register(s5);
SamplerState pointBorderGreySampler: register(s6);
SamplerState linearBorderGreySampler: register(s7);
SamplerState pointWrapSampler: register(s8);
SamplerState linearWrapSampler: register(s9);


#endif
