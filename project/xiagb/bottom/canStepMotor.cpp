#include "canStepMotor.hpp"

namespace xiagb
{
    namespace motorNamespace
    {
        namespace canStepMotorNamespace
        {
            const char *const canStepMotorClass::argFlagStr = "1";
            const char *const canStepMotorClass::resetFlagStr = "2";
            const char *const canStepMotorClass::moveFlagStr = "3";
            const char *const canStepMotorClass::stopFlagStr = "4";
            const char *const canStepMotorClass::keepMoveFlagStr = "5";
            const char *const canStepMotorClass::sensorFlagStr = "6";
            const char *const canStepMotorClass::stepFlagStr = "7";
            const char *const canStepMotorClass::stepCountDownFlagStr = "8";

            canStepMotorClass::canStepMotorClass()
            {
            }

            canStepMotorClass::~canStepMotorClass()
            {
            }

            void canStepMotorClass::init(canClass *canHandle, uint8_t slaveBoardCanId, uint8_t slaveBoardMotorChannel, bool resetDir, bool beltOrGear, uint8_t beltOrGearModel, uint8_t motorGearTeethNumber)
            {
                this->canHandle = canHandle;
                this->slaveBoardCanId = slaveBoardCanId;
                this->slaveBoardMotorChannel = slaveBoardMotorChannel;

                this->state = true; //测试激活
                canHandle->echoPrintf(slaveBoardCanId, "%s %d %d %d %d %d\n", argFlagStr, slaveBoardMotorChannel, resetDir, beltOrGear, beltOrGearModel, motorGearTeethNumber); //设置电机参数
                while (state == true) //等待被激活，如果无法收到反馈，基本就是从机无法接收，就要检查can的连接线
                    osDelay(1);
            }

            void canStepMotorClass::reset(uint16_t max_speed, uint16_t acc_dec_action_interval)
            {
                if (!state)
                {
                    canHandle->echoPrintf(slaveBoardCanId, "%s %d\n", resetFlagStr, slaveBoardMotorChannel, max_speed, acc_dec_action_interval);
                    state = true;
                }
            }

            void canStepMotorClass::reset(canClass *canHandle, uint8_t slaveBoardCanId, uint8_t slaveBoardMotorChannel, uint16_t max_speed, uint16_t acc_dec_action_interval)
            {
                canHandle->echoPrintf(slaveBoardCanId, "%s %d %d %d\n", resetFlagStr, slaveBoardMotorChannel, max_speed, acc_dec_action_interval);
                //方便调试用，所以没有指定状态
            }

            void canStepMotorClass::stop()
            {
                if (state)
                {
                    canHandle->echoPrintf(slaveBoardCanId, "%s %d\n", stopFlagStr, slaveBoardMotorChannel);
                    state = false;
                }
            }

            void canStepMotorClass::stop(canClass *canHandle, uint8_t slaveBoardCanId, uint8_t slaveBoardMotorChannel)
            {
                canHandle->echoPrintf(slaveBoardCanId, "%s %d\n", stopFlagStr, slaveBoardMotorChannel);
            }

            void canStepMotorClass::moveToAbsolutePositionMm(uint16_t absolutePosition, uint16_t maxSpeed, uint16_t acceleration)
            {
                if (!state)
                {
                    canHandle->echoPrintf(slaveBoardCanId, "%s %d %d %d %d\n", moveFlagStr, slaveBoardMotorChannel, absolutePosition, maxSpeed, acceleration);
                    state = true;
                }
            }

            void canStepMotorClass::moveToAbsolutePositionMm(canClass *canHandle, uint8_t slaveBoardCanId, uint8_t slaveBoardMotorChannel, uint16_t absolutePosition, uint16_t maxSpeed, uint16_t acceleration)
            {
                canHandle->echoPrintf(slaveBoardCanId, "%s %d %d %d %d\n", moveFlagStr, slaveBoardMotorChannel, absolutePosition, maxSpeed, acceleration);
                //方便调试用，所以没有指定状态
            }

            void canStepMotorClass::keepMove(bool dir, uint16_t maxSpeed)
            {
                if (!state)
                {
                    canHandle->echoPrintf(slaveBoardCanId, "%s %d %d %d\n", keepMoveFlagStr, slaveBoardMotorChannel, dir, maxSpeed);
                    state = true;
                }
            }

            void canStepMotorClass::keepMove(canClass *canHandle, uint8_t slaveBoardCanId, uint8_t slaveBoardMotorChannel, bool dir, uint16_t maxSpeed)
            {
                canHandle->echoPrintf(slaveBoardCanId, "%s %d %d %d\n", keepMoveFlagStr, slaveBoardMotorChannel, dir, maxSpeed);
            }

            void canStepMotorClass::waitMoveStop()
            {
                while (state)
                    osDelay(1);
            }

            // void canStepMotorClass::setStepCountDown(uint32_t step)
            // {
            //     canHandle->echoPrintf(slaveBoardCanId, "%s %d %d\n", stepCountDownFlagStr, slaveBoardMotorChannel, step);
            //     stepCountDownFlag = true;
            // }
        } // namespace canStepMotorNamespace
    }     // namespace motorNamespace
} // namespace xiagb