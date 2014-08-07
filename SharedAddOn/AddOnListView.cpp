/*******************************/
/* listedes methodes et objets */
/*******************************/

#include "AddOnListView.h"
#include "AddOnConstantes.h"
#include "AddOnListItem.h"
#include "AddOnProgressView.h"
#include "CPreferences.h"

#include <Window.h>
#include <Application.h>
#include <Control.h>

// pour debbuger
//#include <stdio.h>

/**** Constructeur ****/
AddOnListView::AddOnListView(BRect frame)
: BOutlineListView(frame,"list-info",B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL_SIDES,B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS)
{
	CPreferences	prefs(PREF_FILE_NAME,PREF_PATH_NAME);

	// valeur par defaut
	_buffer.SetTo("");
	_keySensitive = B_CONTROL_OFF;
	_downSearch = B_CONTROL_ON;
			
	// charger les valeurs du fichier si c'est possible
	prefs.Load();
	if(prefs.PreferencesLoaded()==B_OK)
	{
		_keySensitive = prefs._keySensitive;
		_downSearch = prefs._downSearch;
	}
}

/**** Destructeur ****/
AddOnListView::~AddOnListView()
{
}


/**** Traitement des messages ****/
void AddOnListView::MessageReceived(BMessage* message)
{
	BListView::MessageReceived(message);
}

//==============================================================
//= Traitements Graphique
//==============================================================

/**** redessiner le "petit triangle" ****/
void AddOnListView::DrawLatch(BRect itemRect, int32 level, bool collapsed,bool highlighted, bool misTracked)
{
	BRect	latchRect;
	float	posStartRect;

	latchRect = itemRect;
	latchRect.right = 11;
	posStartRect = (latchRect.Height() - 11) / 2;
	latchRect.top = itemRect.top + posStartRect;
	latchRect.bottom = latchRect.top + 11;
	latchRect.InsetBy(2,2);
	latchRect.left -=1;
	latchRect.top -=1;
	SetHighColor(219,219,219,0);
	StrokeRect(BRect(latchRect.left+1,latchRect.top+1,latchRect.right+1,latchRect.bottom+1));

	SetHighColor(0,0,0,0);
	StrokeRect(latchRect);

	StrokeLine(BPoint(latchRect.left+2,latchRect.top+4),BPoint(latchRect.right-2,latchRect.top+4));
	if(collapsed)
		StrokeLine(BPoint(latchRect.left+4,latchRect.top+2),BPoint(latchRect.left+4,latchRect.bottom-2));
}

/**** on resize la liste ****/
void AddOnListView::FrameResized(float newW,float newH)
{
	AddOnProgressView	*progress = NULL;
	
	// on resize la vue
	BView::FrameResized(newW,newH);

	BRect frame = Frame();
	if((progress = (AddOnProgressView *)(Window()->FindView("Progress-view")))!=NULL)
		progress->MoveTo(2,frame.bottom+1);
}

//==============================================================
//= Traitement de l'interaction utilisateur (Clavier)
//==============================================================

/**** Clavier ****/
void AddOnListView::KeyDown(const char *bytes, int32 numBytes)
{
	switch(bytes[0])
	{
	case B_ESCAPE:
		// on quitte
		Window()->PostMessage(B_QUIT_REQUESTED);
		break;
	case B_ENTER:
	case B_LEFT_ARROW:
	case B_RIGHT_ARROW:
	case '+':
	case '-':
		// on prend cette item
		if(ExpandClass(bytes[0])!=B_OK)
			SendAddText();
		break;
	case B_UP_ARROW:
	case B_DOWN_ARROW:
		// on ne fait rien on se deplace
		break;
	case B_BACKSPACE:
	case B_DELETE:
		// effacer un caractere
		DeleteCharOfBuffer();
		break;
	case B_FUNCTION_KEY:
		{
			// appel de BeHappy
			BMessage		*msg = Window()->CurrentMessage();
			
			if(msg!=NULL)
			{
				CPreferences	prefs(PREF_FILE_NAME,PREF_PATH_NAME);
				
				prefs.Load();
				if(prefs.PreferencesLoaded()==B_OK && prefs.AskKeyDontWorry(msg))
					DisplayInBeHappy();
			}
		}
		break;
	case B_SPACE:
		// se placer sur le prochain qui correspond a la recherche
		PlaceOnKeyBuffer(true);
		break;
	default:
		// on se place sur la lettre
		AddCharToBuffer(bytes[0]);
	}

	BListView::KeyDown(bytes,numBytes);
}

