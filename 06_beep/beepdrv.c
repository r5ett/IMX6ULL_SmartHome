#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/slab.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
 
#define beep_CNT 1
#define beep_NAME "beep"
#define beepOFF 1
#define beepON 0
 
/*beep设备结构体*/
struct beep_dev{
    dev_t devid;    /* 设备号 	 */
    int major;  /* 主设备号	  */
    int minor;  /* 次设备号   */
    struct cdev cdev;   /* cdev 	*/
    struct class *class; /* 类 */
    struct device *device; /* 设备 */
    struct device_node *nd;/*设备节点*/
    int beep_gpio;/* beep所使用的GPIO编号		*/
}beep;/*beep设备*/
static int beep_open(struct inode *inode, struct file *filp){
    filp->private_data = &beep;
    return 0;
}
static ssize_t beep_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt){
    int ret = 0;
    unsigned char databuf[1];
    struct beep_dev *dev = filp->private_data;/*私有数据*/
    ret = copy_from_user(databuf,buf,cnt);
    if(ret > 0){
        return -EINVAL;
    }
    if(databuf[0]== beepON){
        gpio_set_value(dev->beep_gpio,0);
    }else if(databuf[0]== beepOFF){
        gpio_set_value(dev->beep_gpio,1);
    }
    return 0;
}
static int beep_release(struct inode *inode, struct file *filp){
	return 0;
}
/*beep操作集合函数*/
static const struct file_operations beep_fops = {
    .owner  =   THIS_MODULE,
    .write  =   beep_write,
    .open   =   beep_open,
    .release =  beep_release,
};
 
/*驱动入口函数*/
static int __init beep_init(void){
    int ret = 0;
    /*注册字符设备驱动*/
    beep.major = 0;
    if(beep.major){/*给定主设备号*/
        beep.devid = MKDEV(beep.major,0); /*构建设备号*/
        ret = register_chrdev_region(beep.devid,beep_CNT,beep_NAME); /*注册设备号*/
    }else{/*没给定设备号*/
        ret = alloc_chrdev_region(&beep.devid,0,beep_CNT,beep_NAME); /* 申请设备号 */
        beep.major = MAJOR(beep.devid);   /* 获取分配号的主设备号 */
        beep.minor = MINOR(beep.devid);   /* 获取分配号的次设备号 */
    }
    if(ret < 0){
        goto fail_devid;
    }
    printk("beep major=%d ,beep minor =%d\r\n",beep.major,beep.minor);
    /*初始化cdev*/
    beep.cdev.owner = THIS_MODULE;
    cdev_init(&beep.cdev,&beep_fops);/*初始化的 cdev 结构体变量*/
    ret = cdev_add(&beep.cdev,beep.devid,beep_CNT);/*添加字符设备*/
    if(ret){
        goto fail_cdevadd;
    }
    /*创建类*/
    beep.class = class_create(THIS_MODULE,beep_NAME);
    if(IS_ERR(beep.class)){
        ret = PTR_ERR(beep.class);
        goto fail_class;
    }
    /*创建设备*/
    beep.device = device_create(beep.class,NULL,
                                    beep.devid,NULL,beep_NAME);
    if(IS_ERR(beep.device)){
        ret = PTR_ERR(beep.device);
        goto fail_device;
    } 
    /*初始化BEEP*/
    /*用路径获取设备节点：beep*/
    beep.nd = of_find_node_by_path("/beep");
    if(beep.nd == NULL){
        ret = -EINVAL;
        goto fail_nd;
    }
    /*获取设备树中的gpio属性，得到BEEP所使用的BEEP编号 */
    beep.beep_gpio = of_get_named_gpio(beep.nd,"beep-gpios",0);
    if(beep.beep_gpio < 0){
        ret = -EINVAL;
        goto fail_nd;
    }
    /*申请io*/
    ret = gpio_request(beep.beep_gpio,"beep-gpio");
    if(ret){
        printk("can't request beep gpio\r\n");
        goto fail_nd;
    }
    /* 设置 GPIO5_IO01 为输出，并且输出高电平，默认关闭 BEEP */
    ret = gpio_direction_output(beep.beep_gpio,1);
    if(ret < 0){
        goto fail_set;
    }
    /*设置为低电平,开启蜂鸣器*/
    gpio_set_value(beep.beep_gpio,0);
    return 0;
fail_set:
    gpio_free(beep.beep_gpio);
fail_nd:
    device_destroy(beep.class,beep.devid);
fail_device:
    class_destroy(beep.class);
fail_class:
    cdev_del(&beep.cdev);
fail_cdevadd:
    unregister_chrdev_region(beep.devid,beep_CNT);
fail_devid:
    return ret;
}
 
/*驱动出口函数*/
static void __exit beep_exit(void){
    /*设置为高电平,关闭蜂鸣器*/
    gpio_set_value(beep.beep_gpio,1);
    /*释放io*/
    gpio_free(beep.beep_gpio);
    /*摧毁设备*/
    device_destroy(beep.class,beep.devid);
    /*删除类*/
    class_destroy(beep.class);
    /*注销字符设备驱动*/
    cdev_del(&beep.cdev);
    /*释放设备号*/
    unregister_chrdev_region(beep.devid,beep_CNT);
}
/*注册模块注销模块*/
module_init(beep_init);
module_exit(beep_exit);
MODULE_LICENSE("GPL");