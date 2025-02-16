#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

// Constants
const float PI = 3.14159265f;
const float ROTATION_SPEED = 30.f; // degrees per second

// Gravity and friction constants:
const float GRAVITY = 0.f;         // pixels per second squared (downward)
const float FRICTION_COEFFICIENT = 0.f; // fraction of velocity lost per second

//------------------------------------------------------------
// Utility functions for vector math
//------------------------------------------------------------
inline float dot(const sf::Vector2f &a, const sf::Vector2f &b) {
    return a.x * b.x + a.y * b.y;
}

inline float length(const sf::Vector2f &v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

inline sf::Vector2f normalize(const sf::Vector2f &v) {
    float len = length(v);
    if (len != 0)
        return sf::Vector2f(v.x / len, v.y / len);
    return sf::Vector2f(0.f, 0.f);
}

//------------------------------------------------------------
// Draw a dotted line between two points
//------------------------------------------------------------
void drawDottedLine(sf::RenderWindow &window,
                    const sf::Vector2f &start,
                    const sf::Vector2f &end,
                    float dotSpacing = 10.f,
                    float dotRadius = 2.f)
{
    sf::Vector2f dir = end - start;
    float dist = length(dir);
    if (dist == 0.f)
        return;

    dir = normalize(dir);
    for (float d = 0.f; d < dist; d += dotSpacing) {
        sf::CircleShape dot(dotRadius);
        dot.setFillColor(sf::Color::White);
        dot.setPosition(start + dir * d - sf::Vector2f(dotRadius, dotRadius));
        window.draw(dot);
    }
}

//------------------------------------------------------------
// Check collision of the ball with a line segment defined by points a and b, and reflect velocity
//------------------------------------------------------------
void checkCollisionWithEdge(const sf::Vector2f &a, const sf::Vector2f &b,
                              sf::Vector2f &ballPos, sf::Vector2f &velocity, float ballRadius)
{
    sf::Vector2f edge = b - a;
    // In a convex polygon defined in counterclockwise order, the inward normal is the left-hand normal.
    sf::Vector2f normal(-edge.y, edge.x);
    normal = normalize(normal);

    // Signed distance from ball center to the line
    float dist = dot(ballPos - a, normal);
    if (dist < ballRadius) {
        if (dot(velocity, normal) < 0) { // Ball moving toward the edge
            velocity = velocity - 2.f * dot(velocity, normal) * normal;
            ballPos += (ballRadius - dist) * normal; // Push ball out
        }
    }
}

//------------------------------------------------------------
// Create a regular polygon (ConvexShape) with the given number of sides and radius.
// The polygon is created with its center at (0,0).
//------------------------------------------------------------
sf::ConvexShape createPolygon(int sides, float radius)
{
    sf::ConvexShape polygon;
    polygon.setPointCount(sides);
    for (int i = 0; i < sides; i++) {
        float angle = 2 * PI * i / sides - PI / 2; // start at the top
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle);
        polygon.setPoint(i, sf::Vector2f(x, y));
    }
    polygon.setFillColor(sf::Color::Transparent);
    polygon.setOutlineColor(sf::Color::White);
    polygon.setOutlineThickness(2.f);
    return polygon;
}

//------------------------------------------------------------
// Struct representing a UI tab for shape selection
//------------------------------------------------------------
struct Tab {
    sf::RectangleShape rect;
    sf::Text text;
    int sides; // Number of sides for this shape
};

