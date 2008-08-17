/*
 * $Id: xep.c,v 1.5 2008/08/17 21:28:32 cdidier Exp $
 *
 * Copyright (C) 2007 Colin DIDIER
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "module.h"

#include "disco.h"
#include "ping.h"
#include "vcard.h"
#include "version.h"

void
xep_init(void)
{
	disco_init();
	ping_init();
	vcard_init();
	version_init();
}

void
xep_deinit(void)
{
	disco_deinit();
	ping_deinit();
	vcard_deinit();
	version_deinit();
}
