/* $Id: xmpp.h,v 1.6 2007/12/08 16:51:34 cdidier Exp $ */

#ifndef __XMPP_H
#define __XMPP_H

typedef struct _XMPP_SERVER_CONNECT_REC XMPP_SERVER_CONNECT_REC;
typedef struct _XMPP_SERVER_REC XMPP_SERVER_REC;
typedef struct _XMPP_QUERY_REC XMPP_QUERY_REC;
typedef struct _XMPP_CHANNEL_REC XMPP_CHANNEL_REC;
typedef struct _XMPP_NICK_REC XMPP_NICK_REC;

#define IS_XMPP_ITEM(rec) (IS_XMPP_CHANNEL(rec) || IS_XMPP_QUERY(rec))
#define XMPP_PROTOCOL (chat_protocol_lookup("XMPP"))

#define IRSSI_XMPP_PACKAGE "irssi-xmpp"
#define IRSSI_XMPP_VERSION "1.0-dev"

#endif
