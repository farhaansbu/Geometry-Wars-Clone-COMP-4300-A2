#include "Game.h"
#include "Random.h"
#include <fstream>

#include <iostream>
		

Game::Game(const std::string& config)
	: m_text(m_font), m_high_score_text(m_font)
{
	init(config);
}

void Game::init(const std::string& path)
{
	// TODO: read in config file here
	// use premade PlayerConfig, EnemyConfig, and BulletConfig variables
	std::cout << "Initialzing Game!\n";

	std::ifstream config(path);
	if (config.fail())
	{
		std::cout << "Could not open config file!\n"; exit(-1);
	}

	std::string input_type;

	// Read in window information
	sf::Vector2u windowSize;
	int frameLimit;
	int fullscreen;
	config >> input_type >> windowSize.x >> windowSize.y >> frameLimit >> fullscreen;
	
	//set up window default parameters
	m_window.create(sf::VideoMode(windowSize), "Geometry Wars", static_cast<sf::State>(fullscreen));
	m_window.setFramerateLimit(frameLimit);
	ImGui::SFML::Init(m_window);

	// Read in Font
	std::string fontPath;
	int fontSize, r, g, b;
	config >> input_type >> fontPath >> fontSize >> r >> g >> b;
	if (!m_font.openFromFile(fontPath))
	{
		exit(-1);
	};

	m_text.setCharacterSize(fontSize);
	m_text.setFillColor(sf::Color(r, g, b));
	m_text.setPosition({ 5, 0 });

	// Read in Player config info
	config >> input_type >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR
		>> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB
		>> m_playerConfig.OT >> m_playerConfig.V;

	// Read in Enemy config info
	config >> input_type >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX
		>> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN
		>> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;

	// Read in Bullet config info
	config >> input_type >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR
		>> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB
		>> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;

	config.close();

	// Read in high score 
	std::ifstream score("score.txt");
	if (score.fail())
		m_high_score = 0;
	else
		score >> m_high_score;
	score.close();

	//Scale imgui ui and text size by 2
	ImGui::GetStyle().ScaleAllSizes(2.0f);
	ImGui::GetIO().FontGlobalScale = 2.0f;

	std::cout << "Spawning Player!\n";
	spawnPlayer();

}


// Helper function that returns the player via entity manager
std::shared_ptr<Entity> Game::player()
{
	auto& players = m_entities.getEntities("player");
	if (players.empty())
	{
		spawnPlayer();
	};
	return players.front();
}

void Game::run()
{
	// TODO: add pause functionality in here
	// some systems should function while paused (rendering)
	// some systems shouldn't (movement/input)

	while (m_running)
	{

		// required update call to igui
		ImGui::SFML::Update(m_window, m_deltaClock.restart());

		// Call our systems

		// if not paused
		if (!m_paused)
		{
			// update entity manager
			m_entities.update();

			sEnemySpawner();
			sMovement();
			sCollision();
			sLifespan();

			// increase current frame
			m_currentFrame++;
			if (m_score > m_high_score)
				m_high_score = m_score;
		}
		
		sUserInput();
		sGUI();
		sRender();
	
	}

	std::ofstream score("score.txt");
	score << m_high_score;
	// Cleanup 
	
	score.close();
	m_window.close();
	ImGui::SFML::Shutdown();
}

void Game::spawnPlayer()
{
	// TODO: Finish adding all properties of player with correct values
	auto entity = m_entities.addEntity("player");

	// Give the entity a transform with position, velocity, and angle
	entity->add<CTransform>(Vec2f(m_window.getSize().x/2, m_window.getSize().y/2), Vec2f(0.0f, 0.0f), 0.0f);

	// Give the entity a shape with radius, number of sides, fill color, outline color and thickness
	entity->add<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), 
						sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);

	// Add input component
	entity->add<CInput>();

	// Add collision component
	entity->add<CCollision>(m_playerConfig.CR);

}

