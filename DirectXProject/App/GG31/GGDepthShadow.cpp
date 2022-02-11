#include <App/GG31/GGDepthShadow.h>
#include <App/GG31/GGLevelMain.h>
#include <System/Scene/SceneManager.hpp>
#include <System/Macro.h>
#include <System/Texture/TextureFactory.h>
#include <DirectXMath.h>
#include <System/Camera/CameraManager.hpp>
#include <System/Graphics/Graphics.h>
#include <App/GG31/GGLightCamera.h>

HRESULT GGDepthShadow::Load()
{
	// �萔�o�b�t�@
	m_pMatrix = new ConstantBuffer;
	m_pMatrix->Create(sizeof(DirectX::XMFLOAT4X4) * 3);
	m_pShadowVPS = new ConstantBuffer;
	m_pShadowVPS->Create(sizeof(DirectX::XMFLOAT4X4));

	// �����_�[�^�[�Q�b�g
	m_pRenderTarget = TextureFactory::CreateRenderTarget(DXGI_FORMAT_R32_FLOAT, 1024, 1024);
	m_pDepthStencil = TextureFactory::CreateDepthStencil(1024, 1024);

	// ���C�g�쐬
	CAMERA->Create<GGLightCamera>();

	// ������
	for (int i = 0; i < 10; i++)
	{
		m_fTick[i] = rand() % 50 * 10.0f;
		m_Pos[i] = DirectX::XMFLOAT3((rand() % 7) - 3, 0.0f, (rand() % 7) - 3);
	}

	return S_OK;
}
void GGDepthShadow::Release()
{
	SAFE_DELETE(m_pDepthStencil);
	SAFE_DELETE(m_pRenderTarget);
	SAFE_DELETE(m_pShadowVPS);
	SAFE_DELETE(m_pMatrix);
}
void GGDepthShadow::Update(float tick)
{
	for (int i = 0; i < 10; i++)
	{
		m_fTick[i] += tick;
		m_Pos[i].y = sinf(m_fTick[i] * 0.01f) * 1.5f + 0.5f;
	}
}

void GGDepthShadow::Draw()
{
	GGLevelMain* pMain = SCENE->GetScene<GGLevelMain>(0);
	Camera* pCamera;
	DirectX::XMFLOAT4X4 mat[3];
	for (int i = 0; i < 10; i++)
		DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(0, m_Pos[i].y, 0)));

	// ---�e�N�X�`���֏�������---
	// �����e�i���f������ł���e�j�̕\�� 1�x�ʂ̎��_���猩���i�F���e�N�X�`���ɏ���

	// �F�������̏��Ƃ��Ĉ�����
	// �S��0(���F)->�e�����������镨�̂�����Ɣ��f�����
	float color[] = { 1,1,1,1 };
	GRAPHICS->SetRenderTarget(&m_pRenderTarget, 1, color);
	GRAPHICS->SetDepthStencilView(m_pDepthStencil, true);
	{
		// �J�������猩���i�F���e�N�X�`���ɏ������ނ��߂Ɍ����̎��_�����W�n�ɕϊ�(�s��ݒ�)
		pCamera = CAMERA->Get<GGLightCamera>();
		DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixTranspose(pCamera->GetLookAtMatrix()));
		DirectX::XMStoreFloat4x4(&mat[2], DirectX::XMMatrixTranspose(pCamera->GetProjectionMatrix()));

		m_pMatrix->Write(mat);

		// ���O�̉e�̏����v�Z���Ă���
		// �e�N�X�`���ɏ������񂾏���0�`1���w�肵�Ď��o��
		// �J�����s���-1�`1�͈̔�
		// �J�������猩�������e�N�X�`�����o���̓X�N���[�����W->UV���W�ɕϊ�
		DirectX::XMMATRIX screenMat = DirectX::XMMatrixSet(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f
		);
		// ���O�Ɍv�Z�����e�̏���[vps]�Ɋi�[
		DirectX::XMFLOAT4X4 vps;
		// �e�N�X�`�����W�𐳂����ǂݍ���
		DirectX::XMStoreFloat4x4(&vps, DirectX::XMMatrixTranspose(pCamera->GetLookAtMatrix() * pCamera->GetProjectionMatrix() * screenMat));
		// �e�̃o�b�t�@���X�V
		m_pShadowVPS->Write(&vps);

		// �e�N�X�`����������
		// �e�𗎂Ƃ������I�u�W�F�N�g��`��

		// �V�F�[�_�[�̃Z�b�g
		GetVertexShader(DEPTH_WRITE_VS)->Bind();
		GetPixelShader(DEPTH_WRITE_PS)->Bind();
		// �萔�o�b�t�@���W�X�^�̔ԍ��ɏ�������
		m_pMatrix->BindVS(0);

		// �e
		for (int i = 0; i < 10; i++)
		{
			DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(m_Pos[i].x, m_Pos[i].y, m_Pos[i].z)));
			m_pMatrix->Write(mat);
			pMain->DrawSphere();
		}
	}
	// �e�N�X�`���������݂��I������猳�̕`��ɖ߂�
	GRAPHICS->SetRenderTargetDefault();
	GRAPHICS->SetDepthStencilViewDefault();

	// �ʏ�̃Q�[����ʂ̕\���ɖ߂�(�Q�[���J�����s��)
	pCamera = CAMERA->Get();
	DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixTranspose(pCamera->GetLookAtMatrix()));
	DirectX::XMStoreFloat4x4(&mat[2], DirectX::XMMatrixTranspose(pCamera->GetProjectionMatrix()));
	m_pMatrix->Write(mat);

	// �e���ݒ�
	GetVertexShader(DEPTH_SHADOW_VS)->Bind();
	GetPixelShader(DEPTH_SHADOW_PS)->Bind();
	m_pShadowVPS->BindVS(1);
	SetTexturePS(m_pRenderTarget, 1);

	// �I�u�W�F�N�g
	for (int i = 0; i < 10; i++)
	{
		DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(m_Pos[i].x, m_Pos[i].y, m_Pos[i].z)));
		m_pMatrix->Write(mat);
		pMain->DrawSphere();
	}

	// �n�ʂ͌Œ�
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixIdentity());
	m_pMatrix->Write(&mat);
	pMain->DrawPlane();


	// �����_�[�^�[�Q�b�g�ɕ`�����񂾓��e���f�o�b�O�\��
	GetPixelShader(RENDER_SHADOW_PS)->Bind();
	SetTexturePS(m_pRenderTarget, 0);
	pMain->DrawPolygon();
}