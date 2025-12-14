#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>          // 必须加！struct cdev / cdev_init / cdev_add / cdev_del
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/uaccess.h>       // 必须加！copy_from_user 在这里

#define DTSLED_NAME "r5ett_led"
#define LEDON  1
#define LEDOFF 0

static void __iomem *CCM_CCGR1;
static void __iomem *SW_MUX;
static void __iomem *SW_PAD;
static void __iomem *GPIO5_DR;
static void __iomem *GPIO5_DR;
static void __iomem *GPIO5_GDIR;

struct dtsled_dev {
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct device_node *nd;
};
static struct dtsled_dev dtsled;

static void led_on(void)  { writel(readl(GPIO5_DR) & ~(1 << 3), GPIO5_DR); }
static void led_off(void) { writel(readl(GPIO5_DR) |  (1 << 3), GPIO5_DR); }

static ssize_t dtsled_write(struct file *filp, const char __user *buf,
                            size_t count, loff_t *ppos)
{
    char val;

    if (copy_from_user(&val, buf, 1))
        return -EFAULT;

    if (val)
        led_on();
    else
        led_off();

    return 1;
}

static int dtsled_open(struct inode *inode, struct file *filp)
{
    unsigned int val;

    /* 配置 MUX 为 GPIO5_IO03 */
    val = readl(SW_MUX);
    val &= ~0xF;
    val |= 0x5;
    writel(val, SW_MUX);

    /* PAD 配置（推荐值） */
    writel(0x1F0B0, SW_PAD);

    /* 设置为输出 */
    val = readl(GPIO5_GDIR);
    val |= (1 << 3);
    writel(val, GPIO5_GDIR);

    /* 默认关灯 */
    led_off();

    return 0;
}

static struct file_operations dtsled_fops = {
    .owner = THIS_MODULE,
    .open  = dtsled_open,
    .write = dtsled_write,
};

static int __init dtsled_init(void)
{
    unsigned int val;

    /* 1. 查找设备树节点 */
    dtsled.nd = of_find_node_by_path("/r5ett_led");
    if (!dtsled.nd) {
        printk(KERN_ERR "Cannot find /r5ett_led node!\n");
        return -ENODEV;
    }

    /* 2. 映射寄存器 */
    CCM_CCGR1 = of_iomap(dtsled.nd, 0);
    SW_MUX    = of_iomap(dtsled.nd, 1);
    SW_PAD    = of_iomap(dtsled.nd, 2);
    GPIO5_DR  = of_iomap(dtsled.nd, 3);
    GPIO5_GDIR = of_iomap(dtsled.nd, 4);

    if (!CCM_CCGR1 || !SW_MUX || !SW_PAD || !GPIO5_DR || !GPIO5_GDIR) {
        printk(KERN_ERR "of_iomap failed!\n");
        return -ENOMEM;
    }

    /* 3. 使能 GPIO5 时钟 (bit30:31) */
    val = readl(CCM_CCGR1);
    val &= ~(3 << 30);
    val &= ~(3 << 30);
    val |=  (3 << 30);
    writel(val, CCM_CCGR1);

    /* 4. 注册字符设备 */
    alloc_chrdev_region(&dtsled.devid, 0, 1, DTSLED_NAME);

    cdev_init(&dtsled.cdev, &dtsled_fops);
    cdev_add(&dtsled.cdev, dtsled.devid, 1);

    dtsled.class = class_create(THIS_MODULE, DTSLED_NAME);
    if (IS_ERR(dtsled.class))
        return PTR_ERR(dtsled.class);

    dtsled.device = device_create(dtsled.class, NULL, dtsled.devid, NULL, DTSLED_NAME);
    if (IS_ERR(dtsled.device))
        return PTR_ERR(dtsled.device);

    printk(KERN_INFO "r5ett_led dts driver init OK!\n");
    return 0;
}

static void __exit dtsled_exit(void)
{
    led_off();

    iounmap(CCM_CCGR1);
    iounmap(SW_MUX);
    iounmap(SW_PAD);
    iounmap(GPIO5_DR);
    iounmap(GPIO5_GDIR);

    device_destroy(dtsled.class, dtsled.devid);
    class_destroy(dtsled.class);
    cdev_del(&dtsled.cdev);
    unregister_chrdev_region(dtsled.devid, 1);

    printk(KERN_INFO "r5ett_led dts driver exit\n");
}

module_init(dtsled_init);
module_exit(dtsled_exit);
MODULE_LICENSE("GPL");
