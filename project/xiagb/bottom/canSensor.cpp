#include "canSensor.hpp"
namespace xiagb
{
    namespace sensorNamespace
    {
        namespace canSensorNamespace
        {
            const char *const canSensorClass::sensorFlagStr = "6";

            canSensorClass::canSensorClass()
            {
            }

            canSensorClass::~canSensorClass()
            {
            }

            void canSensorClass::init(canClass *canHandle, uint8_t slaveBoardCanId, uint8_t slaveBoardSensorChannel)
            {
                this->canHandle = canHandle;
                this->slaveBoardCanId = slaveBoardCanId;
                this->slaveBoardSensorChannel = slaveBoardSensorChannel;

                this->state = false;
                canHandle->echoPrintf(slaveBoardCanId, "%s %d\n", sensorFlagStr, slaveBoardSensorChannel);
                while (state == false)
                    osDelay(1);
            }
        } // namespace canSensorNamespace
    }     // namespace sensorNamespace
} // namespace xiagb