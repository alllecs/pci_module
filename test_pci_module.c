#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <linux/init.h>

static int size = 1024;
static int iter = 5;
module_param(size, int, 0);
module_param(iter, int, 0);

static struct pci_device_id i810_ids[] = {
   { PCI_DEVICE(0x191e, 0x5a41) },
   { 0, }
};

MODULE_DEVICE_TABLE( pci, i810_ids );

static void write_pci_mem(int start, u8 __iomem *hwmem_bar1)
{
	int x, i;
	u8 __iomem *hwmem_tmp = hwmem_bar1;

	for (i = start; i < size; i++) {
//		hwmem_bar1[i] = i;
		*(u32 *)hwmem_tmp = i;
		hwmem_tmp += 0x4;
	}

	hwmem_tmp = hwmem_bar1;

	for (i = start; i < size; i++) {
		x = *(u32 *)hwmem_tmp;
		if (x != i)
			printk( KERN_INFO "ERROR PCI MEM %X %X\n", x, i);
		hwmem_tmp += 0x4;
	}
}

static int pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	u16 dval;
	u32 x;
	char byte;
	int j = 0;
	int bar, bar_num = 0;
	unsigned long mmio_start, mmio_len;
	u8 __iomem *hwmem, *hwmem_bar1;

	bar = pci_select_bars(pdev, IORESOURCE_MEM);
	BUG_ON(pci_request_selected_regions_exclusive(pdev, bar, "PCI driver"));

	BUG_ON(pci_enable_device_mem(pdev));
	pci_set_master(pdev);
	// запрашиваем необходимый регион памяти, с определенным ранее типом
	BUG_ON(pci_request_region(pdev, bar, "PCI driver"));
	// получаем адрес начала блока памяти устройства и общую длину этого блока
	mmio_start = pci_resource_start(pdev, bar_num);
	mmio_len = pci_resource_len(pdev, bar_num);
	// мапим выделенную память к аппаратуре
	hwmem = ioremap(mmio_start, mmio_len);

	printk( KERN_INFO "FOUND PCI DEVICE # j = %d\n", j++ );
	printk( KERN_INFO "READING CONFIGURATION REGISTER:\n" );
	printk( KERN_INFO "Bus,Device,Function=%s\n", pci_name( pdev ) );
	pci_read_config_word( pdev, PCI_VENDOR_ID, &dval );
	printk( KERN_INFO " PCI_VENDOR_ID=%x\n", dval );
	pci_read_config_word( pdev, PCI_DEVICE_ID, &dval );
	printk( KERN_INFO " PCI_DEVICE_ID=%x\n", dval );
	pci_read_config_byte( pdev, PCI_REVISION_ID, &byte );
	printk( KERN_INFO " PCI_REVISION_ID=%d\n", byte );
	pci_read_config_byte( pdev, PCI_INTERRUPT_LINE, &byte );
	printk( KERN_INFO " PCI_INTERRUPT_LINE=%d\n", byte );
	pci_read_config_byte( pdev, PCI_LATENCY_TIMER, &byte );
	printk( KERN_INFO " PCI_LATENCY_TIMER=%d\n", byte );
	pci_read_config_word( pdev, PCI_COMMAND, &dval );
	printk( KERN_INFO " PCI_COMMAND=%d\n", dval );

	// READ ID BAR0
	x = ioread32(hwmem + 0x0800);
	printk( KERN_INFO "READ %X\n", x );

	// For BAR1
	bar_num = 1;
	mmio_start = pci_resource_start(pdev, bar_num);
	mmio_len = pci_resource_len(pdev, bar_num);
	hwmem_bar1 = ioremap(mmio_start, mmio_len);

	for (j = 0; j < iter; j++) {
		write_pci_mem(0, hwmem_bar1);
		write_pci_mem(100, hwmem_bar1);
	}

	return 0;
}

static void pci_remove(struct pci_dev *pdev)
{
	pci_release_regions(pdev);
	pci_disable_device(pdev);
	pci_clear_master(pdev);
}
static struct pci_driver own_driver = {
	.name = "pci",
	.id_table = i810_ids,
	.probe = pci_probe,
	.remove = pci_remove
};

static int __init my_init( void ) {
	printk( KERN_INFO "LOADING THE PCI_DEVICE_FINDER\n" );
	return pci_register_driver(&own_driver);
}

static void __exit my_exit( void ) {
	printk( KERN_INFO "UNLOADING THE PCI DEVICE FINDER\n" );
	pci_unregister_driver(&own_driver);
}

module_init( my_init );
module_exit( my_exit );