/**** recuperer le clique de souris ****/
void AddOnListView::MouseDown(BPoint where)
{
	BMessage	*current = NULL;
	int32		nbClick = 0;
	
	// appeler la methode par defaut
	BOutlineListView::MouseDown(where);
		
	// recuperer le message courant et verifier si on a un double clique
	Looper()->Lock();
	current = Looper()->CurrentMessage();
	Looper()->Unlock();

	if(current!=NULL && current->FindInt32("clicks",&nbClick)==B_OK)
	{
		int32	selectedItem = CurrentSelection();

		// verifier si au moins un est selectionné
		if(selectedItem>=0 && nbClick==2)
			if(ExpandClass(B_ENTER)!=B_OK)
				SendAddText();
	}			
}

/**** ajouter un caractere au buffer ****/
void AddOnListView::AddCharToBuffer(char letter)
{
	_buffer << letter;
	PlaceOnKeyBuffer();	
}

/**** effacer un caractere du buffer ****/
void AddOnListView::DeleteCharOfBuffer()
{
	int32	length;

	// verifier que le buffer n'est pas vide
	if((length = _buffer.Length())<=0)
		return;
			
	_buffer.Remove(length-1,1);
	PlaceOnKeyBuffer();
}

/**** se placer sur l'item qui ressemble le plus au buffer ****/
void AddOnListView::PlaceOnKeyBuffer(bool startOnNext)
{
	int8			searchDirection;
	int32			startItem;
	int32			endItem;

	// si on ne specifie pas startOnNext c'est que l'on commence depuis
	// le debut ou la fin (selon l'option des settings)
	// on initialise comme si c'etait pas gerer
	searchDirection = 1;
	startItem = 0;
	endItem = FullListCountItems() - 1;
	if(_downSearch==B_CONTROL_ON)
	{
		// on commence par le bas
		searchDirection = -1;
		startItem = FullListCountItems() - 1;
		endItem = 0;
	}

	// si le buffer est vide on se place sur le premier item
	if(_buffer.Length()<=0)
	{
		Select(IndexOf(FullListItemAt(startItem)));
		ScrollToSelection();
	}
	else
	{
		// sinon on cherche
		AddOnListItem	*current = NULL;
		BString			name;
		int32			charPlace;
		bool			finded;
		bool			exit;
		int32			startAt;
		int32			index;

		// si on est a la fin on revient au debut
		startAt = startItem;
		if(startOnNext)
			startAt = FullListCurrentSelection();

		// faire le tour et ne pas repasser au meme endroit
		exit = false;
		finded = false;
		index = startAt += searchDirection;
		while(!finded && !exit)
		{
			// ne pas depasser les limites
			if((_downSearch==B_CONTROL_OFF && index>endItem) || (_downSearch==B_CONTROL_ON && index<endItem))
				index = startItem;

			// verifier pour chaque item en partant d'un index de depart
			current = (AddOnListItem *)FullListItemAt(index);
			name.SetTo(current->Name(ASK_METHOD));
			if((((charPlace = name.IFindFirst(_buffer)) != B_ERROR) && _keySensitive==B_CONTROL_OFF) 
			|| (((charPlace = name.FindFirst(_buffer)) != B_ERROR) && _keySensitive==B_CONTROL_ON))
			{
				if (charPlace == 0)
				{
					BListItem	*superItem = NULL;

					// chercher le super item
					superItem = Superitem(current);
					if(superItem==NULL)
						break;
				
					// si on le trouve on l'ouvre si c'est pas deja le cas
					if(!(superItem->IsExpanded()))
						Expand(superItem);

					// puis on se place sur l'item qui correspond au critere de recherche
					Select(IndexOf(current));
					ScrollToSelection();
					finded = true;
				}
			}

			// passer au suivant
			index += searchDirection;

			// pas repasser au point de depart
			if(index==startAt)
				exit = true;
		}

		// si on est la, on a rien trouvé
		if(!finded && !exit && FullListCurrentSelection()!=startItem)
		{
			Select(IndexOf(FullListItemAt(startItem)));
			ScrollToSelection();
		}
	}

	BMessage	displayBuffer(DISPLAY_BUFFER_MSG);
	
	displayBuffer.AddString(DISPLAY_BUFFER,_buffer);
	Window()->PostMessage(&displayBuffer);
}

