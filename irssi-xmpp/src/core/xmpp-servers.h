/* $Id: xmpp-servers.h,v 1.5 2007/08/29 08:59:40 cdidier Exp $ */

#ifndef __XMPP_SERVERS_H
#define __XMPP_SERVERS_H

#include "chat-protocols.h"
#include "servers.h"

#include "loudmouth/loudmouth.h"

/* returns XMPP_SERVER_REC if it's XMPP server, NULL if it isn't */
#define XMPP_SERVER(server)						\
	PROTO_CHECK_CAST(SERVER(server), XMPP_SERVER_REC, chat_type, "XMPP")

#define XMPP_SERVER_CONNECT(conn)					\
	PROTO_CHECK_CAST(SERVER_CONNECT(conn), XMPP_SERVER_CONNECT_REC,	\
	    chat_type, "XMPP")

#define IS_XMPP_SERVER(server)						\
	(XMPP_SERVER(server) ? TRUE : FALSE)

#define IS_XMPP_SERVER_CONNECT(conn)					\
	(XMPP_SERVER_CONNECT(conn) ? TRUE : FALSE) 

struct _XMPP_SERVER_CONNECT_REC {
	#include "server-connect-rec.h"
};
#define STRUCT_SERVER_CONNECT_REC XMPP_SERVER_CONNECT_REC

struct _XMPP_SERVER_REC {
	#include "server-rec.h"

	LmConnection	*lmconn;
	char		*ressource;
	int		 priority;
	int		 show;

	GSList		*roster;
};

__BEGIN_DECLS
gboolean	 xmpp_server_is_alive(XMPP_SERVER_REC *);
SERVER_REC	*xmpp_server_init_connect(SERVER_CONNECT_REC *);
void		 xmpp_server_connect(SERVER_REC *);

void        	xmpp_servers_init(void);
void		xmpp_servers_deinit(void);
__END_DECLS

#endif
