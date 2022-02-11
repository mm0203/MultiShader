#include <System/Graphics/Shader.h>
#include <d3dcompiler.h>
#include <stdio.h>
#include <System/Macro.h>

#pragma comment(lib, "d3dcompiler.lib")

//----------
// ��{�N���X
Shader::Shader(Kind kind)
	: m_kind(kind)
{
}
Shader::~Shader()
{
}
HRESULT Shader::Load(const char* pFileName)
{
	HRESULT hr = E_FAIL;

	// �t�@�C����ǂݍ���
	FILE* fp;
	fp = fopen(pFileName, "rb");
	if (!fp) { return hr; }

	// �t�@�C���̃T�C�Y�𒲂ׂ�
	int fileSize = 0;
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);

	// �������ɓǂݍ���
	fseek(fp, 0, SEEK_SET);
	char* pData = new char[fileSize];
	fread(pData, fileSize, 1, fp);
	fclose(fp);

	// �V�F�[�_�[�쐬
	hr = MakeShader(pData, fileSize);
	
	// �I������
	if (pData) { delete[] pData; }
	return hr;
}
HRESULT Shader::Compile(const char *pCode)
{
	static const char *pTargetList[] = 
	{
		"vs_5_0",
		"ps_5_0"
	};

	HRESULT hr;
	ID3DBlob *pBlob;
	ID3DBlob *error;
	UINT compileFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	hr = D3DCompile(pCode, strlen(pCode), nullptr, nullptr, nullptr,
		"main", pTargetList[m_kind], compileFlag, 0, &pBlob, &error);
	if (FAILED(hr)) { return hr; }

	// �V�F�[�_�쐬
	hr = MakeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize());
	SAFE_RELEASE(pBlob);
	SAFE_RELEASE(error);
	return hr;
}



//----------
// ���_�V�F�[�_
VertexShader::ILList VertexShader::m_ilList;
void VertexShader::ReleaseInputLayout()
{
	ILList::iterator it = m_ilList.begin();
	while (it != m_ilList.end())
	{
		SAFE_RELEASE((it->second));
		++it;
	}
}