/**** ouvrir une classe pour voir les methodes ****/
status_t AddOnListView::ExpandClass(uint8 letter)
{
	int32			selectedItem = CurrentSelection();
	status_t		status = B_OK;
	
	// si on fait + ou - d'office c'est comme si on voulais ouvrir la liste
	// verifier si au moins un est selectionné
	if(selectedItem<0)
		return status;
	
	int32			nbItemsUnder;
	AddOnListItem	*current = NULL;

	// element courant
	current = (AddOnListItem *)ItemAt(selectedItem);
	
	// si on obtient 0 c'est pas un superItem !
	// donc on va l'ajouter
	nbItemsUnder = CountItemsUnder(current,true);
	if(nbItemsUnder==0)
		return B_ERROR;

	switch(letter)
	{
	case B_LEFT_ARROW:
	case '-':
		if(current->IsExpanded())
			Collapse(current);
		break;
	case B_RIGHT_ARROW:
	case '+':
		if(!(current->IsExpanded()))
			Expand(current);
		break;
	case B_ENTER:
		if(current->IsExpanded())
			Collapse(current);
		else
			Expand(current);
	}
	
	// status
	return status;
}

/**** message pour informer d'ajouter le texte ****/
void AddOnListView::SendAddText()
{
	BMessage	addTextMsg(ADD_DATAS_TEXT);
	int32		modifierState = 0;

	// recuperer l'etat des touches de controle
	// et l'ajouter au message
	modifierState = modifiers();
	addTextMsg.AddInt32("modifiers",modifierState);

	// envoyer le message a la fenetre
	Window()->PostMessage(&addTextMsg);
}

/**** Gestion de BeHappy ****/
void AddOnListView::DisplayInBeHappy()
{
	AddOnListItem	*current = NULL;
	int32			selectedItem = CurrentSelection();
	
	// verifier si au moins un est selectionné
	if(selectedItem<0)
		return ;

	BString		className;
	BString		methodeName;
	
	// element courant
	current = (AddOnListItem *)ItemAt(selectedItem);

	// selon le type d'element sur lequel on se trouve
	switch(current->Type())
	{
		case TYPE_METHODE:
			{
				BListItem	*superItem = NULL;
				int32		indexOfSuperItem = -1;
				
				methodeName = current->Name(-1,true);
				superItem = Superitem(current);
				if(superItem==NULL)
					return;

				if((indexOfSuperItem = IndexOf((AddOnListItem *)superItem))<0)
					return;

				current = (AddOnListItem *)ItemAt(indexOfSuperItem);
				className = current->Name(-1,true);
			}
			break;
		case TYPE_CLASS:
		case TYPE_VARIABLE:
			return;
		default:
			// c'est normalement impossible !
			return;
	}

	BMessage	message(BH_ADDON_SHOW);
	
	message.AddString(CS_TYPE,className.String());
	message.AddString(CS_NAME,methodeName.String());
	BMessenger(be_app).SendMessage(&message);
}