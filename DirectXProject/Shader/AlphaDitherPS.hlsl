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
	) / 16.0f; // �p�^�[����0~1�Ɏ��߂邽��16�Ŋ���


float4 main(PS_IN pin) : SV_TARGET
{
	// �X�N���[���T�C�Y(-1�`1)����ʃT�C�Y(0�`1280)
	float2 screen = pin.screenPos.xy / pin.screenPos.w;
	screen.x = (screen.x * 0.5f + 0.5f) * 1280.0f;
	screen.y = (screen.y * 0.5f + 0.5f) * 720.0f;

	// ��ʈʒu����f�B�U�t�B���^�[�C���f�b�N�X���v�Z
	int idxX = (int)fmod(screen.x, 4.0f);
	int idxY = (int)fmod(screen.y, 4.0f);

	// �f�B�U�擾
	float dither = BayerMatrix[idxY][idxX];

	float dist = length(pin.worldPos.xyz - cameraPos.xyz);

	dist = dist - clipDist;

	float distAlpha = dist / clipRange;

	// ������0�Ŕj��
	clip(distAlpha - dither);

	// return float4(dither, 0, 0, 1);


	return color;
}