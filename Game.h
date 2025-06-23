#ifndef GAME_H
#define GAME_H
#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

struct PlayerConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };
struct EnemyConfig { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX; };
struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };
struct systems { bool movement{ true }, lifespan{ true }, gui{ true }, collision{ true }, spawning{ true }, rendering{ true }; };

#include "EntityManager.hpp"


class Game
{
	sf::RenderWindow		m_window;		// Window we will draw to
	EntityManager			m_entities;		// vector of entities to maintian
	sf::Font				m_font;			// font we will use to draw
	sf::Text				m_text;			// Score text drawn to screen
	sf::Text				m_high_score_text;	// High score
	PlayerConfig			m_playerConfig;
	EnemyConfig				m_enemyConfig;
	BulletConfig			m_bulletConfig;
	systems					m_systems;
	sf::Clock				m_deltaClock;
	long int				m_score = 0;
	long int				m_high_score;
	int						m_currentFrame = 0;
	int						m_lastEnemySpawnTime = 0;
	bool					m_paused = false;	// whether we update game logic
	bool					m_running = true;	// whether game is running

	void init(const std::string& path);	// Initialize GameState with a config file
	//void setPaused(bool paused);		//	pause the game

	void sMovement();
	void sUserInput();
	void sLifespan();
	void sRender();
	void sGUI();
	void sEnemySpawner();
	void sCollision();

	void spawnPlayer();
	void spawnEnemy();
	void spawnSmallEnemies(std::shared_ptr<Entity> entity);
	void spawnBullet(std::shared_ptr<Entity> entity, const Vec2f& mousePos);
	void spawnSpecialWeapon(std::shared_ptr<Entity> entity);

	std::shared_ptr<Entity> player();
	bool checkCollision(std::shared_ptr<Entity> entity1, std::shared_ptr<Entity> entity2);

public:
	Game(const std::string& config);	//constructor which takes config

	void run();
};






#endif