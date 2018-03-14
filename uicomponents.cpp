#include "stdafx.h"
#include "uicomponents.h"

inline bool UIBaseComponent::IsPointInside(sf::Vector2f P)
{
	return IsPointInside(P.x, P.y);
}

bool UIBaseComponent::IsPointInside(float x, float y)
{
	return ((x >= m_rect.left&&x <= m_rect.left + m_rect.width) && (y >= m_rect.top&&y <= m_rect.top + m_rect.height));
}

bool UIBaseComponent::IsPointingOnThis()
{
	return m_parent->m_mousein == this;
}

void UIButton::SetState(ButtonStatus State)
{
	m_prevstate = m_state;
	m_state = State;
}

UIButton::UIButton(sf::FloatRect Rect)
	: UIBaseComponent(Rect)
	, m_shape(sf::Vector2f(m_rect.width, m_rect.height))
	, m_bar(sf::Vector2f(5, 50))
	, m_color_normal(sf::Color::Transparent)
	, m_color_pointed(sf::Color(128, 128, 128))
	, m_color_pushed(sf::Color::Red)
	, m_state(BS_NORMAL)
{
	m_shape.setPosition(m_rect.left, m_rect.top);
	m_bar.setPosition(m_rect.left + m_rect.width / 2, m_rect.top + m_rect.height / 2);
	m_bar.setOrigin(2.5, 25);
}

UIButton::~UIButton()
{}

void UIButton::SetOnClickEvent(std::function<void()> F)
{
	m_onclick = F;
}

void UIButton::Draw(sf::RenderTarget&Target)
{
	Target.draw(m_shape);
	m_bar.setRotation(45);
	Target.draw(m_bar);
	m_bar.setRotation(135);
	Target.draw(m_bar);
}

void UIButton::Update(float dt)
{
	switch (m_state)
	{
	case BS_HOLDNLEAVE:
	case BS_NORMAL:
		m_shape.setFillColor(m_color_normal);
		m_bar.setFillColor(sf::Color(128, 128, 128));
		break;
	case BS_POINTED:
		m_shape.setFillColor(m_color_pointed);
		m_bar.setFillColor(sf::Color::White);
		break;
	case BS_PUSHED:
		m_shape.setFillColor(m_color_pushed);
		m_bar.setFillColor(sf::Color::White);
		break;
	default:
		break;
	}
}

void UIButton::OnClick()
{
	if (m_onclick != nullptr)
		m_onclick();
}

void UIButton::OnMouseEnter()
{
	if (m_state == BS_NORMAL)
		m_state = BS_POINTED;
	else if (m_state == BS_HOLDNLEAVE)
		m_state = BS_PUSHED;
	SetRedraw();
}

void UIButton::OnMouseLeave()
{
	if (m_state == BS_PUSHED)
		m_state = BS_HOLDNLEAVE;
	else
		m_state = BS_NORMAL;
	SetRedraw();
}

void UIButton::OnMouseDown()
{
	m_state = BS_PUSHED;
	SetRedraw();
}

void UIButton::OnMouseUp()
{
	m_state = BS_POINTED;
	SetRedraw();
}

void UIButton::OnMouseUpAway()
{
	m_state = BS_NORMAL;
	SetRedraw();
}

void UIBaseComponent::SetRedraw(bool Redraw)
{
	m_parent->m_needredraw = Redraw;
}

void UIText::Draw(sf::RenderTarget & Target)
{
	Target.draw(m_text);
}

void UIText::Update(float dt)
{
}

void UIText::OnClick()
{
}

void UIText::OnMouseEnter()
{
}

void UIText::OnMouseLeave()
{
}

void UIText::OnMouseDown()
{
}

void UIText::OnMouseUp()
{
}

void UIText::OnMouseUpAway()
{
}

UIText::UIText(sf::FloatRect Rect,sf::String Text)
	:UIBaseComponent(Rect)
{
	m_font.loadFromFile("C:\\Windows\\Fonts\\ariblk.ttf");
	m_text.setFont(m_font);
	m_text.setString(Text);
	m_text.setPosition(Rect.left, Rect.top);
	m_text.setColor(sf::Color(64, 64, 64));
}

UIText::~UIText()
{
}

void UIText::SetText(sf::String Text)
{
	m_text.setString(Text);
}
