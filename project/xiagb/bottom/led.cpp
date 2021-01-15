#include "led.hpp"

namespace xiagb
{
    namespace pilotLampNamespace
    {
        namespace ledNamespace
        {
            ledClass::ledClass()
            {
            }

            ledClass::~ledClass()
            {
            }

            void ledClass::init(GPIO_TypeDef *gpioGroupHandle, uint16_t gpioPin, bool lightLv, bool defaultState)
            {
                this->gpioGroupHandle = gpioGroupHandle;
                this->gpioPin = gpioPin;
                this->lightLv = lightLv;
                this->defaultState = defaultState;

                HAL_GPIO_WritePin(this->gpioGroupHandle, this->gpioPin, GPIO_PinState(defaultState ? lightLv : !lightLv));
            }

            void ledClass::toggle()
            {
                HAL_GPIO_TogglePin(this->gpioGroupHandle, this->gpioPin);
            }
        } // namespace ledNamespace
    }     // namespace pilotLampNamespace
} // namespace xiagb