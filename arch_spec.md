```
.
├── arch                                                        --- Code of the kernel instruction architecture layer
│   ├── arm                                                     --- ARM32 architecture
│   │   ├── arm9                                                --- ARM9 architecture
│   │   │   └── gcc                                             --- Implementation of the GCC toolchain
│   │   ├── cortex-m3                                           --- Cortex-m3 architecture
│   │   │   └── keil                                            --- Implementation of the keil toolchain
│   │   ├── cortex-m33                                          --- Cortex-m33 architecture
│   │   │   │── gcc                                             --- Implementation of the GCC toolchain
│   │   │   │   │── NTZ                                         --- Cortex-m33 Non-TrustZone architecture
│   │   │   │   └── TZ                                          --- Cortex-m33 TrustZone architecture
│   │   │   └── iar                                             --- Implementation of the IAR toolchain
│   │   │       │── NTZ                                         --- Cortex-m33 Non-TrustZone architecture
│   │   │       └── TZ                                          --- Cortex-m33 TrustZone architecture
│   │   └── cortex-m4                                           --- Cortex-m4 architecture
│   │   │   │── gcc                                             --- Implementation of the GCC toolchain
│   │   │   └── iar                                             --- Implementation of the IAR toolchain
│   │   └── cortex-m7                                           --- Cortex-m7 architecture
│   │       │── gcc                                             --- Implementation of the GCC toolchain
│   │       └── iar                                             --- Implementation of the IAR toolchain
│   ├── csky                                                    --- csky architecture
│   │   └── v2                                                  --- csky v2 architecture
│   │       └── gcc                                             --- Implementation of the GCC toolchain
│   ├── xtensa                                                  --- xtensa architecture
│   │   └── lx6                                                 --- xtensa lx6 architecture
│   │       └── gcc                                             --- Implementation of the GCC toolchain
│   ├── risc-v                                                  --- Risc-v architecture
│   │   ├── nuclei                                              --- Nuclei architecture
│   │   │   └── gcc                                             --- Implementation of the GCC toolchain
│   │   └── riscv32                                             --- Riscv32 architecture
│   │       └── gcc                                             --- Implementation of the GCC toolchain
│   └── include
│       ├── los_arch.h                                          --- Arch initialization
│       ├── los_atomic.h                                        --- Atomic operations
│       ├── los_context.h                                       --- Context switch
│       ├── los_interrupt.h                                     --- Interrupts
│       ├── los_mpu.h                                           --- Memory protection unit operations
│       └── los_timer.h                                         --- Timer operations
├── components                                                  --- Components available for porting and header files exposed externally
│   ├── backtrace                                               --- Backtrace support
│   ├── cppsupport                                              --- C++ support
│   ├── cpup                                                    --- CPU percent (CPUP)
│   ├── dynlink                                                 --- Dynamic loading and linking
│   ├── exchook                                                 --- Exception hooks
│   ├── fs                                                      --- File systems
│   ├── lmk                                                     --- Low memory killer functions
│   ├── lms                                                     --- Lite memory sanitizer functions
│   ├── net                                                     --- Networking functions
│   ├── power                                                   --- Power management
│   ├── security                                                --- Security isolation
│   ├── shell                                                   --- Shell function
│   ├── signal                                                  --- Signal support
│   ├── fs                                                      --- File systems
│   └── trace                                                   --- Trace tool
├── drivers                                                     --- driver Kconfig
├── kal                                                         --- Kernel Abstraction Layer, APIs exposed externally, including CMSIS APIs and part of POSIX APIs
│   ├── cmsis                                                   --- CMSIS
│   └── posix                                                   --- POSIX
├── kernel                                                      --- Code for defining the minimum kernel function set
│   ├── include
│   │   ├── los_config.h                                        --- Configuration parameters
│   │   ├── los_event.h                                         --- Events management
│   │   ├── los_membox.h                                        --- Membox management
│   │   ├── los_memory.h                                        --- Heap memory management
│   │   ├── los_mux.h                                           --- Mutex
│   │   ├── los_queue.h                                         --- Queue
│   │   ├── los_sched.h                                         --- Scheduler
│   │   ├── los_sem.h                                           --- Semaphores
│   │   ├── los_sortlink.h                                      --- Sort link
│   │   ├── los_swtmr.h                                         --- Timer
│   │   ├── los_task.h                                          --- Tasks
│   │   └── los_tick.h                                          --- Tick
│   └── src
├── targets
│   └── riscv_nuclei_demo_soc_gcc
│   │   ├── GCC                                            --- Compilation config
│   │   ├── OS_CONFIG                                      --- Board config
│   │   ├── SoC                                            --- SOC codes
│   │   └── Src                                            --- Application codes
│   └── riscv_nuclei_gd32vf103_soc_gcc
│   └── riscv_sifive_fe310_gcc
├── testsuites                                                 --- Kernel testsuites
├── tools                                                      --- Kernel tools
└── utils
    ├── internal
    ├── BUILD.gn                                               --- Gn build config file
    ├── los_compiler.h                                         --- Compiler configuration
    ├── los_debug.c                                            --- Debugging facilities
    ├── los_debug.h
    ├── los_error.c                                            --- Errors codes and definitions
    ├── los_error.h
    ├── los_hook.c                                             --- Hook function facilities
    ├── los_hook.h
    ├── los_list.h                                             --- Doubly linked list
    └── los_reg.h                                              --- Register macros
```

