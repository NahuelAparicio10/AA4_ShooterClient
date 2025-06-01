#pragma once
#include<SFML/Graphics.hpp>
#include "PhysicsManager.h"
#include "SpriteRenderer.h"

class BulletHandler
{
public:
	BulletHandler(PhysicsManager* pManager);
	void CreateBullet(sf::Vector2f position, sf::Vector2f direction);
	void UpdateBullets(float dt);
	void RenderBullets(sf::RenderWindow* window);
	void SetPhysicsManager(PhysicsManager* pManager) { _physicsManager = pManager; }
private:
	std::vector<GameObject*> _bullets;
	PhysicsManager* _physicsManager;

};

