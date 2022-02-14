struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : TEXCOORD1;
	float4 depth : TEXCOORD2;
};
struct PS_OUT
{
	float4 albedo : SV_TARGET0;
	float4 depth : SV_TARGET1;
};

cbuffer Color : register(b0)
{
	float4 color;
};

PS_OUT main(PS_IN pin)
{
	PS_OUT pout;

	pout.albedo = color;
	pout.depth = float4(pin.depth.z / pin.depth.w, 0, 0, 1);

	return pout;
}