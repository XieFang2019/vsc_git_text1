#include "ivesStepMotor.hpp"
#include <math.h>

namespace xiagb
{
    namespace motorNamespace
    {
        namespace ivesStepMotorNamespace
        {
            float const ivesStepMotorClass::pi = 3.1415926;
            float *ivesStepMotorClass::curve_s_table;
            bool ivesStepMotorClass::curve_s_data_build_flag;

            ivesStepMotorClass::ivesStepMotorClass()
            {
            }

            ivesStepMotorClass::~ivesStepMotorClass()
            {
            }

            void ivesStepMotorClass::curve_s_init()
            {
                curve_s_table = new float[curve_s_num];

                for (uint16_t x = 0; x < curve_s_num; x++)
                    curve_s_table[x] = 1.0 / (1.0 + exp((curve_s_pan_right * 2.0 / curve_s_num * -x) + curve_s_pan_right));

                curve_s_data_build_flag = true;
            }

            void ivesStepMotorClass::init(GPIO_TypeDef *step_gpiox, uint16_t step_gpio_x, GPIO_TypeDef *dir_gpiox, uint16_t dir_gpio_x, GPIO_TypeDef *reset_sensor_gpiox, uint16_t reset_sensor_gpio_x, GPIO_PinState reset_sensor_io_lv, GPIO_PinState reset_dir_io_lv)
            {
                if (curve_s_data_build_flag == false)
                    curve_s_init(), curve_s_data_build_flag = true;

                this->step_gpiox = step_gpiox;
                this->step_gpio_x = step_gpio_x;

                this->dir_gpiox = dir_gpiox;
                this->dir_gpio_x = dir_gpio_x;

                this->reset_sensor_gpiox = reset_sensor_gpiox;
                this->reset_sensor_gpio_x = reset_sensor_gpio_x;
                this->reset_sensor_io_lv = reset_sensor_io_lv;
                this->reset_dir_io_lv = reset_dir_io_lv;

                stop();
            }

            void ivesStepMotorClass::move(int32_t need_run_step, uint16_t max_speed, uint16_t acc_dec_interval)
            {
                if (need_run_step == 0)
                {
                    stop();
                    return;
                }

                this->need_run_step = need_run_step;
                step_counter = 0;
                speed = 0;
                this->max_speed = max_speed > max_speed_threshold - 100 ? max_speed_threshold : max_speed + 100; //防止速度为0
                interrupt_counter_flag = true;
                interrupt_counter = 0;
                acc_dec_counter = 0;
                acc_dec_step = 0;
                this->acc_dec_interval = acc_dec_interval;
                speed_accumulator = 0;

                if (need_run_step > 0)
                {
                    if (reset_dir_io_lv == GPIO_PIN_SET)
                        HAL_GPIO_WritePin(dir_gpiox, dir_gpio_x, GPIO_PIN_RESET);
                    else
                        HAL_GPIO_WritePin(dir_gpiox, dir_gpio_x, GPIO_PIN_SET);
                }
                else
                    HAL_GPIO_WritePin(dir_gpiox, dir_gpio_x, reset_dir_io_lv);
                state = motor_accelerate;
            }

            void ivesStepMotorClass::move_to_position(uint16_t position, uint16_t max_speed, uint16_t acc_dec_action_interval)
            {
                //会将毫米转换成步数
                if (position * mm_setp > this->position)
                    move(position * mm_setp - this->position, max_speed, acc_dec_action_interval);
                else
                    move(-(this->position - position * mm_setp), max_speed, acc_dec_action_interval);
            }

            void ivesStepMotorClass::motor_run_time_check()
            {
                if (HAL_GPIO_ReadPin(reset_sensor_gpiox, reset_sensor_gpio_x) == reset_sensor_io_lv) //检查复位开关
                {
                    if (reset_flag == true)
                    {
                        stop();
                        reset_flag = false, position = 0; //绝对位置置0
                    }
                    else if (need_run_step < 0) //不能继续向复位的方向走，但可以向复位的反方向走
                        stop();
                }
            }