int main()
{
    // Enable anti-aliasing and vertical sync for smoother rendering
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8; // Increase for even smoother edges if desired
    sf::RenderWindow window(sf::VideoMode(800, 600), "Aim & Bounce", sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    // Load font (update path if needed)
    sf::Font font;
    if (!font.loadFromFile("./Arial.ttf")) {
        std::cerr << "Error: Could not load font from ./Arial.ttf.\n";
    }

    // Setup instructions text (centered at the top)
    sf::Text instructions;
    instructions.setFont(font);
    instructions.setString("Aim with mouse, right-click to launch. Click a tab to change shape.");
    instructions.setCharacterSize(16);
    instructions.setFillColor(sf::Color::White);
    sf::FloatRect instrRect = instructions.getLocalBounds();
    instructions.setOrigin(instrRect.left + instrRect.width / 2.0f, instrRect.top + instrRect.height / 2.0f);
    instructions.setPosition(window.getSize().x / 2.0f, 20.f + instrRect.height / 2.0f);

    // Create tabs for shape selection
    std::vector<std::pair<int, std::string>> shapeOptions = {
        {3, "Triangle"},
        {4, "Square"},
        {5, "Pentagon"},
        {6, "Hexagon"},
        {7, "Heptagon"},
        {8, "Octagon"},
        {9, "Nonagon"},
        {10, "Decagon"}
    };
    std::vector<Tab> tabs;
    float tabWidth = 90.f;
    float tabHeight = 30.f;
    float tabMargin = 9.f;
    float startX = tabMargin;
    float startY = 50.f; // Position tabs below the instructions
    for (auto &option : shapeOptions) {
        Tab tab;
        tab.sides = option.first;
        tab.rect.setSize(sf::Vector2f(tabWidth, tabHeight));
        tab.rect.setFillColor(sf::Color(100, 100, 100));
        tab.rect.setOutlineColor(sf::Color::White);
        tab.rect.setOutlineThickness(1.f);
        tab.rect.setPosition(startX, startY);

        tab.text.setFont(font);
        tab.text.setString(option.second);
        tab.text.setCharacterSize(14);
        tab.text.setFillColor(sf::Color::White);
        sf::FloatRect textRect = tab.text.getLocalBounds();
        tab.text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        tab.text.setPosition(startX + tabWidth / 2.0f, startY + tabHeight / 2.0f);

        tabs.push_back(tab);
        startX += tabWidth + tabMargin;
    }

    // Set up initial boundary shape (default: triangle)
    int currentSides = 3;
    float polygonRadius = 250.f;
    sf::Vector2f center(400.f, 320.f);
    sf::ConvexShape polygon = createPolygon(currentSides, polygonRadius);
    // Position the polygon so that its center is at 'center'
    polygon.setPosition(center);

    // Setup the ball (red circle) at the center
    const float ballRadius = 10.f;
    sf::CircleShape ball(ballRadius);
    ball.setFillColor(sf::Color::Red);
    sf::Vector2f ballPosition = center;
    ball.setPosition(ballPosition - sf::Vector2f(ballRadius, ballRadius));
    sf::Vector2f velocity(0.f, 0.f);
    bool launched = false; // Ball remains stationary until launched

    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // Rotate the polygon continuously
        polygon.rotate(ROTATION_SPEED * dt);

        sf::Event event;
        while (window.pollEvent(event)) {
            // Close window
            if (event.type == sf::Event::Closed)
                window.close();

            // Left-click: Check for tab selection (to change shape)
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
                for (auto &tab : tabs) {
                    if (tab.rect.getGlobalBounds().contains(mousePos)) {
                        currentSides = tab.sides;
                        polygon = createPolygon(currentSides, polygonRadius);
                        polygon.setPosition(center);
                        // Reset the ball when shape changes
                        ballPosition = center;
                        ball.setPosition(ballPosition - sf::Vector2f(ballRadius, ballRadius));
                        velocity = sf::Vector2f(0.f, 0.f);
                        launched = false;
                    }
                }
            }
            // Right-click: Launch the ball if not already launched
            else if (event.type == sf::Event::MouseButtonPressed &&
                     event.mouseButton.button == sf::Mouse::Right && !launched) {
                sf::Vector2i mousePosInt = sf::Mouse::getPosition(window);
                sf::Vector2f mousePos(static_cast<float>(mousePosInt.x), static_cast<float>(mousePosInt.y));
                sf::Vector2f dir = mousePos - ballPosition;
                float dist = length(dir);
                if (dist != 0.f)
                    dir = normalize(dir);
                float speed = 300.f;  // initial launch speed
                velocity = dir * speed;
                launched = true;
            }
        }

        // Update ball position if launched (apply gravity and friction)
        if (launched) {
            // Apply gravity (downward acceleration)
            velocity.y += GRAVITY * dt;
            // Apply friction/damping to gradually slow down the ball
            velocity *= (1.0f - FRICTION_COEFFICIENT * dt);
            // Update ball position using the modified velocity
            ballPosition += velocity * dt;

            // Check collision with each edge of the polygon.
            int count = polygon.getPointCount();
            for (int i = 0; i < count; i++) {
                int next = (i + 1) % count;
                // Transform local points to world coordinates (accounting for rotation & position)
                sf::Vector2f pt1 = polygon.getTransform().transformPoint(polygon.getPoint(i));
                sf::Vector2f pt2 = polygon.getTransform().transformPoint(polygon.getPoint(next));
                checkCollisionWithEdge(pt1, pt2, ballPosition, velocity, ballRadius);
            }
            ball.setPosition(ballPosition - sf::Vector2f(ballRadius, ballRadius));
        }

        window.clear(sf::Color::Black);
        window.draw(polygon);

        // Draw the aiming dotted line if the ball hasn't been launched
        if (!launched) {
            sf::Vector2i mousePosInt = sf::Mouse::getPosition(window);
            sf::Vector2f mousePos(static_cast<float>(mousePosInt.x), static_cast<float>(mousePosInt.y));
            sf::Vector2f diff = mousePos - ballPosition;
            float dist = length(diff);
            sf::Vector2f dir(0.f, 0.f);
            if (dist > 0.f)
                dir = diff / dist;
            float lineLength = std::min(dist, 100.f);
            sf::Vector2f endPos = ballPosition + dir * lineLength;
            drawDottedLine(window, ballPosition, endPos, 10.f, 2.f);
        }

        window.draw(ball);
        window.draw(instructions);
        for (auto &tab : tabs) {
            window.draw(tab.rect);
            window.draw(tab.text);
        }
        window.display();
    }

    return 0;
}
