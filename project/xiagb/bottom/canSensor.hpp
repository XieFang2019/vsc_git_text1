#ifndef canSensorH
#define canSensorH

#include "main.h"
#include "can.hpp"

namespace xiagb
{
    namespace sensorNamespace
    {
        namespace canSensorNamespace
        {
            using xiagb::communicationNamespace::canNamespace::canClass;
            
            class canSensorClass
            {
            private:
                static const char *const sensorFlagStr;

                uint8_t slaveBoardCanId, slaveBoardSensorChannel;
                canClass *canHandle;
                bool state;

            public:
                canSensorClass();
                ~canSensorClass();

                void init(canClass *canHandle, uint8_t slaveBoardCanId, uint8_t slaveBoardSensorChannel);

                void setState(bool state) { this->state = state; }
                bool getState() { return state; }

                uint8_t getSlaveBoardCanId() { return slaveBoardCanId; }
                uint8_t getSlaveBoardSensorChannel() { return slaveBoardSensorChannel; }
            };
        } // namespace canSensorNamespace
    }     // namespace sensorNamespace
} // namespace xiagb
#endif