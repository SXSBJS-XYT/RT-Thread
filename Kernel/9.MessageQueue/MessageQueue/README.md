# MessageQueue 工程

基于STM32F429 + RT-Thread的消息队列模板。

## 目录结构

```
MessageQueue/
├── Application/          # 应用层 - 业务逻辑、任务
│   ├── Inc/
│   └── Src/
├── Service/              # 服务层 - 功能模块封装
│   ├── Inc/
│   └── Src/
├── Driver/               # 驱动层 - 硬件抽象
│   ├── Inc/
│   └── Src/
├── Core/                 # CubeMX生成 - HAL配置
├── Drivers/              # CubeMX生成 - HAL库
├── Middlewares/          # 中间件 - RT-Thread
├── MDK-ARM/              # Keil工程文件
├── RT-Thread/            # RT-Thread配置
└── README.md             # 说明文档
```

## 代码分层说明

```
┌─────────────────────────────────────────┐
│            Application                  │  业务逻辑、线程任务
├─────────────────────────────────────────┤
│              Service                    │  LED、按键等功能模块
├─────────────────────────────────────────┤
│              Driver                     │  GPIO、UART等驱动封装
├─────────────────────────────────────────┤
│            Core / Drivers               │  CubeMX生成的HAL库
└─────────────────────────────────────────┘
```

**调用规则：上层调用下层，禁止跨层调用。**

---

## 开发环境

| 工具         | 版本 | 用途             |
| ------------ | ---- | ---------------- |
| Keil MDK     | 5.26 | 编译、调试、烧录 |
| VSCode       | 最新 | 代码编辑、格式化 |
| STM32CubeMX  | 6.15 | 初始化代码生成   |
| clang-format | 最新 | 代码格式化       |

### VSCode扩展

| 扩展名       | 作者         | 用途              |
| ------------ | ------------ | ----------------- |
| C/C++        | Microsoft    | C语言支持         |
| EditorConfig | EditorConfig | 读取.editorconfig |
| Clang-Format | xaver        | 代码格式化        |

---

## 快速开始

### 1. 克隆仓库

```bash
git clone https://github.com/SXSBJS-XYT/RT-Thread.git
cd .\Kernel\9.MessageQueue\MessageQueue\
```

### 2. 打开工程
kprintf
- **Keil：** 打开 `Kernel/9.MessageQueue/MessageQueue/MDK-ARM/MessageQueue.uvprojx`
- **VSCode：** 打开 `RT-Thread` 文件夹

### 3. 编译烧录

在Keil中按 `F7` 编译，`F8` 烧录。

## 维护者

- [SXSBJS-XYT](https://github.com/SXSBJS-XYT)
