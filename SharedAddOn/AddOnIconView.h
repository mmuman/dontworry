/*******************************************/
/* Vue pour afficher un icone dans la      */
/* fenêtre projet						   */
/*******************************************/

#ifndef ADDONICONVIEW_H
#define ADDONICONVIEW_H

#include <View.h>

class BBitmap;
class BPopUpMenu;

#define		DW_ABOUT_MSG	'Dabm'

class AddOnIconView : public BView
{
public:
	AddOnIconView(BRect frame);
	~AddOnIconView();
	
	// pour l'instant on se contantes de
	// redefinir le draw !
	virtual	void	Draw(BRect frame);
	virtual void	MouseDown(BPoint where);
	virtual void	MessageReceived(BMessage* message);

protected:
	BBitmap			*_drawingBitmap;
	BBitmap			*_iconBitmap;
	BBitmap			*_blinkBitmap;
	BPopUpMenu		*_popup;
	bool			_blinkActivated;

		void	BuildMenuPoPup();
};

#endif ADDONICONVIEW_H