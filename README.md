# Geometry Wars  

A Geometry Wars-inspired arcade shooter built in C++ using SFML and ImGui, developed as part of the COMP 4300 Game Programming course (available online) at Memorial University.

<img width="958" height="526" alt="github_screenshot" src="https://github.com/user-attachments/assets/1ffac8e8-bd9f-4e5f-9d5c-fa9f926807d0" />

---

## **Overview**

This project is a simple game inspired by the classic *Geometry Wars* game.  The game challenges players to control a geometric shape in order to dodge and destroy waves of enemy polygons. All graphics are rendered using SFML's primitive shapes, with no textures or external assets, emphasizing fundamental game programming and engine architecture principles.


<video src="https://github.com/user-attachments/assets/ecd98c6c-4a0f-4ad1-ba53-8d1c663ae87c" width="200" controls></video>


---

## **Gameplay Features**

- **Player Control:**  
  - Control a rotating geometric shape using WASD keys.
  - Movement is confined within the game window boundaries.
  - Shoot bullets using the left-click button on the mouse.
  - The game can be paused with the P key, and closed with the ESC key.

- **Enemies:**  
  - Various polygonal enemies spawn at random intervals and locations.
  - Enemies bounce off screen edges and are destroyed by player bullets.
  - Destroying normal-sized enemies split them into smaller enemies (e.g., triangles split into smaller triangles) that have a lifespan.

- **Scoring:**  
  - Points are awarded based on the number of sides of the destroyed enemy. Smaller enemies are worth more points.
  - The score can be seen in the left corner. The player's highest score is shown in the top right corner (preserved even after exit)
 
- **GUI:**  
  - All of the systems in the game can be toggled through the GUI
  - All of the entities that are currently active can be viewed/removed in the GUI.

- **Configuration Driven:**  
  - Game parameters (player/enemy speed, bullet size/speed/lifespan, spawn rates, colors, etc.) are defined in an external configuration file, enabling easy tuning without refactoring.

---

## **C++ Tools & Skills Used/Learned**

- **C++ Fundamentals:**  
  - Extensive use of classes, functions, and modular code organization.
  - Memory management, use of smart pointers, and object lifetime considerations.
  - Event-driven programming for input and game state changes.
 
- **SFML (Simple and Fast Multimedia Library):**  
  - Used for window management, rendering primitive shapes, input handling, and timing.

- **ImGui:**  
  - Integrated a GUI for debugging and real-time parameter adjustment.

- **Entity-Component-System (ECS) Architecture:**  
  - The game is structured using ECS, separating data (entities + components) from logic (systems).

- **Data-Oriented Design:**  
  - All tunable parameters are externalized in a config file, promoting easy configuration/testing.

---

## **Areas For Improvement**
- **`std::string` was used in places where an `enum` would suffice**
- **Using a JSON/YAML format for the config file would be a lot better and more clearer instead of a .txt**
- **the enemies can spawn right on top of the player**
- **code can be written a bit cleaner**
  

