/*
================================================================================
 [Level 2: Constant Buffer(b#) - 상수 버퍼]
================================================================================
 - 관점: CPU에서 GPU로 '가장 빠르게' 데이터를 전달하는 통로.
 - 특이사항: 16바이트(float4) 단위로 데이터를 묶어서 관리하는 것이 하드웨어 규칙임.
 - 용도: 월드 행렬(Transform), 타이머, 빛의 방향 등 '전역 변수' 성격의 데이터.
================================================================================
*/

// [b0 레지스터]: 상수 버퍼 슬롯
// 물체의 위치를 결정하는 행렬과 애니메이션을 위한 시간을 담음.
cbuffer TransformData : register(b0)
{
    matrix g_matWorld; // 64바이트 (4x4 행렬)
    float g_fTime; // 4바이트 (시간 값)
    float3 g_Padding; // 12바이트 (16바이트 정렬을 위해 억지로 채운 빈 공간)
};

// (Level 1에서 배운 텍스처도 함께 사용함)
Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

// -----------------------------------------------------------------------------
// Vertex Shader: 행렬을 이용해 공간을 이동시키고, 시간으로 흔들어줌
// -----------------------------------------------------------------------------
PS_IN VS_Main(VS_IN input)
{
    PS_IN output;

    // 1. 시간(g_fTime)을 이용해 로컬 좌표에서 약간의 애니메이션 추가
    float3 localPos = input.pos;
    localPos.x += cos(g_fTime) * 0.2f; // 좌우로 출렁임

    // 2. CPU에서 보내준 월드 행렬(g_matWorld)을 곱해 최종 위치 결정
    // mul 함수: 행렬과 벡터를 곱해주는 HLSL 내장 함수임.
    output.pos = mul(float4(localPos, 1.0f), g_matWorld);
    
    output.uv = input.uv;
    return output;
}

// -----------------------------------------------------------------------------
// Pixel Shader: 텍스처를 읽으면서 상수를 이용해 밝기를 조절할 수도 있음
// -----------------------------------------------------------------------------
float4 PS_Main(PS_IN input) : SV_Target
{
    float4 color = g_Texture.Sample(g_Sampler, input.uv);
    
    // 예시: 시간에 따라 삼각형이 깜빡거리게 만듦
    color.rgb *= (0.5f + 0.5f * sin(g_fTime));
    
    return color;
}