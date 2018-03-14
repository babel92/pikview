#pragma once
#include "stdafx.h"
#include "UIManager.h"

class UIBaseComponent
{
	friend class UIManager;
protected:
	UIManager* m_parent;
	sf::FloatRect m_rect;
	void SetRedraw(bool Redraw = true);
public:
	UIBaseComponent(sf::FloatRect Rect) 
		:m_rect(Rect)
	{}
	virtual ~UIBaseComponent() {}
	virtual void Draw(sf::RenderTarget&Target) = 0;
	virtual void Update(float dt) = 0;
	virtual void OnClick() = 0;
	virtual void OnMouseEnter() = 0;
	virtual void OnMouseLeave() = 0;
	virtual void OnMouseDown() = 0;
	virtual void OnMouseUp() = 0;
	virtual void OnMouseUpAway() = 0;
	bool IsPointInside(sf::Vector2f P);
	bool IsPointInside(float x, float y);
	bool IsPointingOnThis();
};

class UIButton:public UIBaseComponent
{
private:
	sf::RectangleShape m_shape;
	sf::RectangleShape m_bar;
	sf::Color m_color_normal;
	sf::Color m_color_pointed;
	sf::Color m_color_pushed;
	std::function<void()> m_onclick;
	std::wstring m_text;
	
	enum ButtonStatus
	{
		BS_NORMAL,
		BS_POINTED,
		BS_PUSHED,
		BS_HOLDNLEAVE
	};
	ButtonStatus m_state;
	ButtonStatus m_prevstate;
	void SetState(ButtonStatus State);

	virtual void Draw(sf::RenderTarget&Target);
	virtual void Update(float dt);
	virtual void OnClick();
	virtual void OnMouseEnter();
	virtual void OnMouseLeave();
	virtual void OnMouseDown();
	virtual void OnMouseUp();
	virtual void OnMouseUpAway();
public:
	UIButton(sf::FloatRect Rect);
	~UIButton();
	void SetOnClickEvent(std::function<void()> F);
};

class UIText :public UIBaseComponent
{
private:
	sf::String m_str;
	sf::Font m_font;
	sf::Text m_text;

	virtual void Draw(sf::RenderTarget&Target);
	virtual void Update(float dt);
	virtual void OnClick();
	virtual void OnMouseEnter();
	virtual void OnMouseLeave();
	virtual void OnMouseDown();
	virtual void OnMouseUp();
	virtual void OnMouseUpAway();
public:
	UIText(sf::FloatRect Rect, sf::String Text);
	~UIText();

	void SetText(sf::String Text);
};

