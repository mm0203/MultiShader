struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float4 worldPos : TEXCOORD1;
	float4 screenPos : TEXCOORD2;
}; 


cbuffer Color : register(b0)
{
	float4 color;
};
cbuffer CameraParam : register(b1)
{
	float4 cameraPos;
	float clipDist;
	float clipRange;
	float2 empty;
};

// BayerMAtrix
float4x4 BayerMatrix = float4x4(
	0, 8, 2, 10,
	12, 4, 14, 6,  
	3, 11, 1, 9,
	15, 7, 13, 5
	) / 16.0f; // パターンを0~1に収めるため16で割る


float4 main(PS_IN pin) : SV_TARGET
{
	// スクリーンサイズ(-1～1)→画面サイズ(0～1280)
	float2 screen = pin.screenPos.xy / pin.screenPos.w;
	screen.x = (screen.x * 0.5f + 0.5f) * 1280.0f;
	screen.y = (screen.y * 0.5f + 0.5f) * 720.0f;

	// 画面位置からディザフィルターインデックスを計算
	int idxX = (int)fmod(screen.x, 4.0f);
	int idxY = (int)fmod(screen.y, 4.0f);

	// ディザ取得
	float dither = BayerMatrix[idxY][idxX];

	float dist = length(pin.worldPos.xyz - cameraPos.xyz);

	dist = dist - clipDist;

	float distAlpha = dist / clipRange;

	// 引数を0で破壊
	clip(distAlpha - dither);

	// return float4(dither, 0, 0, 1);


	return color;
}