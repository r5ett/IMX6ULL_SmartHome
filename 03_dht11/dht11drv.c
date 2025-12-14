#include <linux/delay.h>
#include <linux/ktime.h>

#include <linux/module.h>
#include <linux/poll.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/timer.h>

/*  编程思路如下：
 *  设置GPIO；
 *  主机把 GPIO 设为输出引脚，发送开始信号，然后把 GPIO 设置为输入引脚；
 *  主机判断是否收到 DHT11 的回应信号；
 *  接收到回应信号后，开始读取数据；
*/

struct gpio_desc{
	int gpio;// GPIO 编号（如 115 表示 GPIO4_IO19）
	int irq;// 中断号（本驱动不用）
    char name[128];// GPIO 名字，用于 gpio_request
    int key;// 按键值（本驱动不用）
	struct timer_list key_timer;// 按键值（本驱动不用）
} ;

static int count;// GPIO 数量（这里是1）
static struct gpio_desc *gpios;// 动态分配的 GPIO 描述符数组

/* 主设备号 */
static int major = 0;
static struct class *gpio_class;

/* 调试用：记录每个 bit 的高电平持续时间（纳秒） */
int us_array[40];
int time_array[40];
int us_index;
 
/* 复位 DHT11（拉高电平） */
void dht11_reset(void)
{
	gpio_direction_output(gpios[0].gpio, 1);
}
 
/* 发送起始信号：主机拉低 20ms → 拉高 40us → 改为输入 */
void dht11_start(void)
{
	mdelay(30);
	gpio_set_value(gpios[0].gpio, 0);
	mdelay(20);
	gpio_set_value(gpios[0].gpio, 1);
	udelay(40);	
	
	// 配置引脚为输入，准备接收DHT11传来的数据
	gpio_direction_input(gpios[0].gpio);	
	udelay(2);	
}
 
/* 等待 DHT11 响应信号：80us 低 + 80us 高 + 80us 低 */
static int dht11_wait_for_ready(void)
{
 
	int timeout = 200;
	/* 等待 DHT11 拉低（响应信号开始） */
	while(gpio_get_value(gpios[0].gpio) && --timeout)
	{
		udelay(1);
	}
	if(!timeout)
	{
		return -1;
	}
	/* 等待 DHT11 拉高（响应信号结束） */
	while(!gpio_get_value(gpios[0].gpio) && --timeout)
	{
		udelay(1);
	}
	if(!timeout)
	{
		return -1;
	}
	/* 等待 DHT11 再次拉低（准备发送数据） */
	while(gpio_get_value(gpios[0].gpio) && --timeout)
	{
		udelay(1);
	}
	if(!timeout)
	{
		return -1;
	}
	
	return 0;
}

/* 读取一个字节（8 bit） */
static int dht11_read_byte(unsigned char *buf)
{
	int i;
	unsigned char data = 0;
	int timeout_us = 200;
	u64 pre, last;
	int ns;
	
	for (i = 0; i <8; i++)
	{
		/* 等待低电平结束（每个 bit 以 50us 低电平开始） */
		timeout_us = 400;
		while (!gpio_get_value(gpios[0].gpio) && --timeout_us)
		{
			udelay(1);
		}
		if (!timeout_us)
		{
			printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
			return -1;
		}
 
		/* 测量高电平持续时间：>40us 为 1，<40us 为 0 */
		timeout_us = 20000000;
 
		pre = ktime_get_boot_ns();
		while (gpio_get_value(gpios[0].gpio) && --timeout_us)
		{
			//udelay(1);  /* 实际耗时: 1.6us */
			//us++;
		}
 
		last = ktime_get_boot_ns();
		//printk("udelay 1000 ns = %d\n", last-pre);
 
		ns = last - pre;
		if (!timeout_us)
		{
			return -1;
		}
		us_array[us_index] = ns;
		time_array[us_index++] = 20000000 - timeout_us;
		if (ns > 40000)
		{
			/* get bit 1 */
			data = (data << 1) | 1;
		}
		else
		{
			/* get bit 0 */
			data = (data << 1) | 0;
		}
	}
 
	*buf = data;
	return 0;
 
 
}

/* read 接口：用户空间调用 read(fd, buf, 4) 获取温湿度 */
static ssize_t dht11_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	unsigned long flags;
	int i,err;
	unsigned char data[5];

	/* 防止数组越界 */
	us_index = 0;

	if (size != 4)
		return -EINVAL;

	local_irq_save(flags);	  // 关中断
 
	/* 1. 发送高脉冲启动DHT11 */
	dht11_reset();
	dht11_start();
 
	/* 2. 等待DHT11就绪 */
	if (dht11_wait_for_ready())
	{
		local_irq_restore(flags); // 恢复中断
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		return -EAGAIN;
	}
 
	/* 3. 读5字节数据 */
	for (i = 0; i < 5; i++)
	{
		if (dht11_read_byte(&data[i]))
		{
			local_irq_restore(flags); // 恢复中断
			printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
			return -EAGAIN;
		}
	}
 
	dht11_reset();
	
	local_irq_restore(flags); // 恢复中断
 
	/* 4. 根据校验码验证数据 */
	if (data[4] != (data[0] + data[1] + data[2] + data[3]))
	{
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
 
	/* 5. copy_to_user */	
	/* data[0]/data[1] : 湿度 */
	/* data[2]/data[3] : 温度 */
	err = copy_to_user(buf, data, 4);
	return 4;
}


