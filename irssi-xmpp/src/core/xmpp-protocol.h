/* $Id: xmpp-protocol.h,v 1.14 2007/11/19 13:14:26 cdidier Exp $ */

#ifndef __XMPP_PROTOCOL_H
#define __XMPP_PROTOCOL_H

#define XMPP_PROTOCOL_LEVEL 1

#define XMLNS "xmlns"
#define XMLNS_ROSTER "jabber:iq:roster"
#define XMLNS_VERSION "jabber:iq:version"
#define XMLNS_VCARD "vcard-temp"
#define XMLNS_EVENT "jabber:x:event"
#define XMLNS_PING "urn:xmpp:ping"
#define XMLNS_DISCO_INFO "http://jabber.org/protocol/disco#info"
#define XMLNS_MUC "http://jabber.org/protocol/muc"
#define XMLNS_MUC_USER "http://jabber.org/protocol/muc#user"

__BEGIN_DECLS
void	xmpp_send_message(XMPP_SERVER_REC *, const char *, const char *);

void	xmpp_protocol_init(void);
void	xmpp_protocol_deinit(void);
__END_DECLS

#endif
