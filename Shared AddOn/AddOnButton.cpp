/********************/
/* Button de la vue */
/********************/

#include "AddOnButton.h"
#include "AddOnTextView.h"
#include <Application.h>
#include <Window.h>
#include <View.h>

AddOnButton::AddOnButton(BPoint point,const char *name)
: BControl(BRect(point.x,point.y,point.x+12,point.y+12),name,"",NULL,B_FOLLOW_NONE,B_WILL_DRAW)
{
	_selected = false;
}

AddOnButton::~AddOnButton()
{}
	
void AddOnButton::Draw(BRect frame)
{
	BRect	bounds = Bounds();
	
	SetHighColor(148,148,148);
	if(_selected)
	{
		StrokeLine(BPoint(bounds.left+1,bounds.top+1),BPoint(bounds.right,bounds.top+1));
		StrokeLine(BPoint(bounds.left+1,bounds.top+1),BPoint(bounds.left+1,bounds.bottom));
	}
	else
	{
		StrokeLine(BPoint(bounds.left+1,bounds.bottom),BPoint(bounds.right,bounds.bottom));
		StrokeLine(BPoint(bounds.right,bounds.top+1),BPoint(bounds.right,bounds.bottom));
	}
		
	SetHighColor(0,0,0);
	bounds.InsetBy(1,1);
	if(_selected)
		bounds.OffsetBy(1,1);
	StrokeRect(bounds);

	SetHighColor(255,255,255);
	bounds.InsetBy(1,1);
	FillRect(bounds);
	
	SetHighColor(0,0,0);
	bounds.InsetBy(1,1);
	bounds.top += bounds.bottom/2 - 2;
	FillTriangle(BPoint(bounds.left,bounds.top),BPoint(bounds.right,bounds.top),BPoint(bounds.left+3,bounds.top+3));	
}

/**** si on clique on affiche la fenetre et on dessine le boutton ****/
void AddOnButton::MouseDown(BPoint point)
{
	BView		*parent = Parent();
	
	if(dynamic_cast<AddOnTextView *>(parent) == NULL)
		return;
	
	BMessage	addOnMessage = *(((AddOnTextView *)parent)->AddOnMessage());	
	Window()->PostMessage(&addOnMessage,parent);
}

/**** on redessine le bouton ****/
void AddOnButton::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case ADDON_BUTTON_SELECT_MSG:
		_selected = true;
		Invalidate();
		break;
	case ADDON_BUTTON_UNSELECT_MSG:
		_selected = false;
		Invalidate();
		break;
	default:
		BControl::MessageReceived(message);
	}
}