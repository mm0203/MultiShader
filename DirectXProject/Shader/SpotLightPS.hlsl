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
	float3 dir;
	float angle;
};

cbuffer Lights : register(b0)
{
	Light lights[30];
};

float4 main(PS_IN pin) : SV_TARGET
{
    float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);

    float4 spotColor = float4(0, 0, 0, 0);
    for (int i = 0; i < 30; i++)
    {
        // �_�����̏����Ɠ����������⌸���������߂�

        float3 dir = pin.wPos - lights[i].pos;
        float attenuation = length(dir) / lights[i].range;
        attenuation = 1.0f - saturate(attenuation);
        //attenuation = 1.0f - pow(attenuation, 1.0f);
        attenuation = pow(attenuation, 1.0f);

        // �_�����ŋ��߂��������ɃX�|�b�g���C�g�̏�����K�p����
        // �X�|�b�g���C�g�͈̔͂̎��܂��Ă��邩
        // �܂��́A�͈͊O�͌�������0�ɂ���(�����͂��Ȃ�

        // �_��������s�N�Z���Ɍ������x�N�g���ƁA�X�|�b�g���C�g�̌����̃x�N�g��
        // ��̃x�N�g���̊Ԃ̊p�x���Ƃ炷�͈͓��ł���΁A�X�|�b�g���C�g��
        // �Ƃ点��͈͂Ƃ�����
        // ���ς̌������g���āA��̃x�N�g���̂Ȃ��p�����߂�
        // a�Eb(��̃x�N�g���̓��ό���)
        // |a|(�x�N�g��a�̒���)
        // ��(��̃x�N�g���̂Ȃ��p)
        // a�Eb = |a| * |b| * cos(��)
        // a�Eb / (|a| * |b|) = cos(��)
        // acos(a�Eb / (|a| * |b|) ) = ��
        // a��b�̃x�N�g���̒�����1�ł����...
        // acos( a�Eb / ( 1 * 1 ) ) = ��
        // acos( a�Eb ) = ��
        float3 L = normalize(lights[i].dir);
        float3 V = normalize(dir);
        float vecAngle = acos(dot(L, V));

        // �X�|�b�g���C�g�͈̔͂͒[����[�܂ł͈̔�
        // ��̃x�N�g���̊p�x�̓X�|�b�g���C�g�̌�������x�N�g���܂�
        // �X�|�b�g���C�g�̌���(���S)����[�܂ł̊p�x�Ōv�Z�������̂ŁA
        // �Ƃ点��͈͔͂����ɂ��Čv�Z����B

        float angleattenuation = vecAngle / (lights[i].angle * 0.5f);
        angleattenuation = saturate(angleattenuation);
        angleattenuation = 1.0f -  pow(angleattenuation, 5.0f);
        attenuation *= angleattenuation;

        //if (lights[i].angle * 0.5f < vecAngle)
        //  attenuation = 0.0f;

        // �e�X�|�b�g���C�g�����Z���Ă���
        float4 lightColor = lights[i].color * attenuation;
        spotColor += lightColor;

    }
    color.rgb = spotColor.rgb;

    return color;
}