void Game::spawnEnemy()
{
	// TODO: make sure enemy is spawned properly with the m_enemyConfig variables
	
	// enemy mmust be spawned within bounds of window and not on top of player
	auto entity = m_entities.addEntity("enemy");

	// Assign enemy random position within bounds and speed
	int rand_xpos = Random::get(0 + m_enemyConfig.SR, m_window.getSize().x - m_enemyConfig.SR);
	int rand_ypos = Random::get(0 + m_enemyConfig.SR, m_window.getSize().y - m_enemyConfig.SR);

	// Assign random speed between min/max values
	float rand_speed = Random::get(m_enemyConfig.SMIN, m_enemyConfig.SMAX);

	// Choose random direction
	float degrees = Random::get(0.0f, 6.18f);

	float x_speed = cos(degrees) * rand_speed;
	float y_speed = sin(degrees) * rand_speed;

	entity->add<CTransform>(Vec2f(rand_xpos, rand_ypos), Vec2f(x_speed, y_speed), 0.0f);

	// Give entity a shape with radius, rand number of vertices, random fill color, outline color
	int rand_vertices = Random::get(m_enemyConfig.VMIN, m_enemyConfig.VMAX);

	int r = Random::get(0, 255);
	int g = Random::get(0, 255);
	int b = Random::get(0, 255);

	entity->add<CShape>(m_enemyConfig.SR, rand_vertices, sf::Color(r, g, b), sf::Color(m_enemyConfig.OR,
						m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);

	// Add score componenet
	entity->add<CScore>(100 * rand_vertices);

	//Add collision component
	entity->add<CCollision>(m_enemyConfig.CR);

	// Record when most recent enemy was spawned
	m_lastEnemySpawnTime = m_currentFrame;

}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
	// TODO: spawn small enemies at the location of input enemy e

	//we have to read values of original enemies
	// spawn a number of small enemies equal to vertices of original one
	// same color as original, half the size
	// small enemies are worth double the points

	
	float base_speed = sqrt(pow(e->get<CTransform>().velocity.x, 2) + pow(e->get<CTransform>().velocity.y, 2));
	Vec2f base_position = e->get<CTransform>().pos;
	float base_radius = e->get<CShape>().circle.getRadius();

	sf::Color fill = e->get<CShape>().circle.getFillColor();
	sf::Color outline = e->get<CShape>().circle.getOutlineColor();

	int num_vertices = e->get<CShape>().circle.getPointCount();
	float angle = e->get<CTransform>().angle;

	for (int i = 0; i < num_vertices; ++i)
	{
		// Create entity
		auto entity = m_entities.addEntity("sEnemy");

		//Calcualte spawn point
		float x_spawn = base_position.x + cos(angle) * (base_radius);
		float y_spawn = base_position.y + sin(angle) * (base_radius);

		// Calculate velocity
		float x_speed = cos(angle) * base_speed;
		float y_speed = sin(angle) * base_speed;

		// Add components
		entity->add<CTransform>(Vec2f(x_spawn, y_spawn), Vec2f(x_speed, y_speed), 0.0f);
		entity->add<CShape>(base_radius / 2, num_vertices, fill, outline, m_enemyConfig.OT);
		entity->add<CLifespan>(m_enemyConfig.L);
		entity->add<CCollision>(e->get<CCollision>().radius / 2);
		entity->add<CScore>(e->get<CScore>().score * 2);

		// Adjust angle for next enemy
		angle += 6.18 / num_vertices;
	}


}

// spawns a bullet from a given entity to target location
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2f& target)
{
	if (m_paused)
		return;
	// TODO: implement spawning of bullet which travels to target from player location
		// bullet speed is given as a scalar
		// set velocity using trig formulas

	// Calculate angle between mouse and player
	Vec2f player_position = entity->get<CTransform>().pos;
	
	float x_dist = target.x - player_position.x;
	float y_dist = target.y - player_position.y;

	float angle = atan2f(y_dist, x_dist);

	// Determine speed and spawn position based on angle
	float x_vel = cos(angle) * m_bulletConfig.S;
	float y_vel = sin(angle) * m_bulletConfig.S;

	float x_spawn = entity->get<CTransform>().pos.x + cos(angle) * (entity->get<CShape>().circle.getRadius() + m_bulletConfig.SR*1.5);
	float y_spawn = entity->get<CTransform>().pos.y + sin(angle) * (entity->get<CShape>().circle.getRadius() + m_bulletConfig.SR*1.5);

	auto bullet = m_entities.addEntity("bullet");
	bullet->add<CTransform>(Vec2f(x_spawn, y_spawn), Vec2f(x_vel, y_vel), 0.0f);

	// Add shape component
	bullet->add<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB, 255),
						sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);

	// Add collision and lifespan components
	bullet->add<CCollision>(m_bulletConfig.CR);
	bullet->add<CLifespan>(m_bulletConfig.L);

}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{

}

