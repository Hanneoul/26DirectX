/*
================================================================================
 [Level 1: Shader Resource View(t#) & Sampler(s#)]
================================================================================
 - 관점: 외부의 거대한 이미지 데이터를 '읽기 전용'으로 가져옴.
 - 특이사항: 메모리 주소를 직접 뒤지는 게 아니라, 'UV'라는 좌표와 'Sampler'라는 
   필터링 도구를 이용해 값을 추출함.
================================================================================
*/

// [t0 레지스터]: Texture2D 형태의 거대 데이터 창고 (SRV)
Texture2D g_Texture : register(t0);

// [s0 레지스터]: 창고의 데이터를 '어떻게' 읽을지 결정하는 렌즈 (Sampler)
// (예: 멀리 있으면 흐릿하게, 가까이 있으면 선명하게 등)
SamplerState g_Sampler : register(s0);

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD; // 텍스처의 어디를 읽을지 나타내는 0~1 사이 좌표
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

// -----------------------------------------------------------------------------
// Vertex Shader: 위치는 그대로, UV 좌표만 픽셀 셰이더로 배달함
// -----------------------------------------------------------------------------
PS_IN VS_Main(VS_IN input)
{
    PS_IN output;
    output.pos = float4(input.pos, 1.0f);
    output.uv = input.uv; // 래스터라이저를 거치며 픽셀마다 보간됨
    return output;
}

// -----------------------------------------------------------------------------
// Pixel Shader: 샘플러를 이용해 창고(Texture)에서 색상을 뽑아냄
// -----------------------------------------------------------------------------
float4 PS_Main(PS_IN input) : SV_Target
{
    /* [메모리 접근 관점]
       g_Texture.Sample(...) 호출 시:
       1. GPU는 input.uv 좌표를 확인함.
       2. g_Sampler의 설정(Linear, Point 등)에 따라 주변 픽셀들을 계산함.
       3. 최종적으로 가공된 색상(float4)을 메모리에서 뽑아 올림.
    */
    float4 color = g_Texture.Sample(g_Sampler, input.uv);
    
    return color;
}