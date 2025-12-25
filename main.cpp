#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <iomanip>

int main() {
    // ==================== WINDOW SETUP ====================
    sf::RenderWindow window(sf::VideoMode(1200, 800), "Gimball Ski Noel - PRO Edition!");
    window.setFramerateLimit(60);
    srand(time(0));

    // ==================== SIMPLE FIXED BACKGROUND ====================
    sf::Texture bgTex;
    sf::Sprite background;
    bool hasBg = false;

    // Thử load các file background
    std::vector<std::string> bgFiles = {"bg1.jpg", "bg2.jpg", "bg3.jpg", "background.jpg", "bg.png"};

    for (const auto& file : bgFiles) {
        if (bgTex.loadFromFile(file)) {
            hasBg = true;
            background.setTexture(bgTex);

            // Scale cho vừa cửa sổ 1200x800
            float scaleX = 1200.0f / bgTex.getSize().x;
            float scaleY = 800.0f / bgTex.getSize().y;

            // Giữ tỉ lệ, có thể bị crop nhưng đẹp hơn
            float scale = std::max(scaleX, scaleY);

            background.setScale(scale, scale);

            // Căn giữa background
            float bgWidth = bgTex.getSize().x * scale;
            float bgHeight = bgTex.getSize().y * scale;
            background.setPosition((1200 - bgWidth) / 2, (800 - bgHeight) / 2);

            std::cout << "✓ Loaded background: " << file << std::endl;
            std::cout << "  Original size: " << bgTex.getSize().x << "x" << bgTex.getSize().y << std::endl;
            std::cout << "  Scaled to: " << bgWidth << "x" << bgHeight << std::endl;
            break;
        }
    }

    // Fallback background màu gradient đẹp hơn
    sf::RectangleShape fallbackBg(sf::Vector2f(1200, 800));
    fallbackBg.setFillColor(sf::Color(135, 206, 235)); // Màu xanh da trời nhạt

    // ==================== AUDIO SYSTEM ====================
    bool audioEnabled = false;
    sf::SoundBuffer jumpBuffer, duckBuffer, hitBuffer, giftBuffer, bounceBuffer;
    sf::Sound jumpSound, duckSound, hitSound, giftSound, bounceSound;
    sf::Music backgroundMusic;

    // Cố gắng load audio nhưng không bắt buộc
    try {
        if (jumpBuffer.loadFromFile("jump.wav")) {
            jumpSound.setBuffer(jumpBuffer);
            audioEnabled = true;
            std::cout << "✓ Loaded jump sound" << std::endl;
        }
        if (duckBuffer.loadFromFile("duck.wav")) {
            duckSound.setBuffer(duckBuffer);
            std::cout << "✓ Loaded duck sound" << std::endl;
        }
        if (hitBuffer.loadFromFile("hit.wav")) {
            hitSound.setBuffer(hitBuffer);
            std::cout << "✓ Loaded hit sound" << std::endl;
        }
        if (giftBuffer.loadFromFile("gift.wav")) {
            giftSound.setBuffer(giftBuffer);
            std::cout << "✓ Loaded gift sound" << std::endl;
        }
        if (bounceBuffer.loadFromFile("bounce.wav")) {
            bounceSound.setBuffer(bounceBuffer);
            std::cout << "✓ Loaded bounce sound" << std::endl;
        }
        if (backgroundMusic.openFromFile("bg_music.ogg")) {
            backgroundMusic.setLoop(true);
            backgroundMusic.setVolume(40);
            backgroundMusic.play();
            std::cout << "✓ Playing background music" << std::endl;
        }
    } catch (...) {
        std::cout << "⚠ Audio files not found, continuing without sound" << std::endl;
    }

    // ==================== PLAYER WITH SMART HITBOX ====================
    sf::Texture playerTex;
    if (!playerTex.loadFromFile("gimball.png")) {
        // Tạo player mẫu nếu không có file
        sf::Image img;
        img.create(100, 150, sf::Color::Red);
        playerTex.loadFromImage(img);
        std::cout << "⚠ Created default player texture" << std::endl;
    } else {
        std::cout << "✓ Loaded player texture" << std::endl;
    }

    sf::Sprite player(playerTex);
    float playerScale = 0.4f;
    float playerX = 200.0f;
    float playerY = 500.0f;
    sf::Vector2f velocity(0, 0);
    float playerRotation = 0.0f;

    // Physics constants
    const float gravity = 0.8f;
    const float jumpForce = -20.0f;
    float bounceCharge = 0.0f;
    const float maxBounceCharge = 30.0f;
    bool isGrounded = true;
    bool isCharging = false;
    bool isJumping = false;
    bool isDucking = false;
    float duckTime = 0.0f;
    const float maxDuckTime = 5.0f;
    sf::Vector2f chargeDirection(0, -1);
    sf::Clock chargeClock;
    float chargeTime = 0.0f;

    // Player dimensions
    float playerWidth = playerTex.getSize().x * playerScale;
    float playerHeight = playerTex.getSize().y * playerScale;

    // Smart hitbox system
    struct PlayerHitbox {
        sf::FloatRect head;
        sf::FloatRect body;
        sf::FloatRect legs;
        bool headCollision;
        bool bodyCollision;
        bool legsCollision;

        void update(float x, float y, float width, float height, bool ducking) {
            if (ducking) {
                // KHI CÚI: CHỈ CÒN HITBOX Ở CHÂN
                head = sf::FloatRect(0, 0, 0, 0);  // Không có hitbox đầu
                body = sf::FloatRect(0, 0, 0, 0);  // Không có hitbox thân
                legs = sf::FloatRect(x - width*0.4f, y + height*0.1f, width*0.8f, height*0.4f);
            } else {
                // Bình thường: giữ nguyên hitbox cũ
                head = sf::FloatRect(x - width*0.25f, y - height*0.4f, width*0.5f, height*0.2f);
                body = sf::FloatRect(x - width*0.3f, y - height*0.2f, width*0.6f, height*0.4f);
                legs = sf::FloatRect(x - width*0.35f, y + height*0.2f, width*0.7f, height*0.2f);
            }
            headCollision = false;
            bodyCollision = false;
            legsCollision = false;
        }

        void drawDebug(sf::RenderWindow& window) {
            // Vẽ hitbox để debug
            sf::RectangleShape headRect(sf::Vector2f(head.width, head.height));
            headRect.setPosition(head.left, head.top);
            headRect.setFillColor(sf::Color(255, 0, 0, 100));

            sf::RectangleShape bodyRect(sf::Vector2f(body.width, body.height));
            bodyRect.setPosition(body.left, body.top);
            bodyRect.setFillColor(sf::Color(0, 255, 0, 100));

            sf::RectangleShape legsRect(sf::Vector2f(legs.width, legs.height));
            legsRect.setPosition(legs.left, legs.top);
            legsRect.setFillColor(sf::Color(0, 0, 255, 100));

            window.draw(headRect);
            window.draw(bodyRect);
            window.draw(legsRect);
        }
    };

    PlayerHitbox playerHitbox;

    // ==================== OBSTACLE SYSTEM ====================
    enum ObstacleMovement {
        MOV_STATIC,
        MOV_OSCILLATING,
        MOV_SIDEWAYS,
        MOV_ZIGZAG,
        MOV_BOUNCING,
        MOV_HOMING
    };

    struct Obstacle {
        sf::Sprite sprite;
        float x, y;
        float width, height;
        bool passed;
        int type;  // 0=tree, 1=snowball, 2=snowman, 3=gift
        float speedX, speedY;
        bool activeCollision;
        float originalY;
        ObstacleMovement movement;
        float movementTimer;
        float movementParam1, movementParam2;
        bool isLowObstacle;
        float hitboxHeight;
        bool isDangerous;

        Obstacle() : x(0), y(0), width(0), height(0), passed(false), type(0),
                    speedX(5.0f), speedY(0), activeCollision(false),
                    originalY(0), movement(MOV_STATIC), movementTimer(0),
                    movementParam1(0), movementParam2(0), isLowObstacle(false),
                    hitboxHeight(0), isDangerous(true) {}

        std::string getTypeName() const {
            switch(type) {
                case 0: return "Tree";
                case 1: return "Snowball";
                case 2: return "Snowman";
                case 3: return "Gift";
                default: return "Unknown";
            }
        }

        void update(float dt, float playerY) {
            x -= speedX;
            movementTimer += dt;

            // Áp dụng chuyển động
            switch(movement) {
                case MOV_OSCILLATING:
                    y = originalY + sin(movementTimer * movementParam1) * movementParam2;
                    break;

                case MOV_SIDEWAYS:
                    y = originalY + sin(movementTimer * movementParam1) * movementParam2;
                    speedX = 4.0f + sin(movementTimer * 0.5f) * 1.0f;
                    break;

                case MOV_ZIGZAG:
                    y = originalY + sin(movementTimer * movementParam1) * movementParam2;
                    x += cos(movementTimer * movementParam1 * 2) * 2.0f;
                    break;

                case MOV_BOUNCING:
                    if (speedY == 0) speedY = -5.0f;
                    speedY += 0.2f;
                    y += speedY;
                    if (y > originalY) {
                        y = originalY;
                        speedY = -abs(speedY) * 0.8f;
                    }
                    break;

                case MOV_HOMING:
                    if (y < playerY) y += 1.5f;
                    else if (y > playerY) y -= 1.5f;
                    break;

                case MOV_STATIC:
                default:
                    break;
            }

            // Giới hạn vị trí Y
            if (y < 100) y = 100;
            if (y > 600) y = 600;
        }

        sf::FloatRect getHitbox() const {
            float scale = 0.7f;
            return sf::FloatRect(x + width*(1-scale)/2, y + height - hitboxHeight,
                               width*scale, hitboxHeight*scale);
        }

        void drawDebug(sf::RenderWindow& window) {
            sf::FloatRect hitbox = getHitbox();
            sf::RectangleShape rect(sf::Vector2f(hitbox.width, hitbox.height));
            rect.setPosition(hitbox.left, hitbox.top);
            rect.setFillColor(sf::Color(255, 255, 0, 100));
            window.draw(rect);
        }
    };

    std::vector<Obstacle> obstacles;
    float obstacleSpawnTimer = 0.1f;
    float obstacleSpawnInterval = 2.5f;
    const int MAX_OBSTACLES_ON_SCREEN = 6;

    // Load obstacle textures
    sf::Texture treeTex, snowballTex, snowmanTex, giftTex;

    // Sửa lỗi: load từng texture riêng biệt
    bool hasTree = treeTex.loadFromFile("tree.png");
    bool hasSnowball = snowballTex.loadFromFile("snowball.png");
    bool hasSnowman = snowballTex.loadFromFile("snowman.png");
    bool hasGift = giftTex.loadFromFile("gift.png");

    if (hasTree) std::cout << "✓ Loaded tree texture" << std::endl;
    if (hasSnowball) std::cout << "✓ Loaded snowball texture" << std::endl;
    if (hasSnowman) std::cout << "✓ Loaded snowman texture" << std::endl;
    if (hasGift) std::cout << "✓ Loaded gift texture" << std::endl;

    // Nếu không load được texture, tạo hình đơn giản
    auto createSimpleTexture = [](sf::Texture& tex, sf::Color color, int size = 64) {
        sf::Image image;
        image.create(size, size, color);
        return tex.loadFromImage(image);
    };

    if (!hasTree) {
        createSimpleTexture(treeTex, sf::Color::Green, 80);
        std::cout << "⚠ Created default tree texture" << std::endl;
    }
    if (!hasSnowball) {
        createSimpleTexture(snowballTex, sf::Color::White, 60);
        std::cout << "⚠ Created default snowball texture" << std::endl;
    }
    if (!hasSnowman) {
        createSimpleTexture(snowmanTex, sf::Color(255, 200, 200), 70);
        std::cout << "⚠ Created default snowman texture" << std::endl;
    }
    if (!hasGift) {
        createSimpleTexture(giftTex, sf::Color::Red, 50);
        std::cout << "⚠ Created default gift texture" << std::endl;
    }

    // ==================== GAME STATE ====================
    bool gameStarted = false;
    bool gameOver = false;
    float worldSpeed = 1.0f;
    float score = 0.0f;
    int lives = 3;
    int hitsWithoutGift = 0;
    const int maxHitsWithoutGift = 3;
    bool hasGiftPowerup = false;
    int giftPowerupDuration = 0;
    const int maxGiftPowerupDuration = 300;
    sf::Clock gameClock;
    float gameTime = 0.0f;

    // Debug mode
    bool debugMode = false;

    // ==================== UI SYSTEM ====================
    sf::Font font;
    bool hasFont = font.loadFromFile("arial.ttf");

    if (!hasFont) {
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            std::cout << "⚠ Font not found, UI will be limited" << std::endl;
        } else {
            hasFont = true;
            std::cout << "✓ Loaded font" << std::endl;
        }
    }

    // UI Elements
    sf::Text scoreText("", font, 32);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setOutlineColor(sf::Color::Black);
    scoreText.setOutlineThickness(2);

    sf::Text livesText("", font, 32);
    livesText.setFillColor(sf::Color::Green);
    livesText.setOutlineColor(sf::Color::Black);
    livesText.setOutlineThickness(2);

    sf::Text hitsText("", font, 32);
    hitsText.setFillColor(sf::Color::Yellow);
    hitsText.setOutlineColor(sf::Color::Black);
    hitsText.setOutlineThickness(2);

    sf::Text duckText("", font, 24);
    duckText.setFillColor(sf::Color(255, 200, 100));
    duckText.setOutlineColor(sf::Color::Black);
    duckText.setOutlineThickness(1);

    sf::Text powerupText("", font, 24);
    powerupText.setFillColor(sf::Color::Cyan);
    powerupText.setOutlineColor(sf::Color::Black);
    powerupText.setOutlineThickness(1);

    sf::Text chargeText("", font, 20);
    chargeText.setFillColor(sf::Color::Magenta);
    chargeText.setOutlineColor(sf::Color::Black);
    chargeText.setOutlineThickness(1);

    sf::Text instructionText("", font, 24);
    instructionText.setFillColor(sf::Color::Yellow);

    sf::Text gameOverText("", font, 40);
    gameOverText.setFillColor(sf::Color::Red);

    sf::Text debugText("", font, 20);
    debugText.setFillColor(sf::Color::White);

    // ==================== SNOW EFFECT ====================
    std::vector<sf::CircleShape> snowflakes;
    for (int i = 0; i < 80; ++i) {
        sf::CircleShape snow(1.0f + (rand() % 100) / 50.0f);
        snow.setFillColor(sf::Color(255, 255, 255, 150 + rand() % 105));
        snow.setPosition(rand() % 1200, rand() % 800);
        snowflakes.push_back(snow);
    }
    std::cout << "✓ Created snow effect" << std::endl;

    // ==================== GROUND ====================
    sf::RectangleShape ground(sf::Vector2f(1200, 100));
    ground.setFillColor(sf::Color(210, 180, 140, 220));
    ground.setPosition(0, 700);

    sf::RectangleShape groundLine(sf::Vector2f(1200, 3));
    groundLine.setFillColor(sf::Color::White);
    groundLine.setPosition(0, 700);

    std::cout << "\n=== GIMBALL SKI NOEL PRO EDITION ===" << std::endl;
    std::cout << "=== GAME READY ===" << std::endl;
    std::cout << "Press SPACE to begin!" << std::endl;
    std::cout << "Press D to toggle debug mode" << std::endl;
    std::cout << "Press S to duck | Hold DOWN for super jump" << std::endl;

    // ==================== MAIN GAME LOOP ====================
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) window.close();

                // Debug mode toggle
                if (event.key.code == sf::Keyboard::D) {
                    debugMode = !debugMode;
                    std::cout << "Debug mode: " << (debugMode ? "ON" : "OFF") << std::endl;
                }

                // Reset game
                if (event.key.code == sf::Keyboard::R) {
                    gameStarted = false;
                    gameOver = false;
                    playerX = 200.0f;
                    playerY = 500.0f;
                    velocity = sf::Vector2f(0, 0);
                    worldSpeed = 1.0f;
                    score = 0.0f;
                    lives = 3;
                    hitsWithoutGift = 0;
                    hasGiftPowerup = false;
                    giftPowerupDuration = 0;
                    obstacles.clear();
                    obstacleSpawnInterval = 1.8f;
                    bounceCharge = 0.0f;
                    isCharging = false;
                    isJumping = false;
                    isDucking = false;
                    duckTime = 0.0f;
                    gameTime = 0.0f;

                    std::cout << "\n=== GAME RESET ===" << std::endl;
                }

                // Start game
                if (event.key.code == sf::Keyboard::Space && !gameStarted && !gameOver) {
                    gameStarted = true;
                    gameClock.restart();
                    std::cout << "=== GAME STARTED ===" << std::endl;
                }

                // Duck
                if (event.key.code == sf::Keyboard::S && gameStarted && !gameOver && isGrounded && !isCharging) {
                    isDucking = true;
                    duckTime = 0.0f;
                    if (audioEnabled) duckSound.play();
                }
            }

            if (event.type == sf::Event::KeyReleased && gameStarted && !gameOver) {
                // Stop ducking
                if (event.key.code == sf::Keyboard::S) {
                    isDucking = false;
                }

                // Release charged jump
                if (event.key.code == sf::Keyboard::Down) {
                    if (isCharging) {
                        float horizontalBoost = 0.0f;
                        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                            horizontalBoost = -bounceCharge * 0.8f;
                            chargeDirection = sf::Vector2f(-0.7f, -0.3f);
                        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                            horizontalBoost = bounceCharge * 0.8f;
                            chargeDirection = sf::Vector2f(0.7f, -0.3f);
                        } else {
                            chargeDirection = sf::Vector2f(0, -1);
                        }

                        velocity.y = -(jumpForce + bounceCharge * 1.5f);
                        velocity.x = horizontalBoost;

                        isCharging = false;
                        isJumping = true;
                        isGrounded = false;

                        if (audioEnabled) bounceSound.play();

                        bounceCharge = 0.0f;
                    }
                }
            }
        }

        // ==================== MENU SCREEN ====================
        if (!gameStarted && !gameOver) {
            window.clear();

            // Vẽ background
            if (hasBg) {
                window.draw(background);
            } else {
                window.draw(fallbackBg);
            }

            // Draw snow
            for (const auto& snow : snowflakes) {
                window.draw(snow);
            }

            if (hasFont) {
                instructionText.setString(
                    "GIMBALL SKI NOEL - PRO EDITION!\n\n"
                    "Press SPACE to start skiing!\n\n"
                    "CONTROLS:\n"
                    "Left/Right: Move\n"
                    "Up: Normal Jump\n"
                    "S: Duck (upper body only)\n"
                    "Hold DOWN + release: Super Jump\n"
                    "D: Toggle Debug Mode (hitboxes)\n"
                    "R: Restart | ESC: Exit"
                );
                instructionText.setPosition(100, 100);
                instructionText.setCharacterSize(28);
                window.draw(instructionText);
            }

            window.display();
            continue;
        }

        // ==================== GAME OVER SCREEN ====================
        if (gameOver) {
            window.clear();

            if (hasBg) {
                window.draw(background);
            } else {
                window.draw(fallbackBg);
            }

            for (const auto& snow : snowflakes) {
                window.draw(snow);
            }

            if (hasFont) {
                gameOverText.setString(
                    "GAME OVER!\n\n"
                    "Final Score: " + std::to_string((int)score) + "\n\n"
                    "Press R to play again\n"
                    "Press ESC to exit"
                );
                gameOverText.setPosition(350, 250);
                window.draw(gameOverText);
            }

            window.display();
            continue;
        }

        // ==================== GAME UPDATE ====================
        gameTime = gameClock.getElapsedTime().asSeconds();

        // Player horizontal movement
        float moveSpeed = 8.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            playerX -= moveSpeed;
            if (playerX < 50) playerX = 50;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            playerX += moveSpeed;
            if (playerX > 1150) playerX = 1150;
        }

        // Normal jump
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && isGrounded && !isDucking) {
            velocity.y = jumpForce;
            isGrounded = false;
            isJumping = true;
            if (audioEnabled) jumpSound.play();
        }

        // Spring charge
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && isGrounded && !isDucking) {
            if (!isCharging) {
                isCharging = true;
                chargeClock.restart();
            }
            chargeTime = chargeClock.getElapsedTime().asSeconds();
            bounceCharge += 0.8f;
            if (bounceCharge > maxBounceCharge) bounceCharge = maxBounceCharge;

            float chargeRatio = bounceCharge / maxBounceCharge;
            playerRotation = sin(chargeTime * 10.0f) * 10.0f * chargeRatio;
        }

        // Duck handling
        if (isDucking) {
            duckTime += 1.0f / 60.0f;
            if (duckTime > maxDuckTime) {
                isDucking = false;
                duckTime = 0.0f;
            }
            moveSpeed *= 0.7f;
        } else {
            duckTime -= 0.5f / 60.0f;
            if (duckTime < 0) duckTime = 0;
        }

        // Physics
        velocity.y += gravity;
        if (!isGrounded) {
            velocity.x *= 0.98f;
        }

        playerX += velocity.x;
        playerY += velocity.y;

        // Giới hạn màn hình
        if (playerX < 50) {
            playerX = 50;
            velocity.x = 0;
        }
        if (playerX > 1150) {
            playerX = 1150;
            velocity.x = 0;
        }

        // Ground collision
        float groundLevel = 550.0f;
        if (playerY > groundLevel) {
            playerY = groundLevel;
            velocity.y = 0;
            velocity.x = 0;
            isGrounded = true;
            isJumping = false;
            playerRotation = 0.0f;
        }

        // Update player hitbox
        playerHitbox.update(playerX, playerY, playerWidth, playerHeight, isDucking);

        // Update gift powerup
        if (hasGiftPowerup) {
            giftPowerupDuration--;
            if (giftPowerupDuration <= 0) {
                hasGiftPowerup = false;
            }
        }

        // Score and difficulty
        score += worldSpeed * 0.2f;
        worldSpeed += 0.0003f;
        if (worldSpeed > 3.0f) worldSpeed = 3.0f;

        // Spawn obstacles - CHỈ khi chưa đạt giới hạn
        if (obstacles.size() < MAX_OBSTACLES_ON_SCREEN) {
            obstacleSpawnTimer += 1.0f / 60.0f;
            if (obstacleSpawnTimer >= obstacleSpawnInterval) {
                obstacleSpawnTimer = 0.0f;
                obstacleSpawnInterval = 1.0f + (rand() % 100) / 100.0f;

                Obstacle newObstacle;

                // Chọn loại chướng ngại vật
                int randType = rand() % 100;
                sf::Texture* chosenTex = nullptr;
                float scale = 0.5f;

                if (randType < 20) {
                    newObstacle.type = 3; // Gift
                    chosenTex = &giftTex;
                    scale = 0.25f + (rand() % 20) / 100.0f;
                    newObstacle.isDangerous = false;
                    newObstacle.movement = static_cast<ObstacleMovement>(rand() % 3);
                } else if (randType < 40) {
                    newObstacle.type = 0; // Tree
                    chosenTex = &treeTex;
                    scale = 0.35f + (rand() % 25) / 100.0f;
                    newObstacle.isLowObstacle = false;
                    newObstacle.movement = MOV_STATIC;
                } else if (randType < 60) {
                    newObstacle.type = 1; // Snowball
                    chosenTex = &snowballTex;
                    scale = 0.25f + (rand() % 25) / 100.0f;
                    newObstacle.isLowObstacle = true;
                    newObstacle.movement = static_cast<ObstacleMovement>(rand() % 5);
                } else {
                    newObstacle.type = 2; // Snowman
                    chosenTex = &snowmanTex;
                    scale = 0.4f + (rand() % 30) / 100.0f;
                    newObstacle.isLowObstacle = false;
                    newObstacle.movement = static_cast<ObstacleMovement>(rand() % 4);
                }

                newObstacle.sprite.setTexture(*chosenTex);
                newObstacle.x = 1200 + 50 + (rand() % 200);

                // Vị trí Y
                float minY = 150;
                float maxY = groundLevel - chosenTex->getSize().y * scale - 50;

                if (newObstacle.type == 3) { // Gift - bay trên cao
                    newObstacle.y = minY + (rand() % 200);
                } else if (newObstacle.isLowObstacle) { // Snowball - thấp
                    newObstacle.y = maxY - 50 + (rand() % 100);
                } else { // Tree/Snowman - trung bình
                    newObstacle.y = minY + 100 + (rand() % (int)(maxY - minY - 100));
                }

                newObstacle.originalY = newObstacle.y;
                newObstacle.speedX = worldSpeed * (1.8f + (rand() % 50) / 100.0f);
                newObstacle.sprite.setScale(scale, scale);
                newObstacle.width = chosenTex->getSize().x * scale;
                newObstacle.height = chosenTex->getSize().y * scale;

                // Hitbox height
                if (newObstacle.type == 1) { // Snowball
                    newObstacle.hitboxHeight = newObstacle.height * 0.8f;
                } else if (newObstacle.type == 0) { // Tree
                    newObstacle.hitboxHeight = newObstacle.height * 0.7f;
                } else {
                    newObstacle.hitboxHeight = newObstacle.height * 0.9f;
                }

                // Cài đặt tham số chuyển động
                newObstacle.movementParam1 = 1.0f + (rand() % 100) / 100.0f;
                newObstacle.movementParam2 = 20.0f + (rand() % 60);

                obstacles.push_back(newObstacle);
            }
        }

        // Update obstacles và check collision
        float dt = 1.0f / 60.0f;
        for (auto it = obstacles.begin(); it != obstacles.end(); ) {
            it->update(dt, playerY);

            if (!it->activeCollision && it->x < 1200) {
                it->activeCollision = true;
            }

            // SMART COLLISION DETECTION
            if (it->activeCollision) {
                sf::FloatRect obsHitbox = it->getHitbox();

                // Kiểm tra va chạm với từng phần hitbox của player
                bool headHit = playerHitbox.head.intersects(obsHitbox);
                bool bodyHit = playerHitbox.body.intersects(obsHitbox);
                bool legsHit = playerHitbox.legs.intersects(obsHitbox);

                bool anyCollision = headHit || bodyHit || legsHit;

                if (anyCollision) {
                    if (it->type == 3) { // Gift
                        score += 100;
                        hitsWithoutGift = 0;
                        hasGiftPowerup = true;
                        giftPowerupDuration = maxGiftPowerupDuration;

                        if (audioEnabled) giftSound.play();
                        std::cout << "Collected GIFT! +100 points" << std::endl;
                        it = obstacles.erase(it);
                        continue;
                    }
                    else if (!hasGiftPowerup) { // Obstacle
                        // KIỂM TRA NÉ BẰNG CÚI
                        bool canDuckThrough = isDucking && legsHit;

                        if (!canDuckThrough) {
                            hitsWithoutGift++;

                            if (audioEnabled) hitSound.play();

                            std::cout << "Hit " << it->getTypeName() << "! Hits: "
                                      << hitsWithoutGift << "/" << maxHitsWithoutGift << std::endl;

                            // Hiệu ứng bật lại khi bị hit
                            velocity.x = -10.0f;
                            velocity.y = -5.0f;

                            if (hitsWithoutGift >= maxHitsWithoutGift) {
                                lives--;
                                hitsWithoutGift = 0;
                                std::cout << "Lost a life! Lives: " << lives << std::endl;

                                if (lives <= 0) {
                                    gameOver = true;
                                    std::cout << "GAME OVER! Final score: " << score << std::endl;
                                }
                            }
                        } else {
                            std::cout << "Ducked under " << it->getTypeName() << " successfully!" << std::endl;
                            score += 15;
                        }
                    }
                }

                // Kiểm tra đã vượt qua chưa
                if (it->x + it->width < playerX && !it->passed) {
                    it->passed = true;
                    if (it->type != 3) {
                        score += 20;
                    }
                }
            }

            if (it->x < -200) {
                it = obstacles.erase(it);
            } else {
                ++it;
            }
        }

        // Update player appearance
        if (isCharging) {
            float chargeRatio = bounceCharge / maxBounceCharge;
            float springScaleY = 0.7f + chargeRatio * 0.5f;
            player.setScale(playerScale * (1.0f - chargeRatio * 0.2f), playerScale * springScaleY);
            player.setOrigin(playerTex.getSize().x / 2, playerTex.getSize().y * 0.6f);
        }
        else if (isDucking) {
            float duckRatio = duckTime / maxDuckTime;
            float duckScaleY = 0.6f - duckRatio * 0.1f;
            float duckScaleX = 1.0f + duckRatio * 0.1f;
            player.setScale(playerScale * duckScaleX, playerScale * duckScaleY);
            player.setOrigin(playerTex.getSize().x / 2, playerTex.getSize().y * 0.8f);
            playerRotation = 0.0f;
        }
        else {
            player.setScale(playerScale, playerScale);
            player.setOrigin(playerTex.getSize().x / 2, playerTex.getSize().y / 2);
        }

        player.setRotation(playerRotation);
        player.setPosition(playerX, playerY);

        // Update snow effect
        for (auto& snow : snowflakes) {
            snow.move(-0.5f + (rand() % 100) / 100.0f, 1.5f + (rand() % 100) / 100.0f);

            if (snow.getPosition().x < -10) snow.setPosition(1210, rand() % 800);
            if (snow.getPosition().y > 800) snow.setPosition(snow.getPosition().x, -10);
        }

        // ==================== RENDERING ====================
        window.clear();

        // Vẽ BACKGROUND
        if (hasBg) {
            window.draw(background);
        } else {
            window.draw(fallbackBg);
        }

        // Draw snow
        for (const auto& snow : snowflakes) {
            window.draw(snow);
        }

        // Draw ground
        window.draw(ground);
        window.draw(groundLine);

        // Draw obstacles
        for (auto& obstacle : obstacles) {
            obstacle.sprite.setPosition(obstacle.x, obstacle.y);
            window.draw(obstacle.sprite);

            if (debugMode) {
                obstacle.drawDebug(window);
            }
        }

        // Draw player với hiệu ứng powerup
        if (hasGiftPowerup && (giftPowerupDuration / 10) % 2 == 0) {
            player.setColor(sf::Color(255, 255, 150));
        } else if (isDucking) {
            player.setColor(sf::Color(100, 200, 255));
        } else {
            player.setColor(sf::Color::White);
        }

        window.draw(player);

        // Debug: vẽ player hitbox
        if (debugMode) {
            playerHitbox.drawDebug(window);
        }

        // Draw UI
        if (hasFont) {
            // Score display
            std::ostringstream scoreStream;
            scoreStream << "Score: " << (int)score << " | Speed: x" << std::fixed << std::setprecision(1) << worldSpeed;
            scoreText.setString(scoreStream.str());
            scoreText.setPosition(20, 20);
            window.draw(scoreText);

            // Lives display
            std::ostringstream livesStream;
            livesStream << "Lives: " << lives;
            livesText.setString(livesStream.str());
            livesText.setFillColor(lives > 1 ? sf::Color::Green : sf::Color::Red);
            livesText.setPosition(20, 60);
            window.draw(livesText);

            // Hits display
            std::ostringstream hitsStream;
            hitsStream << "Hits: " << hitsWithoutGift << "/" << maxHitsWithoutGift;
            hitsText.setString(hitsStream.str());
            hitsText.setFillColor(hitsWithoutGift >= 2 ? sf::Color::Red :
                                 hitsWithoutGift >= 1 ? sf::Color::Yellow : sf::Color::Green);
            hitsText.setPosition(20, 100);
            window.draw(hitsText);

            // Duck status
            std::ostringstream duckStream;
            if (isDucking) {
                duckStream << "DUCKING: " << std::fixed << std::setprecision(1)
                          << (maxDuckTime - duckTime) << "s";
                duckText.setFillColor(sf::Color(255, 150, 50));
            } else if (duckTime > 0) {
                duckStream << "Duck Recharge: " << std::fixed << std::setprecision(1)
                          << (duckTime * 2) << "s";
                duckText.setFillColor(sf::Color(150, 150, 255));
            } else {
                duckStream << "Duck: READY (Press S)";
                duckText.setFillColor(sf::Color(255, 200, 100));
            }
            duckText.setString(duckStream.str());
            duckText.setPosition(20, 140);
            window.draw(duckText);

            // Debug hitbox info
            if (debugMode) {
                sf::Text hitboxText("", font, 18);
                hitboxText.setFillColor(sf::Color::Cyan);
                hitboxText.setPosition(20, 180);

                if (isDucking) {
                    hitboxText.setString("HITBOX: LEGS ONLY");
                } else {
                    hitboxText.setString("HITBOX: FULL BODY");
                }
                window.draw(hitboxText);
            }

            // Gift powerup indicator
            if (hasGiftPowerup) {
                std::ostringstream powerupStream;
                powerupStream << "INVINCIBLE: " << (giftPowerupDuration / 60) << "s";
                powerupText.setString(powerupStream.str());
                powerupText.setPosition(20, 170);
                window.draw(powerupText);
            }

            // Charge indicator
            if (isCharging) {
                std::ostringstream chargeStream;
                chargeStream << "SPRING CHARGE: " << (int)((bounceCharge / maxBounceCharge) * 100) << "%";
                chargeText.setString(chargeStream.str());

                float chargeTextY = playerY - playerHeight - 40;
                if (chargeTextY < 20) chargeTextY = 20;
                chargeText.setPosition(playerX - 80, chargeTextY);

                // Charge bar
                sf::RectangleShape chargeBarBg(sf::Vector2f(160, 12));
                chargeBarBg.setPosition(playerX - 80, chargeTextY + 25);
                chargeBarBg.setFillColor(sf::Color(50, 50, 50, 200));
                chargeBarBg.setOutlineThickness(1);
                chargeBarBg.setOutlineColor(sf::Color::White);

                sf::RectangleShape chargeBar(sf::Vector2f((bounceCharge / maxBounceCharge) * 156, 8));
                chargeBar.setPosition(playerX - 78, chargeTextY + 27);
                chargeBar.setFillColor(sf::Color(255, 100, 50));

                window.draw(chargeBarBg);
                window.draw(chargeBar);
                window.draw(chargeText);
            }

            // Instruction reminder
            instructionText.setString("Avoid obstacles | Collect Gifts | Jump(UP) | Duck(S) | Charge(DOWN+HOLD)");
            instructionText.setCharacterSize(20);
            instructionText.setPosition(20, 770);
            window.draw(instructionText);

            // Debug info
            if (debugMode) {
                std::ostringstream debugStream;
                debugStream << "DEBUG MODE | Obstacles: " << obstacles.size()
                           << " | FPS: 60 | Player: (" << (int)playerX << "," << (int)playerY << ")";
                debugText.setString(debugStream.str());
                debugText.setPosition(400, 20);
                debugText.setFillColor(sf::Color::Yellow);
                window.draw(debugText);
            }
        }

        window.display();
    }

    return 0;
}
