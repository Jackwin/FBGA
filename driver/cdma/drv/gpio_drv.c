#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/mman.h>
#include <linux/pagemap.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/ioport.h>
#include <linux/of.h>
#include <linux/uaccess.h>

#include <linux/irq.h>
#include <linux/interrupt.h>


#define MAX_BUF 64
#define GPIO_SIZE 1024*128
#define CDMA_REG_SIZE 1024*64
#define GPIO_BASS_ADD 0x41200000
#define CDMA_BASS_ADD 0x7e200000

static void __iomem *gpio_drv_base; //gpio_drv base address
static void __iomem *cdma_drv_base; //cdma_drv base address
static unsigned char *fbga_drv_base; //fbga_drv base address
static unsigned char *kmalloc_area;

int fbga_drv_open(struct inode * inode, struct file *filp)
{
    printk("device is open!\n");
    return 0;
}

int fbga_drv_release(struct inode *inode, struct file *filp)
{
    printk("device is release!\n");
    return 0;
}

static int fbga_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long p = **ppos;
    unsigned int count = size;
    struct fbga_drv_priv_data *devp = filp->private_data;

    if(p >= GPIO_SIZE)
	return count? -EINVAL:0;
    if(count > GPIO_SIZE - p)
	count = GPIO_SIZE - p;
    if(copy_to_user(buf, (void*)(devp->mem+p), count))
    {
	ret = -EINVAL;
    }
    else
    {
	*ppos +=count;
	ret = count;
	printk(KERN_INFO "read %d bytes from %d\n", count, p);
    }
    return ret;
}

static int fbga_drv_write(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos;
    unsigned int count = size;
    int val, ret;

    ret = copy_from_user(&val, buf, count);//copy data from APP buffer to val

    iowrite32(val, fbga_drv_base);

    printk("fbga_drv: write 0x%x to 0x%x. \n", val, (unsigned int)fbga_drv_base);
    return 0;
}

//static int fbga_drv_map(struct file *filp, unsigned char *buffer, struct vm_area_struct *vma)
//{
//    unsigned long page;
//    unsigned char i;
//    unsigned long start; = (unsigned long)vma->vm_start;
//    unsigned long size;
//    void *kaddr;
//
//    start = (unsigned long)vma->vm_start;
//    size = (unsigned long)(vma->vm_end - vma->vm_start);
//    vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;
//    vma->vm_private_date = filp->private_data;
//    kaddr = filp->private_data;
//
//    if(size > GPIO_SIZE)  //MAX mmap size cannot supass GPIO_SIZE
//	size = GPIO_SIZE;
//
//    page = virt_to_phys(kaddr); //Get APP's PHY addr
//
//    if(remap_pfn_range(vma, start, page>>PAGE_SHIFT, size, PAGE_SHARED)) //Remap APP's VIR addr to OS's consective PHY page
//    {
//	printk("Error remap_pfn!\n");
//	return -1;
//    }
//    return 0;
//}


static lofft_t fbga_drv_llseek(struct file *filp, lofft_t offset, int orig)
{
    loff_t ret=0;
    switch(orig)
    {
	case SEEK_SET:
	    if(offset < 0)
	    {
		ret = -EINVAL;
		break;
	    }
	    if((unsigned int) offset > GPIO_SIZE)
	    {
		ret = -EINVAL;
		break;
	    }
	    filp->f_ops = (unsigned int)offset;
	    ret = filp->f_ops;
	    break;
	case SEEK_CUR:
	    if((filp->f_pos+offset)>GPIO_SIZE)
	    {
		ret = -EINVAL;
		break;
	    }
	    if((filp->f_ops+offset)<0)
	    {
		ret = -EINVAL;
		break;
	    }
	    filp->f_ops += offset;
	    ret = filp->f_ops;
	    break;
	default:
	    ret = -EINVAL;
	    break;
    }
    return ret;
}

static const struct file_operations fbga_drv_fops=
{
    .owner = THIS_MODULE,
    .open = fbga_drv_open,
    .release = fbga_drv_release,
    .write = fbga_drv_write,
    .llseek = fbga_drv_llseek,
};

static struct miscdevice fbga_drv_dev=
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = "fbga_drv_dev",
    .fops = &fbga_drv_fops,
};

static int dma_transfer(){

    unsigned long phy_addr;
    phy_addr = virt_to_phys((void *)kmalloc_area); 


}

irqreturn dma_irq_handle(int irq, void *dev_id)
{
    task_schedule(&xxx_tasklet);
    return IRQ_HANDLED;
}

static int fbga_drv_init(void)
{
    int ret;

    gpio_drv_base = ioremap(GPIO_BASS_ADD, GPIO_SIZE); //change phy add to vir add
    cdma_drv_base = ioremap(CDMA_BASS_ADD, CDMA_SIZE); //change phy add to vir add
    
    kmalloc_area = kmalloc(BUFFER_BYTESIZE, __GFP_DMA);
    if(!kmalloc_area) return -EINVAL;

    SetPageReserved(virt_to_page(kmalloc_area);

    result = request_irq(DMA_IRQ_NUM, dma_irq_handle, IRQF_DISABLED, "xxx", NULL);

    printk("GPIO: Access address to device is:0x%x\n", (unsigned int)gpio_drv_base);
    printk("CDMA: Access address to device is:0x%x\n", (unsigned int)cdma_drv_base);
    ret = misc_register(&fbga_drv_dev);
    if(ret)
    {
	printk("fbga_drv:[ERROR] Misc device register faifbga_drv.\n");
	return ret;
    }

    printk("Module init complete!\n");
    return 0;
}

static void fbga_drv_exit(void)
{
    printk("Module exit!\n");
    iounmap(gpio_drv_base);
    iounmap(cdma_drv_base);
    kfree(kmalloc_area);
    free_irq(DMA_IRQ_NUM);
    misc_deregister(&fbga_drv_dev);
}

module_init(fbga_drv_init);
module_exit(fbga_drv_exit);

MODULE_AUTHOR("CHENGL");
MODULE_DESCRIPTION("FBGA_DRV");
MODULE_LICENSE("Dual BSD/GPL");
