#include "uart.hpp"
#include <stdio.h>
#include <stdarg.h>
namespace xiagb
{
    namespace communicationNamespace
    {
        namespace uartNamespace
        {
            uartClass::uartClass()
            {
            }

            uartClass::~uartClass()
            {
            }

            void uartClass::init(USART_TypeDef *uartHardwareHandle, uint32_t baudRate, UART_HandleTypeDef *uartHandle, DMA_HandleTypeDef *dmaRxHandle, DMA_HandleTypeDef *dmaTxHandle)
            {
                rxQueueHandle = osMessageQueueNew(rxQueueSize, sizeof(uint8_t), nullptr);
                while (rxQueueHandle == 0)
                    ;

                mutexHandle = osMutexNew(nullptr);
                while (mutexHandle == 0)
                    ;

                rxBuffer = new uint8_t[rxBufferSize];
                while (rxBuffer == 0)
                    ;

                printfBuffer = new uint8_t[printfBufferSize];
                while (printfBuffer == 0)
                    ;

                this->uartHandle = uartHandle;
                this->dmaRxHandle = dmaRxHandle;
                this->dmaTxHandle = dmaTxHandle;

                uartHandle->Instance = uartHardwareHandle;
                uartHandle->Init.BaudRate = baudRate;
                uartHandle->Init.WordLength = UART_WORDLENGTH_8B;
                uartHandle->Init.StopBits = UART_STOPBITS_1;
                uartHandle->Init.Parity = UART_PARITY_NONE;
                uartHandle->Init.Mode = UART_MODE_TX_RX;
                uartHandle->Init.HwFlowCtl = UART_HWCONTROL_NONE;
                uartHandle->Init.OverSampling = UART_OVERSAMPLING_16;
                HAL_UART_Init(uartHandle); //初始化串口参数

                HAL_UART_Receive_DMA(uartHandle, rxBuffer, rxBufferSize); //启动dma
                __HAL_UART_ENABLE_IT(uartHandle, UART_IT_IDLE);           //开启空闲中断
            }

            void uartClass::send(uint8_t *data, uint16_t size)
            {
                if (size == 0) //如果长度为0，等待发送完成就会成为死循环，因为这时标志是不会变的
                    return;

                osMutexAcquire(mutexHandle, osWaitForever); //获取互斥锁

                // //必须等待。防止HAL_UART_Transmit_DMA重设DMA导致先前的数据终止发送
                // while(HAL_DMA_GetState(dmaTxHandle) == HAL_DMA_STATE_BUSY) //等待DMA完成
                //     osDelay(1);

                HAL_UART_Transmit_DMA(uartHandle, data, size);

                //必须在这等待，而不能在HAL_UART_Transmit_DMA上面，防止函数退出导致认为传输完成，这样别的程序可能会修改data里的数据
                while (!__HAL_UART_GET_FLAG(uartHandle, UART_FLAG_TC)) //等待串口完成
                    osDelay(1);
                __HAL_UART_CLEAR_FLAG(uartHandle, UART_FLAG_TC); //清除标志

                osMutexRelease(mutexHandle); //释放互斥锁
            }

            void uartClass::idleCallback()
            {
                if (__HAL_UART_GET_FLAG(uartHandle, UART_FLAG_IDLE)) //空闲标志
                {
                    // idleFlag = true; //触发了空闲标志
                    __HAL_UART_CLEAR_IDLEFLAG(uartHandle);                            //清除空闲标志
                    uint8_t size = rxBufferSize - __HAL_DMA_GET_COUNTER(dmaRxHandle); //本次的数据数量

                    if (size != 20) //如果接收了20个，数据会被完成中断处理完了，这里就不用再提取数据了
                    {
                        //第一个字节为这次数据的长度
                        while (osMessageQueuePut(rxQueueHandle, &size, 0, 0) != osOK)
                            ;
                        for (uint8_t i = 0; i < size; i++) //后续数据
                        {
                            while (osMessageQueuePut(rxQueueHandle, &rxBuffer[i], 0, 0) != osOK)
                                ;
                        }

                        HAL_UART_AbortReceive(uartHandle);                        //终止接收
                        HAL_UART_Receive_DMA(uartHandle, rxBuffer, rxBufferSize); //终止接收后，才能重设dma
                    }

                    //在队列中添加结束标志，也就是数据数量为0
                    size = 0;
                    while (osMessageQueuePut(rxQueueHandle, &size, 0, 0) != osOK)
                        ;
                }
            }

            void uartClass::rxCompleteCallback()
            {
                // if (idleFlag) //如果触发了空闲标志就直接跳出
                // {
                //     idleFlag = false;
                //     return;
                // }

                // uint8_t rxBufferSize = 20;
                //第一个字节为这次数据的长度
                while (osMessageQueuePut(rxQueueHandle, &rxBufferSize, 0, 0) != osOK)
                    ;
                for (uint8_t i = 0; i < rxBufferSize; i++) //数据
                {
                    while (osMessageQueuePut(rxQueueHandle, &rxBuffer[i], 0, 0) != osOK)
                        ;
                }
            }

            void uartClass::printf(char *fmt, ...)
            {
                // uint8_t buffer[80];
                va_list arg;
                va_start(arg, fmt);
                uint8_t len = vsnprintf((char *)printfBuffer, printfBufferSize, fmt, arg);
                va_end(arg);

                send(printfBuffer, len);
            }
        } // namespace uartNamespace
    }     // namespace communicationNamespace
} // namespace xiagb