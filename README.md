# Space_Game
# Space Galaxy Fighter - B&W 🚀

A classic retro-style, black-and-white 2D space shooter built with C++ and OpenGL (GLUT). Navigate through space, dodge meteors, destroy enemy ships, and avoid the intense gravitational pull of the black hole!

## ✨ Features
* **Classic Arcade Gameplay:** Survive endless waves of enemies and meteors.
* **Dynamic Environment:** A fully functioning black hole that actively pulls the player, enemies, and bullets into its center.
* **Combat Systems:** Manual fire and toggleable Auto-Fire mode (`F`), complete with explosive visual effects.
* **Audio Integration:** Triggered sound effects for enemy takedowns using the Windows Multimedia API (`winmm`).

## 🎮 Controls
* **W / S** - Move Forward / Backward
* **A / D** - Rotate Ship Left / Right
* **Arrow Keys** - Strafe Movement
* **SPACEBAR** - Fire Weapon
* **F** - Toggle Auto-Fire
* **R** - Restart (on Game Over)
* **ESC** - Exit Game

## 🛠️ Setup & Compilation (Code::Blocks)

This project is configured specifically for **Code::Blocks** using the MinGW compiler on Windows.

### Prerequisites
1. **Code::Blocks** installed (with MinGW).
2. **GLUT / FreeGLUT** configured within your Code::Blocks environment.
3. A valid `sound.wav` file and `glut32.dll`.

### Build Instructions
1. Open your project in Code::Blocks.
2. Go to **Project** -> **Build options...**
3. Click on the **name of your project** at the top of the left-hand list (not Debug/Release).
4. Navigate to the **Linker settings** tab.
5. In the **Other linker options** box (on the right), paste the following:
   -lwinmm
   -lopengl32
   -lglu32
   -lglut32
6. Click **OK**.
7. Change your build target from "Debug" to **"Release"** in the top toolbar, then click the **Build** icon.

### Running the Executable
If you want to run the game outside of Code::Blocks (or share it with others):
1. Navigate to your project folder and open `bin\\Release`.
2. Ensure that your compiled **`.exe`**, **`sound.wav`**, and **`glut32.dll`** are all placed together inside this exact folder.
3. Double-click the `.exe` to play!
