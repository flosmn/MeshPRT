bool useTextures;
extern bool renderError = false;
extern bool renderExact = false;

uniform extern float4x4 gWorld;
uniform extern float4x4 gView;
uniform extern float4x4 gProjection;
uniform extern float4x4 gWorldInverseTranspose;

float redSHCoeffsLight[NUM_COEFFICIENTS];
float greenSHCoeffsLight[NUM_COEFFICIENTS];
float blueSHCoeffsLight[NUM_COEFFICIENTS];

uniform extern float3 gEyePosW;
uniform extern float gReflectivity;

uniform extern texture AlbedoTex;
uniform extern texture EnvMap;

float4 CalculateError(float4 c1, float4 c2) {
	float diff = 1.0f/3.0f * (sqrt(abs(c1.r-c2.r))+sqrt(abs(c1.g-c2.g))+sqrt(abs(c1.b-c2.b)));

	// heatmap colors
	float4 colors[3];
    colors[0] = float4(0, 0, 1.0f, 1.0f);
    colors[1] = float4(1.0f, 1.0f, 0, 1.0f);
    colors[2] = float4(1.0f, 0, 0, 1.0f);
        
    float4 error = float4(0.0f,0.0f,0.0f,1.0f);
	
	int ix = diff < 0.5f ? 0 : 1;

	if(ix == 0){    
		error = lerp(colors[0], colors[1], 2*diff-ix);	
	}
	if(ix == 1){
		error = lerp(colors[ix], colors[ix+1], 2*diff-ix);	
	}
	return error;
}

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

struct OutputPrtLightingVS
{
    float4 posH     : POSITION0;    // position of the vertex
    float2 tex		: TEXCOORD0;
    float3 normalW  : TEXCOORD1;
    float3 toEyeW   : TEXCOORD2;
	float4 Diffuse  : COLOR0;      // diffuse color of the vertex  
};

OutputPrtLightingVS PRTDiffuseVS( float3 posL : POSITION0,
								  float3 normalL : NORMAL0, 
								  float2 in_tex : TEXCOORD0,
								  float3 shCoeffs[NUM_COEFFICIENTS] : BLENDWEIGHT0,
								  float3 in_exactSHColor : POSITION2,
								  float3 in_index : POSITION3)
{
    OutputPrtLightingVS outVS;

	outVS.normalW = mul(float4(normalL, 0.0f), gWorldInverseTranspose).xyz;
	outVS.normalW = normalize(outVS.normalW);

	float3 posW = mul( float4( posL, 1.0f ), gWorld).xyz;
	outVS.toEyeW = gEyePosW - posW;
    
    outVS.posH = mul(float4(posL, 1.0f), gWorld);
	outVS.posH = mul(outVS.posH, gView);
	outVS.posH = mul(outVS.posH, gProjection);
    
	float4 color = float4(0.0f, 0.0f, 0.0f, in_index.x);

	for(int i = 0; i < NUM_COEFFICIENTS; ++i) {
		color += float4(redSHCoeffsLight[i]   * shCoeffs[i].r,
						greenSHCoeffsLight[i] * shCoeffs[i].g,
						blueSHCoeffsLight[i]  * shCoeffs[i].b,
						0.0f);
	}
	
	outVS.Diffuse = color;
	if(renderExact) outVS.Diffuse = float4(in_exactSHColor, 1.0f);
	if(renderError) outVS.Diffuse = CalculateError(color, float4(in_exactSHColor, 1.0f));
			
	if( useTextures ) {
		outVS.tex = in_tex;
	}
	else {
		outVS.tex = 0;
	}
    
    return outVS;
}

float4 PRTDiffusePS(float4 posH     : POSITION0,
					float2 tex		: TEXCOORD0,
					float3 normalW  : TEXCOORD1,
					float3 toEyeW   : TEXCOORD2,
					float4 Diffuse  : COLOR0) : COLOR
{
    normalW = normalize(normalW);
	toEyeW = normalize(toEyeW);
	
	float4 diffuseColor = Diffuse;
	
	float4 Albedo = float4(1.0f, 1.0f, 1.0f, 1.0f);
	if( useTextures ) Albedo = tex2D(Sampler, tex);

	diffuseColor = Albedo * diffuseColor;

	float3 envMapTex = reflect(-toEyeW, normalW);
	float4 reflectedColor = texCUBE(EnvSampler, envMapTex);
	
	float3 final = gReflectivity*reflectedColor + 
							(1.0f-gReflectivity)*(diffuseColor);
	
	return float4(final, 1.0f);
}


technique PRTLighting
{
    pass P0
    {          
        vertexShader = compile vs_2_0 PRTDiffuseVS();
        pixelShader  = compile ps_2_0 PRTDiffusePS();
    }
}