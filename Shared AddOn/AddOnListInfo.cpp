/**************************************/
/* Liste d'affichage des informations */
/**************************************/

#include "AddOnListInfo.h"
#include "AddOnConstantes.h"
#include "AddOnListView.h"
#include "AddOnProgressView.h"
#include "AddOnListItem.h"
#include "CSClass.h"
#include "CPreferences.h"

#include <Application.h>
#include <Message.h>
#include <ScrollView.h>
#include <Bitmap.h>
#include <File.h>
#include <Resources.h>

#include <stdio.h>

AddOnListInfo::AddOnListInfo(BRect frame,BView *mother)
: BWindow(frame,"DontWorryWindow",B_BORDERED_WINDOW_LOOK, B_MODAL_SUBSET_WINDOW_FEEL,B_AVOID_FOCUS ),
_motherMessenger(mother)
{
	CPreferences	prefs(PREF_FILE_NAME,PREF_PATH_NAME);
	BRect			listRect = Bounds();
	BScrollView		*scrollinfo;
	BView			*scrollHBar;
	float			scrollHsize;

	_lastClass = NULL;
	_typeRequested = -1;

				
	// initialisees les images
	prefs.Load();
	_classesBitmap = prefs.GetBitmap("classes");
	_metodesBitmap = prefs.GetBitmap("metodes");
	_variablesBitmap = prefs.GetBitmap("variables");
	_privateBitmap = prefs.GetBitmap("private");
	_protectedBitmap = prefs.GetBitmap("protected");
	_virtualBitmap = prefs.GetBitmap("virtual");
	
	listRect.right -= 15;
	listRect.bottom -=15;
	_listOfInfos = new AddOnListView(listRect);
	scrollinfo = new BScrollView("scroll-info",_listOfInfos,B_FOLLOW_ALL_SIDES,0,true,true);
	
	// deplace la scroll bar du bas
	scrollHBar = scrollinfo->FindView("_HSB_");
	scrollHBar->ResizeBy(-70,0);
	scrollHBar->MoveBy(70,0);
	scrollHsize = (scrollHBar->Bounds()).Height()+1;
	
	listRect = scrollinfo->Bounds();
	_progress = new AddOnProgressView(BRect(2,listRect.bottom-scrollHsize,70,listRect.bottom-1));
	scrollinfo->AddChild(_progress);

	AddChild(scrollinfo);
	
	// place le focus sur la liste
	_listOfInfos->MakeFocus(true);
	
	// envoyer le pointer de la fenetre a la vue pour les deplacement
	BMessage	moveMsg(MOVE_WINDOW_PTR);
	
	moveMsg.AddPointer(WINDOW_RESULT_PTR,this);
	_motherMessenger.SendMessage(&moveMsg);
}

AddOnListInfo::~AddOnListInfo()
{
	int32	nbItem;
	
	nbItem = _listOfInfos->CountItems();
	for(int32 index=nbItem;index>0;index--)
		delete _listOfInfos->RemoveItem(index);

	// effacer les images
	delete	_classesBitmap;
	delete	_metodesBitmap;
	delete	_variablesBitmap;	
	delete	_protectedBitmap;
	delete	_privateBitmap;
	delete	_virtualBitmap;
}

/**** interpretation ****/
void AddOnListInfo::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	// barre de progression
	case PROGRESS_START_MSG:
	case PROGRESS_STOP_MSG:
		{
		// recuperer le type de demande :: ou -> ou .
		if(message->FindInt8(CS_ASK_TYPE,&_typeRequested)!=B_OK)
			return;

		BMessenger(_progress).SendMessage(message);		
		
		// on selectionne le premier
		if(_listOfInfos->CountItems()>0)
			_listOfInfos->PlaceOnKeyBuffer();
		}
		break;
	// on recoit des donnees du handler
	case ADD_DATAS_LIST:
		AddDatasList(message);
		break;
	// on recoit des donnees en retour d'une completion
	case ADD_COMPLETION_DATAS:
		AddCompletionDatas(message);
		break;
	// on a choisi un item
	case ADD_DATAS_TEXT:
		AddText(message);
		break;
	// redirige le message pour afficher le buffer
	case DISPLAY_BUFFER_MSG:
		_motherMessenger.SendMessage(message);
		break;
	default:
		BWindow::MessageReceived(message);
	}
}