void Game::sMovement()
{
	if (!m_systems.movement)
		return;

	// TODO: implement all movement in this function
	// Bullet movement
	for (auto& bullet : m_entities.getEntities("bullet"))
	{
		bullet->get<CTransform>().pos += bullet->get<CTransform>().velocity;
	}

	// Enemy Movement
	for (auto& enemy : m_entities.getEntities("enemy"))
	{
		Vec2f& pos = enemy->get<CTransform>().pos;
		Vec2f& vel = enemy->get<CTransform>().velocity;
		float radius = enemy->get<CCollision>().radius;

		pos += vel;

		// Bounce off walls
		if (pos.x + radius >= m_window.getSize().x || pos.x - radius <= 0)
			vel.x *= -1;

		if (pos.y + radius >= m_window.getSize().y || pos.y - radius <= 0)
			vel.y *= -1;
			
		enemy->get<CTransform>().angle += 2.5f;
	}

	// Small enemy Movement
	for (auto& sEnemy : m_entities.getEntities("sEnemy"))
	{
		sEnemy->get<CTransform>().pos += sEnemy->get<CTransform>().velocity;
		sEnemy->get<CTransform>().angle += 2.5f;
	}

	// Player movement based on input
	auto& transform = player()->get<CTransform>();
	
	// Vertical/horizontal speed
	transform.velocity.y = 10 * player()->get<CInput>().down - (10 * player()->get<CInput>().up);
	transform.velocity.x = 10 * player()->get<CInput>().right - (10 * player()->get<CInput>().left);

	// Diagonal speed
	if (transform.velocity.x != 0 && transform.velocity.y != 0)
	{
		transform.velocity.y *= sqrt(2) / 2;
		transform.velocity.x *= sqrt(2) / 2; 
	}

	float player_rad = player()->get<CCollision>().radius;

	// Adjust position and angle
	transform.pos += transform.velocity;

	// Stop player from going outside window
	if (transform.pos.x + player_rad > m_window.getSize().x)
		transform.pos.x = m_window.getSize().x - player_rad;
	else if (transform.pos.x - player_rad < 0)
		transform.pos.x = player_rad;

	if (transform.pos.y + player_rad > m_window.getSize().y)
		transform.pos.y = m_window.getSize().y - player_rad;
	else if (transform.pos.y - player_rad < 0)
		transform.pos.y = player_rad;


	transform.angle += 3.0f;
}

void Game::sLifespan()
{
	if (!m_systems.lifespan)
		return;
	//TODO: implement all lifespan functionality

		// for all entities
			// if entity has no lifespan, skip
			// if entity has > 0 remaining lifespan, skip it
			// if it has lifespan and is alive
				// scale alpha channel properly
			// if it has lifespan and its time is up
				// destroy the entity

	for (auto& e: m_entities.getEntities())
	{
		if (e->has<CLifespan>())
		{
			if (e->get<CLifespan>().remaining > 1)
			{
				e->get<CLifespan>().remaining += -1;
				sf::Color cur_col = e->get<CShape>().circle.getFillColor();
				sf::Color outline_col = e->get<CShape>().circle.getOutlineColor();
				float alpha = static_cast<float>(e->get<CLifespan>().remaining) / static_cast<float>(e->get<CLifespan>().lifespan) * 255.0f;
				e->get<CShape>().circle.setFillColor({ cur_col.r, cur_col.g, cur_col.b, static_cast<uint8_t>(alpha) });
				e->get<CShape>().circle.setOutlineColor({ outline_col.r, outline_col.g, outline_col.b, static_cast<uint8_t>(alpha) });
			}

			else
				e->destroy();
		}
	}



}


void Game::sCollision()
{
	if (!m_systems.collision)
		return;

	// TODO: implement all proper collisions between entities
		//	be sure to use collision radius

	// Check all bullet collisions
	for (auto b : m_entities.getEntities("bullet"))
	{
		// If bullet hits enemies
		for (auto e : m_entities.getEntities("enemy"))
		{
			if (checkCollision(b, e))
			{
				spawnSmallEnemies(e);
				m_score += e->get<CScore>().score;
				e->destroy();
				b->destroy();
				break;
			}

		}

		if (!b->isActive())
			continue;

		// If bullet hits small enemies
		for (auto e : m_entities.getEntities("sEnemy"))
		{
			if (checkCollision(b, e))
			{
				m_score += e->get<CScore>().score;
				e->destroy();
				b->destroy();
			}
		}
	}

	// Check if enemies have hit player
	for (auto e : m_entities.getEntities("enemy"))
	{
		if (checkCollision(e, player()))
		{
			spawnSmallEnemies(e);
			e->destroy();
			player()->get<CTransform>().pos = Vec2f(m_window.getSize().x / 2, m_window.getSize().y / 2);
			m_score = 0;
		}
	}

	// Check if small enemies have hit player
	for (auto e : m_entities.getEntities("sEnemy"))
	{
		if (checkCollision(e, player()))
		{
			e->destroy();
			player()->get<CTransform>().pos = Vec2f(m_window.getSize().x / 2, m_window.getSize().y / 2);
			m_score = 0;
		}
	}

}

