//--------------------------------------------------------------------------------------
// File: SkinningShaders.fx
//
// Copyright (c) Microsoft Corporation.
//--------------------------------------------------------------------------------------
#define NUM_LIGHTS (2)

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
static const unsigned int MAX_NUM_BONES = 256u;
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register (s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnCameraMovement

  Summary:  Constant buffer used for view transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnCameraMovement: register(b0) {
    matrix View;
    float4 CameraPosition;
}

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
cbuffer cbChangesEveryFrame : register(b2)
{
    matrix World;
    float4 OutputColor;
}

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbLights

  Summary:  Constant buffer used for shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbLights : register(b3) {
    float4 LightPositions[NUM_LIGHTS];
    float4 LightColors[NUM_LIGHTS];
}

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbSkinning

  Summary:  Constant buffer used for skinning
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbSkinning : register(b4)
{
    matrix BoneTransforms[MAX_NUM_BONES];
}

//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_INPUT

  Summary:  Used as the input to the vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct VS_SKINNING_INPUT {
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    int4 BoneIndices : BONEINDICES;
    float4 BoneWeights : BONEWEIGHTS;

    
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_PHONG_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_SKINNING_INPUT {
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 WorldPosition : WORLDPOS;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------

PS_SKINNING_INPUT VSSkinning(VS_SKINNING_INPUT input)
{
    PS_SKINNING_INPUT output;
    
    
    output.TexCoord = input.TexCoord;



    matrix skinTransform = (matrix)0;
    skinTransform += mul(input.BoneWeights.x, BoneTransforms[input.BoneIndices.x]);
    skinTransform += mul(input.BoneWeights.y, BoneTransforms[input.BoneIndices.y]);
    skinTransform += mul(input.BoneWeights.z, BoneTransforms[input.BoneIndices.z]);
    skinTransform += mul(input.BoneWeights.w, BoneTransforms[input.BoneIndices.w]);

    output.Position = mul(input.Position, skinTransform);
    output.WorldPosition = mul(output.Position, World);
    output.Position = mul(output.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);

    output.Normal = mul(input.Normal, skinTransform);
    output.Normal = normalize(mul(float4(output.Normal, 1), World).xyz);
    

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 PSSkinning(PS_SKINNING_INPUT input) : SV_Target
{
    float3 diffuse = 0.0f;
    float3 ambience = float3(0.1f, 0.1f, 0.1f);
    float3 ambient = float3(0.0f, 0.0f, 0.0f);

    float3 specular = float3(0.0f, 0.0f, 0.0f);
    float3 viewDirection = normalize(input.WorldPosition - CameraPosition.xyz);

    for (uint i = 0; i < NUM_LIGHTS; ++i)
    {
        ambient +=
            ambience * txDiffuse.Sample(samLinear, input.TexCoord).rgb * LightColors[i].xyz;


        float3 lightDirection = normalize(input.WorldPosition - LightPositions[i].xyz);
        float3 lambertian = dot(normalize(input.Normal), -lightDirection);
        diffuse +=
            saturate(lambertian) * txDiffuse.Sample(samLinear, input.TexCoord).rgb * LightColors[i].xyz;

        float3 reflectDirection = reflect(lightDirection, input.Normal);
        specular += pow(saturate(dot(-viewDirection, reflectDirection)), 20.0f) * LightColors[i].xyz
            * txDiffuse.Sample(samLinear, input.TexCoord).rgb;

    }
    return float4(saturate(diffuse + specular + ambient), 1);


}
