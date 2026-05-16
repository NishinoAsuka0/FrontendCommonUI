# 项目结构索引

> 最后更新: 2026-05-16
> Timestamp: 20260516

---

## 源代码文件 (Source/)

### 核心模块
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI.Target.cs` | 游戏运行时目标配置 |
| `Source/FrontendUIEditor.Target.cs` | 编辑器目标配置 |
| `Source/FrontendUI/FrontendUI.Build.cs` | 模块构建配置（依赖: PropertyPath, CommonInput 等） |
| `Source/FrontendUI/Public/FrontendUI.h` | 模块头文件（注册编辑器面板 Tab） |
| `Source/FrontendUI/Private/FrontendUI.cpp` | 模块实现文件 |

### 控制器 (Controller/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/Controller/FrontendPlayerController.h` | 前端 PlayerController（UI 层） |
| `Source/FrontendUI/Private/Controller/FrontendPlayerController.cpp` | 前端 PlayerController 实现 |
| `Source/FrontendUI/Public/Controller/InGamePlayerController.h` | 游戏内 PlayerController（InputConfig 驱动技能输入绑定，移动/视角/跳跃保持独立） |
| `Source/FrontendUI/Private/Controller/InGamePlayerController.cpp` | 游戏内 PlayerController 实现 |

### PlayerState (PlayerState/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/PlayerState/CharacterState.h` | CharacterState 头文件（PlayerState，持有 ASC + AbilitySets） |
| `Source/FrontendUI/Private/PlayerState/CharacterState.cpp` | CharacterState 实现 |

### 角色 (Character/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/Character/BaseCharacter.h` | 角色基类头文件（移动/视角/ActivateAbilityByTag） |
| `Source/FrontendUI/Private/Character/BaseCharacter.cpp` | 角色基类实现 |
| `Source/FrontendUI/Public/Character/WeaponColliderComponent.h` | 武器碰撞组件头文件（UWeaponColliderComponent + AnimNotify 类） |
| `Source/FrontendUI/Private/Character/WeaponColliderComponent.cpp` | 武器碰撞组件实现 |

### 输入配置 (Input/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/Input/InputConfig.h` | 输入配置 DataAsset（InputAction→GameplayTag 映射，驱动技能输入绑定） |

### 控件 (Widgets/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Widgets/Widget_PrimaryLayout.h` | 主布局控件头文件（管理 WidgetStack 注册） |
| `Source/FrontendUI/Widgets/Widget_PrimaryLayout.cpp` | 主布局控件实现 |
| `Source/FrontendUI/Public/Widgets/Widget_ActivatableBase.h` | 可激活控件基类头文件 |
| `Source/FrontendUI/Private/Widgets/Widget_ActivatableBase.cpp` | 可激活控件基类实现 |
| `Source/FrontendUI/Public/Widgets/Widget_ConfirmScreen.h` | 确认弹窗控件头文件 |
| `Source/FrontendUI/Private/Widgets/Widget_ConfirmScreen.cpp` | 确认弹窗控件实现 |

### 控件组件 (Widgets/Components/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/Widgets/Components/FrontendCommonButtonBase.h` | 按钮基类头文件 |
| `Source/FrontendUI/Private/Widgets/Components/FrontendCommonButtonBase.cpp` | 按钮基类实现 |
| `Source/FrontendUI/Public/Widgets/Components/FrontendCommonRotator.h` | 旋转选择器头文件（继承 UCommonRotator） |
| `Source/FrontendUI/Private/Widgets/Components/FrontendCommonRotator.cpp` | 旋转选择器实现 |
| `Source/FrontendUI/Public/Widgets/Components/FrontendCommonListView.h` | 列表视图头文件 |
| `Source/FrontendUI/Private/Widgets/Components/FrontendCommonListView.cpp` | 列表视图实现 |
| `Source/FrontendUI/Public/Widgets/Components/FrontendTabListWidgetBase.h` | Tab列表控件头文件 |
| `Source/FrontendUI/Private/Widgets/Components/FrontendTabListWidgetBase.cpp` | Tab列表控件实现 |

