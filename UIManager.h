#pragma once

class UIBaseComponent;

class UIManager
{
private:
	friend class UIBaseComponent;
	sf::RenderTarget* m_target;
	std::list<UIBaseComponent*> m_components;
	UIBaseComponent* m_mousein;
	UIBaseComponent* m_mouselbholdon;
	UIBaseComponent* m_mouserbholdon;
	bool m_needredraw;
public:
	UIManager(sf::RenderTarget* Target)
		:m_target(Target)
		, m_mousein(nullptr)
		, m_mouselbholdon(nullptr)
		, m_mouserbholdon(nullptr)
		, m_needredraw(true)
	{}
	~UIManager();

	sf::RenderTarget* GetRenderTarget()
	{
		return m_target;
	}
	void Add(UIBaseComponent*Component);
	void Draw();

	void EventHandler(sf::Event& e);
	bool NeedRedraw();
	void Update(float dt);
};