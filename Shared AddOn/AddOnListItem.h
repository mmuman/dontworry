#ifndef ADDON_LIST_ITEM_H
#define ADDON_LIST_ITEM_H

#include <ListItem.h>
#include <String.h>

class BBitmap;

class AddOnListItem : public BListItem
{
public:
	AddOnListItem(int8 type,BBitmap *image,BMessage *message,int32 indexMessage,BBitmap *state=NULL,BBitmap *iSvirtual=NULL);
	virtual ~AddOnListItem();
	virtual void	DrawItem(BView *owner,BRect frame,bool complete = false);	
	virtual void	Update(BView *owner, const BFont *font);
	
	BString		Name(int8 askType,bool forBeHappy = false,bool forceNoParameters = false);
	
	inline	int8		Type()		{ return _type; }

protected:
	BString		_returnType;
	BString		_name;
	BString		_parameter;
	BBitmap		*_image;
	BBitmap		*_state;
	BBitmap		*_isVirtual;
	int8		_type;
};

# endif