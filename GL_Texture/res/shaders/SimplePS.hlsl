//-----------------------------------------------------------------------------
// File : SimplePS.hlsl
// Desc : Simple Pixel Shader.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#define IMG_BINDING(x)  [[vk::combinedImageSampler]][[vk::binding(x)]]

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] float2 TexCoord : TEXCOORD0;
};

///////////////////////////////////////////////////////////////////////////////
// PSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct PSOutput
{
    [[vk::location(0)]] float4 Color : SV_TARGET0;
};

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
IMG_BINDING(1) Texture2D    ColorMap : register(t0);
IMG_BINDING(1) SamplerState ColorSmp : register(s0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
PSOutput main(const VSOutput input)
{
    PSOutput output = (PSOutput)0;
    output.Color = ColorMap.Sample(ColorSmp, input.TexCoord);
    return output;
}
