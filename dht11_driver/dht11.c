/**
 * 
 * 
 * 
 * 
 */

//linux-2.6.29/include/linux
#include<linux/module.h>                                                /* module                       */
#include<linux/fs.h>                                                    /* file operation               */
#include<linux/miscdevice.h>                                            /* miscdevice                   */

#include <linux/kernel.h>
#include <linux/sysdev.h>
#include <linux/platform_device.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/clk.h>

#include<linux/jiffies.h>
#include <linux/ktime.h>

//linux-2.6.29/arch/arm/include/asm
#include<asm/io.h>                                                      /* ioctl                        */
#include<asm/uaccess.h>                                                 /* get_user()                   */

//#include <string.h>

//linux-2.6.29/arch/arm/mach-mx28/include/mach
#include <mach/hardware.h>
#include <mach/device.h>
#include <mach/regs-timrot.h>
#include <mach/regs-pwm.h>
#include <../mx28_pins.h>
#include <../regs-pinctrl.h>
#include <../regs-clkctrl.h>

//#include "timrot.h"

#define REGS_GPIO_PHYS      0X80018000


#define DEVICE_NAME "dht11_misc"


#define TX_DEBUG_PIN	117     //GPIO3_21  PWM4
#define RX_SAMP_PIN     124	  //GPIO3_28  PWM3


#define SUCCESS 1  
#define FAILT   0

#define DEBUG_DATA_GPIO 0
#define DEBUG_OUTPUT_PIN() gpio_set_value(TX_DEBUG_PIN,!gpio_get_value(TX_DEBUG_PIN))

static volatile unsigned long current_jiffies;
static volatile unsigned long pre_jiffies;
// static void __iomem *timrot_cbase = NULL;
// static void __iomem *pwm_base = NULL;
static void __iomem *gpio_ctrl_base = NULL;
//static void __iomem *clkctrl_base = NULL;

static uint8_t data_duty[6] = {0};

//static struct clk *pwm_clk;
static uint16_t Delay_146us;

inline void rx_direction_output()
{
	iowrite32(1<<28,gpio_ctrl_base+ HW_PINCTRL_DOE3_SET);
}

inline void rx_direction_input()
{
	iowrite32(1<<28,gpio_ctrl_base+ HW_PINCTRL_DOE3_CLR);
}

static uint8_t getOneByte(void)
{
	uint8_t data = 0;
	uint8_t i = 0;
    for( i=0;i<8;i++)	   
    {

        Delay_146us = 65000;	
   		while((!gpio_get_value(RX_SAMP_PIN))\
		   &&Delay_146us++);

#if(DEBUG_DATA_GPIO)  //1
	DEBUG_OUTPUT_PIN();
#endif 
		udelay(30);
#if(DEBUG_DATA_GPIO)  //0
	DEBUG_OUTPUT_PIN();
#endif 
		data <<= 1;
        if(gpio_get_value(RX_SAMP_PIN)) 
			data |= 0x01;
		

        Delay_146us = 65000;
        while((gpio_get_value(RX_SAMP_PIN))\
		&&Delay_146us++);
   	  
        if(gpio_get_value(RX_SAMP_PIN)) 
			break;
    }
	return data;
}

static uint8_t *get5Bytes(uint8_t *buf ,int size )
{
	int i;
	uint8_t *ret = NULL;

	if(size < 5){
		return NULL;
	}
	//温度/湿度变化率不快,可以认为 60s 内读取的结果相同
	current_jiffies = jiffies;  //读取当前的值
	if(time_after(pre_jiffies,current_jiffies)){
		return buf;
	}
	pre_jiffies = current_jiffies + HZ*10;

	for(i=0;i<size;i++){
		buf[i] = 0;
	}

#if(DEBUG_DATA_GPIO)  //0
	DEBUG_OUTPUT_PIN();
#endif 

	rx_direction_output(); //output low 18s

	//mdelay(20);
	msleep(18);

#if(DEBUG_DATA_GPIO) //1
	DEBUG_OUTPUT_PIN();
#endif 
	//禁止中断
	local_irq_disable();
	rx_direction_input();

	//wait client pull low
	Delay_146us = 65000;
	while((gpio_get_value(RX_SAMP_PIN)) \
			&&Delay_146us++);

#if(DEBUG_DATA_GPIO) //0
	DEBUG_OUTPUT_PIN();
#endif 

	if(gpio_get_value(RX_SAMP_PIN))
		goto LabelErr;
	
	// 80us for low level  
	Delay_146us = 65000;
	while((!gpio_get_value(RX_SAMP_PIN)) \
			&&Delay_146us++);
	if(!gpio_get_value(RX_SAMP_PIN))
		goto LabelErr;

	//80us for High level
	Delay_146us = 65000;
	while((gpio_get_value(RX_SAMP_PIN)) \
			&&Delay_146us++);
	if(gpio_get_value(RX_SAMP_PIN))
		goto LabelErr;


	for( i=0;i<5;i++){
		buf[i] = getOneByte();
	}

#if(DEBUG_DATA_GPIO) //1
	DEBUG_OUTPUT_PIN();
#endif 

	ret = buf;
LabelErr:
	local_irq_enable();
	return ret;
}