### 选项界面 (Widgets/Options/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/Widgets/Options/Widget_OptionScreen.h` | 选项界面控件头文件（管理 DataRegistry 生命周期） |
| `Source/FrontendUI/Private/Widgets/Options/Widget_OptionScreen.cpp` | 选项界面控件实现 |
| `Source/FrontendUI/Public/Widgets/Options/Widget_OptionsDetailView.h` | 选项详情面板头文件（右侧描述区，显示标题/图片/描述/动态信息） |
| `Source/FrontendUI/Private/Widgets/Options/Widget_OptionsDetailView.cpp` | 选项详情面板实现 |
| `Source/FrontendUI/Public/Widgets/Options/OptionsDataRegistry.h` | 选项数据注册表头文件（初始化所有选项数据） |
| `Source/FrontendUI/Private/Widgets/Options/OptionsDataRegistry.cpp` | 选项数据注册表实现 |
| `Source/FrontendUI/Public/Widgets/Options/OptionsDataInteractionHelper.h` | 选项数据交互辅助类头文件（通过 PropertyPathHelpers 读写 GameUserSettings） |
| `Source/FrontendUI/Private/Widgets/Options/OptionsDataInteractionHelper.cpp` | 选项数据交互辅助类实现 |
| `Source/FrontendUI/Public/Widgets/Options/DataAsset_DataListEntryMapping.h` | 数据列表项映射 DataAsset 头文件 |
| `Source/FrontendUI/Private/Widgets/Options/DataAsset_DataListEntryMapping.cpp` | 数据列表项映射 DataAsset 实现 |

### 选项数据对象 (Widgets/Options/DataObjects/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/Widgets/Options/DataObjects/ListDataObject_Base.h` | 列表数据对象基类头文件 |
| `Source/FrontendUI/Private/Widgets/Options/DataObjects/ListDataObject_Base.cpp` | 列表数据对象基类实现 |
| `Source/FrontendUI/Public/Widgets/Options/DataObjects/ListDataObject_Value.h` | 值类型数据对象头文件（持有 Getter/Setter 辅助类） |
| `Source/FrontendUI/Private/Widgets/Options/DataObjects/ListDataObject_Value.cpp` | 值类型数据对象实现 |
| `Source/FrontendUI/Public/Widgets/Options/DataObjects/ListDataObject_String.h` | 字符串类型数据对象头文件（如 Difficulty 选项） |
| `Source/FrontendUI/Private/Widgets/Options/DataObjects/ListDataObject_String.cpp` | 字符串类型数据对象实现 |
| `Source/FrontendUI/Public/Widgets/Options/DataObjects/ListDataObject_Collection.h` | 集合类型数据对象头文件（Tab 分组） |
| `Source/FrontendUI/Private/Widgets/Options/DataObjects/ListDataObject_Collection.cpp` | 集合类型数据对象实现 |

### 选项列表项 (Widgets/Options/ListEntry/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/Widgets/Options/ListEntry/Widget_ListEntry_Base.h` | 列表项基类头文件 |
| `Source/FrontendUI/Private/Widgets/Options/ListEntry/Widget_ListEntry_Base.cpp` | 列表项基类实现 |
| `Source/FrontendUI/Public/Widgets/Options/ListEntry/Widget_ListEntry_String.h` | 字符串列表项头文件（绑定 Rotator + 增减按钮） |
| `Source/FrontendUI/Private/Widgets/Options/ListEntry/Widget_ListEntry_String.cpp` | 字符串列表项实现 |

### 子系统 (Subsystems/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/Subsystems/FrontendUISubSystem.h` | UI子系统头文件（GameInstance 级，管理 PrimaryLayout 和控件推送） |
| `Source/FrontendUI/Private/Subsystems/FrontendUISubSystem.cpp` | UI子系统实现 |

