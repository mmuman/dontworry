/*******************************/
/* listedes methodes et objets */
/*******************************/

#ifndef ADDONLISTVIEW_H
#define ADDONLISTVIEW_H

#include <OutlineListView.h>
#include <String.h>

class AddOnListView : public BOutlineListView
{
public:
	AddOnListView(BRect frame);
	~AddOnListView();
	
	virtual void	MessageReceived(BMessage* message);
	virtual void	KeyDown(const char *bytes, int32 numBytes);
	virtual	void	MouseDown(BPoint where);
			void	PlaceOnKeyBuffer(bool startOnNext = false);
	
private:
	BString			_buffer;
	int32			_keySensitive;
	int32			_downSearch;
	
	void		AddCharToBuffer(char);
	void		DeleteCharOfBuffer();
	status_t	ExpandClass(uint8);

	virtual	void	DrawLatch(BRect itemRect, int32 level, bool collapsed,bool highlighted, bool misTracked);
	virtual void	FrameResized(float newW,float newH);
			void 	DisplayInBeHappy();
			void	SendAddText();
};	

#endif ADDONLISTVIEW_H