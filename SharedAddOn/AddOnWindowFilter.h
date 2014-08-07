/**********************/
/* AddOnMessageFilter */
/**********************/

#ifndef _ADDONWINDOWFILTER_H
#define _ADDONWINDOWFILTER_H

#include <MessageFilter.h>

class AddOnTextView;
class BWindow;

class AddOnWindowFilter : public BMessageFilter
{
public:
	AddOnWindowFilter(float height,AddOnTextView *);
	virtual ~AddOnWindowFilter();
	
	virtual filter_result	Filter(BMessage* message, BHandler **target);
	
private:
	AddOnTextView	*_textView;
	BWindow			*_winParsing;
	float			_height;
};

#endif
