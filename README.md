[🇬🇧 English](README.md) | [🇨🇳 简体中文](README.zh-CN.md)

# WFCNA (Advanced Wave Function Collapse) Plugin

> An advanced, production-ready Unreal Engine plugin for procedural generation using the [Wave Function Collapse](https://github.com/mxgmn/WaveFunctionCollapse) algorithm.

Built upon the foundational concepts of [bohdon's WFCPlugin](https://github.com/bohdon/WFCPlugin), this project has been heavily extended and refactored for **industrial-scale game development**. It features true asynchronous generation, hexagonal prism grids, nested large-world generation, high-performance object pooling, and multiplayer network synchronization.

---

## 🌟 Features

- **True Asynchronous & Concurrent Generation**  
  Uses a "Copy out - Process - Copy in" architecture via the UE Task System to ensure the `Game Thread` is never blocked, even for massive grids.
- **Advanced Grid Support**  
  Along with 2D and 3D square grids, natively supports **3D Hexagonal Prism Grids** (Odd-r offset with Cube coordinate mathematics) for strategy and tactical games.
- **Nested WFC (Divide & Conquer)**  
  Supports generating massive worlds by nesting micro-WFC units inside macro-WFC blocks, preventing exponential dimension explosion and heavy retry penalties.

https://github.com/user-attachments/assets/764952d4-27b8-482b-a6be-5df100b022f7
  
- **StackTree Lifecycle Management**  
  Safely manages asynchronous dependencies and memory cleanup for nested generation using a custom `StackTree` (LIFO, Reference Counting) structure.
- **Dual-Track Object Pooling**  
  - **ISM Pool:** Uses 32-bit compressed indices and bucket clustering to render tens of thousands of static meshes in a few Draw Calls.
  - **Actor Pool:** Manages complex logic entities with "spawn-once, sleep-often" mechanics to prevent GC spikes.
- **Low-Bandwidth Multiplayer Sync**  
  Replaces heavy Actor RPCs with continuous memory array Multicasting (for ISMs) and a single-boolean "State Switch" (for Actors), ensuring zero ghosting and zero lag for clients.

https://github.com/user-attachments/assets/3b4d11a2-a935-45b4-99ff-a0138031f6fa
  
- **Data-Driven & Modular Constraints**  
  Configure generation entirely through `Data Assets` and `Gameplay Tags`. Includes new constraint modules: *Edge Block*, *Position*, *Multiple Boundary*, *Fixed Tile*, and *Count Constraints*, optimized with memory **Snapshots** for instant retries.

---

## ⚙️ Overview

### The Core Generation Pipeline

- The `UWFCGenerator` brings together several pieces needed to go from source data to a grid of selected tiles.
  - A `UWFCModel` defines all available tiles and handles tag-based edge matching.
  - `UWFCGridConfig` specifies the grid class (2D, 3D, or **3DHex**), dimensions, and cell size.
  - An array of `UWFCConstraint` objects set the rules for how tiles can be placed.
  - A `UWFCCellSelector` (using Shannon Entropy + Random Noise) picks the optimal cell to collapse next.
- Unlike traditional implementations, our core solver relies on an optimized **AC-4 Arc Consistency Algorithm**, trading minimal memory for extreme constraint propagation speed.
- The heavy lifting is delegated to `FAsyncGenerator` in background threads. If a contradiction occurs (dead end), the background thread safely restores an initialized **Snapshot** and retries instantly without freezing the editor.

### Large World & Asset Management

- `UWFCUnit` and `UWFCUnitManager` handle the macro-to-micro world building. A `UWFCUnit` can spawn child units, enabling deep nested WFC generation.
- `AISMManagerActor` and `UIxObjectPool` work behind the scenes. When async math generation finishes, the `UWFCUnitManager` requests physical representation from these pools rather than spawning raw Actors.

---

## 🚀 Getting Started

1. **Create the Configurations**
   - Create a `UWFCAsset`. Assign your Grid Config (e.g., `UWFCGrid3DHexConfig`), Model, Cell Selector, and Constraint Classes (like Boundary and Edge constraints).
   - Create a `UWFCTileSet` and populate it with `UWFCTileAsset` (e.g., `UWFCTileAsset3DHex`).
   - Define your tiles using **Gameplay Tags** for edges (e.g., `Edge.Grass`, `Edge.Water`).
2. **Setup Object Pools**
   - Create a `UISMManagerConfig` to define which Static Meshes and Materials should be instanced.
   - Create a `UIxObjectPoolConfig` to define which complex Actors should be pooled.
3. **Configure the WFC Unit**
   - Create a Blueprint inheriting from `UWFCUnit`. Assign your `UWFCAsset`, `UISMManagerConfig`, and `UIxObjectPoolConfig`.
   - Define nested behavior if this unit is meant to spawn child WFC units.
4. **Run the Generation**
   - Drop a `AWFCChainActor` into your level.
   - Set its `StartUnitClass` to the `UWFCUnit` Blueprint you created.
   - Check `IsAutoRun` and hit **Play**.
   - The plugin will automatically deploy to background threads, calculate the grid, synchronize instances from the pool, and render the terrain seamlessly.

---

## 🌐 Multiplayer Setup (Client / Server)

To ensure multiplayer synchronization works out of the box:

- **For Static Meshes:**  
  Just add them to the `UISMManagerConfig`. The `AISMManagerActor` on the Server will automatically compress indices and Multicast them to Clients for local GPU batch updating.
  
- **For Actors:**  
  Ensure your Actor implements `UIxActivationInterface` and has a `UIxPoolStateComponent`. Add your spawn/hide visual logic (Timeline, Particles, etc.) to the `Activate` and `Deactivate` blueprint events. The server will flip the pool state, and the client will handle the visuals natively.
