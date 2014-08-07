/**********************/
/* AddOnMessageFilter */
/**********************/

#ifndef _ADDONMESSAGEFILTER_H
#define _ADDONMESSAGEFILTER_H

#include <MessageFilter.h>
#include <Message.h>
#include <Messenger.h>

class CPreferences;

class AddOnMessageFilter : public BMessageFilter
{
public:
	AddOnMessageFilter(BMessage *,BHandler *);
	virtual ~AddOnMessageFilter();
	
	virtual filter_result	Filter(BMessage* message, BHandler **target);
	
private:
	BMessage		_messageToSend;
	BMessenger		_textView;
	CPreferences	*_prefs;
};

#endif
