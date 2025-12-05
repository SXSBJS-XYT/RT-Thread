# RT-Thread
基于STM32F429 + RT-Thread的工程模板。

## 现有工程
- CreateThread
- UartPrintf
- ThreadManagement
- Semaphore

## 工程目录结构
```
RT-Thread/
├── .clang-format             # C代码格式化规则
├── .editorconfig             # 编辑器基础配置
├── .gitattributes            # Git换行符配置
├── .gitignore                # Git忽略规则
├── .vscode/                  # VSCode配置
├── LICENSE                   # 许可证
├── README.md                 # 工程说明文档
└── Kernel/                   # 内核工程
    ├── 1.CreateThread/       # 线程创建示例
    ├── 2.UartPrintf/         # 串口打印示例
    ├── 3.ThreadManagement/   # 线程管理示例
    └── 4.Semaphore/          # 信号量示例



```
## 子工程目录结构

```
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
└── README.md             # 子工程说明文档
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

## 快速开始

### 1. 克隆仓库

```bash
git clone https://github.com/SXSBJS-XYT/RT-Thread.git
```

### 2. 打开项目子工程

### 3. 编译烧录

在Keil中按 `F7` 编译，`F8` 烧录。

---

## 配置文件说明

### .editorconfig

**作用：** 统一编辑器基础配置

| 配置项               | 值    | 说明           |
| -------------------- | ----- | -------------- |
| charset              | utf-8 | 文件编码       |
| indent_style         | space | 用空格缩进     |
| indent_size          | 2     | 缩进2个空格    |
| end_of_line          | crlf  | Windows换行符  |
| insert_final_newline | true  | 文件末尾加换行 |

---

### .clang-format

**作用：** C代码格式化规则

| 规则       | 说明                  |
| ---------- | --------------------- |
| 大括号风格 | Allman（独占一行）    |
| 缩进       | 2空格                 |
| 行宽       | 100字符               |
| 指针对齐   | 靠近变量名 `int *ptr` |

**使用方法：**
- VSCode中按 `Shift + Alt + F` 格式化
- 保存时自动格式化（需配置VSCode）

**格式化效果：**

```c
// 格式化前
void func(void){
if(a==1){
b=2;
}
}

// 格式化后
void func(void)
{
  if (a == 1)
  {
    b = 2;
  }
}
```

---

### .gitignore

**作用：** 告诉Git忽略哪些文件

| 忽略内容          | 原因                     |
| ----------------- | ------------------------ |
| *.o, *.d, *.axf   | 编译中间产物，可重新生成 |
| *.uvguix.*        | Keil界面布局，每人不同   |
| /MDK-ARM/Objects/ | 编译输出目录             |
| Thumbs.db         | Windows系统文件          |

**好处：**
- 仓库体积小
- 避免无意义的冲突
- 保持仓库整洁

---

### .gitattributes

**作用：** 统一Git换行符处理

| 文件类型     | 换行符 | 说明         |
| ------------ | ------ | ------------ |
| *.c, *.h     | CRLF   | Keil兼容     |
| *.bin, *.hex | binary | 二进制不转换 |

**解决问题：** 跨平台协作时换行符不一致

---

### .vscode/settings.json

**作用：** VSCode工程配置

| 配置项                  | 说明             |
| ----------------------- | ---------------- |
| editor.formatOnSave     | 保存时自动格式化 |
| files.encoding          | UTF-8编码        |
| files.eol               | CRLF换行符       |
| clang-format.executable | 格式化工具路径   |

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

## 代码规范

### 命名规范

| 类型       | 规范        | 示例             |
| ---------- | ----------- | ---------------- |
| 文件名     | 小写+下划线 | `app_task.c`     |
| 函数名     | 小写+下划线 | `srv_led_on()`   |
| 宏定义     | 大写+下划线 | `LED_ACTIVE_LOW` |
| 结构体类型 | 小写+_t后缀 | `srv_led_t`      |

### 文件前缀

| 层级        | 前缀 | 示例         |
| ----------- | ---- | ------------ |
| Application | app_ | `app_task.c` |
| Service     | srv_ | `srv_led.c`  |
| Driver      | drv_ | `drv_gpio.c` |

---

## 常见问题

### Q: Keil编译警告 "last line ends without newline"

**A:** 用VSCode打开文件，保存一次即可（会自动添加末尾换行）。

### Q: 中文注释乱码

**A:**
1. Keil: Edit → Configuration → Editor → Encoding → UTF-8
2. 用VSCode重新保存文件

### Q: VSCode格式化不生效

**A:**
1. 确认安装了 clang-format：`pip install clang-format`
2. 确认安装了 VSCode扩展：Clang-Format (xaver)
3. 重启VSCode

---

## 维护者

- [SXSBJS-XYT] (https://github.com/SXSBJS-XYT)

## 许可证

[MIT License](LICENSE)
