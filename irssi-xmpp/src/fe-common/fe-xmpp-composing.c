/*
 * $Id: fe-xmpp-composing.c,v 1.4 2007/08/26 17:03:52 cdidier Exp $
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
#include "module-formats.h"
#include "signals.h"
#include "window-items.h"
#include "levels.h"
#include "printtext.h"
#include "themes.h"
#include "settings.h"
#include "special-vars.h"

#include "fe-xmpp-composing.h"

#include "xmpp-servers.h"
#include "xmpp-queries.h"
#include "xmpp-rosters.h"
#include "xmpp-protocol.h"

#define KEY_TAB 9
#define KEY_RETURN 10
#define KEY_ESCAPE 27
#define KEYS_PAGE 91
#define KEYS_OTHER 126
#define KEY_BACKSPACE 127

struct composing_query {
	XMPP_SERVER_REC *server;
	gchar *full_jid;
};

gboolean	 keylog_active;
int 		 last_key;
time_t		 composing_time;
char		*composing_name;
XMPP_SERVER_REC *composing_server;

static gboolean
stop_composing(gpointer *user_data)
{
	if (composing_time == 0)
		return FALSE;

	/* still composing */
	if (((time(NULL) - composing_time) < XMPP_COMPOSING_TIMEOUT)
	    && user_data == NULL)
		return TRUE;

	/* server still connected */
	if (g_slist_find(servers, composing_server) == NULL)
		return FALSE;

	signal_emit("xmpp composing stop", 2, composing_server,
	    composing_name);
	composing_time = 0;

	return FALSE;
}

static void
sig_gui_key_pressed(int key)
{
	char *str = NULL;

	if (!settings_get_bool("xmpp_send_composing"))
		goto out;

	/* ignore command or empty line */
	str = parse_special_string("$L", active_win->active_server,
	    active_win->active, "", NULL, 0);
	if (str != NULL &&
	    (str[0] == *settings_get_str("cmdchars") || str[0] == '\0'))
		goto out;

	if (key != KEY_TAB && key != KEY_RETURN && last_key != KEY_ESCAPE
	    && key != KEY_ESCAPE && last_key != KEYS_PAGE && key != KEYS_PAGE
	    && key != KEYS_OTHER && key != KEY_BACKSPACE) {

		if (!settings_get_bool("xmpp_send_composing"))
			goto out;

		/* starting composing */
		if (composing_time == 0) {
			composing_time = time(NULL);
			g_timeout_add(XMPP_COMPOSING_TIMEOUT * 1000,
			    (GSourceFunc)stop_composing, NULL);

			signal_emit("xmpp composing start", 2,
			    composing_server, composing_name);
		}

		/* still composing */
		else if ((time(NULL) - composing_time)
		    > (XMPP_COMPOSING_TIMEOUT - 1))
			 composing_time = time(NULL);

	} else if (key == KEY_RETURN) {
		composing_time = 0;
		stop_composing(NULL);
	}

out:
	last_key = key;
	g_free(str);
}

static void
sig_window_changed(WINDOW_REC *new_window, WINDOW_REC *old_window)
{
	XMPP_SERVER_REC *server;
	QUERY_REC *query;

	if (!settings_get_bool("xmpp_send_composing"))
		goto stop;

	server = XMPP_SERVER(active_win->active_server);
	if (server == NULL)
		goto stop;

	query = XMPP_QUERY(active_win->active);
	if (query == NULL || !xmpp_jid_have_ressource(query->name))
		goto stop;

	/* composing in another window */
	if (composing_name != query->name
	    || composing_server != server) {
		stop_composing(NULL);
		composing_name = query->name;
		composing_server = server;
	}

	if (!keylog_active) {
		signal_add_last("gui key pressed",
		    (SIGNAL_FUNC)sig_gui_key_pressed);
		keylog_active = TRUE;
	}

	return;

stop:
	if (keylog_active) {
		signal_remove("gui key pressed",
		    (SIGNAL_FUNC)sig_gui_key_pressed);
		keylog_active = FALSE;

		stop_composing((void *)TRUE);
		composing_name = NULL;
		composing_server = NULL;
	}
}

void
fe_xmpp_composing_init(void) {
	signal_add_last("window changed", (SIGNAL_FUNC)sig_window_changed);

	keylog_active = FALSE;
	last_key = 0;
	composing_time = 0;
	composing_name = NULL;
	composing_server = NULL;
}

void
fe_xmpp_composing_deinit(void) {
	signal_remove("window changed", (SIGNAL_FUNC)sig_window_changed);

}