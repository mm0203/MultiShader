struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float4 shadowUV : TEXCOORD1;
};

Texture2D tex : register(t0);
// �e�̏�񂪊i�[���ꂽ�e�N�X�`��
Texture2D shadowMap : register(t1);
SamplerState samp : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);

	// �����ŕ`�����񂾏����擾���邽�ߊY���̃s�N�Z����UV���W�̂ǂ̈ʒu���v�Z
	float2 uv = pin.shadowUV.xy / pin.shadowUV.w;

	// �������猩�����ɁA�O���̃s�N�Z�����J��������ǂ̒��x����Ă��邩
	float uvDepth = pin.shadowUV.z / pin.shadowUV.w;

	// ���O�ɕ`������ł����J�����̋������擾
	float shadowDepth = shadowMap.Sample(samp, uv).r;

	// �V���h�E�A�N�l
	// �[�x�o�b�t�@�V���h�E�����������Ƃ��ɁA�\�ʂɏo����g�ł����悤�Ȗ͗l
	// �J��������̋����̐��x�̓e�N�X�`���̉𑜓x�ɂ���ĈقȂ�(512x512��1024x1024�ł�
	// �e�����قȂ�)���ۂ̕`��̓s�N�Z���V�F�[�_�ōׂ����v�Z����Ă邽��
	// �v�Z���ꂽ�l�ƁA�e�N�X�`���ɏ������܂�Ă������ł͔����ȍ����o�Ă��܂��B
	// ���̌��ʁA�{���e�����o���Ȃ��ꏊ�ɔg�ł����悤�Ȗ͗l�ȕ����яオ��
	// �����������邽�߂ɂ́A�J��������̋����E�B�������ɂ��炵�Ă����邾���ŉ����ł���B
	// ���ׂ����덷�Ő�����͗l�̂��߁A�덷�������Ȃ��悤�ɒl��傫������
	if (0 < uv.x && uv.x < 1 && 0 < uv.y && uv.y < 1)
	{
		// ���ɕ`�����܂�Ă��鋗���Ɖe�̋������r
		if (shadowDepth + 0.0001f < uvDepth)
		{
			// �e�̋������傫��������e������
			color.rgb = 0;
		}
	}

	//color.rgb = shadowDepth;
	return color;
}