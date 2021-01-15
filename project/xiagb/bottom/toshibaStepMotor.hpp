#ifndef toshibaStepMotorH
#define toshibaStepMotorH

#include "main.h"
namespace xiagb
{
    namespace motorNamespace
    {
        namespace toshibaStepMotorNamespace
        {
            class toshibaStepMotorClass
            {
            private:
                enum motor_state
                {
                    motor_stop,
                    motor_accelerate,
                    motor_max_speed,
                    motor_decelerate
                };

                enum
                {
                    curve_s_pan_right = 3,
                    curve_s_num = 200,
                    a_circle_step = 3200,
                    max_speed_threshold = 65535
                };

                int32_t position;      //当前的绝对位置，以步数为单位
                int32_t need_run_step; //将要运行的步数。负数就往复位方向移动
                uint32_t step_counter; //已经运行的脉冲数。其结果不影响算法
                uint16_t speed;        //当前速度
                uint16_t max_speed;    //用户设定的最大速度
                motor_state state;     //运行状态

                bool interrupt_counter_flag; //定时器中断计数的开关
                uint16_t acc_dec_interval;   //加减速一次的间隔
                uint16_t interrupt_counter;  //加减速时的定时器中断次数计数
                uint32_t acc_dec_counter;    //加减速次数计数
                uint32_t acc_dec_step;       //加减速阶段的脉冲数
                uint32_t speed_accumulator;  //速度累加器

                GPIO_PinState reset_dir_io_lv;                             //复位方向的io电平
                float mm_setp;                                             //电机走1mm所需的步数
                GPIO_TypeDef *step_gpiox, *dir_gpiox, *reset_sensor_gpiox; //IO组，脉冲、方向、复位限位
                uint16_t step_gpio_x, dir_gpio_x, reset_sensor_gpio_x;     //IO号，脉冲、方向、复位限位
                GPIO_PinState reset_sensor_io_lv;                          //复位限位开关被触发后的电平
                bool reset_flag;                                           //是否处于复位
                bool stop_flag;                                            //是否被停止了
                bool keep_turn_flag;                                       //是否要一直转

                //使用pwm控制电流大小
                TIM_HandleTypeDef *power_timer_handle; //电力定时器
                uint8_t power_timer_channel;           //电力通道
                float run_power, hold_power;           //运行功率，保持功率

                static float curve_s_table[curve_s_num];
                static bool curve_s_data_build_flag;

                void motor_run_time_check();

            public:
                toshibaStepMotorClass();
                ~toshibaStepMotorClass();

                static void curve_s_init();
                void init(GPIO_TypeDef *step_gpiox, uint16_t step_gpio_x, GPIO_TypeDef *dir_gpiox, uint16_t dir_gpio_x, GPIO_TypeDef *reset_sensor_gpiox, uint16_t reset_sensor_gpio_x, GPIO_PinState reset_sensor_io_lv, GPIO_PinState reset_dir_io_lv, TIM_HandleTypeDef *power_timer_handle, uint8_t power_timer_channel, float run_power, float hold_power);
                void move(int32_t need_run_step, uint16_t max_speed, uint16_t acc_dec_interval);
                void move_to_position(uint16_t position, uint16_t max_speed, uint16_t acc_dec_action_interval); //在没有使用set_mm_step设置mm_step之前，不要用
                void loop();                                                                                    //在中断里循环
                void reset(uint16_t max_speed = 5000, uint16_t acc_dec_action_interval = 100);
                void stop();
                void keep_turn(bool dir, uint16_t max_speed, uint16_t acc_dec_action_interval); //一直转
                void set_mm_step(float pitch, uint8_t gear_teeth) { mm_setp = a_circle_step / (pitch * gear_teeth); }
                bool get_stop_flag() { return stop_flag; }
                void reset_stop_flag() { stop_flag = false; }
                int32_t getPosition() { return position; }
            };
        } // namespace toshibaStepMotorNamespace
    }     // namespace motorNamespace
} // namespace xiagb
#endif