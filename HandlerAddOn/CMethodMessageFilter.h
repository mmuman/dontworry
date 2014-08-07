/**********************/
/* AddOnMessageFilter */
/**********************/

#ifndef _ADDONMESSAGEFILTER_H
#define _ADDONMESSAGEFILTER_H

#include <MessageFilter.h>
#include <Messenger.h>
#include <String.h>

class BHandler;

class CMethodMessageFilter : public BMessageFilter
{
public:
	CMethodMessageFilter(BHandler *redirect);
	virtual ~CMethodMessageFilter();
	
	virtual filter_result Filter(BMessage* message, BHandler **target);

protected:
		BMessenger	_redirect;
};

#endif