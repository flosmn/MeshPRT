//=============================================================================
// diffuse.fx by Frank Luna (C) 2004 All Rights Reserved.
//
// Does basic diffuse lighting.
//=============================================================================

uniform extern bool useTextures;

uniform extern float4x4 gWorld;
uniform extern float4x4 gView;
uniform extern float4x4 gProjection;
uniform extern float4x4 gWorldInverseTranspose;

uniform extern float3 gEyePosW;
uniform extern float gReflectivity;

uniform extern texture AlbedoTex;
uniform extern texture EnvMap;

sampler Sampler = sampler_state
{ 
    Texture = AlbedoTex;
    MipFilter = LINEAR; 
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

sampler EnvSampler = sampler_state
{ 
    Texture = EnvMap;
    MipFilter = LINEAR; 
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};


struct OutputErrorVisualizationVS
{
    float4 posH     : POSITION0;    // position of the vertex
    float2 tex		: TEXCOORD0;
    float3 normalW  : TEXCOORD1;
    float3 toEyeW   : TEXCOORD2;
	float4 Diffuse  : COLOR0;      // diffuse color of the vertex  
};

OutputErrorVisualizationVS ErrorVisualizationVS( float3 posL : POSITION,
								  float3 normalL : NORMAL0, 
								  float2 in_tex : TEXCOORD0,
								  float4 in_diffuseColorApprox : BLENDWEIGHT1,
								  float4 in_diffuseColor : BLENDWEIGHT2)
{
    OutputErrorVisualizationVS outVS;

	outVS.normalW = mul(float4(normalL, 0.0f), gWorldInverseTranspose).xyz;
	outVS.normalW = normalize(outVS.normalW);

	float3 posW = mul( float4( posL, 1.0f ), gWorld).xyz;
	outVS.toEyeW = gEyePosW - posW;
    
    outVS.posH = mul(float4(posL, 1.0f), gWorld);
	outVS.posH = mul(outVS.posH, gView);
	outVS.posH = mul(outVS.posH, gProjection);
    
	float4 diffApprox = in_diffuseColorApprox;
	float4 diffExact = in_diffuseColor;
	float diff = 	sqrt(abs(diffApprox.x - diffExact.x)) 
				  + sqrt(abs(diffApprox.y - diffExact.y))
				  + sqrt(abs(diffApprox.z - diffExact.z));
	diff = diff / 3.0f;

	float4 red = float4(1.0f, 0.0f, 0.0f, 0.0f);
	float4 white = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
	outVS.Diffuse = diff * red + (1 - diff) * white;
    outVS.tex = 0;
	    
    return outVS;
}

float4 ErrorVisualizationPS(float4 posH     : POSITION0,
					float2 tex		: TEXCOORD0,
					float3 normalW  : TEXCOORD1,
					float3 toEyeW   : TEXCOORD2,
					float4 in_diffuse  : COLOR0) : COLOR
{
    normalW = normalize(normalW);
	toEyeW = normalize(toEyeW);
	return in_diffuse;	
}


technique ErrorVisualization
{
    pass P0
    {          
        vertexShader = compile vs_2_0 ErrorVisualizationVS();
        pixelShader  = compile ps_2_0 ErrorVisualizationPS();
    }
}