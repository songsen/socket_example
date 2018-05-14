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

#include <linux/ktime.h>

//linux-2.6.29/arch/arm/include/asm
#include<asm/io.h>                                                      /* ioctl                        */
#include<asm/uaccess.h>                                                 /* get_user()                   */

//linux-2.6.29/arch/arm/mach-mx28/include/mach
#include <mach/hardware.h>
#include <mach/device.h>
#include <mach/regs-timrot.h>
#include <mach/regs-pwm.h>
#include <../mx28_pins.h>
#include <../regs-pinctrl.h>
#include <../regs-clkctrl.h>

#include "timrot.h"

#define REGS_TIMROT_PHYS	0x80068000
#define REGS_GPIO_PHYS      0X80018000

#define DEVICE_NAME "timrot_misc"

#define PWM3_GPIO3_28	124	
#define PWM4_GPIO3_21   117

#define SUCCESS 1
#define FAILT   0

#define DEBUG_DATA_GPIO 1

#define DEBUG_OUTPUT_PIN() gpio_set_value(PWM3_GPIO3_28,!gpio_get_value(PWM3_GPIO3_28))

#define PWM_OUTPUT_REGISTER 1

/**
 *  	divide by 8
 * 		TEST SIGNAL From 32khz
 * 		select from PWM channel 4
 *      pwm4 作为pwm 测量功能的时钟来源
 * 		pwm3 通道作为pwm 测量通道
 * */
#define reg_TIMROT_TIMCTRL3   \
		BF_TIMROT_TIMCTRL3_PRESCALE(BV_TIMROT_TIMCTRL3_PRESCALE__DIV_BY_8)| \
		BF_TIMROT_TIMCTRL3_TEST_SIGNAL(BV_TIMROT_TIMCTRL3_TEST_SIGNAL__1KHZ_XTAL)| \
		BF_TIMROT_TIMCTRL3_SELECT(BV_TIMROT_TIMCTRL3_SELECT__32KHZ_XTAL)   

#define reg_HW_PWM_CTRL_SET \
		BM_PWM_CTRL_SFTRST|BM_PWM_CTRL_CLKGATE |BM_PWM_CTRL_PWM3_ENABLE

#define LED_FULL 255
#define reg_HW_PWM_PERIODn_SET \
		(BF_PWM_PERIODn_CDIV(5) | /* divide by 64 */ 	\
		BF_PWM_PERIODn_INACTIVE_STATE(BV_PWM_PERIODn_INACTIVE_STATE__0) | /* low */ 	\
		BF_PWM_PERIODn_ACTIVE_STATE(BV_PWM_PERIODn_ACTIVE_STATE__1) | /* high */ 	\
		BF_PWM_PERIODn_PERIOD(LED_FULL)) /* 255 cycles */

#define reg_HW_PWM_ACTIVEn_SET \
		BF_PWM_ACTIVEn_INACTIVE(LED_FULL) | \
		BF_PWM_ACTIVEn_ACTIVE(115)

static void __iomem *timrot_cbase = NULL;
static void __iomem *pwm_base = NULL;
static void __iomem *gpio_ctrl_base = NULL;
static void __iomem *clkctrl_base = NULL;

static uint8_t data_duty[6] = {0};

static struct clk *pwm_clk;
static uint16_t Delay_146us;


static uint8_t getOneByte(void)
{
	uint8_t data = 0;
	uint8_t i = 0;
    for( i=0;i<8;i++)	   
    {

        Delay_146us = 65000;	
   		while((!gpio_get_value(PWM4_GPIO3_21))\
		   &&Delay_146us++);

#if(DEBUG_DATA_GPIO)  //1
	DEBUG_OUTPUT_PIN();
#endif 
		udelay(30);
#if(DEBUG_DATA_GPIO)  //0
	DEBUG_OUTPUT_PIN();
#endif 
		data <<= 1;
        if(gpio_get_value(PWM4_GPIO3_21)) 
			data |= 0x01;
		

        Delay_146us = 65000;
        while((gpio_get_value(PWM4_GPIO3_21))\
		&&Delay_146us++);
   	  
        if(gpio_get_value(PWM4_GPIO3_21)) 
			break;
    }
	return data;
}

