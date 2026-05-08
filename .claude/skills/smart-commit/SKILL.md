---
name: smart-commit
description: "分析代码变更，自动生成 commit 信息，提交并推送到远程仓库。"
risk: safe
source: self
user_invocable: true
---

# Smart Commit

自动分析当前代码变更，生成语义化的 commit 信息，然后提交并推送到远程仓库。

## 触发条件

当用户说以下内容时触发：
- "git commit"
- "/smart-commit"
- "提交代码"
- "commit"

## 执行步骤

1. **收集变更信息**
   - 运行 `git status` 查看工作区状态
   - 运行 `git diff` 查看未暂存的变更
   - 运行 `git diff --cached` 查看已暂存的变更
   - 运行 `git log --oneline -5` 查看最近的提交历史，了解项目的 commit 风格

2. **分析变更内容**
   - 识别变更的类型：新功能(feature)、修复(fix)、重构(refactor)、文档(docs)、样式(style)等
   - 识别变更涉及的模块/文件
   - 理解变更的目的和影响

3. **生成 commit 信息**
   - 使用中文撰写 commit 信息
   - 格式：`<类型>: <简要描述>`
   - 类型包括：feature, fix, refactor, docs, style, chore, perf, test
   - 描述应聚焦于"为什么"改，而非"改了什么"
   - 如果变更涉及多个方面，按重要性选择主要类型

4. **执行提交**
   - 如果没有暂存文件，先用 `git add` 添加所有变更文件（排除不应提交的文件如 .env、密钥等）
   - 使用 `git commit` 提交，commit 信息末尾加上 `Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>`
   - 运行 `git status` 验证提交成功

5. **推送到远程**
   - 执行 `git push` 推送到 origin/main
   - 确认推送结果

## 注意事项

- 如果没有变更，告知用户无需提交
- 如果变更文件可能包含敏感信息（.env、credentials 等），警告用户
- 不要使用 `--no-verify` 跳过 hooks
- 始终创建新提交，不要 amend 已有提交
- commit 信息使用中文
