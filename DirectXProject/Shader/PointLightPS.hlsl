struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 wPos : TEXCOORD0;
};

// ライトの情報
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

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	/* 点光源
	通常の太陽の光は平行光源という。
	平行光源は全体に対して一律の方向から光が当たっている様を表現する。
	点光源では、ワールド空間に配置されている個々の光源から、
	描画しようとしているオブジェクトのピクセルに向かって光が差す。
	点光源は松明やろうそくのようなオブジェクトを表現する時に利用する。
	平行光源が一方向しか指さないのに対し、点光源はありとあらゆる方向に
	向いている。この方向は光源の座標とピクセルの座標を引くことで求められる。
	*/

	// 全ライトを合算した光
	float4 pointColor = float4(0, 0, 0, 0);

	// 全部のライトに対して処理を行う
	for (int i = 0; i < 30; i++)
	{
		// ピクセルのワールド座標と光源の座標から光ベクトルを計算
		float3 dir = pin.wPos - lights[i].pos;

		// ピクセル位置までの距離
		float len = length(dir);

		// 点光源の有効範囲を元に光の弱さを計算
		// 点光源に近い(1:明るい)、遠い(0:暗い)
		// L = 10,R = 50 の場合、明るさ = 0.2
		// L = 40,R = 50 の場合、明るさ = 0.8
		// 上記のままだと近いほど暗くなる 計算した値を1から引くことで正しい関係の数値を求められる
		// 明るさ(減衰率) = 1 - L / R
		float attenuation = 1.0f - len / lights[i].range;

		// 計算結果が0～1に治まるように補正
		attenuation = saturate(attenuation);

		// 光は距離の２乗で減衰するので計算した減衰率を乗算
		attenuation = pow(attenuation, 3.0f);

		// 減衰率と掛けて光源の光を計算
		float4 lightColor = lights[i].color * attenuation;

		// 点光源ごとの光を合算
		pointColor += lightColor;
	}

	// 合算した光を最終カラーとして出力
	color.rgb = pointColor.rgb;

	return color;
}