            void ivesStepMotorClass::loop()
            {
                if (state == motor_stop)
                    return;

                //拉低脉冲信号
                HAL_GPIO_WritePin(step_gpiox, step_gpio_x, GPIO_PIN_RESET);

                bool carry_v = false;
                speed_accumulator += speed;                   //叠加速度
                if (speed_accumulator >= max_speed_threshold) //阈值，如果当前速度达到阈值，发送脉冲的速度就达到最快了，这个阈值就是算法的最大速度
                {
                    carry_v = true;
                    speed_accumulator -= max_speed_threshold;
                }

                if (carry_v == true) //判断是否溢出
                {
                    step_counter++;
                    //拉高脉冲信号产生
                    HAL_GPIO_WritePin(step_gpiox, step_gpio_x, GPIO_PIN_SET);

                    if (need_run_step > 0)
                        position++;
                    else
                        position--;

                    //额外的操作
                    motor_run_time_check();
                }

                //根据电机的状态进行工作
                switch (state)
                {
                case motor_accelerate:

                    //记录加速的步数
                    if (carry_v == true)
                        acc_dec_step++;

                    if (interrupt_counter_flag)
                    {
                        interrupt_counter++;
                        if (interrupt_counter >= acc_dec_interval)
                        {
                            interrupt_counter = 0;
                            acc_dec_counter++; //计录加速的次数

                            speed = curve_s_table[acc_dec_counter] * max_speed; //计算当前速度

                            //如果加速次数达到最高次数，就停止加速，并让速度等于最大速度，再切换状态
                            if (acc_dec_counter >= curve_s_num)
                            {
                                interrupt_counter_flag = false;
                                speed = max_speed;
                                state = motor_max_speed;
                            }
                        }
                    }

                    //如果总步数大于1步
                    if ((uint32_t)fabs(need_run_step) > 1)
                    {
                        if (step_counter >= (uint32_t)fabs(need_run_step) / 2) //如果已走步数大于最大步数的一半
                        {
                            //加速时中断计数被断，所以要调整，最后一次加速多长时间，减速的第一次的速度就维持多长时间
                            interrupt_counter = acc_dec_interval - interrupt_counter;
                            state = motor_decelerate;
                        }
                    }
                    else if (step_counter > 0) //只有1步就至少走1步
                        state = motor_decelerate;

                    break;

                case motor_max_speed:
                    if (keep_turn_flag && carry_v == true) //当处于保持转动的状态时
                    {
                        step_counter--; //加一次就减一次，防止步数达到进入减速的要求
                        if (need_run_step > 0)
                            position--;
                        else
                            position++;
                    }

                    if ((uint32_t)fabs(need_run_step) - step_counter <= acc_dec_step)
                    {
                        interrupt_counter_flag = true;
                        //进入减速状态就要切换为减速状态时的最大速度
                        acc_dec_counter--;
                        speed = curve_s_table[acc_dec_counter] * max_speed; //计算当前速度
                        state = motor_decelerate;
                    }

                    break;

                case motor_decelerate:

                    if (interrupt_counter_flag)
                    {
                        interrupt_counter++;
                        if (interrupt_counter >= acc_dec_interval) //如果中断次数达到设定次数
                        {
                            interrupt_counter = 0;
                            acc_dec_counter--;

                            speed = curve_s_table[acc_dec_counter] * max_speed; //计算当前速度

                            if (acc_dec_counter <= 1)
                                interrupt_counter_flag = false;
                        }
                    }

                    if (step_counter >= (uint32_t)fabs(need_run_step))
                        stop();
                    break;

                default:
                    break;
                }
            }

            void ivesStepMotorClass::reset(uint16_t max_speed, uint16_t acc_dec_action_interval)
            {
                reset_flag = true;
                move(a_circle_step * -1000, max_speed, acc_dec_action_interval);
            }

            void ivesStepMotorClass::stop()
            {
                state = motor_stop;
                stop_flag = true;
                keep_turn_flag = false;
            }

            void ivesStepMotorClass::keep_turn(bool dir, uint16_t max_speed, uint16_t acc_dec_action_interval)
            {
                keep_turn_flag = true;
                if (dir)
                    move(a_circle_step * 100, max_speed, acc_dec_action_interval); //步数至少让加速能加满
                else
                    move(a_circle_step * -100, max_speed, acc_dec_action_interval);
            }
        } // namespace ivesStepMotorNamespace
    }     // namespace motorNamespace
} // namespace xiagb