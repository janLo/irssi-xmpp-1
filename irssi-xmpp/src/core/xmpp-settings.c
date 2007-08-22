/*
 * $Id: xmpp-settings.c,v 1.1 2007/08/22 19:25:06 cdidier Exp $
 *
 * Copyright (C) 2007 Colin DIDIER
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
#include "settings.h"
#include "signals.h"

#include "xmpp-servers.h"
#include "xmpp-rosters.h"

void
read_settings(void)
{
    g_debug("ok");
}

void
xmpp_settings_init(void)
{
    settings_add_int("xmpp", "xmpp_priority", 0);
    settings_add_bool("xmpp", "xmpp_send_version", TRUE);
    settings_add_str("xmpp", "xmpp_default_away_mode", "away");
    settings_add_bool("xmpp", "xmpp_set_nick_as_username", FALSE);
    settings_add_bool("xmpp", "roster_show_offline", TRUE);
    settings_add_bool("xmpp", "roster_show_offline_unsuscribed", TRUE);
    settings_add_str("xmpp", "roster_default_group", "General");
    settings_add_bool("xmpp", "roster_add_send_subscribe", TRUE);

    signal_add("setup changed", (SIGNAL_FUNC) read_settings);
}

void
xmpp_settings_deinit(void)
{
    signal_remove("setup changed", (SIGNAL_FUNC) read_settings);
}
