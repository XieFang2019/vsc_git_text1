#include "machineTask.hpp"
#include "main.h"
#include "cmsis_os2.h"

#include "canStepMotor.hpp"
#include "canSensor.hpp"

#include "cliTask.hpp"

namespace xiagb
{
    namespace upper
    {
        namespace machineTaskNamespace
        {
            using xiagb::motorNamespace::canStepMotorNamespace::canStepMotorClass;
            using xiagb::sensorNamespace::canSensorNamespace::canSensorClass;
            using xiagb::upper::cliTaskNamespace::canCommunication;
            using xiagb::upper::cliTaskNamespace::commandParser;

            const osThreadAttr_t machineTaskAttributes = {.name = "machineTask", .stack_size = 128 * 4, .priority = (osPriority_t)osPriorityNormal};
            const osThreadAttr_t machineTask1Attributes = {.name = "machineTask1", .stack_size = 128 * 4, .priority = (osPriority_t)osPriorityNormal};
            osMessageQueueId_t argQueue;
            uint8_t const stepMotorTotalNumber = 1; //电机个数
            canStepMotorClass *stepMotor[stepMotorTotalNumber];
            uint8_t const sensorTotalNumber = 1; //传感器个数
            canSensorClass *sensor[sensorTotalNumber];

            void machineTask1Start();
            void slaveFeedback(uint8_t feedbackType, uint8_t canId, uint8_t number, int32_t state);

            //主机发来的和从机反馈回来的都在这处理
            void machineTask(void *arg)
            {
                argQueue = osMessageQueueNew(100, sizeof(int32_t), nullptr); //参数总数不能超过100个
                while (argQueue == 0)
                    ;

                commandParser->setEchoQueue(argQueue); //将参数队列添加到echo命令参数的输出里

                int32_t argCount;
                static int32_t argHandle[20];

                for (;;)
                {
                    osDelay(1);

                    if (osMessageQueueGet(argQueue, &argCount, nullptr, 0) == osOK) //第一个参数是参数数量
                    {
                        for (uint8_t i = 0; i < argCount; i++)
                            osMessageQueueGet(argQueue, &argHandle[i], nullptr, osWaitForever);

                        switch (argHandle[0])
                        {
                        case 1:
                            machineTask1Start();
                            break;

                        case 2:
                            slaveFeedback(argHandle[1], argHandle[2], argHandle[3], argHandle[4]);

                        default:
                            break;
                        }
                    }
                }
            }

            void machineTask1(void *arg)
            {
                stepMotor[0] = new canStepMotorClass;
                stepMotor[0]->init(canCommunication, 2, 1, 0, 0, 3, 20);
                stepMotor[0]->reset();
                stepMotor[0]->waitMoveStop();

                for (;;)
                {
                    osDelay(1);
                }
            }

            void machineTaskStart()
            {
                osThreadNew(machineTask, nullptr, &machineTaskAttributes);
            }

            void machineTask1Start()
            {
                osThreadNew(machineTask1, nullptr, &machineTask1Attributes);
            }

            //处理从机返回的状态
            void slaveFeedback(uint8_t feedbackType, uint8_t canId, uint8_t number, int32_t state)
            {
                switch (feedbackType)
                {
                case 1: //电机
                    for (uint8_t i = 0; i < stepMotorTotalNumber; i++)
                    {
                        if (stepMotor[i]->getSlaveBoardCanId() == canId && stepMotor[i]->getSlaveBoardMotorChannel() == number)
                            stepMotor[i]->setState();
                    }
                    break;

                case 2: //传感器
                    for (uint8_t i = 0; i < stepMotorTotalNumber; i++)
                    {
                        if (sensor[i]->getSlaveBoardCanId() == canId && sensor[i]->getSlaveBoardSensorChannel() == number)
                            sensor[i]->setState(state);
                    }
                    break;
                
                default:
                    break;
                }
            }
        } // namespace machineTaskNamespace
    }     // namespace upper
} // namespace xiagb