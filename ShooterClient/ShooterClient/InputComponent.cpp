#include "InputComponent.h"

void InputComponent::ProcessEvent(const sf::Event& event)
{
    if (event.is<sf::Event::KeyPressed>())
    {
        auto code = event.getIf<sf::Event::KeyPressed>()->code;
        if (code == sf::Keyboard::Key::A) moveLeft = true;
        if (code == sf::Keyboard::Key::D) moveRight = true;
        if (code == sf::Keyboard::Key::E) emote = true;
        if (code == sf::Keyboard::Key::Space) jump = true;
    }
    else if (event.is<sf::Event::KeyReleased>())
    {
        auto code = event.getIf<sf::Event::KeyReleased>()->code;
        if (code == sf::Keyboard::Key::A) moveLeft = false;
        if (code == sf::Keyboard::Key::D) moveRight = false;
        if (code == sf::Keyboard::Key::Space) jump = false;
    }
    else if (const sf::Event::MouseButtonPressed* mousePressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        shoot = true;
    }
}

const std::type_index InputComponent::GetType() { return typeid(InputComponent); }
