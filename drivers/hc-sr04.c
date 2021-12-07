#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/timer.h>

#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/interrupt.h> 

#include "hc-sr04.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oleksandr Povshenko");
MODULE_DESCRIPTION("Driver for HC-SR04 ultrasonic sensor");

typedef struct module_s {
    ktime_t start;
    ktime_t end;
    struct timer_list timer;
    volatile bool loop;
    int distance;
    int irq;
    u8 pulse_state;
}   module_t;

module_t hc_sr04;

void generate_pulse(struct timer_list *timer)
{
    module_t *module = container_of(timer, module_t, timer);
    module->pulse_state ^= 0x1;
    
    gpio_set_value(HCSR04_OUTPUT, module->pulse_state);
    
    if(module->pulse_state)
        mod_timer(timer, jiffies + usecs_to_jiffies(1));
}

// Interrupt handler on ECHO signal
static irqreturn_t gpio_isr(int irq, void *data)
{
    ktime_t current_time;
    module_t *module = (module_t *)data;

    current_time = ktime_get();
    
    if (gpio_get_value(HCSR04_INPUT)) {
        printk(KERN_INFO "start time: %lld\n", current_time);
        module->start = current_time;
    } else {
        printk(KERN_INFO "end time: %lld\n", current_time);
        module->end = current_time;
        module->loop = 0;
    }

	return IRQ_HANDLED;
}

static int gpio_init(module_t *module)
{
    int err = 0;

    err = gpio_request(HCSR04_OUTPUT, "TRIGGER");
    if(err) {
        goto err_triger;
    }

	err = gpio_request(HCSR04_INPUT, "ECHO");
    if(err) {
        goto err_echo;
    }

	gpio_direction_output(HCSR04_OUTPUT, 0);
	gpio_direction_input(HCSR04_INPUT);
    module->pulse_state = 0;

	module->irq = gpio_to_irq(HCSR04_INPUT);
	err = request_irq(module->irq , gpio_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "hc-sr04.trigger", module);
    if(err)
        goto err_irq;

    return 0;

err_irq:
	gpio_free(HCSR04_INPUT);
err_echo:
	gpio_free(HCSR04_OUTPUT);
err_triger:
    return -ENODEV;
}

static inline void gpio_deinit(module_t *module)
{
    free_irq(module->irq, NULL);
	gpio_free(HCSR04_INPUT);
	gpio_free(HCSR04_OUTPUT);
}

static ssize_t pulse_show(struct class *class, struct class_attribute *attr, char *buf)
{
	generate_pulse(&hc_sr04.timer);

	// hc_sr04.start = hc_sr04.end = 0;
    // hc_sr04.loop = 1;

	// while (hc_sr04.loop);
    
    // hc_sr04.distance = ktime_to_us(ktime_sub(hc_sr04.end ,hc_sr04.start)) * 170; // [mm]

	// return sprintf(buf, "Distance: %d [mm]\n", hc_sr04.distance); 
    return sprintf(buf, "Pulse send");
}

CLASS_ATTR_RO(pulse);

static struct class *attr_class;

static int hc_sr04_init(void)
{	
    int err;

    err = gpio_init(&hc_sr04);
	if (err)
        return -ENODEV;

	attr_class = class_create(THIS_MODULE, "hc_sr04");
	if (IS_ERR(attr_class)) {
		err = PTR_ERR(attr_class);
		printk(KERN_ERR "hc_sr04: failed to create sysfs class: %d\n", err);
		goto err_class_create;
	}

	err = class_create_file(attr_class, &class_attr_pulse);
    if (err) {
		printk(KERN_ERR "hc_sr04: failed to create sysfs class attribute pulse: %d\n", err);
        goto err_class_file;
	}

    timer_setup(&hc_sr04.timer, generate_pulse, 0); 

	printk(KERN_INFO "HC-SR04 driver initialized.\n");

	return 0;

err_class_file:
	class_unregister(attr_class);
err_class_create:
    gpio_deinit(&hc_sr04);

    return -ENODEV;
}

static void hc_sr04_exit(void)
{
    gpio_deinit(&hc_sr04);
    class_remove_file(attr_class, &class_attr_pulse);
	class_unregister(attr_class);
    del_timer(&hc_sr04.timer);
	printk(KERN_INFO "HC-SR04 disabled.\n");
}
 
module_init(hc_sr04_init);
module_exit(hc_sr04_exit);
