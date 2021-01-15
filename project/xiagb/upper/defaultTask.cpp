#include "main.h"
#include "cmsis_os2.h"

#include "led.hpp"

#include "cliTask.hpp"

namespace defaultTaskNamespace
{
    using xiagb::pilotLampNamespace::ledNamespace::ledClass;
    using xiagb::upper::cliTaskNamespace::cliTaskStart;

    //变量定义
    const osThreadAttr_t defaultTaskAttributes = {.name = "defaultTask", .stack_size = 128 * 4, .priority = (osPriority_t)osPriorityNormal};
    osTimerId_t systemPilotLampTimerHandle; //软件定时器句柄
    ledClass *systemPilotLampHandle; //指示灯句柄

    //函数声明
    void systemPilotLampTimerCallback(void *arg);

    //默认任务
    void defaultTask(void *arg)
    {
        /*led begin*/
        systemPilotLampHandle = new ledClass;
        systemPilotLampHandle->init(GPIOC, GPIO_PIN_13, 0);

        //led的rtos软定时器
        systemPilotLampTimerHandle = osTimerNew(systemPilotLampTimerCallback, osTimerPeriodic, nullptr, nullptr);
        osTimerStart(systemPilotLampTimerHandle, 1000); //每1000tick调用一次
        /*led end*/

        /*other task begin*/
        cliTaskStart();
        /*other task end*/

        osThreadExit(); //结束该线程
    }

    //启动默认任务
    extern "C" void defaultTaskStart()
    {
        osThreadNew(defaultTask, nullptr, &defaultTaskAttributes);
    }

    //led的回调函数
    void systemPilotLampTimerCallback(void *arg)
    {
        systemPilotLampHandle->toggle();
    }
} // namespace defaultTaskNamespace