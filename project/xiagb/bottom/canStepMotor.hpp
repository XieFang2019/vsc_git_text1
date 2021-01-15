#ifndef canMotorH
#define canMotorH

#include "main.h"
#include "can.hpp"

namespace xiagb
{
    namespace motorNamespace
    {
        namespace canStepMotorNamespace
        {
            using xiagb::communicationNamespace::canNamespace::canClass;
            class canStepMotorClass
            {
            private:
                uint8_t slaveBoardCanId, slaveBoardMotorChannel; //所在的从板的canId，所在的从板的电机输出口
                canClass *canHandle;

                static const char *const argFlagStr;
                static const char *const resetFlagStr;
                static const char *const moveFlagStr;
                static const char *const stopFlagStr;
                static const char *const keepMoveFlagStr;
                static const char *const sensorFlagStr;
                static const char *const stepFlagStr;
                static const char *const stepCountDownFlagStr;

                bool state; //true：运行，false：停止。标志由主机打上
                // bool activationFlag;
                // bool stepCountDownFlag; //倒计时步数已走完的标志

            public:
                canStepMotorClass();
                ~canStepMotorClass();

                void setState()
                {
                    state = false;
                    // activationFlag = true;
                }
                // void setStepCountDownFlag() { stepCountDownFlag = false; }
                // bool getStepCountDownFlag() { return stepCountDownFlag; }
                bool getState() { return state; }
                uint8_t getSlaveBoardCanId() { return slaveBoardCanId; }
                uint8_t getSlaveBoardMotorChannel() { return slaveBoardMotorChannel; }

                /*
        初始化时会一直等待从板的反馈
        canHandle：can句柄
        slaveBoardCanId：从板的canId
        slaveBoardMotorChannel：第几个电机
        resetDir：复位方向
        beltOrGear：带动的是皮带还是齿条。0是皮带，1是齿条
        beltOrGearModel：皮带或齿条的模数
        motorGearTeethNumber：电机上的齿轮的齿数
        */
                void init(canClass *canHandle, uint8_t slaveBoardCanId, uint8_t slaveBoardMotorChannel, bool resetDir, bool beltOrGear, uint8_t beltOrGearModel, uint8_t motorGearTeethNumber);
                void reset(uint16_t max_speed = 5000, uint16_t acc_dec_action_interval = 110);
                static void reset(canClass *canHandle, uint8_t slaveBoardCanId, uint8_t slaveBoardMotorChannel, uint16_t max_speed = 5000, uint16_t acc_dec_action_interval = 110);
                void stop();
                static void stop(canClass *canHandle, uint8_t slaveBoardCanId, uint8_t slaveBoardMotorChannel);
                void moveToAbsolutePositionMm(uint16_t absolutePosition, uint16_t maxSpeed, uint16_t acceleration);
                static void moveToAbsolutePositionMm(canClass *canHandle, uint8_t slaveBoardCanId, uint8_t slaveBoardMotorChannel, uint16_t absolutePosition, uint16_t maxSpeed, uint16_t acceleration);
                void keepMove(bool dir, uint16_t maxSpeed);
                static void keepMove(canClass *canHandle, uint8_t slaveBoardCanId, uint8_t slaveBoardMotorChannel, bool dir, uint16_t maxSpeed);
                void waitMoveStop();
                // void setStepCountDown(uint32_t step);
            };
        } // namespace canStepMotorNamespace
    }     // namespace motorNamespace
} // namespace xiagb

#endif