void setup() {                
pwm_channel_t pwm_channel_instance;
pmc_enable_periph_clk(ID_PWM);
pwm_channel_disable(PWM, PWM_CHANNEL_0);
pwm_clock_t clock_setting = {
    .ul_clka = 1000 * 100,
    .ul_clkb = 0,
    .ul_mck = 48000000
};
pwm_init(PWM, &clock_setting);
pwm_channel_instance.ul_prescaler = PWM_CMR_CPRE_CLKA;
pwm_channel_instance.ul_period = 100;
pwm_channel_instance.ul_duty = 50;
pwm_channel_instance.channel = PWM_CHANNEL_0;
pwm_channel_init(PWM, &pwm_channel_instance);
}

void loop() {

  
}
