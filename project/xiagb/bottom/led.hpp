#ifndef ledH
#define ledH

#include "main.h"

namespace xiagb
{
    namespace pilotLampNamespace
    {
        namespace ledNamespace
        {
            class ledClass
            {
            private:
                GPIO_TypeDef *gpioGroupHandle;
                uint16_t gpioPin;
                bool lightLv;
                bool defaultState;

            public:
                ledClass();
                ~ledClass();

                /*
        初始化参数并设定默认状态
        gpioHandle：如GPIOA
        gpioPin：如GPIO_PIN_0
        lightLv：让灯亮的电平，0或1
        defaultState：灯的默认状态，false为灭，true为亮
        */
                void init(GPIO_TypeDef *gpioGroupHandle, uint16_t gpioPin, bool lightLv, bool defaultState = false);

                /*
        翻转一次电平
        */
                void toggle();
            };
        } // namespace ledNamespace
    }     // namespace pilotLampNamespace
} // namespace xiagb
#endif