struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// �T���v��
SamplerState samp : register(s0);

// �e�N�X�`��
Texture2D normalTex : register(t0);
Texture2D blurTex : register(t1);
Texture2D depthTex : register(t2);

// �萔�o�b�t�@
cbuffer DOVParam : register(b0)
{
	float4 cameraPos;
	float focus;	// ��ʊE�[�x�̃t�H�[�J�X����
	float range;	// �t�H�[�J�X�ʒu����{�P�܂ł̋���
	float2 empty;   // ���g�p
};

cbuffer Matrix : register(b1)
{
	float4x4 invVPS;
};

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

	// �[�x�n���烏�[���h���W�����߂�
	float depth = depthTex.Sample(samp, pin.uv).r;
	float4 world = mul(float4(pin.uv, depth, 1.0f), invVPS);
	world /= world.w;

	// �s�N�Z���ƃJ�����̋����𒲂ׂ�
	float dist = length(world.xyz - cameraPos.xyz);

	// �t�H�[�J�X�ʒu���߂���
	float dov = abs(dist - focus);

	// �͈͂��L����
	dov = dov / range;

	//color.rgb = dov;

	// �ʏ�̃e�N�`���Ƃڂ����̃e�N�X�`�����u�����h����
	color.rgb = lerp(
		normalTex.Sample(samp, pin.uv),	// 0�ɋ߂��ƒʏ�
		blurTex.Sample(samp, pin.uv),	// 1�ɋ߂��Ƃڂ���
		saturate(dov)
	);

	return color;
}