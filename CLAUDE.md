# FrontendUI 项目指引

## 项目概述

这是一个 Unreal Engine 5 前端 UI 模块项目，主要用于构建游戏前端界面系统。

---

## 协作规则

**重要**: 所有对话须严格遵守 [`C:\Users\Administrator\.claude\Rules\collaboration.md`](C:\Users\Administrator\.claude\Rules\collaboration.md) 中的协作规则：

1. 全部使用中文回复
2. 不能写兼容性代码，除非用户主动要求
3. 写代码前先提供方案再进行编写
4. 写完代码后，自己提供边缘情况和测试样例进行测试并列出
5. 修改超过三个文件的时候先拆分为子任务
6. 需求模糊时要先询问，获得明确回复后再写代码
7. 出 bug 后先写能重现的测试再进行修复
8. 每次被纠正后，反思并制定不再犯的计划，写进此文件里

---

## 项目结构索引

**搜索代码前优先查阅**: [`rules/structure.md`](rules/structure.md)

该文件记录了：
- 所有源代码文件路径及描述
- 配置文件位置
- 资源文件目录
- 搜索关键词映射表

### 结构文件更新规则

- 文件顶部包含 `Timestamp: YYYYMMDD` 格式的时间戳
- 每过一天需要更新结构文件，刷新 Timestamp 并检查是否有新增/删除的文件
- 新增代码文件时，应同步更新结构文件

---

## 目录结构速览

```
FrontendUI/
├── .claude/                 # Claude 配置
├── rules/                   # 项目规则与结构索引
│   └── structure.md         # 代码结构索引文件
├── Config/                  # 配置文件
├── Content/                 # 资源文件
├── Source/                  # 源代码
│   └── FrontendUI/          # 主模块
│       ├── Public/          # 公开头文件
│       │   ├── Controller/  # 控制器
│       │   ├── Widgets/     # 控件
│       │   ├── Subsystems/  # 子系统
│       │   ├── AsyncAction/  # 异步任务
│       │   └── FrontendSettings/ # 设置
│       └── Private/         # 私有实现
├── FrontendUI.uproject      # 项目文件
└── FrontendUI.sln           # VS解决方案
```