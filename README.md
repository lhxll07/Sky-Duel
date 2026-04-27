# Sky Duel

Sky Duel 是一个用 C++ 写的 2D 飞机射击小游戏。玩家控制底部飞机左右移动并发射子弹，击落不断生成的敌机；分数达到当前关卡目标后进入下一关，生命值归零则游戏结束。

这个项目来自早期课程/练手代码，后来补上了 CMake 构建，并整理了资源目录和 Windows 下的 GLFW / Dear ImGui 依赖。

## 功能

- 玩家飞机左右移动和手动射击
- 敌机随机生成并向下移动
- 子弹、敌机、玩家飞机的碰撞判定
- 分数、生命值、目标分数显示
- 达到目标分数后进入下一关
- 随分数提升玩家速度和射击频率
- 使用 PNG 图片资源渲染飞机和子弹

## 操作

| 按键 | 功能 |
| --- | --- |
| `A` / `Left` | 向左移动 |
| `D` / `Right` | 向右移动 |
| `Space` / `Up` | 发射子弹 |
| `Q` | 返回开始界面 |

## 项目结构

```text
sky-duel/
├── CMakeLists.txt
├── README.md
├── include/
│   └── stb_image.h
├── pictures/
│   ├── bullet.png
│   ├── enemyplane.png
│   └── myplane.png
├── src/
│   └── main.cpp
└── external/
    ├── glad/
    ├── glfw/
    └── imgui/
```

## 构建环境

当前 CMake 配置面向 Windows，支持 MinGW 或 MSVC。项目内已经带有 GLFW、GLAD、Dear ImGui 和 stb_image 相关文件，系统侧需要准备：

- CMake 3.20 或更高版本
- Ninja，或其他 CMake 生成器
- MinGW-w64 或 Visual Studio C++ 编译环境
- 支持 OpenGL 3.3 的显卡/驱动

## 编译运行

使用 MinGW + Ninja：

```powershell
cmake -S . -B build-mingw -G Ninja
cmake --build build-mingw
.\build-mingw\sky-duel.exe
```

如果 CMake 默认找到了不合适的编译器，可以显式指定 MinGW：

```powershell
cmake -S . -B build-mingw -G Ninja `
  -DCMAKE_C_COMPILER="C:/path/to/mingw64/bin/gcc.exe" `
  -DCMAKE_CXX_COMPILER="C:/path/to/mingw64/bin/g++.exe"
cmake --build build-mingw
.\build-mingw\sky-duel.exe
```

CMake 构建后会把 `glfw3.dll` 和 `pictures/` 自动复制到输出目录。直接运行 exe 时，请确保 exe 旁边存在这些文件。

## 规则

- 初始生命值为 200。
- 敌机撞到玩家或越过判定区域会扣除生命值。
- 击毁一架敌机得 10 分。
- 每累计 100 分，玩家移动速度提高，射击冷却缩短。
- 分数达到当前目标后可以进入下一关，下一关目标分数翻倍。

## 第三方依赖

本项目使用并随仓库保留了以下开源库/第三方代码：

| 依赖 | 用途 | 许可证/说明 |
| --- | --- | --- |
| GLFW | 创建窗口、处理输入、管理 OpenGL 上下文 | zlib/libpng license，见 `external/glfw/LICENSE.md` |
| Dear ImGui | 绘制游戏界面和状态文本 | MIT License，见 `external/imgui/LICENSE.txt` |
| GLAD | 加载 OpenGL 函数指针 | 由 glad 生成的 OpenGL loader，代码位于 `external/glad/` |
| stb_image | 读取 PNG 图片资源 | public domain / MIT，见 `include/stb_image.h` 文件末尾的许可证说明 |

第三方代码的版权和许可证归原作者所有，本仓库保留其原始许可证文件和版权声明。

## 说明

这是一个学习性质的小项目，代码重点放在基本游戏循环、输入处理、碰撞判断和 OpenGL/ImGui 的组合使用上。后续可以继续拆分渲染、资源加载和游戏逻辑，让结构更清晰。
