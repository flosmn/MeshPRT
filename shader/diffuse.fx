//=============================================================================
// diffuse.fx by Frank Luna (C) 2004 All Rights Reserved.
//
// Does basic diffuse lighting.
//=============================================================================

#define NUM_CHANNELS	3
float4 aPRTConstants[NUM_CLUSTERS*(1+NUM_CHANNELS*(NUM_PCA/4))];

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

uniform extern float4x4 gWorld;
uniform extern float4x4 gView;
uniform extern float4x4 gProjection;
uniform extern float4x4 gWorldInverseTranspose;

uniform extern float4 gDiffuseMtrl;
uniform extern float4 gSpecularMtrl;
uniform extern float4 gLightColor;
uniform extern float  gSpecularPower;
uniform extern float3 gLightVecW;
uniform extern float3 gLightPosW;
uniform extern float3 gEyePosW;
uniform extern float gReflectivity;


float4 GetPRTDiffuse( int iClusterOffset, float4 vPCAWeights[NUM_PCA/4] )
{         
    float4 vAccumR = float4(0,0,0,0);
    float4 vAccumG = float4(0,0,0,0);
    float4 vAccumB = float4(0,0,0,0);
    
    for (int j=0; j < (NUM_PCA/4); j++) 
    {
        vAccumR += vPCAWeights[j] * aPRTConstants[iClusterOffset+1+(NUM_PCA/4)*0+j];
        vAccumG += vPCAWeights[j] * aPRTConstants[iClusterOffset+1+(NUM_PCA/4)*1+j];
        vAccumB += vPCAWeights[j] * aPRTConstants[iClusterOffset+1+(NUM_PCA/4)*2+j];
    }    

    float4 vDiffuse = aPRTConstants[iClusterOffset];
    vDiffuse.r += dot(vAccumR,1);
    vDiffuse.g += dot(vAccumG,1);
    vDiffuse.b += dot(vAccumB,1);
    
    return vDiffuse;
}


// per vertex lighting -------------------------------------------------------------------------------------------
 
struct OutputPerVertexLightingVS
{
    float4 posH  : POSITION0;
    float4 color : COLOR0;
};


// per pixel lighting -------------------------------------------------------------------------------------------

struct OutputPerPixelLightingVS
{
      float4 posH    : POSITION0;
	  float2 tex	 : TEXCOORD0;
      float3 normalW : TEXCOORD1;
      float3 toEyeW  : TEXCOORD2;
};

OutputPerPixelLightingVS PerPixelLightingVS(float3 posL : POSITION0, 
											float3 normalL : NORMAL0, 
											float2 in_tex : TEXCOORD0,
											uniform bool in_useTex )
{
 	OutputPerPixelLightingVS outVS = (OutputPerPixelLightingVS)0;
	
	outVS.normalW = mul(float4(normalL, 0.0f), gWorldInverseTranspose).xyz;
	outVS.normalW = normalize(outVS.normalW);

	float3 posW = mul( float4( posL, 1.0f ), gWorld).xyz;
	outVS.toEyeW = gEyePosW - posW;

	outVS.posH = mul(float4(posL, 1.0f), gWorld);
	outVS.posH = mul(outVS.posH, gView);
	outVS.posH = mul(outVS.posH, gProjection);
	
	if( in_useTex ) 
		outVS.tex = in_tex;
	else 
		outVS.tex = 0;
	
    return outVS;
}

float4 PerPixelLightingPS(	float4 posH     : POSITION0,
							float2 tex		: TEXCOORD0,
							float3 normalW  : TEXCOORD1,
							float3 toEyeW   : TEXCOORD2,
							uniform bool useTex ) : COLOR
{
	normalW = normalize(normalW);
	toEyeW = normalize(toEyeW);

    float3 r = reflect(gLightVecW, normalW);
    float t  = pow(max(dot(r, toEyeW), 0.0f), gSpecularPower);
	float s = max(dot(-gLightVecW, normalW), 0.0f);

	float4 Albedo = float4(1.0f, 1.0f, 1.0f, 1.0f);
	if( useTex ) Albedo = tex2D(Sampler, tex);

	float3 envMapTex = reflect(-toEyeW, normalW);
	float4 reflectedColor = texCUBE(EnvSampler, envMapTex);

	float4 diffuseMtrl = gReflectivity*reflectedColor + 
							(1.0f-gReflectivity)*(gDiffuseMtrl*Albedo);
	
	float3 spec = t*(gSpecularMtrl*gLightColor).rgb;
    float3 diffuse = s*(diffuseMtrl*gLightColor).rgb;
   	
    float3 final = spec + diffuse;

	return float4(final, gDiffuseMtrl.a * Albedo.a);
}


