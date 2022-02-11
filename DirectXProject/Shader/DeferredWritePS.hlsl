struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : TEXCOORD1;
	float4 depth : TEXCOORD2;
};

// マルチレンダーターゲット
// 1度の描画で複数のテクスチャに書き込む。
// CPUで複数のレンダーターゲットを設定。
// ピクセルシェーダで複数のターゲットに出力設定が必要。

struct PS_OUT
{
	float4 albedo : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 depth : SV_TARGET2;
};

PS_OUT main(PS_IN pin) : SV_TARGET
{
	PS_OUT pout;
	pout.albedo = float4(1, 1, 1, 1);
	pout.normal = float4(pin.normal, 1.0f);
	pout.depth = float4(pin.depth.z / pin.depth.w, 0.0f, 0.0f, 1.0f);
	return pout;
}