/**** ajouter une ligne a la fenetre ****/
void AddOnListInfo::AddDatasList(BMessage *message)
{
	int8			type;
	int16			protection;
	int32			nbDatas;
	bool			isVirtual;
	BBitmap			*state = NULL;
	BBitmap			*bitmap = NULL;

	if(message->FindInt32(NB_DATAS_TO_ADD,&nbDatas)!=B_OK)
		return;
	
	// aller de nbdatas-1 a 0 car les indices commence a 0
	for(int32 i=nbDatas-1;i>=0;i--)
	{
		message->FindInt8(CS_TYPE,i,&type);	
		message->FindInt16(CS_PROTECTION,i,&protection);

		// choisir le type de protection
		switch(protection)
		{
			case CSMT_PRIVATE:
				state = _privateBitmap;
				break;
			case CSMT_PROTECTED:
				state = _protectedBitmap;
				break;
			case CSMT_PUBLIC:
			default:
				state = NULL;
		}
	
		// on ajoute l'item
		switch(type)
		{
		case TYPE_CLASS:
			{
				_lastClass = new AddOnListItem(type,_classesBitmap,message,i,state,NULL);
				_lastClass->SetExpanded(false);
				_listOfInfos->AddItem(_lastClass,0);
				if(_listOfInfos->CountItems()==1)
					(_listOfInfos->ItemAt(0))->SetExpanded(true);
			}
			break;
		case TYPE_METHODE:
			{	
				// trouver si c'est virtual
				bitmap = NULL;
				if(message->FindBool(CS_VIRTUAL,i,&isVirtual)!=B_OK)
					isVirtual = false;
				
				if(isVirtual)
					bitmap = _virtualBitmap;
				
				_listOfInfos->AddUnder(new AddOnListItem(type,_metodesBitmap,message,i,state,bitmap),_lastClass);
			}
			break;
		case TYPE_VARIABLE:
				_listOfInfos->AddUnder(new AddOnListItem(type,_variablesBitmap,message,i,state,NULL),_lastClass);
		}

		// incrementer le compteur
		_progress->IncreaseCount();		
	}
}

/**** ajouter des donnees d'une completion ****/
void AddOnListInfo::AddCompletionDatas(BMessage *message)
{
	int8			type;
	int16			protection;
	int32			nbDatas;
	BBitmap			*bitmap = NULL;
	BBitmap			*virtualBitmap = NULL;
	BBitmap			*state = NULL;
	bool			isVirtual = false;

	if(message->FindInt32(NB_DATAS_TO_ADD,&nbDatas)!=B_OK)
		return;
	
	// aller de nbdatas-1 a 0 car les indices commence a 0
	for(int32 i=nbDatas-1;i>=0;i--)
	{

		message->FindInt8(CS_TYPE,i,&type);	
		message->FindInt16(CS_PROTECTION,i,&protection);

		// pour l'instant on ne s'occupe pas de la protection
		// choisir le type de protection
		switch(protection)
		{
			case CSMT_PRIVATE:
				state = _privateBitmap;
				break;
			case CSMT_PROTECTED:
				state = _protectedBitmap;
				break;
			case CSMT_PUBLIC:
			default:
				state = NULL;
		}
		
		// par defaut on a pas de virtuel
		virtualBitmap = NULL;
		isVirtual = false;
		
		// on ajoute l'item
		switch(type)
		{
		case TYPE_CLASS:
			bitmap = _classesBitmap;
			break;
		case TYPE_METHODE:
			{	
				// image fonction
				bitmap = _metodesBitmap;
				
				// trouver si c'est virtual
				if(message->FindBool(CS_VIRTUAL,i,&isVirtual)!=B_OK)
					isVirtual = false;
				
				if(isVirtual)
					virtualBitmap = _virtualBitmap;				
			}
			break;
		case TYPE_VARIABLE:
			bitmap = _variablesBitmap;
			break;
		} 
		_listOfInfos->AddItem(new AddOnListItem(type,bitmap,message,i,state,virtualBitmap));
		
		// incrementer le compteur
		_progress->IncreaseCount();		
	}
}

/**** on quite la fenetre ****/
bool AddOnListInfo::QuitRequested()
{
	// ajouter le texte tapé si on a rien selectionné
	_motherMessenger.SendMessage(ADD_DATAS_TEXT);

	// indiquer que l'on a fermé la fenetre de resultat
	be_app->PostMessage(CLOSE_WINDOW_MSG);

	// debloquer la fenetre parente
	_motherMessenger.SendMessage(UNLOCK_MOTHER_WIN);

	// invalider le pointer pour le deplacement
	_motherMessenger.SendMessage(MOVE_WINDOW_PTR);

	return true;
}

/**** texte a ajouter ****/
void AddOnListInfo::AddText(BMessage *message)
{
	BString		text("");
	int32		modifier = 0;
	bool		forceNoParameters = false;
	
	if(_listOfInfos->CountItems()<=0)
		return;
	
	// recuperer le text a ajouter
	// et recuperer le type
	int32	index = -1;
	if((index = _listOfInfos->CurrentSelection())<0)
		return;

	// recuperer l'etat des touches de controles
	// si on a appuyer sur la touche schift gauche
	message->FindInt32("modifiers",&modifier);
	forceNoParameters = (modifier & B_LEFT_SHIFT_KEY);
	
	// construire la chaine a retourner
	text = ((AddOnListItem *)(_listOfInfos->ItemAt(index)))->Name(_typeRequested,false,forceNoParameters);
	message->AddString(SRING_TO_ADD,text.String());
	
	// reinitialiser le type de demande
	_typeRequested = -1;
	
	// l'envoyer pour l'inserer puis quitter
	_motherMessenger.SendMessage(message);
	PostMessage(B_QUIT_REQUESTED);
}