// lighting with PRT -------------------------------------------------------------------------------------------

struct OutputPrtLightingVS
{
    float4 posH     : POSITION0;    // position of the vertex
    float2 tex		: TEXCOORD0;
    float3 normalW  : TEXCOORD1;
    float3 toEyeW   : TEXCOORD2;
	float4 Diffuse  : COLOR0;      // diffuse color of the vertex  
};

OutputPrtLightingVS PRTDiffuseVS( float3 posL : POSITION,
								  float3 normalL : NORMAL0, 
								  float2 in_tex : TEXCOORD0,
								  int iClusterOffset : BLENDWEIGHT,
								  float4 vPCAWeights[NUM_PCA/4] : BLENDWEIGHT1,
								  uniform bool useTex)
{
    OutputPrtLightingVS outVS;

	outVS.normalW = mul(float4(normalL, 0.0f), gWorldInverseTranspose).xyz;
	outVS.normalW = normalize(outVS.normalW);

	float3 posW = mul( float4( posL, 1.0f ), gWorld).xyz;
	outVS.toEyeW = gEyePosW - posW;
    
    outVS.posH = mul(float4(posL, 1.0f), gWorld);
	outVS.posH = mul(outVS.posH, gView);
	outVS.posH = mul(outVS.posH, gProjection);
    
    outVS.Diffuse = GetPRTDiffuse( iClusterOffset, vPCAWeights );
    outVS.Diffuse *= gDiffuseMtrl;

	if( useTex ) {
		outVS.tex = in_tex;
	}
	else {
		outVS.tex = 0;
	}
    
    return outVS;
}

float4 PRTDiffusePS(float4 posH     : POSITION0,    // position of the vertex
					float2 tex		: TEXCOORD0,
					float3 normalW  : TEXCOORD1,
					float3 toEyeW   : TEXCOORD2,
					float4 Diffuse  : COLOR0, 
					uniform bool useTex) : COLOR
{
    normalW = normalize(normalW);
	toEyeW = normalize(toEyeW);
	
	float4 diffuseColor = Diffuse;
	
	float4 Albedo = float4(1.0f, 1.0f, 1.0f, 1.0f);
	if( useTex ) Albedo = tex2D(Sampler, tex);

	diffuseColor = Albedo * diffuseColor;

	float3 envMapTex = reflect(-toEyeW, normalW);
	float3 reflectedColor = texCUBE(EnvSampler, envMapTex);

	float3 final = gReflectivity*reflectedColor + 
							(1.0f-gReflectivity)*(diffuseColor);
	
	return float4(final, gDiffuseMtrl.a * Albedo.a);
}


// techniques ------------------------------------------------------------------------

technique PerPixelLightingWithTexture
{
    pass P0
    {
        // Specify the vertex and pixel shader associated with this pass.
        vertexShader = compile vs_2_0 PerPixelLightingVS(true);
        pixelShader  = compile ps_2_0 PerPixelLightingPS(true);
	}
}

technique PerPixelLightingWithoutTexture
{
    pass P0
    {
        // Specify the vertex and pixel shader associated with this pass.
        vertexShader = compile vs_2_0 PerPixelLightingVS(false);
        pixelShader  = compile ps_2_0 PerPixelLightingPS(false);
	}
}

technique PRTLightingWithTexture
{
    pass P0
    {          
        vertexShader = compile vs_2_0 PRTDiffuseVS(true);
        pixelShader  = compile ps_2_0 PRTDiffusePS(true);
    }
}

technique PRTLightingWithoutTexture
{
    pass P0
    {          
        vertexShader = compile vs_2_0 PRTDiffuseVS(false);
        pixelShader  = compile ps_2_0 PRTDiffusePS(false);
    }
}