```
.
├── components                                                  --- 可选组件，可裁剪，依赖kernel
│   ├── cppsupport                                              --- C++支持
│   └── cpup                                                    --- CPUP功能
├── kal                                                         --- 内核抽象层
│   ├── cmsis                                                   --- cmsis标准支持
│   └── posix                                                   --- posix标准支持
├── kernel                                                      --- 内核最小功能集支持
│   ├── arch                                                    --- 硬件架构相关
│   │   ├── arm                                                 --- arm32架构
│   │   │   └── cortex-m4                                       --- cortex-m4架构
│   │   │       └── iar                                         ---
│   │   │           ├── los_atomic.h
│   │   │           ├── los_context.h
│   │   │           ├── los_interrupt.h
│   │   │           └── los_mpu.h
│   │   └── include
│   │       ├── los_arch_atomic.h                               --- 定义通用arch的原子操作
│   │       ├── los_arch_context.h                              --- 定义通用arch的上下文切换
│   │       ├── los_arch.h                                      --- 定义通用arch初始化
│   │       └── los_arch_interrupt.h                            --- 定义通用arch中断
│   ├── include
│   │   ├── los_config.h                                        --- 功能开关和配置参数
│   │   ├── los_event.h                                         --- 事件
│   │   ├── los_liteos.h                                        --- liteos最小功能集对外提供的头文件
│   │   ├── los_memory.h                                        --- 堆内存管理
│   │   ├── los_mutex.h                                         --- 互斥锁
│   │   ├── los_queue.h                                         --- 队列
│   │   ├── los_scheduler.h                                     --- 调度算法
│   │   ├── los_sem.h                                           --- 信号量
│   │   ├── los_task.h                                          --- 任务
│   │   └── los_timer.h                                         --- 定时器
│   └── src
├── targets
│   └── targets
│       └── cortex-m4_stm32f429ig_fire-challenger_iar
│           ├── board
│           ├── dprintf.c
│           ├── Libraries
│           ├── main.c
│           ├── project
│           ├── target_config.h                                 --- 板级配置功能开关和配置参数
│           └── Utilities
└── utils
    ├── include
    │   ├── los_compiler.h                                      --- 编译工具配置，类型定义
    │   ├── los_debug.h                                         --- debug，printf相关
    │   ├── los_error.h                                         --- 错误定义
    │   └── los_list.h
    └── src
```
