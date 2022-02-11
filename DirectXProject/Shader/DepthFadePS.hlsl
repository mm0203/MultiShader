// デプスフェード
// 通常、パーティクルを散らすと周辺のポリゴンに刺さり、
// ポリゴンとパーティクルの境目が目立ってしまうことがある
// 境目を目出さなくするために、境目に近いほど透明になるような処理を行う。
// 処理に際しては深度バッファを使って、今から描画しようとしているオブジェクトの
// 深度と、既に描きこまれているオブジェクトの深度が近ければ、めり込むと見なして、
// 透過表現を行う

struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 shadowPos : TEXCOOR0;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);

	// スクリーン上に変換した座標をUV座標として取得
	float2 uv = pin.shadowPos.xy / pin.shadowPos.w;

	// -1～1 0～1に変換
	uv = uv * 0.5f + 0.5f;
	uv.y = 1.0f - uv.y;

	// スクリーン上へ変化した座標の奥行を0~1で取得
	// zをそのまま使うと単純なカメラの距離(1mとか2m)と見なされて使いにくい
	float objDepth = pin.shadowPos.z / pin.shadowPos.w;

	// テクスチャに書き込んである深度値
	float renderDepth = tex.Sample(samp, uv);

	// 二つの深度値の差から透明具合を計算　差が小さいほど透明
	float fade = saturate(renderDepth - objDepth);

	// 深度値の差はニアとファー程離れていないとはっきり表示されない
	// ニアに近いオブジェ同士の差がはっきりするように数字を掛けて大きくする
	fade *= 1000.0f;

	color.a = fade;

	return color;
}