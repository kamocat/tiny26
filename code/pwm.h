#include <stdint.h>

void init_pwm(void);
void update_pwm(const struct mag * pulse, const struct mag * period);
void update_duty(uint8_t duty, const struct mag * period);
