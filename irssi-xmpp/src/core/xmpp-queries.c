/*
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

#include "xmpp-queries.h"
#include "xmpp-servers.h"
#include "xmpp-protocol.h"
#include "xmpp-rosters.h"

QUERY_REC *
xmpp_query_create(const char *server_tag, const char *nick, int automatic)
{
    QUERY_REC *rec;
    XMPP_SERVER_REC *server;
    XmppRosterUser *user;
    XmppRosterRessource *ressource;

    g_return_val_if_fail(nick != NULL, NULL);

    rec = g_new0(QUERY_REC, 1);
    rec->chat_type = XMPP_PROTOCOL;
    rec->name = NULL;

    /* if unspecified, open queries with the highest ressource */
    if (!xmpp_jid_have_ressource(nick)) {
        server = XMPP_SERVER(server_find_tag(server_tag));
        if (!server)
            goto query_pass_ressource;

        user = xmpp_find_user_from_groups(server->roster, nick, NULL);
        if (!user || !user->ressources)
            goto query_pass_ressource;

        ressource = (XmppRosterRessource *) user->ressources->data;
        if (ressource->name)
           rec->name = g_strdup_printf("%s/%s", nick, ressource->name);
    }

query_pass_ressource:
    if (!rec->name)
        rec->name = g_strdup(nick);

    rec->server_tag = g_strdup(server_tag);
    query_init(rec, automatic);

    return rec;
}
