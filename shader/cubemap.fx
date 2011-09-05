
float4x4 g_InvWVP;

texture g_Texture;

sampler Sampler = sampler_state
{ 
    Texture = (g_Texture);
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

struct VS_Input
{
    float4 Pos : POSITION;
};

struct VS_Output
{
    float4 Pos : POSITION;
    float3 Tex : TEXCOORD0;
};

VS_Output CubeMapVS( VS_Input Input )
{
    VS_Output Output;
    
    Output.Pos = Input.Pos;
    Output.Tex = normalize( mul(Input.Pos, g_InvWVP) );
    
    return Output;
}

float4 CubeMapPS( VS_Output Input ) : COLOR
{
    float4 color = texCUBE( Sampler, Input.Tex );
    color.a = 1.0f;
    return color;
}

technique CubeMap
{
    pass p0
    {
        VertexShader = compile vs_2_0 CubeMapVS();
        PixelShader = compile ps_2_0 CubeMapPS();
    }
}




