# CyberEngine Input System

## 概述

CyberEngine的新输入管理系统是一个现代化、灵活的输入处理框架，设计用于替代gainput库。它提供了更清晰的架构、更好的性能和更易用的API。

## 核心特性

- **现代C++17设计** - 使用智能指针、std::variant、std::optional等现代特性
- **事件驱动架构** - 基于事件队列的输入处理，支持异步操作
- **上下文系统** - 通过InputContext管理不同场景的输入映射（游戏/UI/菜单等）
- **设备抽象** - 统一的设备接口，易于扩展新设备类型
- **热插拔支持** - 运行时设备连接/断开检测
- **输入录制回放** - 支持录制和回放输入序列（待实现）

## 架构设计

### 核心组件

```
Input/
├── Core/                    # 核心系统
│   ├── InputTypes.h        # 类型定义和枚举
│   ├── IInputDevice.h      # 设备接口
│   ├── InputManager.h/cpp  # 输入管理器（单例）
│   └── InputContext.h/cpp  # 输入上下文
├── Devices/                 # 设备实现
│   ├── KeyboardDevice.*    # 键盘设备
│   ├── MouseDevice.*       # 鼠标设备
│   └── GamepadDevice.*     # 手柄设备（待实现）
└── Platform/               # 平台相关
    └── Win32/              # Windows平台实现
```

### 类关系

```
InputManager (Singleton)
    ├── IInputDevice (多个设备实例)
    │   ├── KeyboardDevice
    │   ├── MouseDevice
    │   └── GamepadDevice
    └── InputContext (多个上下文)
        └── InputAction (多个动作)
            └── InputBinding (多个绑定)
```

## 使用指南

### 初始化

```cpp
#include "Input/Core/InputManager.h"

// 获取输入管理器实例
auto& inputMgr = CyberEngine::Input::InputManager::GetInstance();

// 初始化（传入窗口句柄）
inputMgr.Initialize(hwnd);
inputMgr.SetWindowSize(width, height);
```

### 创建输入上下文

```cpp
// 创建游戏上下文
auto* gameContext = inputMgr.CreateContext("Game");

// 定义动作ID
enum GameActions {
    ACTION_MOVE_FORWARD = 0,
    ACTION_JUMP,
    ACTION_FIRE
};

// 创建动作
gameContext->CreateAction(ACTION_MOVE_FORWARD, "MoveForward");
gameContext->CreateAction(ACTION_JUMP, "Jump");

// 绑定按键
gameContext->BindAction(ACTION_MOVE_FORWARD, DeviceType::Keyboard, Key::W);
gameContext->BindAction(ACTION_JUMP, DeviceType::Keyboard, Key::Space);
gameContext->BindAction(ACTION_JUMP, DeviceType::Gamepad, GamepadButton::A);

// 激活上下文
inputMgr.PushContext(gameContext);
```

### 查询输入

```cpp
void Update(float deltaTime) {
    inputMgr.Update(deltaTime);
    
    // 通过上下文查询
    if (gameContext->IsActionPressed(ACTION_MOVE_FORWARD)) {
        // 向前移动
    }
    
    if (gameContext->IsActionJustPressed(ACTION_JUMP)) {
        // 刚按下跳跃键
    }
    
    // 直接查询设备
    if (inputMgr.IsKeyPressed(Key::Escape)) {
        // ESC键被按下
    }
    
    // 获取鼠标位置
    float mouseX, mouseY;
    inputMgr.GetMousePosition(mouseX, mouseY);
    
    // 获取鼠标移动增量
    float deltaX, deltaY;
    inputMgr.GetMouseDelta(deltaX, deltaY);
}
```

### 上下文切换

```cpp
// 切换到菜单
inputMgr.PopContext();  // 移除游戏上下文
inputMgr.PushContext(menuContext);  // 添加菜单上下文

// 返回游戏
inputMgr.PopContext();  // 移除菜单上下文
inputMgr.PushContext(gameContext);  // 添加游戏上下文
```

### 设置回调

```cpp
// 为动作设置回调
auto* fireAction = gameContext->GetAction(ACTION_FIRE);
fireAction->SetCallback([](float value) {
    if (value > 0.0f) {
        std::cout << "Fire!\n";
    }
});

// 注册全局事件回调
inputMgr.RegisterEventCallback([](const InputEvent& event) {
    if (event.type == InputEventType::DeviceConnected) {
        std::cout << "Device connected!\n";
    }
});
```

## 与gainput的对比

| 特性 | gainput | 新输入系统 |
|------|---------|------------|
| C++标准 | C++98 | C++17 |
| 内存管理 | 自定义分配器 | 智能指针 |
| 输入映射 | InputMap | InputContext |
| 事件处理 | 监听器模式 | 事件队列 + 回调 |
| 平台代码 | 分散在各文件 | 集中在Platform目录 |
| 扩展性 | 需要修改源码 | 接口设计，易扩展 |

## 集成到CyberEngine

系统已集成到`Application`类中：

```cpp
// Application类中新增
CyberEngine::Input::InputManager* get_new_input_manager();

// 在on_window_create中初始化
m_pNewInputManager = &InputManager::GetInstance();
m_pNewInputManager->Initialize(hwnd);

// 在update中更新
m_pNewInputManager->Update(deltaTime);

// Windows消息处理
MSG winMsg = { hwnd, msg, wParam, lParam };
m_pNewInputManager->ProcessPlatformEvent(&winMsg);
```

## 待实现功能

- [ ] Gamepad设备支持（XInput/DirectInput）
- [ ] Touch设备支持
- [ ] 输入录制和回放
- [ ] 配置文件系统（JSON）
- [ ] Linux/Mac平台支持
- [ ] 输入映射UI编辑器
- [ ] 振动反馈支持
- [ ] 手势识别系统

## 示例代码

完整示例请参考 `Example/InputExample.cpp`

## 性能优化

- 使用事件队列减少轮询开销
- 设备状态缓存避免重复查询
- 上下文栈实现快速切换
- 编译时类型检查提高安全性

## 调试支持

启用调试日志：
```cpp
#define INPUT_DEBUG 1
```

这将输出详细的输入事件信息，帮助调试输入问题。