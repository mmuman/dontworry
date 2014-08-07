/*******************************************/
/* Vue pour la progression de la recherche */
/* des methodes dans la fenêtre            */
/*******************************************/

#ifndef ADDONPROGRESSVIEW_H
#define ADDONPROGRESSVIEW_H

#include <View.h>
#include <String.h>

class BBitmap;

class AddOnProgressView : public BView
{
public:
	AddOnProgressView(BRect frame);
	~AddOnProgressView();
	
			void	IncreaseCount();
	
	virtual void	AttachedToWindow();
	virtual	void	Draw(BRect frame);
	virtual void	MessageReceived(BMessage* message);
	virtual void	Pulse();

protected:
	BBitmap		*_progressBitmap;
	bool		_progressState;
	int32		_step;
	int32		_nbItem;
	BString		_nbItemStr;
};

#endif ADDONPROGRESSVIEW_H