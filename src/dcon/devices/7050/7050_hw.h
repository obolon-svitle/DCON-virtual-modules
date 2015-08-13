#ifndef _7050_HW_H_
#define _7050_HW_H_

void init_7050(void);
int set_output(unsigned char group, unsigned char value);
unsigned long get_input(void);
unsigned long int get_io_status(void);
float mlx90614_get_temperature(void);

#endif /* _7050_HW_H_ */
