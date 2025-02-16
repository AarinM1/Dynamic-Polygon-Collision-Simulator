# Dynamic Polygon Collision Simulator

# Rotating 2D Shape & Bouncing Ball

This project demonstrates a rotating 2D shape with a bouncing ball affected by gravity and friction. The code uses SFML 2.5 for rendering.

## Prerequisites

- **SFML 2.5**  
  Install SFML via Homebrew (on macOS):

  ```bash
  brew install sfml@2
  brew link --force --overwrite sfml@2


# Compiling the code
g++ -std=c++17 \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib \
    -lsfml-graphics -lsfml-window -lsfml-system \
    bouncing_ball.cpp -o bouncing_ball

# Running the executable
./bouncing_ball

