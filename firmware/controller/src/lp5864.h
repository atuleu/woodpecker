#pragma once

#include <hardware/i2c.h>
#include <pico/types.h>
#include <stdint.h>

struct __attribute__((packed)) Chip_en {
	uint8_t EN : 1;
	uint8_t _reserved : 7;
};

#define LP5864_Chip_en_ADDRESS 0x000
struct LP5864_Config {
	struct __attribute__((packed)) {
		uint8_t PWM_Frequency : 1;
		uint8_t Data_Ref_Mode : 1;
		uint8_t Max_Line_Num : 4;
	} Dev_initial;

	struct __attribute__((packed)) {
		uint8_t CS_ON_Shift : 1;
		uint8_t PWM_PHASE_Shift : 1;
		uint8_t PWM_Scale_Mode : 1;
		uint8_t SW_BLK : 1;
	} Dev_config1;

	struct __attribute__((packed)) {
		uint8_t LSD_removal : 1;
		uint8_t LOD_removal : 1;
		uint8_t Comp_Group1 : 2;
		uint8_t Comp_Group2 : 2;
		uint8_t Comp_Group3 : 2;
	} Dev_config2;

	struct __attribute__((packed)) {
		uint8_t Up_Deghost_enable : 1;
		uint8_t Maximum_Current : 3;
		uint8_t Up_Deghost : 2;
		uint8_t Down_Deghost : 2;
	} Dev_config3;
};

#define LP5864_Config_ADDRESS 0x001

struct LP5864_PWM_Group {
	uint8_t Global_bri;
	uint8_t Group0_bri;
	uint8_t Group1_bri;
	uint8_t Group2_bri;
};

#define LP5864_Global_bri_ADDRESS 0x005

struct __attribute__((packed)) LP5864_Current_Compensation {
	uint8_t Group1 : 7;
	uint8_t _reserved1 : 1;
	uint8_t Group2 : 7;
	uint8_t _reserved2 : 1;

	uint8_t Group3 : 7;
	uint8_t _reserved3 : 1;
};

#define LP5864_CC_ADDRESS 0x009

struct __attribute__((packed)) LP5864_Group_Selection {
	uint8_t CS00 : 2;
	uint8_t CS01 : 2;
	uint8_t CS02 : 2;
	uint8_t CS03 : 2;
	uint8_t CS04 : 2;
	uint8_t CS06 : 2;
	uint8_t CS07 : 2;
	uint8_t CS08 : 2;
	uint8_t CS09 : 2;
	uint8_t CS10 : 2;
	uint8_t CS11 : 2;
	uint8_t CS12 : 2;
	uint8_t CS13 : 2;
	uint8_t CS14 : 2;
	uint8_t CS15 : 2;
	uint8_t CS16 : 2;
	uint8_t CS17 : 2;
};

#define LP5864_GROUP_SEL_L0_ADDRESS 0x00C
#define LP5864_GROUP_SEL_L1_ADDRESS 0x011
#define LP5864_GROUP_SEL_L2_ADDRESS 0x016
#define LP5864_GROUP_SEL_L3_ADDRESS 0x01B

struct __attribute__((packed)) LP5864_Dot_onoff {
	uint8_t CS00 : 1;
	uint8_t CS01 : 1;
	uint8_t CS02 : 1;
	uint8_t CS03 : 1;
	uint8_t CS04 : 1;
	uint8_t CS05 : 1;
	uint8_t CS06 : 1;
	uint8_t CS07 : 1;
	uint8_t CS08 : 1;
	uint8_t CS09 : 1;
	uint8_t CS10 : 1;
	uint8_t CS11 : 1;
	uint8_t CS12 : 1;
	uint8_t CS13 : 1;
	uint8_t CS14 : 1;
	uint8_t CS15 : 1;
	uint8_t CS16 : 1;
	uint8_t CS17 : 1;
};

#define LP5864_DOT_ONOFF_ADDRESS 0x043

#define LP5864_DOT_CS(line, cs)  (0x100 + 18 * (line) + (cs))
#define LP5864_DOT_PWM(line, cs) (0x200 + 18 * (line) + (cs))

int lp5864_read_blocking(
    i2c_inst_t *i2c, uint8_t addr, uint reg_addr, uint8_t *dst, size_t len
);

int lp5864_write_blocking(
    i2c_inst_t *i2c, uint8_t addr, uint reg_addr, const uint8_t *src, size_t len
);
