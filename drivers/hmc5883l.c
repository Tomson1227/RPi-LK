#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "hmc5883l.h"

struct hmc5883l {
	struct i2c_client *client;
    int X_axis;
    int Y_axis;
    int Z_axis;
};

static struct hmc5883l hmc5883l;

static int hmc5883l_read_data(void)
{
	struct i2c_client *drv_client = hmc5883l.client;

	if (drv_client == 0)
		return -ENODEV;

	i2c_smbus_write_byte_data(drv_client, MODE_REG, SINGLE_MEASURE_MODE);
	mdelay(6);
	hmc5883l.X_axis = i2c_smbus_read_word_data(drv_client, REG_AXIS_X_MSB);
	hmc5883l.Y_axis = i2c_smbus_read_word_data(drv_client, REG_AXIS_Y_MSB);
	hmc5883l.Z_axis = i2c_smbus_read_word_data(drv_client, REG_AXIS_Z_MSB);

	dev_info(&drv_client->dev, "read data:\n");
	dev_info(&drv_client->dev, "X_axis: %06d\n", hmc5883l.X_axis);
	dev_info(&drv_client->dev, "Y_axis: %06d\n", hmc5883l.Y_axis);
	dev_info(&drv_client->dev, "Z_axis: %06d\n", hmc5883l.Z_axis);

	return 0;
}

static ssize_t axis_X_show(struct class *class, struct class_attribute *attr, char *buf) {
	hmc5883l_read_data();

	sprintf(buf, "X_axis: %d\n", hmc5883l.X_axis);
	return strlen(buf);
}

static ssize_t axis_Y_show(struct class *class, struct class_attribute *attr, char *buf) {
	hmc5883l_read_data();

	sprintf(buf, "Y_axis: %d\n", hmc5883l.Y_axis);
	return strlen(buf);
}

static ssize_t axis_Z_show(struct class *class, struct class_attribute *attr, char *buf) {
	hmc5883l_read_data();

	sprintf(buf, "Z_axis: %d\n", hmc5883l.Z_axis);
	return strlen(buf);
}

static u8 verify_i2c_device(struct i2c_client *drv_client)
{
	if(i2c_smbus_read_byte_data(drv_client, IDENT_REG_A) == 'H' &&
	   i2c_smbus_read_byte_data(drv_client, IDENT_REG_B) == '4' &&
	   i2c_smbus_read_byte_data(drv_client, IDENT_REG_C) == '3')
	   return 1;
	
	return 0;
}

static void set_device_mode(struct i2c_client *drv_client)
{
	i2c_smbus_write_byte_data(drv_client, CONFIG_REG_A, DEFAULT_REG_A);
	i2c_smbus_write_byte_data(drv_client, CONFIG_REG_B, DEFAULT_REG_B);
	i2c_smbus_write_byte_data(drv_client, MODE_REG, 	DEFAULT_MODE);
}

static int hmc5883l_probe(struct i2c_client *drv_client, const struct i2c_device_id *id)
{
	// int reg = 0;
	// struct device_node *node;
	
	dev_info(&drv_client->dev, "i2c client address is 0x%X\n", drv_client->addr);
	dev_info(&drv_client->dev, "i2c driver probed\n");

	if(!verify_i2c_device(drv_client))
		return -ENODEV;

	hmc5883l.client = drv_client;
	set_device_mode(drv_client);
	hmc5883l_read_data();
	
	// node =	drv_client->dev.of_node;
	// of_property_read_u32(node, "reg", &reg);
	// dev_info(&drv_client->dev, "reg = %d\n", reg);

	return 0;
}

static int hmc5883l_remove(struct i2c_client *drv_client)
{
	hmc5883l.client = 0;
	dev_info(&drv_client->dev, "i2c driver removed\n");
	
	return 0;
}

static const struct i2c_device_id hmc5883l_idtable [] = {
    { "hmc5883l", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, hmc5883l_idtable);

static struct i2c_driver hmc5883l_i2c_driver = {
    .driver = {
   	 	.name = "hmc5883l",
    },

    .probe = hmc5883l_probe,
    .remove = hmc5883l_remove,
    .id_table = hmc5883l_idtable,
};

CLASS_ATTR_RO(axis_X);
CLASS_ATTR_RO(axis_Y);
CLASS_ATTR_RO(axis_Z);

static struct class *attr_class;

static int HMC5883L_init(void)
{	
	int ret;

	ret = i2c_add_driver(&hmc5883l_i2c_driver);
	
    if (ret) {
		printk(KERN_ERR "HMC5883L: failed to add new i2c driver: %d\n", ret);
		return ret;
	}

	attr_class = class_create(THIS_MODULE, "hmc5883l");

	if (IS_ERR(attr_class)) {
		ret = PTR_ERR(attr_class);
		printk(KERN_ERR "HMC5883L: failed to create sysfs class: %d\n", ret);
		goto err1;
	}

	ret = class_create_file(attr_class, &class_attr_axis_X);
    if (ret) {
		printk(KERN_ERR "HMC5883L: failed to create sysfs class attribute axis_X: %d\n", ret);
        goto err2;
	}

    ret = class_create_file(attr_class, &class_attr_axis_Y);
	if (ret) {
		printk(KERN_ERR "HMC5883L: failed to create sysfs class attribute axis_Y: %d\n", ret);
        goto err3;
	}

    ret = class_create_file(attr_class, &class_attr_axis_Z);
	if (ret) {
		printk(KERN_ERR "HMC5883L: failed to create sysfs class attribute axis_Z: %d\n", ret);
        goto err4;
	}

	printk(KERN_INFO "HMC5883L: sysfs class created\n");
	printk(KERN_INFO "HMC5883L: driver initialized.\n");

	return 0;

err4:   class_remove_file(attr_class, &class_attr_axis_Y);
err3:   class_remove_file(attr_class, &class_attr_axis_X);
err2:	i2c_del_driver(&hmc5883l_i2c_driver);
err1:   class_destroy(attr_class);
	printk(KERN_INFO "HMC5883L: module initialization fail.\n");

    return ret;
}
 
static void HMC5883L_exit(void)
{
    if (attr_class) {
		class_remove_file(attr_class, &class_attr_axis_Z);
		class_remove_file(attr_class, &class_attr_axis_Y);
		class_remove_file(attr_class, &class_attr_axis_X);
		printk(KERN_INFO "HMC5883L: sysfs class attributes removed\n");

		class_destroy(attr_class);
		printk(KERN_INFO "HMC5883L: sysfs class destroyed\n");
	}

	i2c_del_driver(&hmc5883l_i2c_driver);
	printk(KERN_INFO "HMC5883L: module disabled.\n");
}
 
module_init(HMC5883L_init);
module_exit(HMC5883L_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oleksandr Povshenko");
MODULE_DESCRIPTION("Driver for I2C magnetometer module HMC5883L");

/****************************************************************************************/
