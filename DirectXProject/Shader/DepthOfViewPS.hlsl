struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// サンプラ
SamplerState samp : register(s0);

// テクスチャ
Texture2D normalTex : register(t0);
Texture2D blurTex : register(t1);
Texture2D depthTex : register(t2);

// 定数バッファ
cbuffer DOVParam : register(b0)
{
	float4 cameraPos;
	float focus;	// 被写界深度のフォーカス距離
	float range;	// フォーカス位置からボケまでの距離
	float2 empty;   // 未使用
};

cbuffer Matrix : register(b1)
{
	float4x4 invVPS;
};

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// 深度地からワールド座標を求める
	float depth = depthTex.Sample(samp, pin.uv).r;
	float4 world = mul(float4(pin.uv, depth, 1.0f), invVPS);
	world /= world.w;

	// ピクセルとカメラの距離を調べる
	float dist = length(world.xyz - cameraPos.xyz);

	// フォーカス位置が近いか
	float dov = abs(dist - focus);

	// 範囲を広げる
	dov = dov / range;

	//color.rgb = dov;

	// 通常のテクチャとぼかしのテクスチャをブレンドする
	color.rgb = lerp(
		normalTex.Sample(samp, pin.uv),	// 0に近いと通常
		blurTex.Sample(samp, pin.uv),	// 1に近いとぼかし
		saturate(dov)
	);

	return color;
}