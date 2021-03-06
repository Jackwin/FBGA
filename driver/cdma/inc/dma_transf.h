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
#include <linux/irqreturn.h>
#include <linux/interrupt.h>


#define XAXICDMA_CR_OFFSET      0x00000000  /**< Control register */
#define XAXICDMA_CR_RESET_MASK  0x00000004 /**< Reset DMA engine */
#define XAXICDMA_SR_OFFSET      0x00000004  /**< Status register */
#define XAXICDMA_SR_IDLE_MASK   0x00000002  /**< DMA channel idle */
#define XAXICDMA_CR_SGMODE_MASK 0x00000008 /**< Scatter gather mode */
#define XAXICDMA_SRCADDR_OFFSET 0x00000018  /**< Source address register */
#define XAXICDMA_DSTADDR_OFFSET 0x00000020  /**< Destination address register */
#define XAXICDMA_BTT_OFFSET     0x00000028  /**< Bytes to transfer */
#define XAXICDMA_XR_IRQ_ALL_MASK    0x00007000 /**< All interrupts */
#define XAXICDMA_XR_IRQ_IOC_MASK    0x00001000 /**< Completion interrupt */
#define XAXICDMA_XR_IRQ_DELAY_MASK  0x00002000 /**< Delay interrupt */
#define XAXICDMA_XR_IRQ_ERROR_MASK  0x00004000 /**< Error interrupt */


static irqreturn_t dma_irq_handle(int irq, void *dev_id);

int reset_cdma(void);

int DMAStart(unsigned int char_cnt, unsigned int direction);

#define PL_BRAM_SRC_ADDRESS  0xC4000000
