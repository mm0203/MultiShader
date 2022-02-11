#include <System/Debug/Menu.h>
#include <System/Macro.h>
#include <System/Input.h>
#include <algorithm>
#include <System/Math/Math.h>

void Menu::Init()
{
	m_pText = new Text;
	m_pText->Create(30);
	m_pText->SetSDF(0.87f);
	m_pText->SetEdge(0.25f);
	m_pText->SetCharaSpace(-15.0f);
}
void Menu::Uninit()
{
	std::function<void(Item::SubMenu*)> RecursiveDelete = [&](Item::SubMenu *pMenu)
	{
		Item::SubMenu::iterator it = pMenu->begin();
		while (it != pMenu->end())
		{
			switch ((*it)->kind)
			{
			case Item::MENU_ITEM:
				RecursiveDelete((*it)->value.pMenu);
				SAFE_DELETE((*it)->value.pMenu);
				break;
			case Item::MENU_TABLE:
				SAFE_DELETE((*it)->value.pTable);
				break;
			case Item::MENU_FUNC:
				SAFE_DELETE((*it)->value.pFunc);
				break;
			}
			SAFE_DELETE(*it);
			++it;
		}
	};
	RecursiveDelete(&m_root);
	SAFE_DELETE(m_pText);
}

void Menu::Update()
{
	// 表示 / 非表示 切り替え
	if (IsKeyTrigger(VK_ESCAPE))
	{
		if (IsOpen())
		{
			Close();
		}
		else
		{
			m_open.push({ &m_root, 0 });
		}
	}
	if (!IsOpen() || m_root.empty())
	{
		return;
	}
	// ひとつ前のメニューに戻る
	if (IsKeyTrigger(VK_BACK))
	{
		// ルートメニュー以外は戻ってもよい
		if (m_open.top().pMenu != &m_root)
		{
			m_open.pop();
			return;
		}
	}

	// 現在表示されているメニューの操作
	Log& current = m_open.top();
	Item::SubMenu* pMenu = current.pMenu;
	if (IsKeyRepeat(VK_UP) && current.no > 0)
	{
		--current.no;
	}
	if (IsKeyRepeat(VK_DOWN) && current.no < pMenu->size() - 1)
	{
		++current.no;
	}

	// 項目別の処理
	Item* pItem = (*pMenu)[current.no];
	switch (pItem->kind)
	{
	case Item::MENU_FLAG:
		OperateFlag(pItem->value);
		break;
	case Item::MENU_INT:
	case Item::MENU_FLOAT:
		OperateValue(pItem->kind, pItem->value);
		break;
	case Item::MENU_TABLE:
		OperateTable(pItem->value);
		break;
	case Item::MENU_FUNC:
		OperateFunc(pItem->value);
		break;
	case Item::MENU_ITEM:
		OperateItem(pItem->value);
		break;
	}
}

void Menu::Draw()
{
	// メニューを開いてなければ処理をしない
	if (!IsOpen())
	{
		return;
	}

	m_pText->SetPos(DirectX::XMFLOAT3(10, 10, 0));

	// メニューに何も登録されてない旨の表示
	Log& current = m_open.top();
	Item::SubMenu* pMenu = current.pMenu;
	if (pMenu->empty())
	{
		m_pText->Print("no registry.");
		m_pText->Draw();
		return;
	}

	// 一覧表示
	Item::SubMenu::iterator it = pMenu->begin();
	while (it != pMenu->end())
	{
		std::string format = current.no == (it - pMenu->begin()) ? "> " : "  ";
		switch ((*it)->kind)
		{
		case Item::MENU_FLAG:
			format += "%s:%s";
			m_pText->Print(format.c_str(), (*it)->name, (*it)->value.flag ? "true" : "false");
			break;
		case Item::MENU_INT:
			format += "%s:%d";
			m_pText->Print(format.c_str(), (*it)->name, (*it)->value.nValue.value);
			break;
		case Item::MENU_FLOAT:
			format += "%s:%f";
			m_pText->Print(format.c_str(), (*it)->name, (*it)->value.fValue.value);
			break;
		case Item::MENU_TABLE:
			format += "%s:%s";
			m_pText->Print(format.c_str(), (*it)->name, (*it)->value.pTable->headers[(*it)->value.pTable->no]);
			break;
		case Item::MENU_FUNC:
			format += "%s()";
			m_pText->Print(format.c_str(), (*it)->name);
			break;
		case Item::MENU_ITEM:
			format += "%s";
			m_pText->Print(format.c_str(), (*it)->name);
			break;
		}
		++it;
	}

	// 登録された文字列を描画
	m_pText->Draw();
}

