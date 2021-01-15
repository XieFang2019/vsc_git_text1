#include "cliTask.hpp"
#include "main.h"
#include "cmsis_os2.h"

#include "cli.hpp"
#include "can.hpp"

#include "machineTask.hpp"

extern CAN_HandleTypeDef hcan;
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;

namespace xiagb
{
    namespace upper
    {
        namespace cliTaskNamespace
        {
            using xiagb::communicationNamespace::canNamespace::canClass;
            using xiagb::communicationNamespace::uartNamespace::uartClass;
            using xiagb::parserNamespace::cliNamespace::cliClass;
            using xiagb::upper::machineTaskNamespace::machineTaskStart;

            canClass *canCommunication;
            cliClass *commandParser;
            uartClass *uartCommunication;

            uint8_t const commandQueueNum = 1;    //有几个从机就写几
            uint8_t const salveCanIdBeginNum = 2; //从机从Id几开始的
            const osThreadAttr_t cliTaskAttributes = {.name = "cliTask", .stack_size = 128 * 4, .priority = (osPriority_t)osPriorityNormal};
            osMessageQueueId_t masterCommandQueue;                 //主机的命令的队列
            osMessageQueueId_t salveCommandQueue[commandQueueNum]; //从机反馈的命令的队列

            //获取主机的命令并放入cli
            void getMasterCommandAndputCli(osMessageQueueId_t srcQueue, osMessageQueueId_t InputQueue)
            {
                uint8_t buffer, dataCount;
                while (osMessageQueueGet(srcQueue, &dataCount, nullptr, 0) == osOK) //获取数据数量
                {
                    if (dataCount == 0) //结束符就直接跳出
                        return;

                    for (uint8_t i = 0; i < dataCount; i++)
                    {
                        osMessageQueueGet(srcQueue, &buffer, nullptr, osWaitForever); //获取数据
                        osMessageQueuePut(InputQueue, &buffer, 0, osWaitForever);     //放入队列
                    }

                    if (buffer == '\n') //如果是结尾就把这个队列里的数据全部放入cli
                        commandParser->receiveQueueStr(InputQueue);
                }
            }

            //获取从机反馈的命令并放入cli
            void getSalveCommandAndputCli(osMessageQueueId_t srcQueue, osMessageQueueId_t *InputQueue)
            {
                uint8_t buffer, id, dataCount;
                while (osMessageQueueGet(srcQueue, &id, nullptr, 0) == osOK) //获取id
                {
                    osMessageQueueGet(srcQueue, &dataCount, nullptr, osWaitForever); //获取数据个数
                    for (uint8_t i = 0; i < dataCount; i++)
                    {
                        osMessageQueueGet(srcQueue, &buffer, nullptr, osWaitForever);                      //获取数据
                        osMessageQueuePut(InputQueue[id - salveCanIdBeginNum], &buffer, 0, osWaitForever); //放入队列
                    }

                    if (buffer == '\n') //如果是结尾就把这个队列里的数据全部放入cli
                        commandParser->receiveQueueStr(InputQueue[id - salveCanIdBeginNum]);
                }
            }

            void cliTask(void *arg)
            {
                uartCommunication = new uartClass;
                uartCommunication->init(USART1, 115200, &huart1, &hdma_usart1_rx, &hdma_usart1_tx);

                canCommunication = new canClass;
                canCommunication->init(CAN1, 1, &hcan);

                commandParser = new cliClass;
                commandParser->init(uartCommunication);

                machineTaskStart(); //启动命令执行任务

                //队列里只存一条完整的命令
                //创建主机命令队列
                masterCommandQueue = osMessageQueueNew(100, sizeof(uint8_t), nullptr); //单次命令不能超过100字节
                while (masterCommandQueue == 0)
                    ;
                //为每个从机创建一个队列，用以接收其反馈信息
                for (uint8_t i = 0; i < commandQueueNum; i++)
                {
                    salveCommandQueue[i] = osMessageQueueNew(100, sizeof(uint8_t), nullptr); //单次命令不能超过100字节
                    while (salveCommandQueue[i] == 0)
                        ;
                }

                for (;;)
                {
                    osDelay(1);

                    getMasterCommandAndputCli(uartCommunication->getRxQueue(), masterCommandQueue);
                    getSalveCommandAndputCli(canCommunication->getRxQueue(), salveCommandQueue);

                    commandParser->loop();
                }
            }

            // 串口1的空闲中断回调函数
            // 放到串口1的全局中断里
            extern "C" void uart1IrqCallback()
            {
                uartCommunication->idleCallback();
            }

            // hal库uart接收完成回调函数
            extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
            {
                if (huart == &huart1)
                    uartCommunication->rxCompleteCallback();
            }

            extern "C" void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
            {
                canCommunication->receiveCallback();
            }

            void cliTaskStart()
            {
                osThreadNew(cliTask, nullptr, &cliTaskAttributes);
            }
        } // namespace cliTaskNamespace
    }     // namespace upper
} // namespace xiagb