static uint8_t *get5Bytes(uint8_t *buf ,int size )
{
	int i;

	if(size < 5){
		return NULL;
	}

#if(DEBUG_DATA_GPIO)  //0
	DEBUG_OUTPUT_PIN();
#endif 

	//gpio_direction_output(PWM4_GPIO3_21,1);  //设置成output话花费较多时间
	iowrite32(1<<21,							gpio_ctrl_base+ HW_PINCTRL_DOE3_SET);

	mdelay(20);

#if(DEBUG_DATA_GPIO) //1
	DEBUG_OUTPUT_PIN();
#endif 

	//gpio_direction_input(PWM4_GPIO3_21);
	iowrite32(1<<21,							gpio_ctrl_base+ HW_PINCTRL_DOE3_CLR);

	//wait client pull low
	Delay_146us = 65000;
	while((gpio_get_value(PWM4_GPIO3_21)) \
			&&Delay_146us++);

#if(DEBUG_DATA_GPIO) //0
	DEBUG_OUTPUT_PIN();
#endif 

	if(gpio_get_value(PWM4_GPIO3_21))
		return NULL;
	
	//wait 80us for low level over 
	Delay_146us = 65000;
	while((!gpio_get_value(PWM4_GPIO3_21)) \
			&&Delay_146us++);
	if(!gpio_get_value(PWM4_GPIO3_21))
		return NULL;

	Delay_146us = 65000;
	while((gpio_get_value(PWM4_GPIO3_21)) \
			&&Delay_146us++);
	if(gpio_get_value(PWM4_GPIO3_21))
		return NULL;


	for( i=0;i<5;i++){
		buf[i] = getOneByte();
	}

#if(DEBUG_DATA_GPIO) //1
	DEBUG_OUTPUT_PIN();
#endif 
	return buf;
}