void Menu::Registry(std::string hieralcy, bool flag)
{
	SplitPath path = GetSplitPath(hieralcy);
	Item* pItem = RegistryMenu(path, &m_root);
	pItem->kind = Item::MENU_FLAG;
	pItem->value.flag = flag;
}
void Menu::Registry(std::string hieralcy, IntValue& nValue)
{
	SplitPath path = GetSplitPath(hieralcy);
	Item* pItem = RegistryMenu(path, &m_root);
	pItem->kind = Item::MENU_INT;
	pItem->value.nValue = nValue;
}
void Menu::Registry(std::string hieralcy, FloatValue& fValue)
{
	SplitPath path = GetSplitPath(hieralcy);
	Item* pItem = RegistryMenu(path, &m_root);
	pItem->kind = Item::MENU_FLOAT;
	pItem->value.fValue = fValue;
}
void Menu::Registry(std::string hieralcy, Table& table)
{
	SplitPath path = GetSplitPath(hieralcy);
	Item* pItem = RegistryMenu(path, &m_root);
	pItem->kind = Item::MENU_TABLE;
	pItem->value.pTable = new Table;
	*pItem->value.pTable = table;
}
void Menu::Registry(std::string hieralcy, Function func)
{
	SplitPath path = GetSplitPath(hieralcy);
	Item* pItem = RegistryMenu(path, &m_root);
	pItem->kind = Item::MENU_FUNC;
	pItem->value.pFunc = new Function;
	*pItem->value.pFunc = func;
}
bool Menu::IsOpen()
{
	return !m_open.empty();
}

bool Menu::IsFlag(std::string parameter)
{
	SplitPath path = GetSplitPath(parameter);
	Item* pItem = FindMenu(path, &m_root);
	if (pItem && pItem->kind == Item::MENU_FLAG)
	{
		return pItem->value.flag;
	}
	return false;
}
int Menu::GetInt(std::string parameter)
{
	SplitPath path = GetSplitPath(parameter);
	Item* pItem = FindMenu(path, &m_root);
	if (pItem && pItem->kind == Item::MENU_INT)
	{
		return pItem->value.nValue.value;
	}
	return 0;
}
float Menu::GetFloat(std::string parameter)
{
	SplitPath path = GetSplitPath(parameter);
	Item* pItem = FindMenu(path, &m_root);
	if (pItem && pItem->kind == Item::MENU_FLOAT)
	{
		return pItem->value.fValue.value;
	}
	return 0.0f;
}
int Menu::GetTable(std::string parameter)
{
	SplitPath path = GetSplitPath(parameter);
	Item* pItem = FindMenu(path, &m_root);
	if (pItem && pItem->kind == Item::MENU_TABLE)
	{
		return pItem->value.pTable->no;
	}
	return -1;
}