### 异步任务 (AsyncAction/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/AsyncAction/AsyncAction_PushSoftWidget.h` | 推送软引用控件异步任务头文件 |
| `Source/FrontendUI/Private/AsyncAction/AsyncAction_PushSoftWidget.cpp` | 推送软引用控件异步任务实现 |
| `Source/FrontendUI/Public/AsyncAction/UAsyncAction_PushConfirmScreen.h` | 推送确认弹窗异步任务头文件 |
| `Source/FrontendUI/Private/AsyncAction/UAsyncAction_PushConfirmScreen.cpp` | 推送确认弹窗异步任务实现 |

### 设置 (FrontendSettings/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/FrontendSettings/FrontendDeveloperSettings.h` | 开发者设置头文件 |
| `Source/FrontendUI/Private/FrontendSettings/FrontendDeveloperSettings.cpp` | 开发者设置实现 |
| `Source/FrontendUI/Public/FrontendSettings/FrontendGameUserSettings.h` | 游戏用户设置头文件（继承 UGameUserSettings，含 Config 属性如 CurrentGameDifficulty） |
| `Source/FrontendUI/Private/FrontendSettings/FrontendGameUserSettings.cpp` | 游戏用户设置实现 |

### 功能库 (FunctionLibrary/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/FrontendFunctionLibrary.h` | 前端功能库头文件 |
| `Source/FrontendUI/Private/FrontendFunctionLibrary.cpp` | 前端功能库实现 |

### 能力系统 (AbilitySystem/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/AbilitySystem/AbilitySet.h` | AbilitySet DataAsset 头文件（Ability/GE/AttributeSet 授予） |
| `Source/FrontendUI/Private/AbilitySystem/AbilitySet.cpp` | AbilitySet DataAsset 实现 |
| `Source/FrontendUI/Public/AbilitySystem/AttributeSet_Base.h` | 基础属性集头文件（HP/MaxHP, MP/MaxMP, Energy/MaxEnergy） |
| `Source/FrontendUI/Private/AbilitySystem/AttributeSet_Base.cpp` | 基础属性集实现（Clamp/Rep/GE回调） |
| `Source/FrontendUI/Public/AbilitySystem/SkillConfigTypes.h` | 技能配置类型定义（FSkillConfigRow, FSkillEffectRow, FBuffConfigRow, FFormulaEvaluator） |
| `Source/FrontendUI/Public/AbilitySystem/SkillConfigSubsystem.h` | 技能配置子系统头文件（运行时 DataTable 查询，GE Spec 创建） |
| `Source/FrontendUI/Private/AbilitySystem/SkillConfigSubsystem.cpp` | 技能配置子系统实现 |
| `Source/FrontendUI/Public/AbilitySystem/SkillGameplayAbility.h` | 数据驱动技能类头文件（UGA_DataDriven，从 DynamicAbilityTags 提取 SkillID） |
| `Source/FrontendUI/Private/AbilitySystem/SkillGameplayAbility.cpp` | 数据驱动技能类实现（按 EffectType/TargetType 分派 Damage/Heal/Buff） |

### 类型定义 (FrontendTypes/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/FrontendTypes/FrontendEnumTypes.h` | 前端枚举类型定义（EOptionsListDataModifyReason, EConfirmScreenType, EConfirmScreenButtonType 等） |

