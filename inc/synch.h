/*
 * synch.h
 *
 *  Created on: Dec 3, 2020
 *      Author: vlykhohub
 */

#ifndef SYNCH_H_
#define SYNCH_H_

#include <linux/ktime.h>
#include <linux/list.h>
#include <linux/spinlock_types.h>

struct time_head_t {
	struct list_head head;
	struct spinlock lock;
	__u32 num;
};

struct time_node_t {
	struct list_head node;
	ktime_t time;
	__u32 num;
};

#endif /* SYNCH_H_ */
