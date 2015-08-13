#ifndef _7017_HW_H_
#define _7017_HW_H_

#define CHANNEL_COUNT 2

void init_7017(void);
void get_voltage(int channel, int *intpart, int *floatpart);

#endif /*_7017_HW_H */
