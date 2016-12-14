#include "debug.h"
#include "print.h"
#include "interrupt.h"

void debugger(char* filename,
              int line,
              const char* func,
              const char* condition) {
    /* uiharuos god debugger:) */
    intr_disable();  // 关中断, 避免其它信息干扰
    print_string("\n\n\n=============== bug! bug! bug! ===============\n");
    print_string("Debugger)-> oh,god, you stepped on the bug!\n");
    print_string("Debugger)-> <FILENAME> ");  print_string(filename);
    print_string("\n");
    print_string("Debugger)-> <LINEON> ");    print_int(line, 'D');
    print_string("\n");
    print_string("Debugger)-> <FUNCTION> ");  print_string(func);
    print_string("\n");
    print_string("Debugger)-> <CONDITION> "); print_string(condition);
    print_string("\n");
    print_string("Debugger)-> please fix me, please!");
    print_string("\n\n\n=============== bug! bug! bug! ===============\n");

    while(1); // 挂起, 内核都有bug了还干什么!
}
