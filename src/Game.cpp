#include "Game.h"

#include <iostream>
#include <math.h>
#include <fstream>
// #include <cstdlib> // For rand() and srand()

Game::Game(const std::string &config)
{
	init(config);
}

void Game::init(const std::string &path)
{
	std::string font_path;
	int win_w, win_h, fps;
	bool fullscreen;

	// read config file start
	std::ifstream fin(path);
	if (!fin.is_open())
	{
		std::cerr << "Error opening file\n";
		exit(1);
	}

	std::string type;
	while (fin >> type)

	{
		if (type == "window")
		{
			fin >> win_w >> win_h >> fps >> fullscreen;
		}
		else if (type == "Font")
		{
			fin >> m_fontPath >> m_fontSize >> m_fontR >> m_fontG >> m_fontB;
		}
		else if (type == "Player")
		{

			fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
		}
		else if (type == "Enemy")
		{
			fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
		}
		else if (type == "Bullet")
		{
			fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
		}
	}
	// read config file end

	// set default window params
	if (fullscreen)
	{
		auto mode = sf::VideoMode::getDesktopMode();
		m_window.create(sf::VideoMode(mode), "AAA Game", sf::Style::Fullscreen);
	}
	else
	{
		m_window.create(sf::VideoMode(win_w, win_h), "AAA Game");
	}
	m_window.setFramerateLimit(fps);

	// load the font and set defaults for score and player lifes
	sf::Vector2 window_size = m_window.getSize();

	if (!m_font.loadFromFile(m_fontPath))
	{
		fprintf(stderr, "failed at loading font from a file");
		exit(1);
	}
	
	// defaults for the score
	m_scoreText.setFont(m_font);
	m_scoreText.setCharacterSize(m_fontSize);
	m_scoreText.setFillColor(sf::Color(m_fontR, m_fontG, m_fontB));
	sf::FloatRect scoreBounds = m_scoreText.getLocalBounds();
	m_scoreText.setOrigin(scoreBounds.width / 2, scoreBounds.height / 2);
	m_scoreText.setPosition(window_size.x / 2, 0 + scoreBounds.height / 2);

	// defaults for the players life
	m_playerLifeText.setFont(m_font);
	m_playerLifeText.setCharacterSize(m_fontSize);
	m_playerLifeText.setFillColor(sf::Color(sf::Color::Cyan));
	sf::FloatRect playerLifebounds = m_playerLifeText.getLocalBounds();
	m_playerLifeText.setOrigin(playerLifebounds.width / 2, playerLifebounds.height / 2);
	m_playerLifeText.setPosition(window_size.x / 2, (0 + playerLifebounds.height / 2) + scoreBounds.height + 20);

	// Points needed text
	m_pointsNeededText.setFont(m_font);
	m_pointsNeededText.setCharacterSize(m_fontSize);
	m_pointsNeededText.setFillColor(sf::Color(sf::Color::Green));
	sf::FloatRect pointsbounds = m_pointsNeededText.getLocalBounds();
	m_pointsNeededText.setOrigin(pointsbounds.width / 2, pointsbounds.height / 2);
	m_pointsNeededText.setPosition(window_size.x / 2, (0 + pointsbounds.height / 2) + scoreBounds.height + playerLifebounds.height + 40);



	// game over text
	m_gameOverText.setFont(m_font);
	m_gameOverText.setCharacterSize(m_fontSize * 3); // make it big
	m_gameOverText.setFillColor(sf::Color::Magenta);
	m_gameOverText.setString("YOU LOST");

	// center of screen
	sf::Vector2 center_of_window(window_size.x / 2.0f, window_size.y / 2.0f);
	sf::FloatRect GameOverbounds = m_gameOverText.getLocalBounds();
	m_gameOverText.setOrigin(GameOverbounds.width / 2, GameOverbounds.height / 2);
	m_gameOverText.setPosition(center_of_window);

	m_textPlayerWon.setFont(m_font);
	m_textPlayerWon.setCharacterSize(m_fontSize * 3); // make it big
	m_textPlayerWon.setFillColor(sf::Color::Blue);
	m_textPlayerWon.setString("YOU WON!");

	sf::FloatRect playerWonbounds = m_textPlayerWon.getLocalBounds();
	m_textPlayerWon.setOrigin(playerWonbounds.width / 2, playerWonbounds.height / 2);
	m_textPlayerWon.setPosition(center_of_window);

	spawnPlayer();
}

