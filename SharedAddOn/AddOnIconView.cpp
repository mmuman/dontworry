/*******************************************/
/* Vue pour afficher un icone dans la      */
/* fenêtre projet						   */
/*******************************************/

#include "AddOnIconView.h"
#include "AddOnInfoParsing.h"
#include "AddOnConstantes.h"
#include "CPreferences.h"

#include <Bitmap.h>
#include <File.h>
#include <AppFileInfo.h>
#include <InterfaceDefs.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <time.h>

/**** constructeur ****/
AddOnIconView::AddOnIconView(BRect frame)
: BView(frame,"Icon-view",B_FOLLOW_RIGHT,B_WILL_DRAW)
{
	CPreferences	prefs(PREF_FILE_NAME,PREF_PATH_NAME);

	// recuperer la couleur de l'interface
	rgb_color	menuColor = ui_color(B_MENU_BACKGROUND_COLOR);
	
	// fixer la couleur de la vue
	SetViewColor(menuColor);
	
	// charger l'image
	_iconBitmap = prefs.GetBitmap("Activate");
	_blinkBitmap = prefs.GetBitmap("Blink");
	
	// creer le menu popup
	BuildMenuPoPup();
	
	// le clignotement n'est pas activé
	_drawingBitmap = _iconBitmap;
	_blinkActivated = false;
}

/**** destructeur ****/
AddOnIconView::~AddOnIconView()
{
	// liberer la memoire prise par l'image
	// et le popup
	delete _iconBitmap;
	delete _blinkBitmap;
	delete _popup;
}
	
/**** dessin ****/
void AddOnIconView::Draw(BRect frame)
{
	BRect		bounds = Bounds();

	// appel du draw normal
	BView::Draw(frame);

	// dessiner le cadre
	SetHighColor(tint_color(ui_color(B_MENU_BACKGROUND_COLOR),B_LIGHTEN_2_TINT));
	StrokeLine(BPoint(bounds.left,bounds.top),BPoint(bounds.right,bounds.top));	
	StrokeLine(BPoint(bounds.left,bounds.top),BPoint(bounds.left,bounds.bottom));	

	SetHighColor(tint_color(ui_color(B_MENU_BACKGROUND_COLOR),B_DARKEN_2_TINT));
	StrokeLine(BPoint(bounds.left,bounds.bottom),BPoint(bounds.right,bounds.bottom));	
	StrokeLine(BPoint(bounds.right,bounds.top+1),BPoint(bounds.right,bounds.bottom));	
	
	SetHighColor(tint_color(ui_color(B_MENU_BACKGROUND_COLOR),B_DARKEN_1_TINT));
	StrokeLine(BPoint(bounds.left+1,bounds.bottom-1),BPoint(bounds.right-1,bounds.bottom-1));	
	
	// dessiner l'icone en transparence
	if(_iconBitmap!=NULL)
	{
		SetDrawingMode(B_OP_ALPHA);
		DrawBitmap(_drawingBitmap,BPoint(4,1));
		SetDrawingMode(B_OP_COPY);
	}
}

/**** surcharge du mouseDown ****/
void AddOnIconView::MouseDown(BPoint where)
{
	BPoint	point;
	uint32	button;

	GetMouse(&point,&button);

	// affiche le menu popup		
	if(button & B_SECONDARY_MOUSE_BUTTON)
	{
		BMenuItem *selected = _popup->Go(ConvertToScreen(where));
		if (selected)
			Window()->PostMessage(selected->Message(),this);
	}
}

/**** reception des message ****/
void AddOnIconView::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
	case DW_ABOUT_MSG:
		{
			AddOnInfoParsing	*info = NULL;
			info = new AddOnInfoParsing(true,true);
			info->Show();
		}
		break;
	case BLINK_ICONVIEW_MSG:
		{
			_blinkActivated = !_blinkActivated;
			
			// ok le clignotement est activé
			if(_blinkActivated)
				_drawingBitmap = _blinkBitmap;
			else
				_drawingBitmap = _iconBitmap;
				
			// rafraichir
			Invalidate();
		}
		break;
	default:
		BView::MessageReceived(message);
	}
}

/**** creer le popup en fonction de l'objet ****/
void AddOnIconView::BuildMenuPoPup()
{
	// le menu existe t-il deja
	_popup = new BPopUpMenu("about");
	_popup->AddItem(new BMenuItem("About...", new BMessage(DW_ABOUT_MSG)));
}