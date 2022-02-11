struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// ���C�g
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

cbuffer Matrix : register(b1)
{
	float4x4 invVPS;
};

Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D depthTex : register(t2);
SamplerState samp : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);

	float4 pointColor = float4(0, 0, 0, 0);

	// GBuffer����e������擾
	color = albedoTex.Sample(samp, pin.uv);
	float3 N = normalTex.Sample(samp, pin.uv).rgb;
	float depth = depthTex.Sample(samp, pin.uv).r;

	// �[�x�l���烏�[���h���W���Čv�Z
	// �ϊ��������t�菇�ł��΃��[���h���W�����܂�B
	// UV�̒l�����W�ɂȂ�B
	float4 wolrdPos = float4(pin.uv, depth, 1.0f);
	wolrdPos = mul(wolrdPos, invVPS);
	wolrdPos.xyz /= wolrdPos.w;

	color.rgb = wolrdPos.xyz;

	// �S���̃��C�g�ɑ΂��ď������s��
	for (int i = 0; i < 30; i++)
	{
		// �s�N�Z���̃��[���h���W�ƌ����̍��W������x�N�g�����v�Z
		float3 dir = wolrdPos.xyz - lights[i].pos;

		// �s�N�Z���ʒu�܂ł̋���
		float len = length(dir);

		float attenuation = 1.0f - len / lights[i].range;

		// ���͋����̂Q��Ō�������̂Ōv�Z��������������Z
		attenuation = saturate(attenuation);
		attenuation = pow(attenuation, 3.0f);

		// �������Ɗ|���Č����̌����v�Z
		float4 lightColor = lights[i].color * attenuation;

		// �_�������Ƃ̌������Z
		pointColor += lightColor;
	}

	// �v�Z�������[���h���W�����Ƃɓ_�����̏������s��
	color.rgb = pointColor;

	return color;
}