// Helper function which checks if two entities are colliding
bool Game::checkCollision(std::shared_ptr<Entity> entity1, std::shared_ptr<Entity> entity2)
{
	// Calculate distance between entities
	double distance = sqrt(pow(entity1->get<CTransform>().pos.x - entity2->get<CTransform>().pos.x, 2) +
		pow(entity1->get<CTransform>().pos.y - entity2->get<CTransform>().pos.y, 2));

	// Check for collision using distance 
	if (distance <= entity1->get<CCollision>().radius + entity2->get<CCollision>().radius)
		return true;
	else
		return false;

}




void Game::sEnemySpawner()
{
	if (!m_systems.spawning)
		return;
	
	// If spawn interval time has passed
	if (m_currentFrame >= m_lastEnemySpawnTime + m_enemyConfig.SI)
		spawnEnemy();

}

void Game::sGUI()
{
	if (!m_systems.gui)
		return;

	ImGui::SetNextWindowSize(ImVec2(550, 650), ImGuiCond_Appearing);
	ImGui::Begin("Geometry Wars");


	// Two tabs
	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
	if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
	{
		if (ImGui::BeginTabItem("Systems"))
		{
			ImGui::Checkbox("Movement", &m_systems.movement);
			ImGui::Checkbox("Lifespan", &m_systems.lifespan);
			ImGui::Checkbox("Collision", &m_systems.collision);
			ImGui::Checkbox("Spawning", &m_systems.spawning);
			ImGui::Indent();
			ImGui::SliderInt("Spawn", &m_enemyConfig.SI, 0, 120);
			if (ImGui::Button("Manual Spawn", { 200, 35 }))
			{
				spawnEnemy();
			}
			ImGui::Unindent();
			ImGui::Checkbox("GUI", &m_systems.gui);
			ImGui::Checkbox("Rendering", &m_systems.rendering);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Entities"))
		{
			int i = 0;
			if (ImGui::CollapsingHeader("Entities"))
			{
				ImGui::Indent();
				if (ImGui::CollapsingHeader("bullet"))
				{
					float indent = 60.0f;
					ImGui::Indent();
					for (auto& b : m_entities.getEntities("bullet"))
					{
						ImGui::PushID(i++);

						sf::Color col = b->get<CShape>().circle.getFillColor();
						sf::Vector2f pos = b->get<CShape>().circle.getPosition();
						std::string position = "(" + std::to_string(static_cast<int>(pos.x)) + ", " + std::to_string(static_cast<int>(pos.y)) + ")";

						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, col.a / 255.0f));
						if (ImGui::Button("D", { 30, 40 }))
						{
							b->destroy();
						}
						ImGui::PopStyleColor();

						ImGui::SameLine(); ImGui::Indent(indent);
						ImGui::Text(std::to_string(b->id()).c_str());
						ImGui::SameLine(); ImGui::Indent(indent);
						ImGui::Text(b->tag().c_str());
						ImGui::SameLine(); ImGui::Indent(indent * 2);
						ImGui::Text(position.c_str());
						ImGui::Unindent(4 * indent);
						ImGui::PopID();
					}
					ImGui::Unindent();

				}

				if (ImGui::CollapsingHeader("enemy"))
				{
					float indent = 60.0f;
					ImGui::Indent();
					for (auto& e : m_entities.getEntities("enemy"))
					{
						ImGui::PushID(i++);
						
						sf::Color col = e->get<CShape>().circle.getFillColor();
						sf::Vector2f pos = e->get<CShape>().circle.getPosition();
						std::string position = "(" + std::to_string(static_cast<int>(pos.x)) + ", " + std::to_string(static_cast<int>(pos.y)) + ")";

						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, col.a / 255.0f));
						if (ImGui::Button("D", { 30, 40 }))
						{
							e->destroy();
						}
						ImGui::PopStyleColor();

						ImGui::SameLine(); ImGui::Indent(indent);
						ImGui::Text(std::to_string(e->id()).c_str());
						ImGui::SameLine(); ImGui::Indent(indent);
						ImGui::Text(e->tag().c_str());
						ImGui::SameLine(); ImGui::Indent(indent * 2);
						ImGui::Text(position.c_str());
						ImGui::Unindent(4 * indent);
						ImGui::PopID();
					}
					ImGui::Unindent();
				}

				if (ImGui::CollapsingHeader("player"))
				{
					//::PushID(0);
					ImGui::Indent();
					float indent = 60.0f;
					sf::Color col = player()->get<CShape>().circle.getFillColor();
					sf::Vector2f pos = player()->get<CShape>().circle.getPosition();
					std::string position = "(" + std::to_string(static_cast<int>(pos.x)) + ", " + std::to_string(static_cast<int>(pos.y)) + ")";

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, col.a / 255.0f));
					if (ImGui::Button("D", { 30, 40 }))
					{
						player()->destroy();
					}
					ImGui::PopStyleColor();

					ImGui::SameLine(); ImGui::Indent(indent);
					ImGui::Text(std::to_string(player()->id()).c_str());
					ImGui::SameLine(); ImGui::Indent(indent);
					ImGui::Text(player()->tag().c_str());
					ImGui::SameLine(); ImGui::Indent(indent * 2);
					ImGui::Text(position.c_str());
					ImGui::Unindent(4 * indent);
					ImGui::Unindent();
					//ImGui::PopID();
				}

				if (ImGui::CollapsingHeader("small enemy"))
				{
					float indent = 60.0f;
					ImGui::Indent();
					for (auto& e : m_entities.getEntities("sEnemy"))
					{
						ImGui::PushID(i++);

						sf::Color col = e->get<CShape>().circle.getFillColor();
						sf::Vector2f pos = e->get<CShape>().circle.getPosition();
						std::string position = "(" + std::to_string(static_cast<int>(pos.x)) + ", " + std::to_string(static_cast<int>(pos.y)) + ")";

						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, col.a / 255.0f));
						if (ImGui::Button("D", { 30, 40 }))
						{
							e->destroy();
						}
						ImGui::PopStyleColor();

						ImGui::SameLine(); ImGui::Indent(indent);
						ImGui::Text(std::to_string(e->id()).c_str());
						ImGui::SameLine(); ImGui::Indent(indent);
						ImGui::Text(e->tag().c_str());
						ImGui::SameLine(); ImGui::Indent(indent * 2);
						ImGui::Text(position.c_str());
						ImGui::Unindent(4 * indent);
						ImGui::PopID();
					}
					ImGui::Unindent();
				}
				ImGui::Unindent();
			} // End of entities dropdown
			
			if (ImGui::CollapsingHeader("All Entities"))
			{
				float indent = 60.0f;
				ImGui::Indent();
				for (auto& e : m_entities.getEntities())
				{
					ImGui::PushID(i++);

					sf::Color col = e->get<CShape>().circle.getFillColor();
					sf::Vector2f pos = e->get<CShape>().circle.getPosition();
					std::string position = "(" + std::to_string(static_cast<int>(pos.x)) + ", " + std::to_string(static_cast<int>(pos.y)) + ")";

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, col.a / 255.0f));
					if (ImGui::Button("D", { 30, 40 }))
					{
						e->destroy();
					}
					ImGui::PopStyleColor();

					ImGui::SameLine(); ImGui::Indent(indent);
					ImGui::Text(std::to_string(e->id()).c_str());
					ImGui::SameLine(); ImGui::Indent(indent);
					ImGui::Text(e->tag().c_str());
					ImGui::SameLine(); ImGui::Indent(indent * 2);
					ImGui::Text(position.c_str());
					ImGui::Unindent(4 * indent);
					ImGui::PopID();
				}
				ImGui::Unindent();

			}
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}


	ImGui::End();

}
	
