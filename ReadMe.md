# Car Steering Game

![Car Steering Game Banner](assets/images/banner.png)

Welcome to the **Car Steering Game**! This is a fun and engaging game where you control a car using gyroscope data from a connected sensor device. Dodge obstacles, rack up points, and strive to achieve the highest score!

## Table of Contents

- [Features](#features)
- [Demo](#demo)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
  - [Running the Game](#running-the-game)
  - [Controls](#controls)
- [Connecting to the Sensor Server](#connecting-to-the-sensor-server)
- [Game Mechanics](#game-mechanics)
- [Assets](#assets)
- [High Score](#high-score)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgements](#acknowledgements)

## Features

- **Responsive Controls:** Utilize gyroscope data to steer the car smoothly.
- **Dynamic Obstacles:** Experience varying obstacle types and patterns.
- **High Score Tracking:** Keep track of your best performance across sessions.
- **Calibrating Phase:** Ensures accurate sensor data before gameplay starts.
- **Full-Screen Mode:** Immerse yourself in the game with a full-screen experience.
- **Customizable Settings:** Easily adjust game parameters and assets.

## Demo

![Gameplay Screenshot](assets/images/Screenshot.png)

*Figure: Screenshot of the Car Steering Game in action.*

## Requirements

- **Operating System:** Windows, macOS, or Linux
- **Compiler:** GCC, Clang, or MSVC with C++11 support
- **Libraries:**
  - [SFML 2.5+](https://www.sfml-dev.org/)
- **Assets:**
  - Car and obstacle images located in `assets/images/`
  - Font file `arial.ttf` located in the root directory or specified assets folder.

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/yourusername/car-steering-game.git
cd car-steering-game
```

### 2. Install SFML

#### On Windows:

Download the SFML SDK from the [official website](https://www.sfml-dev.org/download.php). Follow the installation instructions provided in the [SFML tutorials](https://www.sfml-dev.org/tutorials/2.5/).

#### On macOS using Homebrew:

```bash
brew install sfml
```

#### On Linux (Ubuntu/Debian):

```bash
sudo apt-get update
sudo apt-get install libsfml-dev
```

### 3. Compile the Game

Use your preferred C++ compiler. Below are example commands for different platforms.

#### Using g++:

```bash
g++ main.cpp -o car_game -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network
```

#### Using CMake:

If you prefer using CMake, create a `CMakeLists.txt` with the following content:

```cmake
cmake_minimum_required(VERSION 3.5)
project(CarSteeringGame)

set(CMAKE_CXX_STANDARD 11)

find_package(SFML 2.5 COMPONENTS graphics window system network REQUIRED)

add_executable(car_game main.cpp)

target_link_libraries(car_game sfml-graphics sfml-window sfml-system sfml-network)
```

Then build:

```bash
mkdir build
cd build
cmake ..
make
```

### 4. Prepare Assets

Ensure that the `assets/images/` directory contains the following image files:

- `car.png`
- `obstacle1.png`
- `obstacle2.png`
- `obstacle3.png`

Also, ensure that the `arial.ttf` font file is present in the project root or the specified assets folder.

## Usage

### Running the Game

Execute the compiled binary with the required command-line arguments: the sensor server's IP address and port number.

```bash
./car_game <Sensor_Server_IP> <Port>
```

**Example:**

```bash
./car_game 192.168.1.100 55001
```

### Controls

The game utilizes gyroscope data from a connected sensor device to control the car. Ensure your sensor device is properly calibrated and connected to provide real-time gyroscopic input.

**Note:** There are no manual controls; movement is based entirely on the sensor data.

## Connecting to the Sensor Server

The game requires a sensor server that streams gyroscope data over TCP. Ensure that:

1. The sensor server is running and accessible over the network.
2. You have the correct IP address and port number.
3. The sensor server sends data in the expected format, with gyroscope Y and Z values at indices 26 and 27, respectively.

**Example Sensor Data Format:**

```
timestamp, ..., gyroY, gyroZ, ...
```

Ensure that each data line ends with a newline character (`\n`).

## Game Mechanics

1. **Main Menu:**
   - **Start Button:** Begins calibration and transitions to gameplay.
   - **Quit Button:** Exits the game.

2. **Calibrating:**
   - A calibration phase ensures that the sensor data offsets are accurately measured.
   - Keep your device steady during calibration.

3. **Playing:**
   - Use your device's gyroscope to steer the car left and right.
   - Avoid incoming obstacles to survive longer and score higher.

4. **Game Over:**
   - Upon collision, the game ends.
   - Displays your final score and the high score.
   - Options to retry or quit.

## Assets

All visual assets are located in the `assets/images/` directory:

- **Car Image:** `car.png`
- **Obstacle Images:** `obstacle1.png`, `obstacle2.png`, `obstacle3.png`
- **Banner and Screenshots:** For promotional purposes.

Ensure these files are present for the game to run correctly. You can replace these with your own images, but maintain the same file names or update the code accordingly.

## High Score

The game tracks the highest score achieved and saves it to a `highscore.txt` file in the project directory. If the file doesn't exist, it will be created upon achieving a high score.

**Note:** Ensure the game has write permissions to the directory to save the high score successfully.

## Troubleshooting

- **Failed to Connect to Sensor Server:**
  - Ensure the server IP and port are correct.
  - Check network connectivity.
  - Verify that the sensor server is running.

- **Missing Assets:**
  - Ensure all required image files and the font file are in the correct directories.

- **Compilation Errors:**
  - Verify that SFML is correctly installed and linked.
  - Ensure your compiler supports C++11.

- **High Score Not Saving:**
  - Check if the game has write permissions to the project directory.

## Contributing

Contributions are welcome! If you have suggestions, bug fixes, or enhancements, feel free to open an issue or submit a pull request.

1. Fork the repository.
2. Create a new branch: `git checkout -b feature/YourFeature`
3. Commit your changes: `git commit -m "Add your message"`
4. Push to the branch: `git push origin feature/YourFeature`
5. Open a pull request.

## License

This project is licensed under the [MIT License](LICENSE).

## Acknowledgements

- **SFML:** The Simple and Fast Multimedia Library for handling graphics, windowing, and networking.
- **OpenAI:** For providing guidance and support in developing this README.
- **Contributors:** Thanks to all the contributors who help improve the game.

---

*Enjoy steering your way to victory! 🚗💨*