static int timrot_misc_open (struct inode *inode, struct file *fp) 
{

	try_module_get(THIS_MODULE);   

	return 0;
}

static int timrot_misc_release (struct inode *inode, struct file *fp)
{
	//printk(KERN_INFO DEVICE_NAME "closed!\n");
	module_put(THIS_MODULE);
    
	return 0;
}

static ssize_t timrot_misc_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{ 
	uint8_t *ret;

    if( count > 6){
        printk("Max length is 64\n");
        count = 6;
    }

	//preempt_disable();
	//local_irq_disable();
	ret = get5Bytes(data_duty,sizeof(data_duty));
	//local_irq_enable();
	//preempt_enable();

	// printk("####mosi:%d.%d  tem:%d.%d  sum:%d\n",
	// data_duty[0],
	// data_duty[1],
	// data_duty[2],
	// data_duty[3],
	// data_duty[4],
	// data_duty[5]);

	if(ret == NULL){
		printk("device err\n");
		return -EFAULT;  
	}

	if (copy_to_user((void *)buf, data_duty, count)) {
		printk("copy_to_user err\n");
		return -EFAULT;  
    }

	return count;
}


struct file_operations timrot_misc_fops = 
{
	.owner		= THIS_MODULE,
	.open		= timrot_misc_open,
	.release	= timrot_misc_release,
    .read 		= timrot_misc_read,
	//.ioctl      	= timrot_misc_iotcl,
};

static struct miscdevice timrot_cmiscdev = 
{
	.minor	        = MISC_DYNAMIC_MINOR,
   	.name	        = DEVICE_NAME,
    .fops	        = &timrot_misc_fops,
};


static int __init timrotModule_init (void)
{
    int ret=0;

#if(DEBUG_DATA_GPIO) //1
	gpio_free(TX_DEBUG_PIN);
	if(gpio_request(TX_DEBUG_PIN,"pwm gener")){
		printk("####request %s :%d io error!\n","debug pin",TX_DEBUG_PIN);
		return -1;
	}
#endif
	
	gpio_free(RX_SAMP_PIN);
	if(gpio_request(RX_SAMP_PIN,"pwm gener")){
		printk("####request %s :%d io error!\n","measure pin",RX_SAMP_PIN);
		return -1;
	}

	gpio_ctrl_base = ioremap(REGS_GPIO_PHYS,0x1B80*4);
	if(gpio_ctrl_base == NULL){
		printk("GPIO ioremap error!\n");
		return -1;
	}

	ret = misc_register(&timrot_cmiscdev);
	if (ret) {
		printk("register failed!\n");

#if(DEBUG_DATA_GPIO) //1
		gpio_free(TX_DEBUG_PIN);
#endif 
		gpio_free(RX_SAMP_PIN);
		iounmap(gpio_ctrl_base);
		return -1;
	} 

	//enable normal operation 
	iowrite32(BM_PINCTRL_CTRL_SFTRST|BM_PINCTRL_CTRL_CLKGATE,	gpio_ctrl_base+ HW_PINCTRL_CTRL_CLR);
	//IO MUXSEL config , as GPIO 
	iowrite32(BF_PINCTRL_MUXSEL7_BANK3_PIN21(3), /* PWM4*/		gpio_ctrl_base+ HW_PINCTRL_MUXSEL7_CLR); 
	iowrite32(BF_PINCTRL_MUXSEL7_BANK3_PIN21(3), /* PWM4*/		gpio_ctrl_base+ HW_PINCTRL_MUXSEL7_SET); 
	//diasable pull register
	iowrite32(BM_PINCTRL_PULL3_BANK3_PIN21, /* PWM4*/		gpio_ctrl_base+ HW_PINCTRL_PULL3_SET); 

	rx_direction_input();
	gpio_set_value(RX_SAMP_PIN,0);


#if(DEBUG_DATA_GPIO) //1
	iowrite32(BF_PINCTRL_MUXSEL7_BANK3_PIN28(3), /* PWM3*/		gpio_ctrl_base+ HW_PINCTRL_MUXSEL7_CLR);
	iowrite32(BM_PINCTRL_PULL3_BANK3_PIN28, /* PWM3*/		gpio_ctrl_base+ HW_PINCTRL_PULL3_SET); 
	iowrite32(BF_PINCTRL_MUXSEL7_BANK3_PIN28(3), /* PWM3*/		gpio_ctrl_base+ HW_PINCTRL_MUXSEL7_SET); 
	gpio_direction_output(TX_DEBUG_PIN,1);
#endif

	//加载后12s才能读取传感器
	pre_jiffies = jiffies + HZ*12;
	return 0;
}

static void __exit timrotModule_exit (void)                                /* warning:return void          */
{

#if(DEBUG_DATA_GPIO) //1
	gpio_direction_output(TX_DEBUG_PIN,1);
	gpio_free(TX_DEBUG_PIN);
#endif

	iounmap(gpio_ctrl_base);
	gpio_free(RX_SAMP_PIN);
	misc_deregister(&timrot_cmiscdev);
}

module_init(timrotModule_init);
module_exit(timrotModule_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("youngsongsen@gmail.com");
MODULE_DESCRIPTION("pwm measure  ");


