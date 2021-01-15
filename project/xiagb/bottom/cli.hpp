#ifndef cliH
#define cliH

#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "uart.hpp"

namespace xiagb
{
    namespace parserNamespace
    {
        namespace cliNamespace
        {
            using xiagb::communicationNamespace::uartNamespace::uartClass;
            
            class cliClass
            {
            private:
                uint16_t rxQueueCount = 1000;    //输入缓冲区大小。命令进来太多就需要加大，否则接收不到
                uint16_t inputBufferSize = 100;  //单条命令的最大长度
                uint16_t outputBufferSize = 500; //结果输出缓冲区，输出会全部放这里，再全部一次发出去

                uartClass *uartHandle;
                // void (*send)(uint8_t *dataHandle, uint16_t lenght);

                osMessageQueueId_t rxQueue;
                uint8_t *inputBuffer, *lastCommandBuffer;
                uint16_t inputBufferIndex;
                uint8_t *outputBuffer;
                bool feedbackFlag = true; //串口输出的开关
                osMutexId_t mutex;
                static bool registerFlag; //避免重复添加命令

                void outputChar(uint8_t data);
                void outputStr(uint8_t *data);
                osStatus_t getChar(uint8_t *pcRxedChar);
                static void vRegisterSampleCLICommands();

            public:
                cliClass();
                ~cliClass();

                void init(uartClass *uartHandle);
                void loop();
                void setChar(uint8_t *data); //只能放一个字节
                // void setStr(uint8_t *data, uint16_t size);
                void receiveQueueStr(osMessageQueueId_t queue); //输入一个队列里的所有字符

                //使用echo-parameters命令时使用。
                void setEchoQueue(osMessageQueueId_t argQueueInt32); //队列的数据类型必须是4字节的

                void setFeedbackFlag(bool value) { this->feedbackFlag = value; } //设置为false后，命令处理速度会快很多
            };
        } // namespace cliNamespace
    }     // namespace parserNamespace
} // namespace xiagb

/*
先使用init，如果要用参数解析就用setEchoDataQueue和setEchoDataQueue
setChar是放入一个字节。
loop需要放到一个循环里，不断的执行。
*/

#endif