/*
 * Copyright © 2015, Avinaba Dalal "corphish" <d97.avinaba@gmail.com>
 *                                           mod by yoAeroA00 for Nokia X2
 *
 * Charger Control driver for qpnp-charger
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#ifndef __QPNP_CHARGER_CONTROL_H__

#define __QPNP_CHARGER_CONTROL_H__

extern int custom_current;

extern int master_switch;

int check_switch_validity (int input_switch);

int check_current_limits (int input_current);

#endif 
