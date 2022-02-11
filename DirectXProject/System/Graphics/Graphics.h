#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <System/Graphics/DirectX.h>
#include <System/Singleton.hpp>
#include <System/Graphics/BlendState.h>
#include <System/Graphics/SamplerState.h>
#include <System/Graphics/DepthStencilState.h>
#include <System/Texture/Texture.h>

#define GRAPHICS (Graphics::GetInstance())

enum CullingMode
{
	CULL_NONE,
	CULL_FRONT,
	CULL_BACK,
	CULL_MAX
};

class Graphics : public Singleton<Graphics>
{
	SINGLETON_CONSTRACTOR(Graphics);
public:
	enum SamplerStateKind
	{
		SAMPLER_POINT,
		SAMPLER_LINEAR,
		MAX_SAMPLER
	};
	enum DepthStencilKind
	{
		DEPTHSTENCIL_OFF,
		DEPTHSTENCIL_ON,
		MAX_DEPTHSTENCIL
	};
	enum BlendStateKind
	{
		BLEND_ALPHA,
		BLEND_ADD,
		MAX_BLEND
	};

public:
	void Init() final;
	void Uninit() final;
	void BeginDraw();
	void EndDraw();

	// レンダーターゲット
	void SetRenderTarget(Texture** ppRenderTarget, UINT numView, float* pClearColor = nullptr);
	void SetRenderTargetDefault(float* pClearColor = nullptr);
	void SetDepthStencilView(Texture* pDepthStencilView, bool isClear = false);
	void SetDepthStencilViewDefault(bool isClear = false);

	// ブレンド
	void SetBlendState(BlendStateKind kind);
	void SetBlendState(BlendState* pState);
	BlendState* GetBlendState();
	// サンプラー
	void SetSamplerState(SamplerStateKind kind);
	void SetSamplerState(SamplerState *pState);
	SamplerState* GetSamplerState();
	// 深度ステンシル
	void SetDepthStencilState(DepthStencilKind kind);
	void SetDepthStencilState(DepthStencilState* pState);
	DepthStencilState* GetDepthStencilState();

private:
	void UpdateTargetView();
private:
	Texture* m_pDefRenderTarget;
	Texture* m_pDefDepthStencil;
	Texture* m_pRenderTarget[4];
	UINT m_renderTargetNum;
	Texture* m_pDepthStencilView;
	BlendState* m_pDefBlendState[MAX_BLEND];
	BlendState* m_pBlendState;
	SamplerState* m_pDefSamplerState[MAX_SAMPLER];
	SamplerState* m_pSamplerState;
	DepthStencilState* m_pDefDepthStencilState[MAX_DEPTHSTENCIL];
	DepthStencilState* m_pDepthStencilState;
};

void SetCulling(CullingMode cull);

#endif // __DIRECT_X_H__