void Menu::Close()
{
	while (!m_open.empty())
	{
		m_open.pop();
	}
}
void Menu::OperateFlag(Item::Value& value)
{
	if (IsKeyTrigger(VK_RETURN))
	{
		value.flag ^= true;
	}
	else if (IsKeyTrigger(VK_LEFT))
	{
		value.flag = true;
	}
	else if (IsKeyTrigger(VK_RIGHT))
	{
		value.flag = false;
	}
}
void Menu::OperateValue(Item::Kind kind, Item::Value& value)
{
	float dir = 0;
	bool isSkip = IsKeyPress(VK_LSHIFT);
	if (IsKeyRepeat(VK_LEFT))
	{
		dir = -1.0f;
	}
	else if (IsKeyRepeat(VK_RIGHT))
	{
		dir = 1.0f;
	}

	switch (kind)
	{
	case Item::MENU_INT:
		value.nValue.value += static_cast<int>(dir * (isSkip ? value.nValue.skipAdd : value.nValue.add));
		value.nValue.value = Clamp(value.nValue.value, value.nValue.min, value.nValue.max);
		break;
	case Item::MENU_FLOAT:
		value.fValue.value += dir * (isSkip ? value.fValue.skipAdd : value.fValue.add);
		value.fValue.value = Clamp(value.fValue.value, value.fValue.min, value.fValue.max);
		break;
	}
}
void Menu::OperateTable(Item::Value& value)
{
	if (IsKeyRepeat(VK_LEFT))
	{
		if (--value.pTable->no < 0)
		{
			value.pTable->no = value.pTable->headers.size() - 1;
		}
	}
	else if (IsKeyRepeat(VK_RIGHT))
	{
		if (value.pTable->headers.size() <= ++value.pTable->no)
		{
			value.pTable->no = 0;
		}
	}
}
void Menu::OperateFunc(Item::Value& value)
{
	if (IsKeyTrigger(VK_RETURN))
	{
		Close();
		(*value.pFunc)();
	}
}
void Menu::OperateItem(Item::Value& value)
{
	if (IsKeyTrigger(VK_RETURN))
	{
		m_open.push({ value.pMenu, 0 });
	}
}

Menu::SplitPath Menu::GetSplitPath(std::string& path)
{
	// 区切り文字を'/'で統一
	std::string::iterator it = path.begin();
	while (it != path.end())
	{
		if (*it == '\\')
		{
			*it = '/';
		}
		++it;
	}

	// 区切り文字で分割
	SplitPath split;
	size_t start = 0;
	size_t pos = std::string::npos;
	do
	{
		start = pos + 1;
		pos = path.find("/", start);
		if (pos == std::string::npos)
		{
			split.push(path.substr(start));
		}
		else
		{
			split.push(path.substr(start, pos - start));
		}
	} while (pos != std::string::npos);

	return split;
}
Menu::Item* Menu::RegistryMenu(SplitPath& split, Item::SubMenu* pMenu)
{
	Item* pItem;

	// パスが含まれていない（項目のみ
	if (split.size() <= 1)
	{
		pItem = new Item;
		pItem->name = split.front();
		pMenu->push_back(pItem);
		return pItem;
	}

	// パスがメニュー内に存在しなければ追加する
	Item::SubMenu::iterator it = std::find_if(pMenu->begin(), pMenu->end(),
		[&split](Item*& pItem) {
			return pItem->name == split.front();
		}
	);
	if (it == pMenu->end())
	{
		pItem = new Item;
		pItem->kind = Item::MENU_ITEM;
		pItem->name = split.front();
		pItem->value.pMenu = new Item::SubMenu;
		pMenu->push_back(pItem);
	}
	else
	{
		pItem = (*it);
	}

	// 次の階層へ移動
	split.pop();
	return RegistryMenu(split, pItem->value.pMenu);
}


Menu::Item* Menu::FindMenu(SplitPath& split, Item::SubMenu* pMenu)
{
	// フォルダ探索
	Item::SubMenu::iterator it = pMenu->begin();
	while (it != pMenu->end())
	{
		if ((*it)->name == split.front())
		{
			// メニュー以外のアイテムが見つかった
			if ((*it)->kind != Item::MENU_ITEM)
			{
				return *it;
			}
			else
			{
				// さらに探索
				split.pop();
				return FindMenu(split, (*it)->value.pMenu);
			}
		}
		++it;
	}

	// 見つからなかった
	return nullptr;
}