VertexShader::VertexShader()
	: Shader(Shader::VertexShader)
	, m_pVS(nullptr)
	, m_pInputLayout(nullptr)
{
}
VertexShader::~VertexShader()
{
	SAFE_RELEASE(m_pVS);
}
void VertexShader::Bind(void)
{
	ID3D11DeviceContext* pContext =	GetContext();
	pContext->VSSetShader(m_pVS, NULL, 0);
	pContext->IASetInputLayout(m_pInputLayout);
}
HRESULT VertexShader::MakeShader(void* pData, UINT size)
{
	HRESULT hr;
	ID3D11Device* pDevice = GetDevice();
	
	// �V�F�[�_�[�쐬
	hr = pDevice->CreateVertexShader(pData, size, NULL, &m_pVS);
	if(FAILED(hr)) { return hr; }



	/*
	�V�F�[�_�쐬���ɃV�F�[�_���t���N�V������ʂ��ăC���v�b�g���C�A�E�g���擾
	�Z�}���e�B�N�X�̔z�u�Ȃǂ��环�ʎq���쐬
	���ʎq���o�^�ρ��ė��p�A�Ȃ���ΐV�K�쐬
	https://blog.techlab-xe.net/dxc-shader-reflection/
	*/

	ID3D11ShaderReflection *pReflection;
	D3D11_SHADER_DESC shaderDesc;
	D3D11_INPUT_ELEMENT_DESC* pInputDesc;
	D3D11_SIGNATURE_PARAMETER_DESC sigDesc;
	std::string key = "";

	DXGI_FORMAT formats[][4] =
	{
		{
			DXGI_FORMAT_R32_UINT,
			DXGI_FORMAT_R32G32_UINT,
			DXGI_FORMAT_R32G32B32_UINT,
			DXGI_FORMAT_R32G32B32A32_UINT,
		}, {
			DXGI_FORMAT_R32_SINT,
			DXGI_FORMAT_R32G32_SINT,
			DXGI_FORMAT_R32G32B32_SINT,
			DXGI_FORMAT_R32G32B32A32_SINT,
		}, {
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_R32G32_FLOAT,
			DXGI_FORMAT_R32G32B32_FLOAT,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
		},
	};

	hr = D3DReflect(pData, size, IID_PPV_ARGS(&pReflection));
	if (FAILED(hr)) { return hr; }

	pReflection->GetDesc(&shaderDesc);
	pInputDesc = new D3D11_INPUT_ELEMENT_DESC[shaderDesc.InputParameters];
	for(int i = 0; i < shaderDesc.InputParameters; ++ i)
	{
		pReflection->GetInputParameterDesc(i, &sigDesc);
		pInputDesc[i].SemanticName = sigDesc.SemanticName;
		pInputDesc[i].SemanticIndex = sigDesc.SemanticIndex;

		// http://marupeke296.com/TIPS_No17_Bit.html
		BYTE elementCount = sigDesc.Mask;
		elementCount = (elementCount & 0x05) + ((elementCount >> 1) & 0x05);
		elementCount = (elementCount & 0x03) + ((elementCount >> 2) & 0x03);

		switch (sigDesc.ComponentType)
		{
			case D3D_REGISTER_COMPONENT_UINT32:
				pInputDesc[i].Format = formats[0][elementCount - 1];
				break;
			case D3D_REGISTER_COMPONENT_SINT32:
				pInputDesc[i].Format = formats[1][elementCount - 1];
				break;
			case D3D_REGISTER_COMPONENT_FLOAT32:
				pInputDesc[i].Format = formats[2][elementCount - 1];
				break;
		}
		pInputDesc[i].InputSlot = 0;
		pInputDesc[i].AlignedByteOffset = i == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
		pInputDesc[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		pInputDesc[i].InstanceDataStepRate = 0;

		key += sigDesc.SemanticName;
		key += '0' + sigDesc.SemanticIndex;
	}

	ILList::iterator it = m_ilList.begin();
	while (it != m_ilList.end())
	{
		if (it->first == key)
		{
			m_pInputLayout = it->second;
			break;
		}
		++it;
	}
	if (it == m_ilList.end())
	{
		hr = pDevice->CreateInputLayout(
			pInputDesc, shaderDesc.InputParameters,
			pData, size, &m_pInputLayout
		);
		if (SUCCEEDED(hr))
		{
			m_ilList.insert(ILKey(key, m_pInputLayout));
		}
	}

	SAFE_DELETE_ARRAY(pInputDesc);
	return hr;
}
//----------
// �s�N�Z���V�F�[�_
PixelShader::PixelShader()
	: Shader(Shader::PixelShader)
	, m_pPS(nullptr)
{
}
PixelShader::~PixelShader()
{
	SAFE_RELEASE(m_pPS);
}
void PixelShader::Bind(void)
{
	ID3D11DeviceContext* pContext = GetContext();
	pContext->PSSetShader(m_pPS, nullptr, 0);
}
HRESULT PixelShader::MakeShader(void* pData, UINT size)
{
	HRESULT hr;
	ID3D11Device* pDevice = GetDevice();

	hr = pDevice->CreatePixelShader(pData, size, NULL, &m_pPS);
	return hr;
}
//----------
// �萔�o�b�t�@
ConstantBuffer::ConstantBuffer()
	: m_pBuffer(nullptr)
{
}
ConstantBuffer::~ConstantBuffer()
{
	SAFE_RELEASE(m_pBuffer);
}
HRESULT ConstantBuffer::Create(UINT size)
{
	/* �萔�o�b�t�@�쐬���̒���
	�萔�o�b�t�@�̑傫����4�o�C�g��؂�(
	4�o�C�g�A���C�����g)�łȂ��ƍ쐬�ł��Ȃ�
	*/
	HRESULT hr;

	// �쐬����o�b�t�@�̏��
	D3D11_BUFFER_DESC bufDesc = {};
	bufDesc.ByteWidth = size;
	bufDesc.Usage = D3D11_USAGE_DEFAULT;
	bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	// �o�b�t�@�̍쐬
	ID3D11Device* pDevice = GetDevice();
	hr = pDevice->CreateBuffer(&bufDesc, nullptr, &m_pBuffer);

	return hr;
}
void ConstantBuffer::Write(const void* pData)
{
	// �萔�o�b�t�@�ւ̏�������
	ID3D11DeviceContext* pContext = GetContext();
	pContext->UpdateSubresource(m_pBuffer, 0, nullptr, pData, 0, 0);
}
void ConstantBuffer::BindVS(UINT slot)
{
	/*
	�萔�o�b�t�@�̃f�[�^�𑗂�ہA
	�ǂ̈ʒu�Ɋi�[���邩��1�ڂ̈���(StartSlot)��
	�w�肷��B
	hlsl�̃R�[�h�ł�register(bX)�Ńo�b�t�@�̊i�[
	�ʒu�����߂Ă����B
	*/
	ID3D11DeviceContext* pContext = GetContext();
	pContext->VSSetConstantBuffers(slot, 1, &m_pBuffer);
}
void ConstantBuffer::BindPS(UINT slot)
{
	ID3D11DeviceContext* pContext = GetContext();
	pContext->PSSetConstantBuffers(slot, 1, &m_pBuffer);
}


void SetTextureVS(Texture* pTex, UINT slot)
{
	ID3D11ShaderResourceView* pResource = pTex->GetResource();
	GetContext()->VSSetShaderResources(slot, 1, &pResource);
}
void SetTexturePS(Texture* pTex, UINT slot)
{
	ID3D11ShaderResourceView* pResource = pTex->GetResource();
	GetContext()->PSSetShaderResources(slot, 1, &pResource);
}