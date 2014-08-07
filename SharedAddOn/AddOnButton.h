/********************/
/* Button de la vue */
/********************/

#ifndef _ADDONBUTTON_H
#define _ADDONBUTTON_H

// definition des constantes
#define	ADDON_BUTTON_SELECT_MSG		'ABse'
#define	ADDON_BUTTON_UNSELECT_MSG	'ABus'

#include <Control.h>

class AddOnButton : public BControl
{
public:
	AddOnButton(BPoint point,const char *name);
	~AddOnButton();
	
	virtual	void	Draw(BRect frame);
	virtual void	MouseDown(BPoint point);
	virtual void	MessageReceived(BMessage *message);

protected:
	bool	_selected;
};

#endif _ADDONBUTTON_H