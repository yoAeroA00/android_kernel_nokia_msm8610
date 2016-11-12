/*
 * Copyright (C) 2012 Motorola Mobility LLC.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/switch.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/slab.h>


#define H2W_HS_NO_DEVICE	0
#define H2W_HS_HEADSET		1
#define H2W_HS_HEADPHONE	2

struct alsa_to_h2w_data {
	struct switch_dev sdev;
};

static char *name_headsets_with_mic = "Headset with a mic";
static char *name_headsets_no_mic = "Headphone";
static char *name_headsets_pull_out = "No Device";
static char *state_headsets_with_mic = "1";
static char *state_headsets_no_mic = "2";
static char *state_headsets_pull_out = "0";

static struct alsa_to_h2w_data *headset_switch_data;

static void alsa_to_h2w_headset_report(int state)
{
	if (headset_switch_data)
		switch_set_state(&headset_switch_data->sdev, state);
}

static int switch_to_h2w(unsigned long switch_state)
{
	if (switch_state & (1 << SW_HEADPHONE_INSERT)) {
		if (switch_state & (1 << SW_MICROPHONE_INSERT))
			return H2W_HS_HEADSET;
		else
			return H2W_HS_HEADPHONE;
	}
	return H2W_HS_NO_DEVICE;
}


static int alsa_to_h2w_connect(struct input_handler *handler,
		struct input_dev *dev, const struct input_device_id *id)
{
	int ret;
	struct input_handle *handle;

	handle = kzalloc(sizeof(*handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;
	handle->dev = dev;
	handle->handler = handler;
	handle->name = "alsa_to_h2w";

	ret = input_register_handle(handle);
	if (ret)
		goto err_input_register_handle;

	ret = input_open_device(handle);
	if (ret)
		goto err_input_open_device;

	alsa_to_h2w_headset_report(switch_to_h2w(dev->sw[0]));

	return 0;

err_input_open_device:
	input_unregister_handle(handle);
err_input_register_handle:
	kfree(handle);
	return ret;
}

static void alsa_to_h2w_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

static bool alsa_to_h2w_filter(struct input_handle *handle, unsigned int type,
						unsigned int code, int value)
{
	if ((code == SW_HEADPHONE_INSERT) || (code == SW_MICROPHONE_INSERT))
		alsa_to_h2w_headset_report(switch_to_h2w(handle->dev->sw[0]));
	return false;
}


static const struct input_device_id alsa_to_h2w_ids[] = {
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT,
		.evbit = { BIT_MASK(EV_SW) },
	},
	{ },
};

MODULE_DEVICE_TABLE(input, alsa_to_h2w_ids);

static struct input_handler alsa_to_h2w_handler = {
	.filter     = alsa_to_h2w_filter,
	.connect    = alsa_to_h2w_connect,
	.disconnect = alsa_to_h2w_disconnect,
	.name       = "alsa_to_h2w",
	.id_table   = alsa_to_h2w_ids,
};



static ssize_t headset_print_name(struct switch_dev *sdev, char *buf)
{
	const char *name;

	if (!buf)
		return -EINVAL;

	switch (switch_get_state(sdev)) {
	case 0:
		name = name_headsets_pull_out;
		break;
	case 1:
		name = name_headsets_with_mic;
		break;
	case 2:
		name = name_headsets_no_mic;
		break;
	default:
		name = NULL;
		break;
	}

	if (name)
		return snprintf(buf, sizeof(name) + 2, "%s\n", name);
	else
		return -EINVAL;
}

static ssize_t headset_print_state(struct switch_dev *sdev, char *buf)
{
	const char *state;

	if (!buf)
		return -EINVAL;

	switch (switch_get_state(sdev)) {
	case 0:
		state = state_headsets_pull_out;
		break;
	case 1:
		state = state_headsets_with_mic;
		break;
	case 2:
		state = state_headsets_no_mic;
		break;
	default:
		state = NULL;
		break;
	}

	if (state)
		return snprintf(buf, sizeof(state) + 2, "%s\n", state);
	else
		return -EINVAL;
}

static int __devinit alsa_to_h2w_probe(struct platform_device *pdev)
{
	struct alsa_to_h2w_data *switch_data;
	int ret = 0;

	switch_data = kzalloc(sizeof(struct alsa_to_h2w_data), GFP_KERNEL);
	if (!switch_data)
		return -ENOMEM;
	headset_switch_data = switch_data;

	switch_data->sdev.name = "h2w";
	switch_data->sdev.print_name = headset_print_name;
	switch_data->sdev.print_state = headset_print_state;

	ret = switch_dev_register(&switch_data->sdev);
	if (ret < 0)
		goto err_switch_dev_register;

	platform_set_drvdata(pdev, switch_data);

	if (input_register_handler(&alsa_to_h2w_handler))
		pr_info("input_register_handler failed\n");
	return 0;

err_switch_dev_register:
	kfree(switch_data);
	return ret;
}

static int __devexit alsa_to_h2w_remove(struct platform_device *pdev)
{
	struct alsa_to_h2w_data *switch_data = platform_get_drvdata(pdev);

	switch_dev_unregister(&switch_data->sdev);
	kfree(switch_data);
	headset_switch_data = NULL;

	return 0;
}

static struct platform_driver alsa_to_h2w_driver = {
	.probe		= alsa_to_h2w_probe,
	.remove		= __devexit_p(alsa_to_h2w_remove),
	.driver		= {
		.name	= "alsa-to-h2w",
		.owner	= THIS_MODULE,
	},
};

static int __init alsa_to_h2w_init(void)
{
	return platform_driver_register(&alsa_to_h2w_driver);
}

static void __exit alsa_to_h2w_exit(void)
{
	platform_driver_unregister(&alsa_to_h2w_driver);
}

module_init(alsa_to_h2w_init);
module_exit(alsa_to_h2w_exit);

MODULE_DESCRIPTION("Headset ALSA to H2w driver");
MODULE_LICENSE("GPL");
