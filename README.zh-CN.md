[🇬🇧 English](README.md) | [🇨🇳 简体中文](README.zh-CN.md)

# WFCNA (高级波函数坍塌) 插件

> 一款先进的、面向工业级生产的虚幻引擎程序化生成插件，基于 [波函数坍塌 (Wave Function Collapse)](https://github.com/mxgmn/WaveFunctionCollapse) 算法构建。

本项目基于 [bohdon 的 WFCPlugin](https://github.com/bohdon/WFCPlugin) 的基础概念，针对**工业级游戏开发**进行了深度扩展与重构。具备真正的异步并发生成、六棱柱网格、嵌套大世界生成、高性能对象池以及多人联机网络同步等高级特性。

---

## 🌟 核心特性

- **真正的异步与并发生成**  
  通过 UE 的任务系统（Task System）采用 Copy out - Process - Copy in 架构，确保在生成超大网格时也绝不阻塞`游戏主线程（Game Thread）`。
- **高级网格支持**  
  除了 2D 和 3D 方块网格，原生支持 **3D 六棱柱网格（3D Hexagonal Prism Grids）**（基于 Odd-r 偏移坐标与立方体 Cube 坐标数学体系），非常适合策略与战棋类游戏。
- **嵌套式 WFC (分治法)**  
  支持通过在宏观 WFC 区块中嵌套微观 WFC 单元来生成庞大的大世界，有效防止维度爆炸与沉重的重试惩罚。

https://github.com/user-attachments/assets/764952d4-27b8-482b-a6be-5df100b022f7
  
- **StackTree 生命周期管理**  
  使用自定义的 `StackTree`（基于 LIFO 栈与引用计数）结构，安全地管理嵌套生成的异步依赖与内存清理。
- **双轨制对象池 (Dual-Track Object Pooling)**  
  - **ISM 池：** 采用 32 位压缩索引与特征分桶聚集，通过极少的 Draw Call 即可渲染数以万计的静态网格体。
  - **Actor 池：** 采用“生成一次，多次休眠”的机制管理复杂的逻辑实体，彻底杜绝 GC（垃圾回收）卡顿。
- **低带宽多端网络同步**  
  废弃了繁重的 Actor RPC 指令，改用连续内存数组多播（针对 ISM）和单布尔值“状态开关”（针对 Actor），确保客户端零残影、零延迟。

https://github.com/user-attachments/assets/3b4d11a2-a935-45b4-99ff-a0138031f6fa
  
- **数据驱动与模块化约束**  
  完全通过 `数据资产 (Data Assets)` 和 `游戏标签 (Gameplay Tags)` 进行配置。包含全新的约束模块：*边缘排斥 (Edge Block)*、*空间区域 (Position)*、*多重边界 (Multiple Boundary)*、*固定位置 (Fixed Tile)* 以及 *数量限制 (Count Constraints)*，并利用内存**快照 (Snapshots)** 机制实现瞬间重试。

---

## ⚙️ 架构概览

### 核心生成管线

- `UWFCGenerator` 汇集了从源数据到生成最终图块网格所需的核心组件：
  - `UWFCModel`：定义所有可用的图块，并处理基于标签的边缘匹配逻辑。
  - `UWFCGridConfig`：指定网格类（2D、3D 或 **3DHex**）、网格尺寸及单元大小。
  - `UWFCConstraint` 数组：设置图块放置的宏观约束与规则。
  - `UWFCCellSelector`：基于香农熵（Shannon Entropy）+ 随机噪声，挑选下一个最适合坍缩的单元。
- 不同于传统实现，本系统的核心求解器依赖于高度优化的 **AC-4 弧一致性算法 (Arc Consistency Algorithm)**，以极小的内存开销换取极限的约束传播速度。
- 繁重的计算任务被委托给后台线程的 `FAsyncGenerator`。如果发生矛盾（遇到死胡同），后台线程会安全地恢复已初始化的**快照 (Snapshot)** 并瞬间进行重试，全程不卡死编辑器。

### 大世界与资产管理

- `UWFCUnit` 与 `UWFCUnitManager` 负责处理宏观到微观的世界构建。一个 `UWFCUnit` 可以派生出多个子单元，从而实现深度的嵌套 WFC 生成。
- `AISMManagerActor` 与 `UIxObjectPool` 在幕后协同工作。当后台异步数学生成完成后，`UWFCUnitManager` 会向这些对象池请求物理资产，而不是直接粗暴地 Spawn 新的 Actor。

---

## 🚀 快速上手

1. **创建配置 (Create the Configurations)**
   - 创建一个 `UWFCAsset`。为其分配你的网格配置（如 `UWFCGrid3DHexConfig`）、图块模型、选择器以及约束类（如边界约束和边缘约束）。
   - 创建一个 `UWFCTileSet` 并在其中填充 `UWFCTileAsset`（如 `UWFCTileAsset3DHex`）。
   - 使用 **Gameplay Tags** 为图块定义边缘属性（例如：`Edge.Grass`，`Edge.Water`）。
2. **设置对象池 (Setup Object Pools)**
   - 创建一个 `UISMManagerConfig` 以定义哪些静态网格体（Static Meshes）和材质应当被实例化渲染。
   - 创建一个 `UIxObjectPoolConfig` 以定义哪些复杂的 Actor 需要被池化管理。
3. **配置 WFC 单元 (Configure the WFC Unit)**
   - 创建一个继承自 `UWFCUnit` 的蓝图。将前面创建的 `UWFCAsset`、`UISMManagerConfig` 和 `UIxObjectPoolConfig` 分配给它。
   - 如果该单元需要派生子 WFC 单元，配置其嵌套行为。
4. **运行生成 (Run the Generation)**
   - 将一个 `AWFCChainActor` 拖入你的关卡中。
   - 将其 `StartUnitClass` 设置为你刚刚创建的 `UWFCUnit` 蓝图。
   - 勾选 `IsAutoRun` 并点击 **Play**。
   - 插件会自动将任务部署到后台线程，计算网格，同步对象池中的实例，并无缝渲染出整个地形。

---

## 🌐 联机同步设置

为了确保多人联机同步能够开箱即用：

- **对于静态网格体 (Static Meshes)：**  
  只需将它们添加到 `UISMManagerConfig` 中。服务端的 `AISMManagerActor` 会自动压缩坐标索引，并通过多播（Multicast）将其发送给所有客户端，客户端会在本地进行 GPU 批处理更新。
  
- **对于 Actor 实体：**  
  确保你的 Actor 实现了 `UIxActivationInterface` 接口，并挂载了 `UIxPoolStateComponent` 组件。将你的生成/隐藏视觉逻辑（如 Timeline 动画、粒子特效等）添加到蓝图的 `Activate` 和 `Deactivate` 事件中。服务端仅负责翻转对象池状态，客户端会自发且原生地上演所有的视觉表现。
  
--- 

## 🖼️ 演示图

<img width="556" height="219" alt="Image" src="https://github.com/user-attachments/assets/b5021d81-a15c-4461-831f-941a5a863ab2" />

<img width="555" height="211" alt="Image" src="https://github.com/user-attachments/assets/07f28b6c-2707-4dc6-9b56-272cd67bbcd4" />

<img width="651" height="265" alt="Image" src="https://github.com/user-attachments/assets/48cc7f04-44ea-4a95-a1df-8c64555f2b86" />

<img width="594" height="306" alt="Image" src="https://github.com/user-attachments/assets/2c3dc401-4b59-4ef3-aade-2079f3bf752b" />

<img width="833" height="311" alt="Image" src="https://github.com/user-attachments/assets/b4477b58-5ed3-4d25-8e31-327a25210775" />

<img width="809" height="305" alt="Image" src="https://github.com/user-attachments/assets/51a754df-d398-4660-b154-0eef7736ad7a" />
