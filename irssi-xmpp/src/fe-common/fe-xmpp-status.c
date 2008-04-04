/*
 * $Id: fe-xmpp-status.c,v 1.5 2008/04/04 20:27:19 cdidier Exp $
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

#include <string.h>

#include "module.h"
#include "levels.h"
#include "module-formats.h"
#include "printtext.h"
#include "settings.h"
#include "signals.h"
#include "window-items.h"

#include "xmpp-servers.h"
#include "xmpp-rosters.h"
#include "xmpp-rosters-tools.h"

const char *fe_xmpp_presence_show[] = {
	"error",
	"Offline",
	"Not Available",
	"Busy",
	"Away",
	"Available",
	"Free for Chat"
};

static char *
get_window_name(XMPP_SERVER_REC *server)
{
	g_return_val_if_fail(IS_XMPP_SERVER(server), NULL);

	return g_strconcat("(", (server->connrec->chatnet == NULL ||
	    *server->connrec->chatnet == '\0') ? server->jid :
	    server->connrec->chatnet, ")", NULL);
}

char *
fe_xmpp_status_get_window_name(XMPP_SERVER_REC *server)
{
	WINDOW_REC *window;
	char *name;

	g_return_val_if_fail(IS_XMPP_SERVER(server), NULL);

	name = get_window_name(server);
	if (name == NULL)
		return NULL;

	window = window_find_name(name);
	g_free(name);

	return (window != NULL) ? window->name : NULL;
}

WINDOW_REC *
fe_xmpp_status_get_window(XMPP_SERVER_REC *server)
{
	WINDOW_REC *window;
	char *name;

	g_return_val_if_fail(IS_XMPP_SERVER(server), NULL);

	name = get_window_name(server);
	window = window_find_name(name);
	if (window == NULL) {
		window = window_create(NULL, TRUE);
		window_set_name(window, name);
		window_change_server(window, server);
	}

	g_free(name);

	return window;
}

static void
sig_presence_changed(XMPP_SERVER_REC *server, const char *full_jid,
   int show, const char *status)
{
	XMPP_ROSTER_USER_REC *user;
	WINDOW_REC *window;
	const char *msg, *name;
	char *jid = NULL;

	g_return_if_fail(IS_XMPP_SERVER(server));
	g_return_if_fail(full_jid != NULL);
	g_return_if_fail(0 <= show && show < XMPP_PRESENCE_SHOW_LEN);	

	window = fe_xmpp_status_get_window(server);

	msg = fe_xmpp_presence_show[show];

	user = xmpp_rosters_find_user(server->roster, full_jid, NULL);
	if (user != NULL) {
		name = user->name;
		jid = g_strconcat(" (", full_jid, ")", NULL);
	} else
		name = full_jid;

	if (status != NULL)
		printformat_module_window(MODULE_NAME, window, MSGLEVEL_CRAP,
		    XMPPTXT_PRESENCE_CHANGE_REASON, name, jid, msg, status);
	else
		printformat_module_window(MODULE_NAME, window, MSGLEVEL_CRAP,
		    XMPPTXT_PRESENCE_CHANGE, name, jid, msg);
	
	g_free(jid);
}

static void
sig_setup_changed(void)
{
	signal_remove("xmpp presence changed",
	    (SIGNAL_FUNC)sig_presence_changed);

	if (settings_get_bool("xmpp_status_window"))
		signal_add("xmpp presence changed",
		    (SIGNAL_FUNC)sig_presence_changed);
}

static void
sig_server_connecting(XMPP_SERVER_REC *server)
{
	if (!IS_XMPP_SERVER(server))
		return;

	if (settings_get_bool("xmpp_status_window"))
		fe_xmpp_status_get_window(server);
}

void
fe_xmpp_status_init(void)
{
	signal_add("server connecting", (SIGNAL_FUNC)sig_server_connecting);
	signal_add("setup changed", (SIGNAL_FUNC)sig_setup_changed);

	settings_add_bool("xmpp_lookandfeel", "xmpp_status_window", FALSE);

	if (settings_get_bool("xmpp_status_window"))
		signal_add("xmpp presence changed",
		    (SIGNAL_FUNC)sig_presence_changed);
}

void
fe_xmpp_status_deinit(void)
{
	signal_remove("server connecting", (SIGNAL_FUNC)sig_server_connecting);
	signal_remove("setup changed", (SIGNAL_FUNC)sig_setup_changed);
	signal_remove("xmpp presence changed",
	    (SIGNAL_FUNC)sig_presence_changed);
}
