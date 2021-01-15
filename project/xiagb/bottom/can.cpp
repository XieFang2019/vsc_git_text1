#include "can.hpp"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

namespace xiagb
{
    namespace communicationNamespace
    {
        namespace canNamespace
        {
            const char *const canClass::echoCommandStr = "echo";

            canClass::canClass()
            {
            }

            canClass::~canClass()
            {
            }

            void canClass::init(CAN_TypeDef *canHardwareHandle, uint8_t id, CAN_HandleTypeDef *canHandle)
            {
                rxQueueHandle = osMessageQueueNew(rxQueueSize, sizeof(uint8_t), nullptr);
                while (rxQueueHandle == 0)
                    ;
                mutex = osMutexNew(nullptr);
                while (mutex == 0)
                    ;

                this->canHandle = canHandle;
                this->id = id;

                canHandle->Instance = CAN1;
                canHandle->Init.Prescaler = 4;
                canHandle->Init.Mode = CAN_MODE_NORMAL;
                canHandle->Init.SyncJumpWidth = CAN_SJW_1TQ;
                canHandle->Init.TimeSeg1 = CAN_BS1_9TQ;
                canHandle->Init.TimeSeg2 = CAN_BS2_8TQ;
                canHandle->Init.TimeTriggeredMode = DISABLE;
                canHandle->Init.AutoBusOff = DISABLE;
                canHandle->Init.AutoWakeUp = DISABLE;
                canHandle->Init.AutoRetransmission = ENABLE;
                canHandle->Init.ReceiveFifoLocked = DISABLE;
                canHandle->Init.TransmitFifoPriority = DISABLE;
                HAL_CAN_Init(canHandle);

                CAN_FilterTypeDef canFilter;

                canFilter.FilterIdHigh = (uint32_t)id << 5; //32 位 ID在16位里的高11位 id由这两个的低16位组成
                canFilter.FilterIdLow = 0;
                canFilter.FilterFIFOAssignment = CAN_RX_FIFO0; //过滤器 0 关联到 FIFO0
                canFilter.FilterBank = 0;                      //过滤器 0
                canFilter.FilterMode = CAN_FILTERMODE_IDLIST;  //唯一id模式
                canFilter.FilterScale = CAN_FILTERSCALE_32BIT;
                canFilter.FilterActivation = ENABLE; //激活滤波器 0
                canFilter.SlaveStartFilterBank = 0;

                HAL_CAN_ConfigFilter(canHandle, &canFilter);
                HAL_CAN_Start(canHandle);
                HAL_CAN_ActivateNotification(canHandle, CAN_IT_RX_FIFO0_MSG_PENDING);

                txHeader.StdId = (uint32_t)id;
                txHeader.RTR = CAN_RTR_DATA;
                txHeader.IDE = CAN_ID_STD;
                txHeader.TransmitGlobalTime = DISABLE;
            }

            void canClass::send(uint8_t *data, uint8_t size)
            {
                uint32_t txMailBox;
                txHeader.DLC = size;

                while (HAL_CAN_GetTxMailboxesFreeLevel(canHandle) == 0)
                    osDelay(1);
                while (HAL_CAN_AddTxMessage(canHandle, &txHeader, data, &txMailBox) != HAL_OK)
                    osDelay(1);
                osDelay(2);
            }

            //发送出去的帧格式为id data1 data2 ... data7
            void canClass::echoPrintf(uint8_t targetId, const char *fmt, ...)
            {
                const uint8_t bufferSize = 80;
                osMutexAcquire(mutex, osWaitForever);

                static uint8_t buffer[bufferSize]; //大量使用栈内存可能会导致栈溢出

                //复制字符串
                uint8_t size = strlen(echoCommandStr);
                strncpy((char *)buffer, echoCommandStr, size + 1);
                buffer[size] = ' ';
                size++;

                va_list arg;
                va_start(arg, fmt);
                size += vsnprintf((char *)(buffer + size), bufferSize - size, fmt, arg);
                va_end(arg);

                txHeader.StdId = targetId;

                for (uint8_t i = 0; i < size;)
                {
                    uint8_t buffer2[8];
                    buffer2[0] = id; //第一个字节为自己的id
                    uint8_t num = (size - i) >= 7 ? 7 : (size - i);
                    for (size_t i2 = 0; i2 < num; i2++)
                        buffer2[i2 + 1] = buffer[i + i2];
                    i += num;

                    send(buffer2, num + 1);
                }
                osMutexRelease(mutex);
            }

            void canClass::receiveCallback()
            {
                uint8_t buffer[8];
                if (HAL_CAN_GetRxMessage(canHandle, CAN_RX_FIFO0, &rxHeader, buffer) == HAL_OK)
                {
                    while (osMessageQueuePut(rxQueueHandle, &buffer[0], 0, 0) != osOK) //id
                        ;

                    uint8_t bufferU8 = rxHeader.DLC - 1;
                    while (osMessageQueuePut(rxQueueHandle, &bufferU8, 0, 0) != osOK) //数据个数
                        ;

                    for (uint8_t i = 0; i < bufferU8; i++)
                    {
                        while (osMessageQueuePut(rxQueueHandle, &buffer[i + 1], 0, 0) != osOK)
                            ;
                    }
                }
            }
        } // namespace canNamespace
    }     // namespace communicationNamespace
} // namespace xiagb