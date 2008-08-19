/*
 * $Id: fe-xep.c,v 1.3 2008/08/19 23:57:15 cdidier Exp $
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

#include "fe-composing.h"
#include "fe-vcard.h"
#include "fe-version.h"

void
fe_xep_init(void)
{
	fe_composing_init();
	fe_vcard_init();
	fe_version_init();
}

void
fe_xep_deinit(void)
{
	fe_composing_deinit();
	fe_vcard_deinit();
	fe_version_deinit();
}
