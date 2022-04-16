//--------------------------------------------------------------------------------------
// File: Shadersasd.fx
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License (MIT).
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register (s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnCameraMovement

  Summary:  Constant buffer used for view transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnCameraMovement:register(b0) {
    matrix View;
};



/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnResize

  Summary:  Constant buffer used for projection transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnResize:register(b1) {
    matrix Projection;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangesEveryFrame

  Summary:  Constant buffer used for world transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnResize:register(b2) {
    matrix World;
};


//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_INPUT

  Summary:  Used as the input to the vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 TexCoord : TEXCOORD0;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.TexCoord = input.TexCoord;

    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
    //return float4(1.0f, 0.0f, 0.0f, 1.0f);
    float4 albedo = txDiffuse.Sample(
        samLinear,
        input.TexCoord);
    return albedo.rgba;
}