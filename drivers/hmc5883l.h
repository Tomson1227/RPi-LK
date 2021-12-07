#ifndef HMC5883L_DRIVER_H
#define HMC5883L_DRIVER_H

#define CONFIG_REG_A 	0x00
#define CONFIG_REG_B 	0x01
#define MODE_REG 		0x02
#define REG_AXIS_X_MSB 	0x03
#define REG_AXIS_X_LSB 	0x04
#define REG_AXIS_Y_MSB 	0x05
#define REG_AXIS_Y_LSB 	0x06
#define REG_AXIS_Z_MSB 	0x07
#define REG_AXIS_Z_LSB 	0x08
#define STATUS_REG 		0x09
#define IDENT_REG_A 	0x0A
#define IDENT_REG_B 	0x0B
#define IDENT_REG_C 	0x0C

/* Reg A configuration */
// Number of samples averaged per measurement output
#define SAMPLES_N_1 0x00
#define SAMPLES_N_2 0x20
#define SAMPLES_N_4 0x40
#define SAMPLES_N_8 0x60

// Typical Data Output Rate (Hz)
#define OUTPUT_RATE_0_75    0x00 
#define OUTPUT_RATE_1_5     0x04
#define OUTPUT_RATE_3       0x08 
#define OUTPUT_RATE_7_5     0x0c 
#define OUTPUT_RATE_15      0x10 
#define OUTPUT_RATE_30      0x14 
#define OUTPUT_RATE_75      0x18

#define NORMAL_CONFIG   0x00
#define NEG_CONFIG      0x01
#define POS_CONFIG      0x02

#define DEFAULT_REG_A (SAMPLES_N_1 | OUTPUT_RATE_15 | NORMAL_CONFIG)

/* Reg A configuration */
//Gain Configuration
#define GAIN_0  0x00    // +-0.88 Ga  1370 LSb/Gauss     0.73 mG/LSb
#define GAIN_1  0x20    // +-1.3 Ga   1090 LSb/Gauss     0.92 mG/LSb
#define GAIN_2  0x40    // +-1.9 Ga   820  LSb/Gauss     1.22 mG/LSb
#define GAIN_3  0x60    // +-2.5 Ga   660  LSb/Gauss     1.52 mG/LSb
#define GAIN_4  0x80    // +-4.0 Ga   440  LSb/Gauss     2.27 mG/LSb
#define GAIN_5  0xa0    // +-4.7 Ga   390  LSb/Gauss     2.56 mG/LSb
#define GAIN_6  0xc0    // +-5.6 Ga   330  LSb/Gauss     3.03 mG/LSb
#define GAIN_7  0xe0    // +-8.1 Ga   230  LSb/Gauss     4.35 mG/LSb

#define DEFAULT_REG_B (GAIN_1)

/* Mode register configuration */
#define I2C_HS_ON   0x80
#define I2C_HS_OFF  0x00
#define CONTINUOUS_MEASURE_MODE 0x00
#define SINGLE_MEASURE_MODE 0x01
#define IDLE_MODE   0x02

#define DEFAULT_MODE (I2C_HS_OFF | SINGLE_MEASURE_MODE)

#endif /* HMC5883L_DRIVER_H */