void Game::run()
{

	while (m_running)
	{
		m_entities.update();

		if (!m_paused)
		{
			sEnemySpawner();
			sMovement();
			sCollision();
			sLifespan();
		}
		sUserInput();
		sRender();

		// increment the current frame;
		m_currentFrame++;
	}
}

void Game::setPaused(bool paused) // TODO Unused. Use when the p button is pressed
{
	m_paused = paused;
}

void Game::spawnPlayer()
{
	
	auto entity = m_entities.addEntity("player");

	// get cent of the window
	// to get in the center of the window we get window size(width, height)  and devide it by 2
	sf::Vector2 window_size = m_window.getSize();
	Vec2 center_of_window(window_size.x / 2.0f, window_size.y / 2.0f); // vec2 with x, y of center of the window

	entity->cTransform = std::make_shared<CTransform>(center_of_window, Vec2(10.0f, 0.0f), 0); // * NOTE: assumed that we start at 0 vel and angle

	entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V,
											  sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB),
											  sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);

	entity->cInput = std::make_shared<CInput>();

	entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

	m_player = entity;
}

void Game::spawnEnemy()
{
	auto entity = m_entities.addEntity("enemy");

	srand(static_cast<unsigned int>(time(0))); // seed the rand generator

	// configure the enemy
	//  get rand pos x
	int x_start = m_enemyConfig.SR;
	int x_end = m_window.getSize().x - m_enemyConfig.SR;
	float x_rand = x_start + (rand() % (x_end - x_start));

	// get rand pos y
	int y_start = m_enemyConfig.SR;
	int y_end = m_window.getSize().y - m_enemyConfig.SR;
	float y_rand = y_start + (rand() % (y_end - y_start));

	// get the velocity
	float speed_min = m_enemyConfig.SMIN;
	float speed_max = m_enemyConfig.SMAX;
	float random_speed = speed_min + (static_cast<float>(rand()) / RAND_MAX) * (speed_max - speed_min);

	// rand angle for shapes movement inside the window
	float random_angle = rand() % 360;

	float v_x = cos(random_angle) * random_speed; // * Note check the angle
	float v_y = sin(random_angle) * random_speed; // * Note check the angle
	Vec2 velocity(v_x, v_y);

	entity->cTransform = std::make_shared<CTransform>(Vec2(x_rand, y_rand), velocity, random_angle, random_speed);

	int max_vertices = m_enemyConfig.VMAX;
	int min_vertices = m_enemyConfig.VMIN;
	int random_vertices = min_vertices + (rand() % (max_vertices - min_vertices));
	entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR, random_vertices, // put the random vertices here
											  sf::Color(0, 0, 0),
											  sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);

	entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);

	// record the most recernt enemy was spawned
	m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> e) // TODO: isnt used after the enemy dies, need to add it there.
{
	// spanw smaller enemies at location of the enemy e
	size_t numOfSides = e->cShape->circle.getPointCount();

	// spawn n spawn enemies for n sides of e
	// set same colour and half the size
	float completeAngle = 360;
	float angleSteps = completeAngle / numOfSides;

	float currentAngle = 0;
	std::cout << "num of sides: " << numOfSides << '\n';

	for (int i = 0; i < numOfSides; i++)
	{
			std::cout << "curr angle: " << currentAngle << " i: " << i << '\n'
					  << "angleSteps: " << angleSteps << '\n';
			// create the entity
			auto entity = m_entities.addEntity("enemy");

			// extract shape data from the dying enemy and put it in its dying fragments
			float radius = e->cShape->circle.getRadius() / 2;
			int vertices = e->cShape->circle.getPointCount();
			sf::Color fillColor = e->cShape->circle.getFillColor();
			sf::Color outlineColor = e->cShape->circle.getOutlineColor();
			float outThickness = e->cShape->circle.getOutlineThickness();
			entity->cShape = std::make_shared<CShape>(radius, vertices, fillColor, outlineColor, outThickness);

			// give it the transform
			// use the current pos of the dying entity
			 Vec2 pos(e->cTransform->pos.x, e->cTransform->pos.y);

			// get the speed
			float dyingEntitySpeed = e->cTransform->speed - 10;

			// get the x and y dir movement
			float radians = currentAngle * (M_PI/ 180.0f);

			float v_x = cos(radians) * dyingEntitySpeed;
			float v_y = sin(radians) * dyingEntitySpeed;
			Vec2 newVelocity(v_x, v_y);

			entity->cTransform = std::make_shared<CTransform>(pos, newVelocity, 0);

			// set 2 points for killing this fragments
			entity->setPointForSmallEnemies();
			entity->setIsSmallEnemy();

			// give an arbitray life, for testing
			entity->cLifespan = std::make_shared<CLifespan>(40);

			entity->cCollision = std::make_shared<CCollision>(radius + 5);

			// set the angle for next spawn
			currentAngle += angleSteps;
	}
}

