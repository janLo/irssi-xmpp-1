/*
 * $Id: xmpp-settings.c,v 1.8 2007/09/30 12:12:36 cdidier Exp $
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

#include <string.h>

#include "module.h"
#include "settings.h"
#include "signals.h"

#include "xmpp-servers.h"
#include "xmpp-rosters.h"
#include "xmpp-protocol.h"

void
read_settings(void)
{
	GSList *tmp;
	XMPP_SERVER_REC *server;
	
	for (tmp = servers; tmp != NULL; tmp = tmp->next) {

		server = XMPP_SERVER(tmp->data);
		if (server == NULL)
			continue;

		/* update priority */
		if (server->priority != settings_get_int("xmpp_priority")) {
			signal_emit("xmpp own_presence", 4, server,
			    server->show, server->away_reason,
			    settings_get_int("xmpp_priority"));
		}

		/* update nick */
		if (settings_get_bool("xmpp_set_nick_as_username")) {
			if (strcmp(server->connrec->nick,
			    server->connrec->username) != 0) {
				g_free(server->connrec->nick);
				g_free(server->nick);
				server->connrec->nick =
				    g_strdup(server->connrec->username);
				server->nick = g_strdup(server->connrec->nick);
			}
		} else {
			if (strcmp(server->connrec->nick,
			    server->connrec->realname) != 0) {
				g_free(server->connrec->nick);
				g_free(server->nick);
				server->connrec->nick =
				    g_strdup(server->connrec->realname);
				server->nick = g_strdup(server->connrec->nick);
			}
		}

	}
}

void
xmpp_settings_init(void)
{
	signal_add("setup changed", (SIGNAL_FUNC)read_settings);
}

void
xmpp_settings_deinit(void)
{
	signal_remove("setup changed", (SIGNAL_FUNC)read_settings);
}
