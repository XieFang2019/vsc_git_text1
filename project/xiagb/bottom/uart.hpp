#ifndef uartH
#define uartH

#include "main.h"
#include "cmsis_os2.h"

/*
打开dma的发送和接收，循环模式，使能硬件的全局中断
空闲中断处理最后的数据，串口完成中断处理过程数据
空闲中断只会被接收触发，不会被发送触发
dma接收完成中断最后调用的是串口接收完成的回调函数，发送是调用串口发送完成的
第一个字节是数据个数，后面的是数据，如果数据个数为0，说明是空闲中断，这次数据传输就结束了
dma得到的个数是继续上一次的

如果空闲和完成同时触发？接收完成先辈调用
*/

namespace xiagb
{
    namespace communicationNamespace
    {
        namespace uartNamespace
        {
            class uartClass
            {
            private:
                uint8_t rxBufferSize = 20;  //单次缓存容量
                uint16_t rxQueueSize = 300; //队列容量
                uint16_t printfBufferSize = 80;

                UART_HandleTypeDef *uartHandle;
                DMA_HandleTypeDef *dmaRxHandle, *dmaTxHandle;
                osMessageQueueId_t rxQueueHandle;
                osMutexId_t mutexHandle;
                uint8_t *rxBuffer;
                uint8_t *printfBuffer;

            public:
                uartClass();
                ~uartClass();

                void init(USART_TypeDef *uartHardwareHandle, uint32_t baudRate, UART_HandleTypeDef *uartHandle, DMA_HandleTypeDef *dmaRxHandle, DMA_HandleTypeDef *dmaTxHandle);
                void idleCallback();       //在全局中断里执行
                void rxCompleteCallback(); //在HAL_UART_RxCpltCallback里执行
                void send(uint8_t *data, uint16_t size);
                void printf(char *fmt, ...);

                osMessageQueueId_t getRxQueue() { return rxQueueHandle; }
            };
        } // namespace uartNamespace
    }     // namespace communicationNamespace
} // namespace xiagb

// 串口1的空闲中断回调函数
// 放到串口1的全局中断里
// extern "C" void uart1IrqCallback()
// {
//     sysCommunicationHandle->idleCallback();
// }

// hal库接收完成回调函数
// extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
// {
//     if (huart == &huart1)
//         sysCommunicationHandle->rxCompleteCallback();
// }
#endif