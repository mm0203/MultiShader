#ifndef __MENU_H__
#define __MENU_H__

#include <System/Debug/Debug.h>
#include <System/Singleton.hpp>
#include <System/Graphics/Text.h>
#include <vector>
#include <string>
#include <functional>
#include <stack>
#include <queue>

#define MENU (Menu::GetInstance())

class Menu : public Singleton<Menu>
{
	SINGLETON_CONSTRACTOR(Menu);
public:
	template<typename T>
	struct TValue
	{
		T value;
		T min;
		T max;
		T add;
		T skipAdd;
	};
	using IntValue = TValue<int>;
	using FloatValue = TValue<float>;
	struct Table
	{
		using Header = std::vector<std::string>;
		Header headers;
		int no;
	};
	using Function = std::function<void()>;
public:
	struct Item
	{
		enum Kind
		{
			MENU_FLAG,
			MENU_INT,
			MENU_FLOAT,
			MENU_TABLE,
			MENU_FUNC,
			MENU_ITEM,
		};
		using SubMenu = std::vector<Item*>;
		union Value
		{
			bool flag;
			IntValue nValue;
			FloatValue fValue;
			Table *pTable;
			Function *pFunc;
			SubMenu *pMenu;
		};

		std::string name;
		Kind kind;
		Value value;
	};
	struct Log
	{
		Item::SubMenu* pMenu;
		int no;
	};
	using SplitPath = std::queue<std::string>;

public:
	void Init() final;
	void Uninit() final;

	void Update();
	void Draw();

	void Registry(std::string hieralcy, bool flag);
	void Registry(std::string hieralcy, IntValue& nValue);
	void Registry(std::string hieralcy, FloatValue& fValue);
	void Registry(std::string hieralcy, Table& table);
	void Registry(std::string hieralcy, Function func);

	bool IsOpen();

	bool IsFlag(std::string parameter);
	int GetInt(std::string parameter);
	float GetFloat(std::string parameter);
	int GetTable(std::string parameter);

private:
	void Close();
	void OperateFlag(Item::Value &value);
	void OperateValue(Item::Kind kind, Item::Value& value);
	void OperateTable(Item::Value& value);
	void OperateFunc(Item::Value& value);
	void OperateItem(Item::Value& value);

	SplitPath GetSplitPath(std::string &path);
	Item* RegistryMenu(SplitPath& split, Item::SubMenu* pMenu);
	Item* FindMenu(SplitPath& split, Item::SubMenu* pMenu);

private:
	Text* m_pText;
	Item::SubMenu m_root;
	std::stack<Log> m_open;
};

#endif // __MENU_H__