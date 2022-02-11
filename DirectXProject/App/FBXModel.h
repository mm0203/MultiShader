#ifndef __FBX_MODEL_H__
#define __FBX_MODEL_H__

#include <DirectXMath.h>
#include <System/Graphics/DXBuffer.h>
#include <vector>
#include <string>

class FBXModel
{
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
	};
	struct Blend
	{
		// メッシュとボーンを結びつけるためのデータ
		float weight[4]; // それぞれの骨からどの程度影響を受けるか
		unsigned long index[4]; // どの骨に引っ付いて動くか
	};
	/* スキンメッシュアニメーション
	骨を動かした結果をモデルの頂点に適用する
	そのまま適用してしまうと、骨が原点から移動してきた分、表示しようとしている頂点も移動してしまう。
	この問題を解決するには、一度頂点を原点に移動させてから骨の移動を適用する必要がある。
	その際、FBXモデルは一つのメッシュ(例えば頭だけ)をアニメーションさせる時、
	影響を受ける必要最低限の骨の情報しか持たない(頭を動かす場合、足の骨の情報はいらない)
	そのため、各メッシュごとにどの骨の影響を受けるのかをまとめた情報が必要になる
	*/
	struct Skin
	{
		std::string bone;
		// もともとのモデルはTポーズを取っている
		// そのTポーズの移動情報を打ち消し、新たにアニメーションの
		// 行列を適用することで、頂点が正しいアニメーションの位置に移動する
		DirectX::XMMATRIX inverse; // Tポーズの移動を打ち消す逆行列
	};
	using SkinList = std::vector<Skin>;

	struct Mesh
	{
		DXBuffer buffer;
		void* pVtx;
		SkinList* pSkinList;
	};
	using MeshList = std::vector<Mesh>;

	using DrawSettingCallBack = void(*)(UINT mesh);

public:
	FBXModel();
	~FBXModel();

	HRESULT Load(const char *fileName);

	/*関数ポインタ
	   実行したい関数がどこに配置されているかを示すポインタ
	   変数のポインタと用途は変わらない
	   関数名の前に*を付けて()で囲うと宣言できる
	   戻り値、引数を呼び出したい関数と揃えないといけない
	 メッシュごとに別々の処理を呼び出したいので、引数に関数ポインタを設定 
	*/
	void Draw(DrawSettingCallBack CallBackFunc = nullptr);

private:
	// 全てのメモリを安全に解放するための関数
	void Reset();

private:
	MeshList m_meshList;
};

#endif
