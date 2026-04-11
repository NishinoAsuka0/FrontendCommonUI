# 项目结构索引

> 最后更新: 2026-04-06
> Timestamp: 20260406

---

## 源代码文件 (Source/)

### 核心模块
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI.Target.cs` | 游戏运行时目标配置 |
| `Source/FrontendUIEditor.Target.cs` | 编辑器目标配置 |
| `Source/FrontendUI/FrontendUI.Build.cs` | 模块构建配置 |
| `Source/FrontendUI/FrontendUI.h` | 模块头文件 |
| `Source/FrontendUI/FrontendUI.cpp` | 模块实现文件 |

### 控制器 (Controller/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/Controller/FrontendPlayerController.h` | 玩家控制器头文件 |
| `Source/FrontendUI/Private/Controller/FrontendPlayerController.cpp` | 玩家控制器实现 |

### 控件 (Widgets/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Widgets/Widget_PrimaryLayout.h` | 主布局控件头文件 |
| `Source/FrontendUI/Widgets/Widget_PrimaryLayout.cpp` | 主布局控件实现 |
| `Source/FrontendUI/Public/Widgets/Widget_ActivatableBase.h` | 可激活控件基类头文件 |
| `Source/FrontendUI/Private/Widgets/Widget_ActivatableBase.cpp` | 可激活控件基类实现 |

### 子系统 (Subsystems/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/Subsystems/FrontendUISubSystem.h` | UI子系统头文件 |
| `Source/FrontendUI/Private/Subsystems/FrontendUISubSystem.cpp` | UI子系统实现 |

### 异步任务 (AsyncAction/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/AsyncAction/AsyncAction_PushSoftWidget.h` | 推送软引用控件异步任务头文件 |
| `Source/FrontendUI/Private/AsyncAction/AsyncAction_PushSoftWidget.cpp` | 推送软引用控件异步任务实现 |

### 设置 (Settings/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/FrontendSettings/FrontendDeveloperSettings.h` | 开发者设置头文件 |
| `Source/FrontendUI/Private/FrontendSettings/FrontendDeveloperSettings.cpp` | 开发者设置实现 |

### 功能库 (FunctionLibrary/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/FrontendFunctionLibrary.h` | 前端功能库头文件 |
| `Source/FrontendUI/Private/FrontendFunctionLibrary.cpp` | 前端功能库实现 |

### 其他
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/FrontendGamePlayTags.h` | GameplayTags头文件 |
| `Source/FrontendUI/Private/FrontendGamePlayTags.cpp` | GameplayTags实现 |
| `Source/FrontendUI/Public/FrontendDebugHelper.h` | 调试辅助头文件 |

---

## 配置文件 (Config/)

| 文件路径 | 描述 |
|---------|------|
| `Config/DefaultEngine.ini` | 引擎默认配置 |
| `Config/DefaultEditor.ini` | 编辑器默认配置 |
| `Config/DefaultGame.ini` | 游戏默认配置 |
| `Config/DefaultInput.ini` | 输入默认配置 |

---

## 资源文件 (Content/)

### 字体资源
| 目录 | 描述 |
|-----|------|
| `Content/Assets/Fonts/Cinzel/` | Cinzel字体系列 |
| `Content/Assets/Fonts/Din/` | DIN字体 |

### 控制器图标
| 目录 | 描述 |
|-----|------|
| `Content/Assets/ControllerIcons/MouseKeyboard/` | 鼠标键盘图标资源 |

---

## 项目文件

| 文件路径 | 描述 |
|---------|------|
| `FrontendUI.uproject` | UE项目文件 |
| `FrontendUI.sln` | Visual Studio解决方案 |

---

## 搜索关键词映射

当搜索以下关键词时，优先查找对应文件：

| 关键词 | 对应文件 |
|-------|---------|
| PlayerController, 控制器 | `Source/FrontendUI/Public/Controller/FrontendPlayerController.h` |
| Widget, 控件, UI | `Source/FrontendUI/Public/Widgets/Widget_ActivatableBase.h` |
| PrimaryLayout, 主布局 | `Source/FrontendUI/Widgets/Widget_PrimaryLayout.h` |
| SubSystem, 子系统 | `Source/FrontendUI/Public/Subsystems/FrontendUISubSystem.h` |
| AsyncAction, 异步 | `Source/FrontendUI/Public/AsyncAction/AsyncAction_PushSoftWidget.h` |
| Settings, 设置 | `Source/FrontendUI/Public/FrontendSettings/FrontendDeveloperSettings.h` |
| GamePlayTags, 标签 | `Source/FrontendUI/Public/FrontendGamePlayTags.h` |
| FunctionLibrary, 功能库 | `Source/FrontendUI/Public/FrontendFunctionLibrary.h` |
| Debug, 调试 | `Source/FrontendUI/Public/FrontendDebugHelper.h` |
| Build, 构建 | `Source/FrontendUI/FrontendUI.Build.cs` |