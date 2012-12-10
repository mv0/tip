/*
 * Support for AMBA devices (both APB and AHB) behind a PCI bridge
 * Copyright 2012 ST Microelectronics (Alessandro Rubini)
 * GNU GPL version 2.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/amba/bus.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/sizes.h>

static int __devinit pci_amba_probe(struct pci_dev *pdev,
				     const struct pci_device_id *id)
{
	struct amba_device *adev;
	char *name;
	int ret;

	pci_enable_msi(pdev);
	ret = pci_enable_device(pdev);
	if (ret)
		return ret;

	/* Create a name: each of them must be different */
	name = devm_kzalloc(&pdev->dev, strlen(dev_name(&pdev->dev)) + 6,
		GFP_KERNEL);
	sprintf(name, "amba-%s", dev_name(&pdev->dev));

	/* Simply build an amba device and register it */
	adev = amba_device_alloc(name,  pdev->resource[0].start, SZ_4K);
	if (!adev)
		return -ENOMEM;
	adev->irq[0] = pdev->irq;

	/* This bridge can host both APB and AHB devices, so set master */
	pci_set_master(pdev);
	if (pdev->vendor == PCI_VENDOR_ID_STMICRO) {
		/* Under sta2x11, DMA is there but limited to 512M */
		adev->dma_mask = SZ_512M - 1;
		adev->dev.coherent_dma_mask = SZ_512M - 1;
	}

	adev->dev.platform_data = pdev->dev.platform_data;
	pci_set_drvdata(pdev, adev);

	return amba_device_add(adev, &pdev->resource[0]);
};

static void __devexit pci_amba_remove(struct pci_dev *pdev)
{
	struct amba_device *adev = pci_get_drvdata(pdev);
	amba_device_unregister(adev);
	pci_disable_msi(pdev);
}

static DEFINE_PCI_DEVICE_TABLE(pci_amba_table) = {
	{PCI_VDEVICE(STMICRO, PCI_DEVICE_ID_STMICRO_UART_HWFC)},
	{PCI_VDEVICE(STMICRO, PCI_DEVICE_ID_STMICRO_UART_NO_HWFC)},
	{PCI_VDEVICE(STMICRO, PCI_DEVICE_ID_STMICRO_SOC_DMA)},
	{PCI_VDEVICE(STMICRO, PCI_DEVICE_ID_STMICRO_I2C)},
	{PCI_VDEVICE(STMICRO, PCI_DEVICE_ID_STMICRO_SPI_HS)},
	{PCI_VDEVICE(STMICRO, PCI_DEVICE_ID_STMICRO_SDIO_EMMC)},
	{PCI_VDEVICE(STMICRO, PCI_DEVICE_ID_STMICRO_SDIO)},
	{PCI_VDEVICE(STMICRO, PCI_DEVICE_ID_STMICRO_AUDIO_ROUTER_DMA)},
	{PCI_VDEVICE(STMICRO, PCI_DEVICE_ID_STMICRO_AUDIO_ROUTER_MSPS)},
	{0,}
};

static struct pci_driver pci_amba_driver = {
	.name		= "pci-amba",
	.id_table	= pci_amba_table,
	.probe		= pci_amba_probe,
	.remove		= __devexit_p(pci_amba_remove),
};

static int __init pci_amba_init(void)
{
	return pci_register_driver(&pci_amba_driver);
}

static void __exit pci_amba_exit(void)
{
	pci_unregister_driver(&pci_amba_driver);
}

module_init(pci_amba_init);
module_exit(pci_amba_exit);

MODULE_LICENSE("GPL");
