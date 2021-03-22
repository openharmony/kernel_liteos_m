```
.
├── components                                                  --- Optional components
│   ├── cppsupport                                              --- C++
│   └── cpup                                                    --- CPUP
├── kal                                                         --- Kernel Abstraction Layer
│   ├── cmsis                                                   --- cmsis
│   └── posix                                                   --- posix
├── kernel                                                      --- Minimalistic kernel funtionalities
│   ├── arch                                                    --- Support for hardware architectures
│   │   ├── arm                                                 --- arm32
│   │   │   └── cortex-m4                                       --- cortex-m4
│   │   │       └── iar                                         
│   │   │           ├── los_atomic.h
│   │   │           ├── los_context.h
│   │   │           ├── los_interrupt.h
│   │   │           └── los_mpu.h
│   │   └── include
│   │       ├── los_arch_atomic.h                               --- Atomic operations
│   │       ├── los_arch_context.h                              --- Context switch
│   │       ├── los_arch.h                                      --- Initialization
│   │       └── los_arch_interrupt.h                            --- Interrupts
│   ├── include
│   │   ├── los_config.h                                        --- Configuration parameters
│   │   ├── los_event.h                                         --- Events management
│   │   ├── los_liteos.h                                        --- Kernel types and functions
│   │   ├── los_memory.h                                        --- Heap memory management
│   │   ├── los_mutex.h                                         --- Mutex
│   │   ├── los_queue.h                                         --- Queue
│   │   ├── los_scheduler.h                                     --- Scheduler
│   │   ├── los_sem.h                                           --- Semaphores
│   │   ├── los_task.h                                          --- Tasks
│   │   └── los_timer.h                                         --- Timer
│   └── src
├── targets
│   └── targets
│       └── cortex-m4_stm32f429ig_fire-challenger_iar
│           ├── board
│           ├── dprintf.c
│           ├── Libraries
│           ├── main.c
│           ├── project
│           ├── target_config.h                                 --- Hardware target configuration
│           └── Utilities
└── utils
    ├── include
    │   ├── los_compiler.h                                      --- Compiler configuration
    │   ├── los_debug.h                                         --- Debugging facilities
    │   ├── los_error.h                                         --- Errors codes and definitions
    │   └── los_list.h
    └── src
```
