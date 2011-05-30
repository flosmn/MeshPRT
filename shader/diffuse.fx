//=============================================================================
// diffuse.fx by Frank Luna (C) 2004 All Rights Reserved.
//
// Does basic diffuse lighting.
//=============================================================================

#define NUM_CHANNELS	3
float4 aPRTConstants[NUM_CLUSTERS*(1+NUM_CHANNELS*(NUM_PCA/4))];

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

OutputPerVertexLightingVS PerVertexLightingVS(float3 posL : POSITION0, float3 normalL : NORMAL0)
{
	OutputPerVertexLightingVS outVS = (OutputPerVertexLightingVS)0;
	
	float3 normalW = mul(float4(normalL, 0.0f), gWorldInverseTranspose).xyz;
	normalW = normalize(normalW);

	float3 posW = mul( float4( posL, 1.0f ), gWorld).xyz;

	float3 toEye = normalize( gEyePosW - posW );

	float3 r = reflect( gLightVecW, normalW );
	
	float t = pow(max(dot(r, toEye), 0.0f), gSpecularPower);
	float s = max(dot(-gLightVecW, normalW), 0.0f);
	float3 diffuse = s*(gDiffuseMtrl*gLightColor).rgb;
	float3 specular = t*(gSpecularMtrl*gLightColor).rgb;
	outVS.color.rgb = diffuse + specular;	
	outVS.color.a   = gDiffuseMtrl.a;
	
	outVS.posH = mul(float4(posL, 1.0f), gWorld);
	outVS.posH = mul(outVS.posH, gView);
	outVS.posH = mul(outVS.posH, gProjection);

    return outVS;
}

float4 PerVertexLightingPS(float4 c : COLOR0) : COLOR
{
    return c;
}

// per pixel lighting -------------------------------------------------------------------------------------------

struct OutputPerPixelLightingVS
{
      float4 posH    : POSITION0;
      float3 normalW : TEXCOORD0;
      float3 posW    : TEXCOORD1;
};

OutputPerPixelLightingVS PerPixelLightingVS(float3 posL : POSITION0, float3 normalL : NORMAL0)
{
 	OutputPerPixelLightingVS outVS = (OutputPerPixelLightingVS)0;
	
	outVS.normalW = mul(float4(normalL, 0.0f), gWorldInverseTranspose).xyz;
	outVS.normalW = normalize(outVS.normalW);

	outVS.posW = mul( float4( posL, 1.0f ), gWorld).xyz;
	outVS.posH = mul(float4(posL, 1.0f), gWorld);
	outVS.posH = mul(outVS.posH, gView);
	outVS.posH = mul(outVS.posH, gProjection);
	
    return outVS;
}

float4 PerPixelLightingPS(float3 normalW : TEXCOORD0,
						  float3 posW : TEXCOORD1) : COLOR
{
	normalW = normalize(normalW);

    float3 toEye = normalize(gEyePosW - posW);

    float3 r = reflect(gLightVecW, normalW);
    float t  = pow(max(dot(r, toEye), 0.0f), gSpecularPower);
	float s = max(dot(-gLightVecW, normalW), 0.0f);

    float3 spec = t*(gSpecularMtrl*gLightColor).rgb;
    float3 diffuse = s*(gDiffuseMtrl*gLightColor).rgb;
   	
    return float4(diffuse + spec, gDiffuseMtrl.a);
}

// lighting with PRT -------------------------------------------------------------------------------------------


struct OutputPrtLightingVS
{
    float4 Position  : POSITION;    // position of the vertex
    float4 Diffuse   : COLOR0;      // diffuse color of the vertex
};

OutputPrtLightingVS PRTDiffuseVS( float3 vPos : POSITION, int iClusterOffset : BLENDWEIGHT, float4 vPCAWeights[NUM_PCA/4] : BLENDWEIGHT1)
{
    OutputPrtLightingVS Output;
    
    Output.Position = mul(float4(vPos, 1.0f), gWorld);
	Output.Position = mul(Output.Position, gView);
	Output.Position = mul(Output.Position, gProjection);
    
    Output.Diffuse = GetPRTDiffuse( iClusterOffset, vPCAWeights );
    
    Output.Diffuse *= gDiffuseMtrl;
    
    return Output;
}

float4 PRTDiffusePS(float4 c : COLOR0) : COLOR
{
    return c;
}


// techniques ------------------------------------------------------------------------

technique PerVertexLighting
{
    pass P0
    {
        // Specify the vertex and pixel shader associated with this pass.
        vertexShader = compile vs_2_0 PerVertexLightingVS();
        pixelShader  = compile ps_2_0 PerVertexLightingPS();
	}
}

technique PerPixelLighting
{
    pass P0
    {
        // Specify the vertex and pixel shader associated with this pass.
        vertexShader = compile vs_2_0 PerPixelLightingVS();
        pixelShader  = compile ps_2_0 PerPixelLightingPS();
	}
}

technique PRTLighting
{
    pass P0
    {          
        vertexShader = compile vs_2_0 PRTDiffuseVS();
        pixelShader  = compile ps_2_0 PRTDiffusePS();
    }
}