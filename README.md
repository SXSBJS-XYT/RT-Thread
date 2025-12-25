# RT-Thread
> 本仓库是作者在RT-Thread学习与实践过程中诞生的产物，Kernel文件夹下项目基于STM32F429搭载RT-Thread的Nano版本进行的Demo工程开发，Device文件夹下的项目在RT-Thread Studio进行开发。

- RT-Thread的内核原理、设备与驱动理论详情见：[RT-Thread官方文档](https://www.rt-thread.org/document/site/)

- RT-Thread API参考手册：[RT-Thread API参考手册](https://www.rt-thread.org/document/api/)

- 在此也特别推荐野火的[《RT-Thread内核实现与应用开发实战》](/RT-Thread/[野火®]《RT-Thread%20内核实现与应用开发实战—基于STM32》.pdf)教程

## Demo工程
**Kernel/**
- CreateThread
- UartPrintf
- ThreadManagement
- Semaphore
- PriorityInversion
- Mutex
- Event
- Mailbox
- MessageQueue
- Signal
- SmallmemTutorial
- MempoolTutorial

**Device/**
- Dev_Pin
- Dev_Uart

## 工程目录结构
```
RT-Thread/
├── .clang-format             # C代码格式化规则
├── .editorconfig             # 编辑器基础配置
├── .gitattributes            # Git换行符配置
├── .gitignore                # Git忽略规则
├── .vscode/                  # VSCode配置
├── LICENSE                   # 许可证
├── Kernel/                   # 内核工程
│    ├── 1.CreateThread/       # 线程创建示例
│    ├── 2.UartPrintf/         # 串口打印示例
│    ├── 3.ThreadManagement/   # 线程管理示例
│    ├── 4.Semaphore/          # 信号量示例
│    ├── 5.PriorityInversion/  # 优先级翻转示例
│    ├── 6.Mutex/              # 互斥量示例
│    ├── 7.Event/              # 事件示例
│    ├── 8.Mailbox/            # 邮箱示例
│    ├── 9.MessageQueue/       # 消息队列示例
│    ├── 10.Signal/            # 信号示例
│    ├── 11.SmallmemTutorial   # 小内存算法示例
│    └── 12.MempoolTutorial    # 内存池算法示例
├── Device/                   # 设备驱动工程
│    ├── 1.Dev_Pin/            # PIN设备示例
│    └── 2.Dev_Uart/           # UART设备示例
└── README.md                 # 工程说明文档
```

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
| RT-Thread Studio  | 2.2.9 | 编译、调试、烧录   |
| clang-format | 最新 | 代码格式化       |

### VSCode扩展

| 扩展名       | 作者         | 用途              |
| ------------ | ------------ | ----------------- |
| C/C++        | Microsoft    | C语言支持         |
| EditorConfig | EditorConfig | 读取.editorconfig |
| Clang-Format | xaver        | 代码格式化        |

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