### 其他
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/FrontendGamePlayTags.h` | GameplayTags头文件 |
| `Source/FrontendUI/Private/FrontendGamePlayTags.cpp` | GameplayTags实现 |
| `Source/FrontendUI/Public/FrontendDebugHelper.h` | 调试辅助头文件 |

### 编辑器工具 (Editor/)
| 文件路径 | 描述 |
|---------|------|
| `Source/FrontendUI/Public/Editor/SDataTableImportPanel.h` | JSON → DataTable 批量导入面板 (Slate Widget) |
| `Source/FrontendUI/Private/Editor/SDataTableImportPanel.cpp` | 导入面板实现 |

---

## 配置文件 (Config/)

| 文件路径 | 描述 |
|---------|------|
| `Config/DefaultEngine.ini` | 引擎默认配置 |
| `Config/DefaultEditor.ini` | 编辑器默认配置 |
| `Config/DefaultEditorSettings.ini` | 编辑器设置配置 |
| `Config/DefaultGame.ini` | 游戏默认配置 |
| `Config/DefaultInput.ini` | 输入默认配置 |

---

## 资源文件 (Content/)

### 顶层蓝图 (Content/UI/)
| 文件路径 | 描述 |
|---------|------|
| `Content/UI/BP_FrontendController.uasset` | 前端 PlayerController 蓝图 |
| `Content/UI/BP_FrontendGameMode.uasset` | 前端 GameMode 蓝图 |
| `Content/UI/WPP_Template_Layout.uasset` | Widget 蓝图模板（PrimaryLayout 模板） |

### 通用控件 (Content/UI/Widgets/)
| 文件路径 | 描述 |
|---------|------|
| `Content/UI/Widgets/WBP_CUW_PrimaryLayout.uasset` | PrimaryLayout 蓝图实现 |
| `Content/UI/Widgets/WBP_CAW_PressAnyKey.uasset` | 按任意键启动界面 |
| `Content/UI/Widgets/WBP_CAW_MainMenu.uasset` | 主菜单界面 |
| `Content/UI/Widgets/WBP_CAW_ConfirmScreen.uasset` | 确认弹窗界面 |
| `Content/UI/Widgets/WBP_CAW_StoryScreen.uasset` | 剧情界面 |
| `Content/UI/Widgets/WBP_Text_ButtonDescription.uasset` | 按钮描述文本控件（底部提示栏） |

### 按钮控件 (Content/UI/Widgets/Buttons/)
| 文件路径 | 描述 |
|---------|------|
| `Content/UI/Widgets/Buttons/WBP_Button_Default.uasset` | 默认按钮 |
| `Content/UI/Widgets/Buttons/WBP_Button_ConfirmScreen.uasset` | 确认弹窗按钮 |
| `Content/UI/Widgets/Buttons/WBP_Button_BoundAction.uasset` | 绑定操作按钮（显示 InputAction 图标） |
| `Content/UI/Widgets/Buttons/WBP_Button_ListEntryImg.uasset` | 列表项图像按钮 |
| `Content/UI/Widgets/Buttons/WBP_Rotator_Options.uasset` | 选项旋转选择器 |

### 选项界面 (Content/UI/Widgets/Options/)
| 文件路径 | 描述 |
|---------|------|
| `Content/UI/Widgets/Options/WBP_CAW_OptionScreen.uasset` | 选项界面主控件 |
| `Content/UI/Widgets/Options/WBP_CAW_TabList.uasset` | 选项 Tab 列表控件 |
| `Content/UI/Widgets/Options/WBP_DetailsView_Options.uasset` | 选项详情面板控件 |
| `Content/UI/Widgets/Options/DA_DadaListEntryMapping.uasset` | DataObject → ListEntry 控件映射 DataAsset |
| `Content/UI/Widgets/Options/ListEntry/WBP_ListEntry_String.uasset` | 字符串选项列表项 |
| `Content/UI/Widgets/Options/ListEntry/WBP_ListEntry_InvalidRow.uasset` | 无效行占位 |
| `Content/UI/Widgets/Options/ListEntry/SizeBox_ListEntry.uasset` | 列表项尺寸盒 |

### 样式资源 (Content/UI/Styles/)
| 文件路径 | 描述 |
|---------|------|
| `Content/UI/Styles/Button/Style_Button_Clear.uasset` | 透明按钮样式 |
| `Content/UI/Styles/Button/Style_Button_Clear_Menu.uasset` | 透明菜单按钮样式 |
| `Content/UI/Styles/Button/Style_Button_Clear_NoSound.uasset` | 透明无声按钮样式 |
| `Content/UI/Styles/Button/Style_Button_ConfirmScreen.uasset` | 确认弹窗按钮样式 |
| `Content/UI/Styles/Button/Style_Button_Options.uasset` | 选项按钮样式 |
| `Content/UI/Styles/Text/Style_Text_Default.uasset` | 默认文本样式 |
| `Content/UI/Styles/Text/Style_Text_Default_Highlight.uasset` | 默认文本高亮样式 |
| `Content/UI/Styles/Text/Style_Text_ActionButtons.uasset` | 操作按钮文本样式 |
| `Content/UI/Styles/Text/Style_Text_ButtonDescription.uasset` | 按钮描述文本样式 |
| `Content/UI/Styles/Text/Style_Text_ConfirmScreen_Default.uasset` | 确认弹窗默认文本样式 |
| `Content/UI/Styles/Text/Style_Text_ConfirmScreen_Highlight.uasset` | 确认弹窗高亮文本样式 |
| `Content/UI/Styles/Text/Style_Text_ListEntry_Default.uasset` | 列表项默认文本样式 |
| `Content/UI/Styles/Text/Style_Text_ListEntry_Highlight.uasset` | 列表项高亮文本样式 |
| `Content/UI/Styles/Text/Style_Text_OptionsDetail_Title.uasset` | 选项详情标题文本样式 |
| `Content/UI/Styles/Text/DT_RichTextStyle_OptionScreen.uasset` | 选项界面富文本样式 DataTable |

### CommonInput 数据 (Content/UI/CommonInputData/)
| 文件路径 | 描述 |
|---------|------|
| `Content/UI/CommonInputData/InputData_Default.uasset` | 默认输入数据（CommonUI） |
| `Content/UI/CommonInputData/ControllerData_MouseKeyBoard.uasset` | 鼠键控制器数据 |
| `Content/UI/CommonInputData/ControllerData_Gameped.uasset` | 手柄控制器数据 |
| `Content/UI/CommonInputData/DT_CommonInputKeyMenu.uasset` | 输入按键菜单 DataTable |

### 配置工具 (Config/)
| 目录/文件 | 描述 |
|-----------|------|
| `Content/Config/Tool/xlsx_to_json.py` | Python xlsx→JSON 转换脚本 |
| `Content/Config/Data/` | xlsx 源数据目录 |
| `Content/Config/Json/` | 生成的 JSON 输出目录 |

### 字体资源
| 目录 | 描述 |
|-----|------|
| `Content/Assets/Fonts/Cinzel/` | Cinzel字体系列 |
| `Content/Assets/Fonts/Din/` | DIN字体 |

### 控制器图标
| 目录 | 描述 |
|-----|------|
| `Content/Assets/ControllerIcons/MouseKeyboard/` | 鼠标键盘图标 |
| `Content/Assets/ControllerIcons/PS4/` | PS4 手柄图标 |
| `Content/Assets/ControllerIcons/XBox/` | XBox 手柄图标 |

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
| InGamePlayerController, 游戏内控制器, Attack输入 | `Source/FrontendUI/Public/Controller/InGamePlayerController.h` |
| BaseCharacter, 角色基类, 攻击 | `Source/FrontendUI/Public/Character/BaseCharacter.h` |
| WeaponCollider, 武器碰撞, Attack, 攻击, AnimNotify | `Source/FrontendUI/Public/Character/WeaponColliderComponent.h` |
| Widget, 控件, UI | `Source/FrontendUI/Public/Widgets/Widget_ActivatableBase.h` |
| PrimaryLayout, 主布局 | `Source/FrontendUI/Widgets/Widget_PrimaryLayout.h` |
| SubSystem, 子系统 | `Source/FrontendUI/Public/Subsystems/FrontendUISubSystem.h` |
| AsyncAction, 异步推送控件 | `Source/FrontendUI/Public/AsyncAction/AsyncAction_PushSoftWidget.h` |
| ConfirmScreen, 确认弹窗 | `Source/FrontendUI/Public/Widgets/Widget_ConfirmScreen.h` |
| OptionScreen, 选项界面 | `Source/FrontendUI/Public/Widgets/Options/Widget_OptionScreen.h` |
| OptionsDetailView, 详情面板, 选项详情 | `Source/FrontendUI/Public/Widgets/Options/Widget_OptionsDetailView.h` |
| OptionsDataRegistry, 选项数据注册 | `Source/FrontendUI/Public/Widgets/Options/OptionsDataRegistry.h` |
| OptionsDataInteractionHelper, 数据交互 | `Source/FrontendUI/Public/Widgets/Options/OptionsDataInteractionHelper.h` |
| ListDataObject, 列表数据对象 | `Source/FrontendUI/Public/Widgets/Options/DataObjects/ListDataObject_Base.h` |
| ListDataObject_String, 字符串选项 | `Source/FrontendUI/Public/Widgets/Options/DataObjects/ListDataObject_String.h` |
| ListDataObject_Value, 值选项 | `Source/FrontendUI/Public/Widgets/Options/DataObjects/ListDataObject_Value.h` |
| ListDataObject_Collection, Tab分组 | `Source/FrontendUI/Public/Widgets/Options/DataObjects/ListDataObject_Collection.h` |
| ListEntry, 列表项 | `Source/FrontendUI/Public/Widgets/Options/ListEntry/Widget_ListEntry_Base.h` |
| ListEntry_String, 字符串列表项 | `Source/FrontendUI/Public/Widgets/Options/ListEntry/Widget_ListEntry_String.h` |
| FrontendCommonRotator, 旋转选择器 | `Source/FrontendUI/Public/Widgets/Components/FrontendCommonRotator.h` |
| FrontendCommonButtonBase, 按钮 | `Source/FrontendUI/Public/Widgets/Components/FrontendCommonButtonBase.h` |
| FrontendCommonListView, 列表视图 | `Source/FrontendUI/Public/Widgets/Components/FrontendCommonListView.h` |
| FrontendTabListWidgetBase, Tab列表 | `Source/FrontendUI/Public/Widgets/Components/FrontendTabListWidgetBase.h` |
| DataListEntryMapping, 列表项映射 | `Source/FrontendUI/Public/Widgets/Options/DataAsset_DataListEntryMapping.h` |
| FrontendGameUserSettings, 用户设置, Config | `Source/FrontendUI/Public/FrontendSettings/FrontendGameUserSettings.h` |
| FrontendDeveloperSettings, 开发者设置 | `Source/FrontendUI/Public/FrontendSettings/FrontendDeveloperSettings.h` |
| Settings, 设置 | `Source/FrontendUI/Public/FrontendSettings/FrontendDeveloperSettings.h` |
| GamePlayTags, 标签 | `Source/FrontendUI/Public/FrontendGamePlayTags.h` |
| InputConfig, 输入配置, InputAction映射, DA_InputConfig | `Source/FrontendUI/Public/Input/InputConfig.h` |
| FunctionLibrary, 功能库 | `Source/FrontendUI/Public/FrontendFunctionLibrary.h` |
| FrontendEnumTypes, 枚举类型 | `Source/FrontendUI/Public/FrontendTypes/FrontendEnumTypes.h` |
| Debug, 调试 | `Source/FrontendUI/Public/FrontendDebugHelper.h` |
| AbilitySet, 能力集, GAS | `Source/FrontendUI/Public/AbilitySystem/AbilitySet.h` |
| AttributeSet, 属性集, HP, MP, Energy | `Source/FrontendUI/Public/AbilitySystem/AttributeSet_Base.h` |
| SkillConfigTypes, 技能配置类型, FSkillConfigRow, FSkillEffectRow, FBuffConfigRow, FFormulaEvaluator | `Source/FrontendUI/Public/AbilitySystem/SkillConfigTypes.h` |
| SkillConfigSubsystem, 技能配置子系统, DataTable查询, GE创建 | `Source/FrontendUI/Public/AbilitySystem/SkillConfigSubsystem.h` |
| SkillConfig, 技能配置, SkillConfigTypes, FSkillConfigRow, FSkillEffectRow, FBuffConfigRow, FFormulaEvaluator, 公式求值 | `Source/FrontendUI/Public/AbilitySystem/SkillConfigTypes.h` |
| UGA_DataDriven, 数据驱动技能, SkillGameplayAbility, GameplayAbility | `Source/FrontendUI/Public/AbilitySystem/SkillGameplayAbility.h` |
| PlayerState, CharacterState, 玩家状态 | `Source/FrontendUI/Public/PlayerState/CharacterState.h` |
| GameMode, 游戏模式 | `Content/UI/BP_FrontendGameMode.uasset` |
| Build, 构建 | `Source/FrontendUI/FrontendUI.Build.cs` |
| SDataTableImportPanel, JSON导入, DataTable导入面板, xlsx转json | `Source/FrontendUI/Public/Editor/SDataTableImportPanel.h` |
| xlsx_to_json, Excel转换脚本 | `Content/Config/Tool/xlsx_to_json.py` |
| WBP_OptionScreen, 选项界面蓝图 | `Content/UI/Widgets/Options/WBP_CAW_OptionScreen.uasset` |
| WBP_TabList, Tab列表蓝图 | `Content/UI/Widgets/Options/WBP_CAW_TabList.uasset` |
| WBP_DetailsView, 详情面板蓝图 | `Content/UI/Widgets/Options/WBP_DetailsView_Options.uasset` |
| WBP_ListEntry_String, 字符串列表项蓝图 | `Content/UI/Widgets/Options/ListEntry/WBP_ListEntry_String.uasset` |
| DA_DadaListEntryMapping, 列表项映射资源 | `Content/UI/Widgets/Options/DA_DadaListEntryMapping.uasset` |
| WBP_PrimaryLayout, 主布局蓝图 | `Content/UI/Widgets/WBP_CUW_PrimaryLayout.uasset` |
| WBP_ConfirmScreen, 确认弹窗蓝图 | `Content/UI/Widgets/WBP_CAW_ConfirmScreen.uasset` |
| WBP_StoryScreen, 剧情界面蓝图 | `Content/UI/Widgets/WBP_CAW_StoryScreen.uasset` |
| WBP_MainMenu, 主菜单蓝图 | `Content/UI/Widgets/WBP_CAW_MainMenu.uasset` |
| WBP_PressAnyKey, 启动界面蓝图 | `Content/UI/Widgets/WBP_CAW_PressAnyKey.uasset` |
| InputData, 输入数据 | `Content/UI/CommonInputData/InputData_Default.uasset` |

---

## 搜索缓存

> 项目外文件（引擎源码、插件等）的搜索记录，超过一天需重新验证。

| 文件路径 | 关键词 | 时间戳 | 状态 |
|---------|--------|--------|------|
| `Engine/Plugins/Runtime/CommonUI/Source/CommonUI/Public/CommonRotator.h` | CommonRotator, 旋转器, UCommonRotator | 20260412 | 需重新验证 |
| `Engine/Plugins/Runtime/CommonUI/Source/CommonUI/Public/CommonTextBlock.h` | CommonTextBlock, MyText | 20260412 | 需重新验证 |
| `Engine/Source/Runtime/PropertyPath/Public/PropertyPathHelpers.h` | FCachedPropertyPath, PropertyPathHelpers, 属性路径 | 20260412 | 需重新验证 |
| `D:/Project/UnrealEngine/UE_5.7/Engine/Plugins/Runtime/CommonUI/Source/CommonUI/Public/Input/UIActionBindingHandle.h` | FUIInputConfig, InputConfig | 20260426 | 有效 |
| `D:/Project/UnrealEngine/UE_5.7/Engine/Plugins/Runtime/CommonUI/Source/CommonInput/Public/CommonInputModeTypes.h` | ECommonInputMode, Menu/Game/All | 20260426 | 有效 |
| `D:/Project/UnrealEngine/UE_5.7/Engine/Plugins/Runtime/CommonUI/Source/CommonUI/Public/CommonActivatableWidget.h` | UCommonActivatableWidget, GetDesiredInputConfig | 20260426 | 有效 |
