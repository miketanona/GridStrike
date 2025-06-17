# GridStrike

GridStrike is a Visual C++ application that demonstrates a pathfinding system for real-time strategy (RTS) battle units. It runs simulations using multiple search strategies and visualizes the results in a tile-based grid alongside a companion performance chart that shows number of steps vs. Manhattan distance.

Both the grid and chart are rendered using OpenGL for efficient, real-time visualization. This project was developed as part of a technical evaluation and is designed to be self-contained and buildable out of the box.

---

## 🛠 Build Instructions

GridStrike was developed using **Microsoft Visual Studio 2022** with the **MSVC v143 toolset (compiler version 19.44.x)**.

### Requirements
- Windows 10 or later
- Visual Studio 2022 with the **Desktop Development with C++** workload
- No external libraries or downloads needed

### Build Steps
1. Clone this repository:
   ```bash
   git clone https://github.com/miketanona/GridStrike.git
   ```
2. Open `GridStrike.sln` in Visual Studio.
3. Set your preferred build configuration (**Debug** or **Release**).
4. Build the solution (**Ctrl+Shift+B**).
5. Run from within Visual Studio or execute the built binary.


