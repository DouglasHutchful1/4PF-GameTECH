## 4PF-GameTECH

## Vampire Hood Game

A game called Vampire hood takes place in an environment reminiscent of Minecraft and is based around the unfortunate apocalypse. We created a unique game engine just for this project, which is used to create the game. The game engine can be used to create additional games because it is made to be modular and extendable. OpenGL and C++ are both used in its construction.

## Table of Contents

1. Game Overview
2. Game Screenshots
3. How to run the game
4. Game Engine Overview
5. Contributers


## Overview for the game

Because of a crazed scientist's error while attempting to reverse-engineer a virus, the globe has been overrun by zombies. By mistake, he unleashed a virus that transformed everyone into zombies. The only chance humanity has to survive the zombie apocalypse in this chaotic planet is by doing so. In order to survive the zombie apocalypse, you must be one of the survivors. Several other survivors and you were imprisoned in a village. You are trapped by the zombies and the lava after it erupted from the ground and formed a massive lava wall. The good news is that you discovered a "valeryan steel" sword hidden in your grandparents' room that could slay the undead. If you have the intelligence to stay clear of the lava and eliminate all vampires so you will be able to survive.

## Screenshots
 


 


## How to run the game

The game is built using the custome engine which is written using openGL and C++. It has been tested on Windows and Linux. To run the game, you need to have openGL installed on your system.

To build the game files from the source, you need to have CMake tool installed on your system. You can download CMake from [here](https://cmake.org/download/). After you have CMake installed, you can build the game by following these steps:

1. Clone the repository or download the zip file from the github website
2. Run `cmake .` in the root directory of the repository in the terminal
3. Run ./bin/GAME_APPLICATION.exe -c='config/light-test.jsonc to run the game.
git clone https: //github.com/DouglasHutchful1/4PF-GameTECH.git
cd vampirehood
cmake ./bin/GAME_APPLICATION.exe -c='config/light-test.jsonc



## Game Engine Overview

The game engine is built using openGL and C++ and is cross-platform. It has been tested on Windows and Linux. It is designed to be modular and extensible. It uses the Entity-Component-System (ECS) architecture to model the game entities. 

To build the game engine from source, you could follow the same steps as the ones used to build the game.

To use the game engine to build your own game, you can follow these steps to add your game logic:

1. Create new classes that inherit from the `State` class to model your game states. You can use the `PlayState` class as a template for your game states.
2. Create new classes that inherit from the `Component` class to model your game components. You can use the `TransformComponent` class as a template for your game components.
3. Create new classes that inherit from the `System` class to model your game systems. You can use the `RenderSystem` class as a template for your game systems.
4. Create your own shaders and textures to use in your game. You can use the `shaders/vertex.glsl` and `shaders/fragment.glsl` files as a template for your game shaders. You can use the `textures/texture.png` file as a template for your game textures.
5. Create you json config file to configure your game. You can use the `config/light-test.jsonc` file as a template for your game config file.


## Contributers

- Carson Nii Sowah Quaye (10869280)
- Douglas Hutchful (10910520)




	