void Game::spawnBullet(std::shared_ptr<Entity> e, const Vec2 &target)
{
	// set velocity

	auto bullet = m_entities.addEntity("bullet");
	// a little bit of MATHH!!
	float dx = target.x - m_player->cTransform->pos.x;
	float dy = target.y - m_player->cTransform->pos.y;

	// direction vector normalized
	float totalLen = std::sqrt(dx * dx + dy * dy);
	Vec2 dir(0, 0);
	if (totalLen != 0)
	{
		dir.x = dx / totalLen;
		dir.y = dy / totalLen;
	}

	Vec2 velocity = {dir.x * m_bulletConfig.S, dir.y * m_bulletConfig.S};

	Vec2 start_pos = {m_player->cTransform->pos.x, m_player->cTransform->pos.y};
	bullet->cTransform = std::make_shared<CTransform>(start_pos, velocity, 0);

	bullet->cShape = std::make_shared<CShape>(
		m_bulletConfig.SR, m_bulletConfig.V,
		sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB),
		sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB),
		m_bulletConfig.OT);

	bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);

	// give a lifespan
	float totalLifespan = totalLen / m_bulletConfig.S;
	std::cout << "Bullet totalLifespan is: " << totalLifespan << "\n"
			  << "total len: " << totalLen << "\n";
	bullet->cLifespan = std::make_shared<CLifespan>(totalLifespan + 4);

	std::cout << "CP after bullet life has been blessed to it\n";
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> e)
{
	// TODO THINK YO!
}

void Game::sMovement()
{
	// get the x and y movement for the velocity
	float speed = m_playerConfig.S; // speed is the movement in pixel per frame. so it is also hypotenus if the movemnt is not in x or y exactly

	// Set directiona nd normalize
	Vec2 dir(0, 0);
	if (m_player->cInput->up)
		dir.y -= 1;
	if (m_player->cInput->down)
		dir.y += 1;
	if (m_player->cInput->left)
		dir.x -= 1;
	if (m_player->cInput->right)
		dir.x += 1;

	float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	if (length != 0)
	{
		dir.x /= length;
		dir.y /= length;
	}

	// set vel to 0 at every frame
	m_player->cTransform->velocity = {0, 0};
	// apply speed
	m_player->cTransform->velocity.x = dir.x * speed;
	m_player->cTransform->velocity.y = dir.y * speed;

	m_player->cTransform->pos.x += m_player->cTransform->velocity.x;
	m_player->cTransform->pos.y += m_player->cTransform->velocity.y;

	// for other entites
	for (auto e : m_entities.getEntities("enemy"))
	{
		e->cTransform->pos.x += e->cTransform->velocity.x;
		e->cTransform->pos.y += e->cTransform->velocity.y;
	}

	for (auto e : m_entities.getEntities("bullet"))
	{
		e->cTransform->pos.x += e->cTransform->velocity.x;
		e->cTransform->pos.y += e->cTransform->velocity.y;
	}
}