void Game::sRender()	
{
	// TODO: change code to draw ALL of the entities

	m_window.clear();
	if (!m_systems.rendering)
	{
		ImGui::SFML::Render(m_window);
		m_window.display();
		return;
	}

	// Draw bullets
	for (auto& bullet : m_entities.getEntities("bullet"))
	{
		bullet->get<CShape>().circle.setPosition(bullet->get<CTransform>().pos);
		m_window.draw(bullet->get<CShape>().circle);
	}

	// Draw enemies
	for (auto& enemy : m_entities.getEntities("enemy"))
	{
		enemy->get<CShape>().circle.setPosition(enemy->get<CTransform>().pos);
		enemy->get<CShape>().circle.setRotation(sf::degrees(enemy->get<CTransform>().angle));
		m_window.draw(enemy->get<CShape>().circle);
	}

	//Draw small enemies
	for (auto& sEnemy : m_entities.getEntities("sEnemy"))
	{
		sEnemy->get<CShape>().circle.setPosition(sEnemy->get<CTransform>().pos);
		sEnemy->get<CShape>().circle.setRotation(sf::degrees(sEnemy->get<CTransform>().angle));
		m_window.draw(sEnemy->get<CShape>().circle);
	}

	// set position of shape based on entity's transform pos
	player()->get<CShape>().circle.setPosition(player()->get<CTransform>().pos);
	// Set rotation of player based on entity's transform angle
	player()->get<CShape>().circle.setRotation(sf::degrees(player()->get<CTransform>().angle));

	//draw player
	m_window.draw(player()->get<CShape>().circle);

	//Draw score
	m_text.setString("Score:  " + std::to_string(m_score));
	m_window.draw(m_text);

	//Draw high score
	m_high_score_text.setString("High  Score:  " + std::to_string(m_high_score));
	sf::FloatRect textBounds = m_high_score_text.getLocalBounds();
	m_high_score_text.setOrigin({ textBounds.size.x, 0});
	m_high_score_text.setPosition({ static_cast<float>(m_window.getSize().x)- 10.0f, 0 });
	m_window.draw(m_high_score_text);

	// Draw the ui last
	ImGui::SFML::Render(m_window);

	m_window.display();

}