static int timrot_misc_open (struct inode *inode, struct file *fp) 
{

	try_module_get(THIS_MODULE);   

	//
	iowrite32(reg_TIMROT_TIMCTRL3,	timrot_cbase+ HW_TIMROT_TIMCTRL3_SET);

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
    if( count > 6){
        printk("Max length is 64\n");
        count = 6;
    }

	if(get5Bytes(data_duty,sizeof(data_duty)) == NULL){
		printk("device err\n");
		return -EFAULT;  
	}

	if (copy_to_user((void *)buf, data_duty, count)) {
		printk("copy_to_user err\n");
		return -EFAULT;  
    }

	return count;
#if 0

	iowrite32(BM_TIMROT_TIMCTRL3_DUTY_CYCLE,
	timrot_cbase + HW_TIMROT_TIMCTRL3_SET);
    //....从设备获取数据
	data_duty[1] = ioread32(timrot_cbase+ HW_TIMROT_RUNNING_COUNT3);
	data_duty[2] = ioread32(timrot_cbase+ HW_TIMROT_FIXED_COUNT3);

    if (copy_to_user((void *)buf, data_duty, count)) {
        printk("copy_to_user err\n");
		iowrite32(BM_TIMROT_TIMCTRL3_DUTY_CYCLE,
		timrot_cbase + HW_TIMROT_TIMCTRL3_CLR);
        return -EFAULT;  
    }

	iowrite32(BM_TIMROT_TIMCTRL3_DUTY_CYCLE,
	timrot_cbase + HW_TIMROT_TIMCTRL3_CLR);
    return count;
#endif
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

void printmsg(void){
		printk("#\n \
			HW_PINCTRL_MUXSEL7:%08x\n \
			HW_PWM_ACTIVEn:%08x\n \
			HW_PWM_PERIODn:%08x\n \
			HW_PINCTRL_CTRL:%08x\n \
			HW_PINCTRL_DRIVE15:%08x\n\
			HW_CLKCTRL_XTAL:%08x\n\
			HW_PWM_CTRL:%08x\n",\
	ioread32(gpio_ctrl_base + HW_PINCTRL_MUXSEL7),
	ioread32(pwm_base+ HW_PWM_ACTIVEn(3)),
	ioread32(pwm_base+ HW_PWM_PERIODn(3)),
	ioread32(gpio_ctrl_base+ HW_PINCTRL_CTRL),
	ioread32(gpio_ctrl_base+HW_PINCTRL_DRIVE15),
	ioread32(clkctrl_base+ HW_CLKCTRL_XTAL),
	ioread32(pwm_base+ HW_PWM_CTRL)
	);
}

static int __init timrotModule_init (void)
{
    int ret=0;
//	int err = -1;
#if PWM_OUTPUT_REGISTER
	//clk ctrl 每个外围设备需要启动clk才能够工作
	clkctrl_base = ioremap(0x80040000,0x200*4);
	if(clkctrl_base == NULL){
		printk("clr ioremap error!\n");
		return -1;
	}
	// timer register map requst
	timrot_cbase = ioremap(REGS_TIMROT_PHYS,0x120*4);    
	if(timrot_cbase == NULL){
		printk("timer ioremap error!\n");
		return -1;
	}
	//pwm register map requst
	pwm_base = ioremap(REGS_PWM_PHYS,0x110*4);
	if(pwm_base == NULL){
		printk("tick ioremap error!\n");
		iounmap(timrot_cbase);
		return -1;
	}
#endif

	gpio_free(PWM3_GPIO3_28);
	gpio_free(PWM4_GPIO3_21);
	if(gpio_request(PWM3_GPIO3_28,"pwm measure")
	&&gpio_request(PWM4_GPIO3_21,"pwm gener")){
		printk("####request %s :%d,%d io error!\n","pwm measure",PWM4_GPIO3_21,PWM3_GPIO3_28);
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
		gpio_free(PWM3_GPIO3_28);
		gpio_free(PWM4_GPIO3_21);
		iounmap(gpio_ctrl_base);
		return -1;
	} 

#define GPIO_TEST  0
#if GPIO_TEST   //GPIO test
	iowrite32(BM_PINCTRL_CTRL_SFTRST|BM_PINCTRL_CTRL_CLKGATE,	gpio_ctrl_base+ HW_PINCTRL_CTRL_CLR);
	iowrite32(BF_PINCTRL_MUXSEL7_BANK3_PIN28(3),				gpio_ctrl_base+ HW_PINCTRL_MUXSEL7_SET);
	iowrite32(BF_PINCTRL_MUXSEL7_BANK3_PIN21(3),				gpio_ctrl_base+ HW_PINCTRL_MUXSEL7_SET);

	iowrite32(1<<28,							gpio_ctrl_base+ HW_PINCTRL_DOE3_SET);

	iowrite32(1<<28,						    gpio_ctrl_base+ HW_PINCTRL_DOUT3_SET);
	mdelay(100);
	iowrite32(1<<28,						    gpio_ctrl_base+ HW_PINCTRL_DOUT3_CLR);
	mdelay(100);
		iowrite32(1<<28,						    gpio_ctrl_base+ HW_PINCTRL_DOUT3_SET);
	mdelay(100);
		iowrite32(1<<28,						    gpio_ctrl_base+ HW_PINCTRL_DOUT3_CLR);
	mdelay(100);
		iowrite32(1<<28,						    gpio_ctrl_base+ HW_PINCTRL_DOUT3_SET);

	iowrite32(BM_PINCTRL_PULL3_BANK3_PIN28, /* PWM3*/		gpio_ctrl_base+ HW_PINCTRL_PULL3_SET); 
	iowrite32(BM_PINCTRL_PULL3_BANK3_PIN21, /* PWM4*/		gpio_ctrl_base+ HW_PINCTRL_PULL3_SET); 

#endif

	//IO config reset
	iowrite32(BM_PINCTRL_CTRL_SFTRST|BM_PINCTRL_CTRL_CLKGATE,	gpio_ctrl_base+ HW_PINCTRL_CTRL_CLR);
	iowrite32(BF_PINCTRL_MUXSEL7_BANK3_PIN21(3), /* PWM4*/		gpio_ctrl_base+ HW_PINCTRL_MUXSEL7_CLR); 
	iowrite32(BF_PINCTRL_MUXSEL7_BANK3_PIN28(3), /* PWM3*/		gpio_ctrl_base+ HW_PINCTRL_MUXSEL7_CLR);
	iowrite32(BF_PINCTRL_MUXSEL7_BANK3_PIN26(3), /* PWM7*/		gpio_ctrl_base+ HW_PINCTRL_MUXSEL7_CLR);
	//IO Config set
	iowrite32(BF_PINCTRL_MUXSEL7_BANK3_PIN28(0), /* PWM3*/		gpio_ctrl_base+ HW_PINCTRL_MUXSEL7_SET); 
	iowrite32(BF_PINCTRL_MUXSEL7_BANK3_PIN21(1), /* PWM4*/		gpio_ctrl_base+ HW_PINCTRL_MUXSEL7_SET); 
	iowrite32(BF_PINCTRL_MUXSEL7_BANK3_PIN26(1), /* PWM4*/		gpio_ctrl_base+ HW_PINCTRL_MUXSEL7_SET); 	

#if PWM_OUTPUT_REGISTER

	//等同于 : iowrite32( 1<<29,clkctrl_base+ HW_CLKCTRL_XTAL_CLR);
	//iowrite32( BM_CLKCTRL_XTAL_PWM_CLK24M_GATE,clkctrl_base+ HW_CLKCTRL_XTAL_SET); //disable XTAL24M
	pwm_clk = clk_get(NULL, "pwm");
	if (IS_ERR(pwm_clk)) {
			ret = PTR_ERR(pwm_clk);
			return ret;
	}
	clk_enable(pwm_clk);


	iowrite32( BM_PWM_CTRL_SFTRST|BM_PWM_CTRL_CLKGATE,			pwm_base+ HW_PWM_CTRL_CLR);
	iowrite32( BM_PWM_CTRL_PWM7_ENABLE|BM_PWM_CTRL_PWM4_ENABLE,	pwm_base+ HW_PWM_CTRL_CLR);

	iowrite32(reg_HW_PWM_ACTIVEn_SET,							pwm_base + HW_PWM_ACTIVEn_SET(7));
	iowrite32(reg_HW_PWM_PERIODn_SET, 							pwm_base + HW_PWM_PERIODn_SET(7));

	iowrite32( BM_PWM_CTRL_PWM7_ENABLE|BM_PWM_CTRL_PWM4_ENABLE,	pwm_base+ HW_PWM_CTRL_SET);
#endif 

	return 0;
}

static void __exit timrotModule_exit (void)                                /* warning:return void          */
{
#if	PWM_OUTPUT_REGISTER
	iounmap(timrot_cbase);
	iounmap(pwm_base);
	iounmap(clkctrl_base);
#endif 

	gpio_direction_output(PWM3_GPIO3_28,1);

	iounmap(gpio_ctrl_base);
	
	gpio_free(PWM3_GPIO3_28);
	gpio_free(PWM4_GPIO3_21);

	clk_disable(pwm_clk);
    clk_put(pwm_clk);

	misc_deregister(&timrot_cmiscdev);
}

module_init(timrotModule_init);
module_exit(timrotModule_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("youngsongsen@gmail.com");
MODULE_DESCRIPTION("pwm measure  ");


