#ifndef canH
#define canH

#include "main.h"
#include "cmsis_os2.h"

namespace xiagb
{
    namespace communicationNamespace
    {
        namespace canNamespace
        {
            class canClass
            {
            private:
                static const char *const echoCommandStr;
                uint16_t rxQueueSize = 300; //队列容量

                CAN_HandleTypeDef *canHandle;
                uint8_t id;
                CAN_TxHeaderTypeDef txHeader;
                CAN_RxHeaderTypeDef rxHeader;
                osMutexId_t mutex;
                osMessageQueueId_t rxQueueHandle;

                void send(uint8_t *data, uint8_t size); //size小于等于8

            public:
                canClass();
                ~canClass();

                void init(CAN_TypeDef *canHardwareHandle, uint8_t id, CAN_HandleTypeDef *canHandle);
                void echoPrintf(uint8_t targetId, const char *fmt, ...);
                void receiveCallback();

                uint8_t getId() { return id; }
                osMessageQueueId_t getRxQueue() { return rxQueueHandle; }
            };
        } // namespace canNamespace
    }     // namespace communicationNamespace
} // namespace xiagb
#endif