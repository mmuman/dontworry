//==================================================================
//	MTextAddOn.h
//	Copyright 1996  Metrowerks Corporation, All Rights Reserved.
//==================================================================
//	This is a proxy class used by Editor add_ons.  It does not inherit from BView
//	but provides an abstract interface to a text engine.
 
#ifndef _MTEXTADDON_H
#define _MTEXTADDON_H

#include <SupportKit.h>

class MIDETextView;
class BWindow;
struct entry_ref;


class MTextAddOn
{
public:
								MTextAddOn(
									MIDETextView&	inTextView);
	virtual						~MTextAddOn();
	virtual	const char*			Text();
	virtual	int32				TextLength() const;
	virtual	void				GetSelection(
									int32 *start, 
									int32 *end) const;
	virtual	void				Select(
									int32 newStart, 
									int32 newEnd);
	virtual void				Delete();
	virtual void				Insert(
									const char* inText);
	virtual void				Insert(
									const char* text, 
									int32 length);

	virtual	BWindow*			Window();
	virtual status_t			GetRef(
									entry_ref&	outRef);
	virtual bool				IsEditable();
	
private:

	MIDETextView&				fText;
};

#endif
