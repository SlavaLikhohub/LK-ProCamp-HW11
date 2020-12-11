// SPDX-License-Identifier: GPL-2.0

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#define DEBUG

#include "synch.h"
#include "gpio_helpers.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/types.h>

#define MS_TO_NS(x) ((x) * NSEC_PER_MSEC)

MODULE_AUTHOR("Viaheslav Lykhohub <viacheslav.lykhohub@globallogic.com>");
MODULE_DESCRIPTION("Synchronization methods");
MODULE_LICENSE("GPL");

/* Time list */
static struct time_head_t time_head = {
	//.node = LIST_HEAD_INIT(time_head.node),
	.lock = __SPIN_LOCK_UNLOCKED(time_head.lock),
	.num = 0,
};

static struct time_node_t *node_arr = NULL;

/* Jiffies timer */
static struct timer_list jiff_timer;
static u64 delay_ms = 1000L;

/* IRQ */
int button_irq = -1;
int button_irq_counter = 0;

static int init_list(u32 len)
{
	typeof(len) i;

	node_arr = kmalloc_array(len, sizeof(*node_arr), GFP_KERNEL);

	INIT_LIST_HEAD(&time_head.node);

	if (unlikely(!node_arr)) {
		pr_crit("Cannot allocate memory. Abort...\n");
		return -ENOMEM;
	}

	for (i = 0; i < len; i++) {
		node_arr[i].num = 0;
		node_arr[i].time = 0;

		list_add(&node_arr[i].node, &time_head.node);
	}

	pr_info("Time list has been initialized\n");

	return 0;
}

/* IRQ functions */
irqreturn_t button_irq_handler(int irq, void *data)
{
	return IRQ_WAKE_THREAD;
}

irqreturn_t button_thread_handler(int irq, void *data)
{
	struct time_node_t *node;
	_Bool found = 0;

	pr_debug("Entered button_thread_handler\n");

	spin_lock(&time_head.lock);
	list_for_each_entry(node, &time_head.node, node) {
		if (node->num != 0) {
			pr_info("Index: %d, time: %lli", node->num, node->time);
			node->num = 0;
			node->time = 0;
			found = 1;
			break;
		}
	}
	spin_unlock(&time_head.lock);

	if (!found)
		pr_warn("List is empty\n");

	return IRQ_HANDLED;
}

static int init_button_irq(void)
{
	int rc;

	button_irq = gpio_to_irq(BUTTON);
	if (unlikely(button_irq < 0))
		return button_irq;

	rc = request_threaded_irq(	button_irq,
					button_irq_handler,
					button_thread_handler,
					IRQF_TRIGGER_FALLING,
					"button_irq",
					(void *)&button_irq_counter);

	if (unlikely(rc < 0)) {
		pr_crit("Cannot initialize button IRQ...\n");
		return rc;
	}

	pr_info("Button IRQ has been initialized\n");

	return 0;
}

static inline void deinit_button_irq(void)
{
	free_irq(button_irq, (void *)&button_irq_counter);
}

static void timer_func(struct timer_list *timer)
{
	struct time_node_t *node_i;
	pr_debug("Entered timer_func\n");

	spin_lock(&time_head.lock);
	time_head.num++;

	node_i = list_first_entry(&time_head.node, struct time_node_t, node);
	if (node_i->num == 0) {
		node_i->num = time_head.num;
		node_i->time = ktime_get();
		list_rotate_left(&time_head.node);
	} else {
		pr_err("List is overflowed. Current index %d\n", time_head.num);
	}

	spin_unlock(&time_head.lock);
	mod_timer(timer, jiffies + msecs_to_jiffies(delay_ms));
}

static int __init synch_init(void)
{
	int rc;

	rc = init_list(5);
	if (rc)
		return rc;

	rc = init_button_irq();
	if (rc)
		goto err_button_irq_init;

	rc = init_gpios(BUTTON, LED);
	if (rc)
		goto err_gpios_init;

	timer_setup(&jiff_timer, timer_func, 0);
	rc = mod_timer(&jiff_timer, jiffies + msecs_to_jiffies(delay_ms));
	if (rc)
		goto err_timer_fire;

	return 0;

err_timer_fire:
	deinit_gpios(BUTTON, LED);
err_gpios_init:
	deinit_button_irq();
err_button_irq_init:
	kfree(node_arr);
	return rc;
}

static void __exit synch_exit(void)
{
	del_timer(&jiff_timer);
	deinit_gpios(BUTTON, LED);
	deinit_button_irq();
	kfree(node_arr);
}

module_init(synch_init);
module_exit(synch_exit);