void Game::sLifespan()
{
	for (auto e : m_entities.getEntities())
	{
		if (e->cLifespan)
		{
			if (e->cLifespan->remaining > 0)
			{
				e->cLifespan->remaining -= 1;
				// int currentAlpha = e->getCurrentAlpha();
				float ratioOfAlphaToSet = float(e->cLifespan->remaining) / float(e->cLifespan->total);
				int alphaToSet = ratioOfAlphaToSet * 255;
				// std::cout << "ratio " << ratioOfAlphaToSet << "Alpha: " << alphaToSet << "\n";
				e->setAlpha(alphaToSet);
			}
			if (e->isActive() && e->cLifespan->remaining <= 0)
			{
				e->destroy();
			}
		}
	}
}

void Game::sCollision()
{

	sf::Vector2 window_size = m_window.getSize();

	for (auto e : m_entities.getEntities())
	{
		float radius = e->cShape->circle.getRadius();
		if (e->tag() == "player")
		{
			// check collision with the window border
			if (e->cTransform->pos.x < radius)
				e->cTransform->pos.x = radius;
			if (e->cTransform->pos.x > window_size.x - radius)
				e->cTransform->pos.x = window_size.x - radius;

			if (e->cTransform->pos.y < radius)
				e->cTransform->pos.y = radius;
			if (e->cTransform->pos.y > window_size.y - radius)
				e->cTransform->pos.y = window_size.y - radius;
		}

		if (e->tag() == "enemy")
		// check collision with the window border
		{
			if (e->cTransform->pos.x < radius || e->cTransform->pos.x > window_size.x - radius)
			{
				e->cTransform->velocity.x *= -1;
				// something gpt suggested me to look into. SOMETIMES ITS GOOD TO HAVE SOMEONE (SOMETHING) GIVING FEEDBACK and suggestions!!
				e->cTransform->pos.x = std::clamp(e->cTransform->pos.x, radius, (float)window_size.x - radius);
				// now that I think about it, it was much simpler the way i had done it above, just have few more lines of code
			}

			if (e->cTransform->pos.y < radius || e->cTransform->pos.y > window_size.y - radius)
			{
				e->cTransform->velocity.y *= -1;
				e->cTransform->pos.y = std::clamp(e->cTransform->pos.y, radius, (float)window_size.y - radius);
			}
		}
	}


	float player_rad = m_player->cCollision->radius;
	float playerPosX = m_player->cTransform->pos.x;
	float playerPosY = m_player->cTransform->pos.y;

	for (auto enemy : m_entities.getEntities("enemy"))
	{
		float enemy_rad = enemy->cCollision->radius;
		float requiredDist = player_rad + enemy_rad;

		float distX = playerPosX - enemy->cTransform->pos.x;
		float distY = playerPosY - enemy->cTransform->pos.y;
		float actualDist = distX * distX + distY * distY;

		if (actualDist < requiredDist * requiredDist)
		{
			m_player->destroy();
			spawnPlayer();
			m_playerLife--;
			if (m_playerLife == 0)
			{
				setGameOver(true);
			}
		}
	}

	// collision between bullet and the enemy
	for (auto b : m_entities.getEntities("bullet"))
	{
		for (auto e : m_entities.getEntities("enemy"))
		{
			if (b->isActive() && e->isActive())
			{

				Vec2 new_vec = {e->cTransform->pos.x - b->cTransform->pos.x, e->cTransform->pos.y - b->cTransform->pos.y};
				float squared_dist = new_vec.x * new_vec.x + new_vec.y * new_vec.y;

				if (squared_dist < b->cCollision->radius * b->cCollision->radius + e->cCollision->radius * e->cCollision->radius)
				{
					m_score += e->getPoints(); // TODO set the point in smallenemies

					if (m_score == m_scoreToBeat)
					{
						setPlayerWon(true);
					}
					// spawn small enemies when the e is hit
					if (!e->isSmallEmeny())
					{
						spawnSmallEnemies(e);
					}

					if (b)
						b->destroy();
					if (e)
						e->destroy();
				}
			}
		}
	}
}

