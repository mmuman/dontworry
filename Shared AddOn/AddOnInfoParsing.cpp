/*************************************/
/* fenetre de progression du parsing */
/*************************************/

#include "AddOnInfoParsing.h"
#include "AddOnConstantes.h"
#include "CPreferences.h"

#include <View.h>
#include <Screen.h>
#include <Message.h>
#include <ScrollView.h>
#include <Bitmap.h>
#include <Button.h>


//=====================================
// la vue
//=====================================
DisplayView::DisplayView(BRect frame,bool splashScreen,bool aboutBox)
: BView(frame,"support-view",B_FOLLOW_NONE,B_WILL_DRAW)
{
	// initialiser les variables
	_splashScreen = splashScreen;
	_aboutBox = aboutBox;
	if(_aboutBox)
		_splashScreen = true;
	
	_nbFiles = 0;
	_counter = 0;
	_counterStep = 0;
	_parsingRect.Set(10,15,140,25);
	
	// fixer les couleurs
	SetLowColor(115,109,181);
	SetViewColor(115,109,181);
	
	// taille des fontes
	SetFontSize(10);
	
	if(_splashScreen)
	{
		CPreferences	prefs(PREF_FILE_NAME,PREF_PATH_NAME);
				
		// charger les prefs
		// charger l'images du splash-screen
		prefs.Load();
		if(prefs.PreferencesLoaded()==B_OK)
			_screenBitmap = prefs.GetBitmap("SplashScreen");
		
		_parsingRect.Set(220,240,350,250);
	}
	
	if(_aboutBox)
		AddChild(new BButton(BRect(frame.right-65,frame.bottom-30,frame.right-5,frame.bottom-15),"ok-about","Ok...",new BMessage(B_QUIT_REQUESTED)));
}

DisplayView::~DisplayView()	
{
	if(_splashScreen)
		delete _screenBitmap;
}

void DisplayView::Draw(BRect updateRect)
{
	BRect	statusRect = _parsingRect;

	BView::Draw(updateRect);
	

	if(_splashScreen)
	{
		// afficher l'image
		if(_screenBitmap!=NULL)
			DrawBitmap(_screenBitmap,Bounds());

		// titre
		SetFontSize(25);
		SetHighColor(255,255,255);
		DrawString("DontWorry Beta 3.1",BPoint(150,35));

		// a propos
		SetFontSize(10);
		DrawString("Created by :",BPoint(10,265));
		DrawString("Sylvain Tertois",BPoint(75,265));
		DrawString("Vincent Cedric",BPoint(75,276));
		DrawString("Name by : Feldis Sebastien",BPoint(160,265));
		DrawString("Splash-Screen by : joseph",BPoint(160,276));
		DrawString("ppc version by : Drewke Andreas",BPoint(160,287));
	}
	else
	{
		SetHighColor(0,0,0);
		StrokeRect(Bounds());
	}

	if(!_aboutBox)
	{
		// info
		SetHighColor(255,255,255);
		DrawString("Parsing Header files...",BPoint(statusRect.left,statusRect.top-5));
	
		// dessiner l'evolution
		SetHighColor(25,25,25);
		StrokeRect(statusRect);
		statusRect.InsetBy(1,1);
		statusRect.right = statusRect.left + _counter;
		SetHighColor(105,109,181);
		StrokeLine(BPoint(statusRect.right,statusRect.top),BPoint(statusRect.right,statusRect.bottom));
		StrokeLine(BPoint(statusRect.left,statusRect.bottom),BPoint(statusRect.right,statusRect.bottom));
		SetHighColor(185,189,255);
		StrokeLine(BPoint(statusRect.left,statusRect.top),BPoint(statusRect.left,statusRect.bottom));
		StrokeLine(BPoint(statusRect.left,statusRect.top),BPoint(statusRect.right,statusRect.top));
		SetHighColor(145,149,221);
		statusRect.InsetBy(1,1);
		FillRect(statusRect);
	}
}

/**** on defini le nombre de fichiser ****/
void DisplayView::SetNbFiles(float nbFiles)
{
	_nbFiles = nbFiles;
	_counterStep = 128 / nbFiles;
}

//=====================================
// la fenetre
//=====================================
AddOnInfoParsing::AddOnInfoParsing(bool splashScreen, bool aboutBox)
: BWindow(GetFrame(splashScreen,aboutBox),"InfoParsing",B_NO_BORDER_WINDOW_LOOK,B_MODAL_APP_WINDOW_FEEL,B_AVOID_FOCUS)
{
	// creer la vue support
	_view = new DisplayView(Bounds(),splashScreen, aboutBox);

	AddChild(_view);
	_view->Invalidate();
}

AddOnInfoParsing::~AddOnInfoParsing()
{
}

void AddOnInfoParsing::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case PROGRESS_START_MSG:
		{
			int32	nbFiles;
			
			// recuperer le nombre de fichier
			if(message->FindInt32(PROJECT_FILE_TOTAL,&nbFiles)==B_OK)
				_view->SetNbFiles(nbFiles);
		}
		break;
	case PROGRESS_INCREASE_MSG:
		_view->IncreaseCounter();
		_view->InvalidateStatus();
		break;
	default:
		BWindow::MessageReceived(message);
	}
}

BRect AddOnInfoParsing::GetFrame(bool splashScreen,bool aboutBox)
{
	BRect		newFrame;
	BScreen		screen;
	
	if(!screen.IsValid())
		return BRect(200,150,600,450);
	
	newFrame = screen.Frame();

	// dans le cas du splash-screen ca fait 400*300 pixels
	if(splashScreen)
	{
		// regler la largeur
		newFrame.left = (newFrame.right / 2) -200;
		newFrame.right = newFrame.left + 399;
		// regler la hauteur
		newFrame.top = (newFrame.bottom / 2) -150;
		newFrame.bottom = newFrame.top + 299;
	}
	else
	{
		// regler la largeur
		newFrame.left = (newFrame.right / 2) -75;
		newFrame.right = newFrame.left + 149;
		// regler la hauteur
		newFrame.top = (newFrame.bottom / 2) -15;
		newFrame.bottom = newFrame.top + 30;
	}
	
	return newFrame;
}
