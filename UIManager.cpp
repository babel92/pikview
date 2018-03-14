#include "stdafx.h"
#include "UIManager.h"
#include "uicomponents.h"

UIManager::~UIManager()
{
	for (auto p : m_components)
		delete p;
}

void UIManager::Add(UIBaseComponent * Component)
{
	if (Component) {
		Component->m_parent = this;
		m_components.push_front(Component);
	}
}

void UIManager::Draw()
{
	for (auto p : m_components)
		p->Draw(*m_target);
}

#define DROP_EVENT (e.type=sf::Event::EventType::Count)

void UIManager::EventHandler(sf::Event & e)
{
	switch (e.type)
	{
	case sf::Event::KeyPressed:

		break;
	case sf::Event::MouseButtonPressed:
		for (auto c : m_components)
		{
			if (m_mousein == c)
			{
				switch (e.mouseButton.button)
				{
				case sf::Mouse::Left:
					m_mouselbholdon = c;
					c->OnMouseDown();
					break;
				}
				DROP_EVENT;
			}
			else
			{

			}
		}
		break;
	case sf::Event::MouseButtonReleased:
		for (auto c : m_components)
		{
			if (m_mousein == c)
			{
				switch (e.mouseButton.button)
				{
				case sf::Mouse::Left:
					if (m_mouselbholdon == c)
					{
						c->OnMouseUp();
						c->OnClick();
						m_mouselbholdon = nullptr;
					}
					else
					{
						// Mouse pressed on it but released on other component
						m_mouselbholdon->OnMouseUpAway();
					}
					break;
				case sf::Mouse::Right:
					break;
				default:
					break;
				}
				return;
			}
			else
			{

			}
		}
		// Mouse released on the main form
		if ((size_t)m_mouselbholdon > 1) {
			m_mouselbholdon->OnMouseUpAway();
		}
		m_mouselbholdon = (UIBaseComponent*)1;
		break;
	case sf::Event::MouseMoved:
		for (auto c : m_components)
		{
			if (c->IsPointInside((float)e.mouseMove.x, (float)e.mouseMove.y))
			{
				if (m_mousein == c)
				{
					//c->OnMouseMove();
				}
				else
				{
					if (m_mousein)
						m_mousein->OnMouseLeave();
					m_mousein = c;
					c->OnMouseEnter();
					break; // Only one at a time
				}
			}
			else
			{
				// Mouse is not inside any component, i.e. it's in the main form
				if (m_mousein)
					m_mousein->OnMouseLeave();
				m_mousein = nullptr;
			}
		}
		break;
	default:
		break;
	}
}

bool UIManager::NeedRedraw()
{
	bool ret = m_needredraw;
	m_needredraw = false;
	return ret;
}

void UIManager::Update(float dt)
{
	for (auto c : m_components)
		c->Update(dt);
}


