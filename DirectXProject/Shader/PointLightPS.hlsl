struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 wPos : TEXCOORD0;
};

// ���C�g�̏��
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
	/* �_����
	�ʏ�̑��z�̌��͕��s�����Ƃ����B
	���s�����͑S�̂ɑ΂��Ĉꗥ�̕�����������������Ă���l��\������B
	�_�����ł́A���[���h��Ԃɔz�u����Ă���X�̌�������A
	�`�悵�悤�Ƃ��Ă���I�u�W�F�N�g�̃s�N�Z���Ɍ������Č��������B
	�_�����͏�����낤�����̂悤�ȃI�u�W�F�N�g��\�����鎞�ɗ��p����B
	���s����������������w���Ȃ��̂ɑ΂��A�_�����͂���Ƃ����������
	�����Ă���B���̕����͌����̍��W�ƃs�N�Z���̍��W���������Ƃŋ��߂���B
	*/

	// �S���C�g�����Z������
	float4 pointColor = float4(0, 0, 0, 0);

	// �S���̃��C�g�ɑ΂��ď������s��
	for (int i = 0; i < 30; i++)
	{
		// �s�N�Z���̃��[���h���W�ƌ����̍��W������x�N�g�����v�Z
		float3 dir = pin.wPos - lights[i].pos;

		// �s�N�Z���ʒu�܂ł̋���
		float len = length(dir);

		// �_�����̗L���͈͂����Ɍ��̎コ���v�Z
		// �_�����ɋ߂�(1:���邢)�A����(0:�Â�)
		// L = 10,R = 50 �̏ꍇ�A���邳 = 0.2
		// L = 40,R = 50 �̏ꍇ�A���邳 = 0.8
		// ��L�̂܂܂��Ƌ߂��قǈÂ��Ȃ� �v�Z�����l��1����������ƂŐ������֌W�̐��l�����߂���
		// ���邳(������) = 1 - L / R
		float attenuation = 1.0f - len / lights[i].range;

		// �v�Z���ʂ�0�`1�Ɏ��܂�悤�ɕ␳
		attenuation = saturate(attenuation);

		// ���͋����̂Q��Ō�������̂Ōv�Z��������������Z
		attenuation = pow(attenuation, 3.0f);

		// �������Ɗ|���Č����̌����v�Z
		float4 lightColor = lights[i].color * attenuation;

		// �_�������Ƃ̌������Z
		pointColor += lightColor;
	}

	// ���Z���������ŏI�J���[�Ƃ��ďo��
	color.rgb = pointColor.rgb;

	return color;
}