/* 定义自己的file_operations结构体             */
static struct file_operations gpio_key_drv = {
	.owner	 = THIS_MODULE,
	.read    = dht11_read,
};


/* probe：设备树匹配成功时调用 */
static int gpio_drv_probe(struct platform_device *pdev)
{
    int err = 0;
    int i;
	struct device_node *np = pdev->dev.of_node;
	struct resource *res;
    
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	

	/* 从platfrom_device获得引脚信息 
	 * 1. pdev来自c文件
     * 2. pdev来自设备树
	 */
	
	if (np)
	{
		/* pdev来自设备树 : 示例
        reg_usb_ltemodule: regulator@1 {
            compatible = "100ask,gpiodemo";
            gpios = <&gpio5 5 GPIO_ACTIVE_HIGH>, <&gpio5 3 GPIO_ACTIVE_HIGH>;
        };
		*/
		count = of_gpio_count(np);
		if (!count)
			return -EINVAL;

		gpios = kmalloc(count * sizeof(struct gpio_desc), GFP_KERNEL);
		for (i = 0; i < count; i++)
		{
			gpios[i].gpio = of_get_gpio(np, i);
			sprintf(gpios[i].name, "%s_pin_%d", np->name, i);
		}
	}
	else
	{
		/* pdev来自c文件 
		static struct resource omap16xx_gpio3_resources[] = {
			{
					.start  = 115,
					.end    = 115,
					.flags  = IORESOURCE_IRQ,
			},
			{
					.start  = 118,
					.end    = 118,
					.flags  = IORESOURCE_IRQ,
			},		};		
		*/
		count = 0;
		while (1)
		{
			res = platform_get_resource(pdev, IORESOURCE_IRQ, count);
			if (res)
			{
				count++;
			}
			else
			{
				break;
			}
		}

		if (!count)
			return -EINVAL;

		gpios = kmalloc(count * sizeof(struct gpio_desc), GFP_KERNEL);
		for (i = 0; i < count; i++)
		{
			res = platform_get_resource(pdev, IORESOURCE_IRQ, i);
			gpios[i].gpio = res->start;
			sprintf(gpios[i].name, "%s_pin_%d", pdev->name, i);
		}

	}

	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	/* set pin */

	for (i = 0; i < count; i++)
	{
		err = gpio_request(gpios[i].gpio, gpios[i].name);
		gpio_direction_output(gpios[i].gpio, 1);

	}

	/* 注册file_operations 	*/
	major = register_chrdev(0, "r5ett_dht11", &gpio_key_drv);  /* /proc/devices */

	gpio_class = class_create(THIS_MODULE, "r5ett_dht11_class");
	if (IS_ERR(gpio_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "r5ett_dht11");
		return PTR_ERR(gpio_class);
	}

	device_create(gpio_class, NULL, MKDEV(major, 0), NULL, "r5ett_dht11"); /* /dev/r5ett_dht11 */

	
	return err;
}

/* 有入口函数就应该有出口函数：卸载驱动程序时，就会去调用这个出口函数
 */
static int gpio_drv_remove(struct platform_device *pdev)
{
	int i;
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	device_destroy(gpio_class, MKDEV(major, 0));
	class_destroy(gpio_class);
	unregister_chrdev(major, "r5ett_dht11");

	/* free pin */
	for (i = 0; i < count; i++)
	{
		gpio_free(gpios[i].gpio);
	}

	return 0;
}

static const struct of_device_id gpio_dt_ids[] = {
        { .compatible = "r5ett,dht11", },
        { /* sentinel */ }
};

static struct platform_driver gpio_platform_driver = {
	.driver		= {
		.name	= "r5ett_dht11_plat_drv",
		.of_match_table = gpio_dt_ids,
	},
	.probe		= gpio_drv_probe,
	.remove		= gpio_drv_remove,
};


static int __init dht11_drv_init(void)
{
	/* 注册platform_driver */
	return platform_driver_register(&gpio_platform_driver);
}

static void __exit dht11_drv_exit(void)
{
	/* 反注册platform_driver */
	platform_driver_unregister(&gpio_platform_driver);
}



/* 7. 其他完善：提供设备信息，自动创建设备节点                           */

module_init(dht11_drv_init);
module_exit(dht11_drv_exit);
MODULE_LICENSE("GPL");

