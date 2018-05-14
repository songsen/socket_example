#ifndef __LRADC_H
#define __LRADC_H

#define IMX28_ADC_IOC_MAGIC     'j'

#define IMX28_ADC_CH0     	_IOW(IMX28_ADC_IOC_MAGIC, 10, int)	/* 通道0			*/
#define IMX28_ADC_CH1     	_IOW(IMX28_ADC_IOC_MAGIC, 11, int)	/* 通道1			*/
#define IMX28_ADC_CH2     	_IOW(IMX28_ADC_IOC_MAGIC, 12, int)	/* 通道2			*/
#define IMX28_ADC_CH3   	_IOW(IMX28_ADC_IOC_MAGIC, 13, int)	/* 通道3			*/
#define IMX28_ADC_CH4     	_IOW(IMX28_ADC_IOC_MAGIC, 14, int)	/* 通道4			*/
#define IMX28_ADC_CH5     	_IOW(IMX28_ADC_IOC_MAGIC, 15, int)	/* 通过5			*/
#define IMX28_ADC_CH6     	_IOW(IMX28_ADC_IOC_MAGIC, 16, int)	/* 通道6			*/
#define IMX28_ADC_VBAT     	_IOW(IMX28_ADC_IOC_MAGIC, 17, int)	/* 测量电池电压			*/

#define IMX28_ADC_CH0_DIV2     	_IOW(IMX28_ADC_IOC_MAGIC, 20, int)	/* 通道0,开启除 2		*/
#define IMX28_ADC_CH1_DIV2     	_IOW(IMX28_ADC_IOC_MAGIC, 21, int)	/* 通道1,开启除 2		*/
#define IMX28_ADC_CH2_DIV2     	_IOW(IMX28_ADC_IOC_MAGIC, 22, int)	/* 通道2,开启除 2		*/
#define IMX28_ADC_CH3_DIV2   	_IOW(IMX28_ADC_IOC_MAGIC, 23, int)	/* 通道3,开启除 2		*/
#define IMX28_ADC_CH4_DIV2     	_IOW(IMX28_ADC_IOC_MAGIC, 24, int)	/* 通道4,开启除 2		*/
#define IMX28_ADC_CH5_DIV2     	_IOW(IMX28_ADC_IOC_MAGIC, 25, int)	/* 通过5,开启除 2		*/
#define IMX28_ADC_CH6_DIV2     	_IOW(IMX28_ADC_IOC_MAGIC, 26, int)	/* 通道6,开启除 2		*/
#define IMX28_ADC_VBAT_DIV4     _IOW(IMX28_ADC_IOC_MAGIC, 27, int)	/* 测量电池电压,开启除4		*/

#endif
