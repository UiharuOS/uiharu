#ifndef KERNEL_DEBUG_H
#define KERNEL_DEBUG_H

// debugger 函数, 打印debugger调试信息
void debugger(char* filename, int line, const char* func, const char* condition);
#define DEBUGGE(...) debugger(__FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef NO_DEBUG
    #define ASSERT(CONDITION) ((void)0)
#else
    #define ASSERT(CONDITION)       \
        if (CONDITION) {            \
        } else {                    \
            DEBUGGE(#CONDITION);    \
        }

#endif

#endif