// Handle user input here
void Game::sUserInput()
{
	// set player's input component variables

	 // Handle Events
	while (const std::optional<sf::Event> event = m_window.pollEvent()) {

		// Pass event to imgui
		ImGui::SFML::ProcessEvent(m_window, *event);

		// Window close
		if (event->is<sf::Event::Closed>()) {
			m_running = false;
		}
		
		// Keyboard Press
		else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			switch (keyPressed->scancode)
			{
			case sf::Keyboard::Scan::W:
				std::cout << "W Key Pressed\n";
				player()->get<CInput>().up = true;
				break;
			case sf::Keyboard::Scan::S:
				std::cout << "S Key Pressed\n";
				player()->get<CInput>().down = true;
				break;
			case sf::Keyboard::Scan::A:
				std::cout << "A Key Pressed\n";
				player()->get<CInput>().left = true;
				break;
			case sf::Keyboard::Scan::D:
				std::cout << "D Key Pressed\n";
				player()->get<CInput>().right = true;
				break;
			case sf::Keyboard::Scan::P:
				std::cout << "P Key Pressed, Pausing/Resuming!\n";
				m_paused = !m_paused;
				break;
			case sf::Keyboard::Scan::Escape:
				std::cout << "Exiting game!\n";
				m_running = false;
				break;
			case sf::Keyboard::Scan::G:
				std::cout << "Toggling GUI!\n";
				m_systems.gui = !m_systems.gui;
			default:
					break;
			}
		}

		//Keyboard Release
		else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
		{
			switch (keyReleased->scancode)
			{
			case sf::Keyboard::Scan::W:
				std::cout << "W Key released\n";
				player()->get<CInput>().up = false;
				break;
			case sf::Keyboard::Scan::S:
				std::cout << "S Key Released\n";
				player()->get<CInput>().down = false;
				break;
			case sf::Keyboard::Scan::A:
				std::cout << "A Key Released\n";
				player()->get<CInput>().left = false;
				break;
			case sf::Keyboard::Scan::D:
				std::cout << "D Key Released\n";
				player()->get<CInput>().right = false;
				break;
			default:
				break;
			}
		}

		//Mouse buttons
		else if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>())
		{
			// Ignore mouse events if ImGui is being clicked
			if (ImGui::GetIO().WantCaptureMouse) { continue; }

			// Spawn bullet if left click
			if (mouseButtonPressed->button == sf::Mouse::Button::Left)
			{
				std::cout << "Left mouse button was pressed at position (" << mouseButtonPressed->position.x << ", " << mouseButtonPressed->position.y << ")\n";
				spawnBullet(player(), sf::Vector2f(mouseButtonPressed->position));
			}

			// Spawn special weapon if right click
			else if (mouseButtonPressed->button == sf::Mouse::Button::Right)
			{
				std::cout << "Right mouse button was pressed";
				spawnSpecialWeapon(player());
			}
		}

	}// End of event loop
}





