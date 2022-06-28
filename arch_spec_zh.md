```
.
├── arch                                                        --- 内核指令架构层代码
│   ├── arm                                                     --- ARM32架构
│   │   ├── arm9                                                --- arm9架构
│   │   │   └── gcc                                             --- gcc 编译工具链实现
│   │   ├── cortex-m3                                           --- Cortex-m3架构
│   │   │   └── keil                                            --- Keil编译工具链实现
│   │   ├── cortex-m33                                          --- Cortex-m33架构
│   │   │   │── gcc                                             --- GCC编译工具链实现
│   │   │   │   │── NTZ                                         --- Cortex-m33非TrustZone架构实现
│   │   │   │   └── TZ                                          --- Cortex-m33 TrustZone架构实现
│   │   │   └── iar                                             --- IAR编译工具链实现
│   │   │       │── NTZ                                         --- Cortex-m33非TrustZone架构实现
│   │   │       └── TZ                                          --- Cortex-m33 TrustZone架构实现
│   │   └── cortex-m4                                           --- Cortex-m4架构
│   │   │   │── gcc                                             --- GCC编译工具链实现
│   │   │   └── iar                                             --- IAR编译工具链实现
│   │   └── cortex-m7                                           --- Cortex-m7架构
│   │       │── gcc                                             --- GCC编译工具链实现
│   │       └── iar                                             --- IAR编译工具链实现
│   ├── csky                                                    --- csky架构
│   │   └── v2                                                  --- csky v2架构
│   │       └── gcc                                             --- GCC编译工具链实现
│   ├── xtensa                                                  --- xtensa架构
│   │   └── lx6                                                 --- xtensa lx6架构
│   │       └── gcc                                             --- GCC编译工具链实现
│   ├── risc-v                                                  --- Risc-v架构
│   │   ├── nuclei                                              --- Nuclei架构
│   │   │   └── gcc                                             --- GCC编译工具链实现
│   │   └── riscv32                                             --- Riscv32架构
│   │       └── gcc                                             --- GCC编译工具链实现
│   └── include
│       ├── los_arch.h                                          --- 定义arch初始化
│       ├── los_atomic.h                                        --- 定义通用arch原子操作
│       ├── los_context.h                                       --- 定义通用arch上下文切换
│       ├── los_interrupt.h                                     --- 定义通用arch中断
│       ├── los_mpu.h                                           --- 定义通用arch内存保护
│       └── los_timer.h                                         --- 定义通用arch定时器
├── components                                                  --- 移植可选组件，依赖内核，单独对外提供头文件
│   ├── backtrace                                               --- 栈回溯功能
│   ├── cppsupport                                              --- C++支持
│   ├── cpup                                                    --- CPUP功能
│   ├── dynlink                                                 --- 动态加载与链接
│   ├── exchook                                                 --- 异常钩子
│   ├── fs                                                      --- 文件系统
│   ├── lmk                                                     --- Low memory killer 机制
│   ├── lms                                                     --- Lite memory sanitizer 机制
│   ├── net                                                     --- Network功能
│   ├── power                                                   --- 低功耗管理
│   ├── security                                                --- 安全隔离
│   ├── shell                                                   --- shell功能
│   ├── signal                                                  --- signal支持
│   └── trace                                                   --- trace 工具
├── drivers                                                     --- 驱动框架Kconfig
├── kal                                                         --- 内核抽象层，提供内核对外接口，当前支持CMSIS接口和部分POSIX接口
│   ├── cmsis                                                   --- CMSIS标准支持
│   └── posix                                                   --- POSIX标准支持
├── kernel                                                      --- 内核最小功能集代码
│   ├── include
│   │   ├── los_config.h                                        --- 功能开关和配置参数
│   │   ├── los_event.h                                         --- 事件
│   │   ├── los_membox.h                                        --- 静态内存管理
│   │   ├── los_memory.h                                        --- 动态内存管理
│   │   ├── los_mux.h                                           --- 互斥锁
│   │   ├── los_queue.h                                         --- 队列
│   │   ├── los_sched.h                                         --- 调度算法
│   │   ├── los_sem.h                                           --- 信号量
│   │   ├── los_sortlink.h                                      --- 排序链表
│   │   ├── los_swtmr.h                                         --- 定时器
│   │   ├── los_task.h                                          --- 任务
│   │   └── los_tick.h                                          --- Tick时钟
│   └── src
├── testsuites                                                  --- 内核测试用例
├── tools                                                       --- 内核工具
└── utils
    ├── internal
    ├── BUILD.gn                                                --- Gn构建文件
    ├── los_compiler.h                                          --- 编译工具配置，类型定义
    ├── los_debug.c                                             --- Debug，printf相关
    ├── los_debug.h
    ├── los_error.c                                             --- 错误处理
    ├── los_error.h
    ├── los_hook.c                                              --- 钩子函数注册和调用
    ├── los_hook.h
    ├── los_list.h                                              --- 双向链表
    └── los_reg.h                                               --- 寄存器读写宏定义
```

