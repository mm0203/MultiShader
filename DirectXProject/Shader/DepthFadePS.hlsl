// �f�v�X�t�F�[�h
// �ʏ�A�p�[�e�B�N�����U�炷�Ǝ��ӂ̃|���S���Ɏh����A
// �|���S���ƃp�[�e�B�N���̋��ڂ��ڗ����Ă��܂����Ƃ�����
// ���ڂ�ڏo���Ȃ����邽�߂ɁA���ڂɋ߂��قǓ����ɂȂ�悤�ȏ������s���B
// �����ɍۂ��Ă͐[�x�o�b�t�@���g���āA������`�悵�悤�Ƃ��Ă���I�u�W�F�N�g��
// �[�x�ƁA���ɕ`�����܂�Ă���I�u�W�F�N�g�̐[�x���߂���΁A�߂荞�ނƌ��Ȃ��āA
// ���ߕ\�����s��

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

	// �X�N���[����ɕϊ��������W��UV���W�Ƃ��Ď擾
	float2 uv = pin.shadowPos.xy / pin.shadowPos.w;

	// -1�`1 0�`1�ɕϊ�
	uv = uv * 0.5f + 0.5f;
	uv.y = 1.0f - uv.y;

	// �X�N���[����֕ω��������W�̉��s��0~1�Ŏ擾
	// z�����̂܂܎g���ƒP���ȃJ�����̋���(1m�Ƃ�2m)�ƌ��Ȃ���Ďg���ɂ���
	float objDepth = pin.shadowPos.z / pin.shadowPos.w;

	// �e�N�X�`���ɏ�������ł���[�x�l
	float renderDepth = tex.Sample(samp, uv);

	// ��̐[�x�l�̍����瓧������v�Z�@�����������قǓ���
	float fade = saturate(renderDepth - objDepth);

	// �[�x�l�̍��̓j�A�ƃt�@�[������Ă��Ȃ��Ƃ͂�����\������Ȃ�
	// �j�A�ɋ߂��I�u�W�F���m�̍����͂����肷��悤�ɐ������|���đ傫������
	fade *= 1000.0f;

	color.a = fade;

	return color;
}