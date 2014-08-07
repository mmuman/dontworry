/*******************************************/
/* Vue pour la progression de la recherche */
/* des methodes dans la fenêtre            */
/*******************************************/

#include "AddOnProgressView.h"
#include "AddOnConstantes.h"
#include <Bitmap.h>
#include <Window.h>
#include <Message.h>

// image de la progress bar 
const uint8 progressBits[] = {
	0x13, 0x27, 0x21, 0x21, 0x1b, 0x1b, 0x15, 0x27, 
	0x27, 0x27, 0x21, 0x3f, 0x1b, 0x1b, 0x27, 0x27, 
	0x27, 0x27, 0x1b, 0x3f, 0x1b, 0x21, 0x27, 0x27, 
	0x27, 0x15, 0x1b, 0x3f, 0x21, 0x21, 0x27, 0x13, 
	0x13, 0x15, 0x1b, 0x21, 0x21, 0x21, 0x15, 0x13, 
	0x13, 0x15, 0x21, 0x21, 0x21, 0x1b, 0x15, 0x13, 
	0x13, 0x27, 0x21, 0x21, 0x1b, 0x1b, 0x15, 0x27, 
	0x27, 0x27, 0x21, 0x3f, 0x1b, 0x1b, 0x27, 0x27, 
	0x27, 0x27, 0x1b, 0x3f, 0x1b, 0x21, 0x27, 0x27, 
	0x27, 0x15, 0x1b, 0x3f, 0x21, 0x21, 0x27, 0x13, 
	0x13, 0x15, 0x1b, 0x21, 0x21, 0x21, 0x15, 0x13, 
	0x13, 0x15, 0x21, 0x21, 0x21, 0x1b, 0x15, 0x13, 
	0x13, 0x27, 0x21, 0x21, 0x1b, 0x1b, 0x15, 0x27, 
	0x27, 0x27, 0x21, 0x3f, 0x1b, 0x1b, 0x27, 0x27, 
	0x27, 0x27, 0x1b, 0x3f, 0x1b, 0x21, 0x27, 0x27, 
	0x27, 0x15, 0x1b, 0x3f, 0x21, 0x21, 0x27, 0x13, 
	0x13, 0x15, 0x1b, 0x21, 0x21, 0x21, 0x15, 0x13, 
	0x13, 0x15, 0x21, 0x21, 0x21, 0x1b, 0x15, 0x13
};


AddOnProgressView::AddOnProgressView(BRect frame)
: BView(frame,"Progress-view",B_FOLLOW_NONE,B_WILL_DRAW | B_PULSE_NEEDED)
{
	_step = 0;
	_progressBitmap = new BBitmap(BRect(0,0,7,11),B_CMAP8);
	_progressBitmap->SetBits(progressBits,96,0,B_CMAP8);
	
	// par defaut on ne l'active pas
	_progressState = false;
	// est on a pas d'item
	_nbItem = 0;
}

AddOnProgressView::~AddOnProgressView()
{
	delete _progressBitmap;
}

void AddOnProgressView::AttachedToWindow()
{

	// appel de la methode par defaut
	BView::AttachedToWindow();

	// on fixe la vitesse du pulse pour la progress bar
	Window()->SetPulseRate(0);
	
	//taille des fontes pour afficher le nombre d'item
	SetFontSize(9);
}

void AddOnProgressView::Draw(BRect frame)
{
	BRect		bounds = Bounds();

	// appel du draw normal
	BView::Draw(frame);

	SetHighColor(156,154,156);
	StrokeLine(BPoint(bounds.left,bounds.top),BPoint(bounds.right,bounds.top));	
	
	bounds.top++;
	SetHighColor(181,178,181);
	StrokeLine(BPoint(bounds.left,bounds.bottom),BPoint(bounds.right,bounds.bottom));	
	StrokeLine(BPoint(bounds.right,bounds.top),BPoint(bounds.right,bounds.bottom));	

	bounds.InsetBy(1,1);
	SetHighColor(222,219,222);
	FillRect(bounds);
	
	SetLowColor(222,219,222);
	SetHighColor(0,0,0);
	_nbItemStr.SetTo("");
	_nbItemStr << _nbItem << " items";
	DrawString(_nbItemStr.String(),BPoint(11,11));
	
	if(_progressState)
		DrawBitmap(_progressBitmap,BPoint(3,2));
}

void AddOnProgressView::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
	case PROGRESS_START_MSG:
		_progressState = true;
		Window()->SetPulseRate(100000);	
		break;
	case PROGRESS_STOP_MSG:
		_progressState = false;
		Window()->SetPulseRate(0);
		// rafraichir pour eviter de laisser un dessin de progression
		Invalidate();
		break;
	default:
		BView::MessageReceived(message);
	}
}

/**** incrementer le compteur ****/
void AddOnProgressView::IncreaseCount()
{
	_nbItem++;
	Invalidate();
}

void AddOnProgressView::Pulse()
{
	_step += 8;
	if(_step==48)
		_step = 0;
		
	_progressBitmap->SetBits(progressBits+_step,96,0,B_CMAP8);
	Invalidate(BRect(3,2,65,13));
}
