struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// ライト
struct Light
{
	float3 pos;
	float range;
	float4 color;
};
cbuffer Lights : register(b0)
{
	Light lights[30];
};

cbuffer Matrix : register(b1)
{
	float4x4 invVPS;
};

Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D depthTex : register(t2);
SamplerState samp : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);

	float4 pointColor = float4(0, 0, 0, 0);

	// GBufferから各種情報を取得
	color = albedoTex.Sample(samp, pin.uv);
	float3 N = normalTex.Sample(samp, pin.uv).rgb;
	float depth = depthTex.Sample(samp, pin.uv).r;

	// 深度値からワールド座標を再計算
	// 変換処理を逆手順でやればワールド座標が求まる。
	// UVの値が座標になる。
	float4 wolrdPos = float4(pin.uv, depth, 1.0f);
	wolrdPos = mul(wolrdPos, invVPS);
	wolrdPos.xyz /= wolrdPos.w;

	color.rgb = wolrdPos.xyz;

	// 全部のライトに対して処理を行う
	for (int i = 0; i < 30; i++)
	{
		// ピクセルのワールド座標と光源の座標から光ベクトルを計算
		float3 dir = wolrdPos.xyz - lights[i].pos;

		// ピクセル位置までの距離
		float len = length(dir);

		float attenuation = 1.0f - len / lights[i].range;

		// 光は距離の２乗で減衰するので計算した減衰率を乗算
		attenuation = saturate(attenuation);
		attenuation = pow(attenuation, 3.0f);

		// 減衰率と掛けて光源の光を計算
		float4 lightColor = lights[i].color * attenuation;

		// 点光源ごとの光を合算
		pointColor += lightColor;
	}

	// 計算したワールド座標をもとに点光源の処理を行う
	color.rgb = pointColor;

	return color;
}