void Game::setGameOver(bool)
{
	m_gameOver = true;
}

void Game::setPlayerWon(bool)
{
	m_playerWon = true;
}
void Game::sEnemySpawner()
{
	// TODO enemy spawning will go here

	// to get how long it has ben since last enemy spawned
	if (m_currentFrame - m_lastEnemySpawnTime >= m_enemyConfig.SI)
	{
		spawnEnemy();
	}
}

void Game::sRender()
{
	m_window.clear();
	if (m_gameOver)
	{
		m_window.draw(m_gameOverText);
	}
	else if (m_playerWon)
	{
		m_window.draw(m_textPlayerWon);
	}
	else
	{

		for (auto e : m_entities.getEntities())
		{
			if (e->cShape && e->cTransform && e->isActive())
			{

				e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);

				e->cTransform->angle += 4.0f;
				e->cShape->circle.setRotation(e->cTransform->angle);

				m_window.draw(e->cShape->circle);
			}
		}
		m_scoreText.setString("Score: " + std::to_string(m_score));
		m_playerLifeText.setString("Life left: " + std::to_string(m_playerLife));
		m_pointsNeededText.setString("Needed score: " + std::to_string(m_scoreToBeat));

		m_window.draw(m_pointsNeededText);
		m_window.draw(m_scoreText);
		m_window.draw(m_playerLifeText);
	}
	m_window.display();
}

void Game::sUserInput()
{
	sf::Event event;

	while (m_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			m_running = false;
		}

		// when key press event
		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::Escape:
			{
				m_running = false;
				break;
			}

			case sf::Keyboard::P:
			{
				if (m_paused)
				{
					m_paused = false;
				}
				else
				{
					m_paused = true;
				}

				break;
			}

			case sf::Keyboard::W:
			{
				std::cout << "W pressed\n";
				m_player->cInput->up = true;
				break;
			}

			case sf::Keyboard::S:
			{
				std::cout << "S pressed\n";
				m_player->cInput->down = true;
				break;
			}

			case sf::Keyboard::A:
			{
				std::cout << "A pressed\n";
				m_player->cInput->left = true;
				break;
			}

			case sf::Keyboard::D:
			{
				std::cout << "D pressed\n";
				m_player->cInput->right = true;
				break;
			}
			}
		}
		if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W:
			{
				std::cout << "W released\n";
				m_player->cInput->up = false;
				break;
			}

			case sf::Keyboard::S:
			{
				std::cout << "S released\n";
				m_player->cInput->down = false;
				break;
			}

			case sf::Keyboard::A:
			{
				std::cout << "A released\n";
				m_player->cInput->left = false;
				break;
			}

			case sf::Keyboard::D:
			{
				std::cout << "D released\n";
				m_player->cInput->right = false;
				break;
			}
			}
		}

		if (event.type == sf::Event::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				std::cout << "Left mouse Button at (" << event.mouseButton.x << ","
						  << event.mouseButton.y << ")\n";
				spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
			}

			if (event.mouseButton.button == sf::Mouse::Right)
			{
				std::cout << "Left mouse Button at (" << event.mouseButton.x << ","
						  << event.mouseButton.y << ")\n";
				// call special weapon
				spawnSpecialWeapon(m_player);
			}
		}
	}
}
