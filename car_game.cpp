#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <algorithm>

using namespace std;
using namespace sf;

enum class GameState
{
    MainMenu,
    Calibrating,
    Playing,
    GameOver
};

const int GYRO_Y_INDEX = 26;
const int GYRO_Z_INDEX = 27;

vector<string> split(const string &s, char delimiter)
{
    vector<string> tokens;
    stringstream ss(s);
    string token;
    while (getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

class Button
{
public:
    Button(const Font &font, const string &textString, unsigned int characterSize,
           Color textColor, Color buttonColor, Vector2f size, Vector2f position)
    {

        buttonShape.setSize(size);
        buttonShape.setFillColor(buttonColor);
        buttonShape.setPosition(position);

        text.setFont(font);
        text.setString(textString);
        text.setCharacterSize(characterSize);
        text.setFillColor(textColor);

        FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width / 2.0f,
                       textRect.top + textRect.height / 2.0f);
        text.setPosition(
            position.x + size.x / 2.0f,
            position.y + size.y / 2.0f);
    }

    void draw(RenderWindow &window) const
    {
        window.draw(buttonShape);
        window.draw(text);
    }

    bool isClicked(Vector2f mousePos) const
    {
        return buttonShape.getGlobalBounds().contains(mousePos);
    }

private:
    RectangleShape buttonShape;
    Text text;
};

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "Usage: ./car_game <Sensor_Server_IP> <Port>" << endl;
        return 1;
    }

    string server_ip = argv[1];
    int port = stoi(argv[2]);

    TcpSocket socket;
    if (socket.connect(server_ip, port) != Socket::Done)
    {
        cout << "Connection to sensor server failed." << endl;
        return 1;
    }

    cout << "Connected to Sensor server at " << server_ip << ":" << port << endl;

    socket.setBlocking(false);

    VideoMode desktop = VideoMode::getDesktopMode();
    Vector2u screenSize(desktop.width, desktop.height);

    RenderWindow window(desktop, "Car Steering Game", Style::Fullscreen);
    window.setFramerateLimit(60);

    float roadWidth = screenSize.x * 0.5f;
    float roadHeight = static_cast<float>(screenSize.y);
    roadHeight*=2;
    RectangleShape road(Vector2f(roadWidth, roadHeight));
    road.setFillColor(Color(50, 50, 50));
    road.setPosition((screenSize.x - roadWidth) / 2.0f, 0.0f);

    float laneMarkWidth = roadWidth * 0.025f;
    float laneMarkHeight = screenSize.y * 0.083f;
    float laneMarkSpacing = screenSize.y * 0.166f;
    vector<RectangleShape> laneMarks;
    float laneMarkX = road.getPosition().x + roadWidth / 2.0f - laneMarkWidth / 2.0f;

    for (float y = 0; y < screenSize.y; y += laneMarkHeight + laneMarkSpacing)
    {
        RectangleShape mark(Vector2f(laneMarkWidth, laneMarkHeight));
        mark.setFillColor(Color::White);
        mark.setPosition(laneMarkX, y);
        laneMarks.push_back(mark);
    }

    Texture carTexture;
    if (!carTexture.loadFromFile("car.png"))
    {
        cout << "Failed to load car texture." << endl;
        return 1;
    }

    Sprite car(carTexture);

    float carWidth = screenSize.x * 0.0625f;
    float carHeight = screenSize.y * 0.166f;
    car.setScale(carWidth / carTexture.getSize().x, carHeight / carTexture.getSize().y);

    car.setPosition(
        road.getPosition().x + roadWidth / 2.0f - carWidth / 2.0f,
        screenSize.y - carHeight - screenSize.y * 0.05f);

    Texture obstacleTexture;
    if (!obstacleTexture.loadFromFile("obstacle1.png"))
    {
        cout << "Failed to load obstacle texture." << endl;
        return 1;
    }

    float obstacleWidth = screenSize.x * 0.0625f;
    float obstacleHeight = screenSize.y * 0.133f;
    obstacleTexture.setSmooth(true);

    vector<Sprite> obstacles;
    srand(static_cast<unsigned int>(time(0)));

    Clock obstacleClock;
    float obstacleSpawnTime = 2.0f;

    Clock gameClock;
    bool gameOver = false;

    double gyroY = 0.0;
    double gyroZ = 0.0;

    string residual_data = "";

    const float calibrationDuration = 2.0f;
    Clock calibrationClock;
    double gyroYOffset = 0.0;
    double gyroZOffset = 0.0;
    int calibrationSamples = 0;
    const int maxCalibrationSamples = 100;

    deque<double> gyroYBuffer;
    deque<double> gyroZBuffer;
    const size_t bufferSize = 10;

    const float movementScalingFactor = 200.0f;
    const float zoomSpeed = 0.1f;

    float currentZoom = 1.0f;

    bool isCalibrating = true;

    GameState currentState = GameState::MainMenu;

    Font font;
    if (!font.loadFromFile("arial.ttf"))
    {
        cout << "Failed to load font." << endl;
        return 1;
    }

    float buttonWidth = screenSize.x * 0.2f;
    float buttonHeight = screenSize.y * 0.1f;
    float buttonSpacing = screenSize.y * 0.05f;

    Button startButton(font, "Start", static_cast<unsigned int>(screenSize.y * 0.05f),
                       Color::White, Color::Blue,
                       Vector2f(buttonWidth, buttonHeight),
                       Vector2f((screenSize.x - buttonWidth) / 2.0f,
                                screenSize.y * 0.4f));

    Button quitButton(font, "Quit", static_cast<unsigned int>(screenSize.y * 0.05f),
                      Color::White, Color::Red,
                      Vector2f(buttonWidth, buttonHeight),
                      Vector2f((screenSize.x - buttonWidth) / 2.0f,
                               screenSize.y * 0.4f + buttonHeight + buttonSpacing));

    Button retryButton(font, "Retry", static_cast<unsigned int>(screenSize.y * 0.05f),
                       Color::White, Color::Green,
                       Vector2f(buttonWidth, buttonHeight),
                       Vector2f((screenSize.x - buttonWidth) / 2.0f,
                                screenSize.y * 0.4f));

    Button gameOverQuitButton(font, "Quit", static_cast<unsigned int>(screenSize.y * 0.05f),
                              Color::White, Color::Red,
                              Vector2f(buttonWidth, buttonHeight),
                              Vector2f((screenSize.x - buttonWidth) / 2.0f,
                                       screenSize.y * 0.4f + buttonHeight + buttonSpacing));

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
            {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
                if (currentState == GameState::MainMenu)
                {
                    if (startButton.isClicked(mousePos))
                    {
                        currentState = GameState::Calibrating;

                        isCalibrating = true;
                        calibrationClock.restart();
                        gyroYOffset = 0.0;
                        gyroZOffset = 0.0;
                        calibrationSamples = 0;
                        gyroYBuffer.clear();
                        gyroZBuffer.clear();

                        obstacles.clear();
                        gameOver = false;
                        gameClock.restart();
                        obstacleClock.restart();

                        currentZoom = 1.0f;
                        window.setView(window.getDefaultView());
                    }
                    if (quitButton.isClicked(mousePos))
                    {
                        window.close();
                    }
                }
                else if (currentState == GameState::GameOver)
                {
                    if (retryButton.isClicked(mousePos))
                    {
                        currentState = GameState::Calibrating;

                        isCalibrating = true;
                        calibrationClock.restart();
                        gyroYOffset = 0.0;
                        gyroZOffset = 0.0;
                        calibrationSamples = 0;
                        gyroYBuffer.clear();
                        gyroZBuffer.clear();

                        obstacles.clear();
                        gameOver = false;
                        gameClock.restart();
                        obstacleClock.restart();

                        currentZoom = 1.0f;
                        window.setView(window.getDefaultView());
                    }
                    if (gameOverQuitButton.isClicked(mousePos))
                    {
                        window.close();
                    }
                }
            }
        }

        if (currentState == GameState::Calibrating)
        {

            char buffer[4096];
            size_t received;
            Socket::Status status = socket.receive(buffer, sizeof(buffer) - 1, received);
            if (status == Socket::Done)
            {
                buffer[received] = '\0';
                residual_data += string(buffer, received);

                size_t pos;
                while ((pos = residual_data.find('\n')) != string::npos)
                {
                    string line = residual_data.substr(0, pos);
                    residual_data.erase(0, pos + 1);

                    if (line.empty() || line.find("loggingTime") != string::npos)
                    {
                        continue;
                    }

                    vector<string> values = split(line, ',');

                    if (values.size() > GYRO_Z_INDEX)
                    {
                        try
                        {

                            double currentGyroY = -stod(values[GYRO_Y_INDEX]);
                            double currentGyroZ = -stod(values[GYRO_Z_INDEX]);

                            if (isCalibrating)
                            {

                                gyroYOffset += currentGyroY;
                                gyroZOffset += currentGyroZ;
                                calibrationSamples++;
                                if (calibrationClock.getElapsedTime().asSeconds() >= calibrationDuration || calibrationSamples >= maxCalibrationSamples)
                                {
                                    gyroYOffset /= calibrationSamples;
                                    gyroZOffset /= calibrationSamples;
                                    isCalibrating = false;
                                    cout << "Calibration complete. GyroY Offset: " << gyroYOffset << ", GyroZ Offset: " << gyroZOffset << endl;
                                    currentState = GameState::Playing;
                                }
                            }
                        }
                        catch (const invalid_argument &e)
                        {
                            cout << "[ERROR] Invalid gyroscope value: " << e.what() << endl;
                        }
                    }
                    else
                    {
                        cout << "[WARN] Incomplete data line received. Skipping." << endl;
                    }
                }
            }
            else if (status == Socket::Disconnected)
            {
                cout << "Sensor server disconnected." << endl;
                currentState = GameState::MainMenu;
            }

            window.clear();

            window.draw(road);
            for (auto &mark : laneMarks)
                window.draw(mark);

            window.draw(car);

            for (auto &obstacle : obstacles)
                window.draw(obstacle);

            Text calibrationText("Calibrating...\nPlease keep your device steady.", font, static_cast<unsigned int>(screenSize.y * 0.04f));
            calibrationText.setFillColor(Color::Yellow);

            FloatRect textRect = calibrationText.getLocalBounds();
            calibrationText.setOrigin(textRect.left + textRect.width / 2.0f,
                                      textRect.top + textRect.height / 2.0f);
            calibrationText.setPosition(screenSize.x / 2.0f, screenSize.y / 2.0f);
            window.draw(calibrationText);

            window.display();
            continue;
        }
        else if (currentState == GameState::Playing)
        {
            if (!gameOver)
            {

                char buffer[4096];
                size_t received;
                Socket::Status status = socket.receive(buffer, sizeof(buffer) - 1, received);
                if (status == Socket::Done)
                {
                    buffer[received] = '\0';
                    residual_data += string(buffer, received);

                    size_t pos;
                    while ((pos = residual_data.find('\n')) != string::npos)
                    {
                        string line = residual_data.substr(0, pos);
                        residual_data.erase(0, pos + 1);

                        if (line.empty() || line.find("loggingTime") != string::npos)
                        {
                            continue;
                        }

                        vector<string> values = split(line, ',');

                        if (values.size() > GYRO_Z_INDEX)
                        {
                            try
                            {

                                double currentGyroY = -stod(values[GYRO_Y_INDEX]);
                                double currentGyroZ = -stod(values[GYRO_Z_INDEX]);

                                double calibratedGyroY = currentGyroY - gyroYOffset;
                                double calibratedGyroZ = currentGyroZ - gyroZOffset;

                                gyroYBuffer.push_back(calibratedGyroY);
                                if (gyroYBuffer.size() > bufferSize)
                                {
                                    gyroYBuffer.pop_front();
                                }

                                double sumY = 0.0;
                                for (const auto &val : gyroYBuffer)
                                {
                                    sumY += val;
                                }
                                double averageGyroY = sumY / gyroYBuffer.size();

                                averageGyroY = clamp(averageGyroY, -10.0, 10.0);

                                gyroY = averageGyroY;

                                gyroZBuffer.push_back(calibratedGyroZ);
                                if (gyroZBuffer.size() > bufferSize)
                                {
                                    gyroZBuffer.pop_front();
                                }

                                double sumZ = 0.0;
                                for (const auto &val : gyroZBuffer)
                                {
                                    sumZ += val;
                                }
                                double averageGyroZ = sumZ / gyroZBuffer.size();

                                averageGyroZ = clamp(averageGyroZ, -10.0, 10.0);

                                gyroZ = averageGyroZ;
                            }
                            catch (const invalid_argument &e)
                            {
                                cout << "[ERROR] Invalid gyroscope value: " << e.what() << endl;
                            }
                        }
                        else
                        {
                            cout << "[WARN] Incomplete data line received. Skipping." << endl;
                        }
                    }
                }
                else if (status == Socket::Disconnected)
                {
                    cout << "Sensor server disconnected." << endl;
                    currentState = GameState::MainMenu;
                }

                float deltaTime = gameClock.restart().asSeconds();

                float movement = static_cast<float>(gyroZ) * movementScalingFactor * deltaTime;
                car.move(movement, 0.0f);

                float carX = car.getPosition().x;
                if (carX < road.getPosition().x)
                    car.setPosition(road.getPosition().x, car.getPosition().y);
                if (carX > road.getPosition().x + roadWidth - car.getGlobalBounds().width)
                    car.setPosition(road.getPosition().x + roadWidth - car.getGlobalBounds().width, car.getPosition().y);

                currentZoom += static_cast<float>(gyroY) * zoomSpeed * deltaTime;
                currentZoom = clamp(currentZoom, 0.5f, 2.0f);

                View view = window.getView();
                view.setSize(screenSize.x * currentZoom, screenSize.y * currentZoom);
                window.setView(view);

                if (obstacleClock.getElapsedTime().asSeconds() > obstacleSpawnTime)
                {
                    Sprite obstacle(obstacleTexture);

                    float xPos = road.getPosition().x + static_cast<float>(rand()) / RAND_MAX * (roadWidth - obstacleWidth);
                    obstacle.setScale(obstacleWidth / obstacleTexture.getSize().x, obstacleHeight / obstacleTexture.getSize().y);
                    obstacle.setPosition(xPos, -obstacleHeight);
                    obstacles.push_back(obstacle);
                    obstacleClock.restart();
                }

                for (auto &obstacle : obstacles)
                {
                    obstacle.move(0.0f, 300.0f * deltaTime);
                }

                obstacles.erase(remove_if(obstacles.begin(), obstacles.end(), [&](Sprite &o)
                                          { return o.getPosition().y > screenSize.y; }),
                                obstacles.end());

                for (auto &obstacle : obstacles)
                {
                    if (car.getGlobalBounds().intersects(obstacle.getGlobalBounds()))
                    {
                        gameOver = true;
                        currentState = GameState::GameOver;
                        break;
                    }
                }
            }

            window.clear();

            window.draw(road);
            for (auto &mark : laneMarks)
                window.draw(mark);

            window.draw(car);

            for (auto &obstacle : obstacles)
                window.draw(obstacle);

            window.display();
        }
        else if (currentState == GameState::GameOver)
        {

            window.clear();

            window.draw(road);
            for (auto &mark : laneMarks)
                window.draw(mark);

            window.draw(car);

            for (auto &obstacle : obstacles)
                window.draw(obstacle);

            Text gameOverText("Game Over", font, static_cast<unsigned int>(screenSize.y * 0.08f));
            gameOverText.setFillColor(Color::Red);

            FloatRect textRect = gameOverText.getLocalBounds();
            gameOverText.setOrigin(textRect.left + textRect.width / 2.0f,
                                   textRect.top + textRect.height / 2.0f);
            gameOverText.setPosition(screenSize.x / 2.0f, screenSize.y * 0.3f);
            window.draw(gameOverText);

            retryButton.draw(window);
            gameOverQuitButton.draw(window);

            window.display();
        }
        else if (currentState == GameState::MainMenu)
        {

            window.clear();

            window.draw(road);
            for (auto &mark : laneMarks)
                window.draw(mark);

            window.draw(car);

            for (auto &obstacle : obstacles)
                window.draw(obstacle);

            Text title("Car Steering Game", font, static_cast<unsigned int>(screenSize.y * 0.1f));
            title.setFillColor(Color::Cyan);

            FloatRect titleRect = title.getLocalBounds();
            title.setOrigin(titleRect.left + titleRect.width / 2.0f,
                            titleRect.top + titleRect.height / 2.0f);
            title.setPosition(screenSize.x / 2.0f, screenSize.y * 0.2f);
            window.draw(title);

            startButton.draw(window);
            quitButton.draw(window);

            window.display();
        }
    }

    socket.disconnect();
    return 0;
}