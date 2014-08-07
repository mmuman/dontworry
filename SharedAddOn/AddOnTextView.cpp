/*
 * Vue incructée dans la fenêtre texte
 *
 */

#include "AddOnTextView.h"
#include "AddOnMessageFilter.h"
#include "AddOnWindowFilter.h"
#include "AddOnConstantes.h"
#include "MTextAddOn.h"
#include "AddOnButton.h"
#include "CSPSourceFile.h"
#include "CSLibrary.h"
#include "CPreferences.h"
#include "AddOnTextExtractor.h"
#include "AddOnExtractedString.h"

#include <Application.h>
#include <Window.h>
#include <Menu.h>
#include <MenuItem.h>
#include <String.h>
#include <ctype.h>
#include <Window.h>

// debug
#include <stdio.h>

// taille verticale de la vue
const float ADDON_VIEW_HEIGHT = 20;

AddOnTextView::AddOnTextView(const char *pathFile)
: BView(BRect(0,0,100,ADDON_VIEW_HEIGHT),ADDON_VIEW_NAME,B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP,B_WILL_DRAW | B_FRAME_EVENTS)
{
	BRect	bounds = Bounds();
	
	SetViewColor(235,235,235);
	
	// ajout d'une stringview qui affiche temporairement des informations
	_StringView = new BStringView(BRect(10,2,90,ADDON_VIEW_HEIGHT-2),NULL,"AddOn...",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	AddChild(_StringView);
	
	_addOnButton = new AddOnButton(BPoint(bounds.right-14,3),"AddOnButton");
	AddChild(_addOnButton);
	
	// retenir le chemin du fichier (si il existe)
	_currentPathFile = "";
	if(pathFile!=NULL)
		_currentPathFile = pathFile;
		
	// le pointer sur la fenetre de resultat doit etre initialiser a NULL
	// on ne parse pas des la construction
	// option sur l'ajout de texte
	_askRefreshHParsing = false;
	_addTextOption = NO_ADD_TEXT_OPTION;

	// creer l'extracteur
	_extractor = new AddOnTextExtractor();
}

AddOnTextView::~AddOnTextView()
{
	// liberer la memoire
	delete _extractor;
	delete _CPPParser;
}

void AddOnTextView::AttachedToWindow()
{
	BWindow *editWindow = Window();
	
	// appel de la methode par defaut
	BView::AttachedToWindow();
	
	// recherche de l'item de menu qui appelle l'add-on, et de son message
	BMenu *menuBar = dynamic_cast<BMenu*>(editWindow->FindView("Text Window"));
	BMenuItem *item = menuBar->FindItem(ADDON_NAME);
	_AddOnMessage = item->Message();
	
	// recherche et déplacement de la vue 'texte'
	BView *view = editWindow->FindView("text");
	
	BRect frame = view->Frame();
	view->MoveBy(0,ADDON_VIEW_HEIGHT+1);
	view->ResizeBy(0,-ADDON_VIEW_HEIGHT-1);
	
	// installation d'un filtre sur la vue texte
	view->AddFilter(new AddOnMessageFilter(_AddOnMessage,this));
		
	// pareil pour le slider vertical
	view = editWindow->FindView("vert");
	view->MoveBy(0,ADDON_VIEW_HEIGHT+1);
	view->ResizeBy(0,-ADDON_VIEW_HEIGHT-1);
	
	// bricolage du rectangle pour placer correctement la vue
	frame.right = view->Frame().right;
	frame.bottom = frame.top+ADDON_VIEW_HEIGHT;
	
	// mise en place
	MoveTo(frame.LeftTop());
	ResizeTo(frame.Width(),frame.Height());

	// installer le filtre sur la fenetre
	// on doit avoir deplacer deja la vue texte car on se base
	// dessus pour le decalage de la fenetre
	editWindow->AddFilter(new AddOnWindowFilter(view->Frame().top,this));
	
	// on defini la cible du boutton
	_addOnButton->SetTarget(this);
	
	// mettre a zero le text a ajouter
	_currentText.SetTo("");
	
	// creer le parsing du fichier
	_CPPParser = new CSPSourceFile();
	
	// annuler la completion
	_askCompletion = false;
}

/**** detacher la vue ****/
void AddOnTextView::DetachedFromWindow()
{
	// il faut peux-etre mettre a jour les données
	UpdateFile();
	
	BView::DetachedFromWindow();
}

/**** appel de l'addon ****/
void AddOnTextView::AddOnCalled(MTextAddOn *text)
{
	// si c'est une mise a jour du header !!!
	if(_askRefreshHParsing)
	{
		BMessage		message(REFRESH_HEADER_MSG);
		BString			headerText;

		// recuperer le text et le parser
		headerText = text->Text();
		
		// proteger le parsing
		message.AddString(FILE_NAME,_currentPathFile);
		message.AddString(HEADER_TEXT,headerText);
		BMessenger(be_app).SendMessage(&message);

		// annuler le refresh du header
		_askRefreshHParsing = false;
		
		// quitter
		return;
	}

	int32		start,end;

	// on regarde toujous ou l'on est !
	text->GetSelection(&start,&end);

	// si on a _currentText deja renseigner c'est que l'on doit ajouter
	// le text contenu dedans
	if(_currentText.Length()>0)
	{
		int32	sizeAddText = 0;

		// si c'est la completion on enleve le debut	
		sizeAddText = _currentVariable.Length();
		if(_typeAsk==ASK_COMPLETION)
		{
			// effacer le debut de la chaine a competer
			text->Select(start-sizeAddText,end);
			text->Delete();

			// recuperer la position courante
			text->GetSelection(&start,&end);
		}
		else
		{
			// si on est pas dans la completion on tiens
			// compte de l'option d'ajout du ; a la fin
			if(_addTextOption & ADD_ENDLINE_OPTION)
				_currentText.Append(";");
		}

		// dans le cas d'une fonction on se place devant
		// la parenthese ouvrante
		sizeAddText = _currentText.FindFirst("(");
		if(sizeAddText == B_ERROR)
			sizeAddText = _currentText.Length();
		else
			sizeAddText++;

		// ajouter le texte
		text->Insert(_currentText.String());

		// puis se placer a l'endroit determiné
		sizeAddText += start;
		if(_addTextOption & PLACE_ON_END_LINE_OPTION)
			sizeAddText = start + _currentText.Length();
		text->Select(sizeAddText,sizeAddText);

		// vider le texte courant a ajouter
		_currentText.SetTo("");
		
		// quitter
		return;
	}

	// chercher le texte
	Parse(text->Text(),start,end);
	
	// afficher les infos
	DisplayInormations();
}

/**** extraction du text ****/
void AddOnTextView::Parse(const char *currentText,int32 start,int32 end)
{
	// réinitialisation
	_typeAsk = -1;
	_currentVariable.SetTo("");
	_currentSelection.SetTo("");	
	_parseWithoutError = false;
	_varIsLocal = false;

	_typeAsk = SelectTypeAsk(currentText,start,end);
	// on a rien a faire
	if(_typeAsk==-1)
		return;

	// on est en debut de text
	if(end<0)
	{
		_StringView->SetText("Start of file");
		return;
	}

	// sinon on va voir ce qui est demandé
	BString					extractString("");
	AddOnExtractedString	*extracted = NULL;

	// chercher tout ce qui est locale
	_CPPParser->Parse(currentText,currentText+end+1);

	// sauvegarder la classe courante
	_currentClasse = _CPPParser->CurrentClass();

	// extraire la chaine
	_extractor->Extract(currentText,start,(_typeAsk==ASK_COMPLETION));
	extracted = _extractor->Extracted();

	// on doit recuperer quelque chose
	if(extracted==NULL)
		return;

	// recuperer le niveau d'extraction
	_extractLevel = extracted->_recursLevel;

	// recuperer la chaine
	extractString = extracted->_string;
		
	switch(_typeAsk)
	{
	// si c'est BeHappy ok on demande BeHappy
	case ASK_HELP_BEHAPPY:
		{
			// rechercher le type de la chaine extraite
			unsigned int	pointerLevel = 0;

			_currentVariable = extractString;
			// cela peux etre une variable locale

			// est-ce une variable
			extractString = _CPPParser->TypeOfVariable(_currentVariable.String(),&pointerLevel);
			if(extractString!="")
				_currentVariable = extractString;
		}
		break;
	// on veux completer
	case ASK_COMPLETION:
		{
			// plus de 2 caracteres !!!!
			if(extractString.Length()<3)
				return;

			// ok on retient ce qu'on a trouvé
			_currentVariable = extractString;
		}
		break;
	// sinon on cherche un objet
	case ASK_OBJECT:
	case ASK_POINTER:
	case ASK_METHOD:
		{
			// on est peux etre dans le cas d'un cast
			bool			asCast = extracted->_operatorAsCast;
			char			operatorCast = extracted->_identifier;
			unsigned int	pointerLevel = 0;
				
			// c'est le cas d'un cast (exemple *)maVar
			if(asCast && operatorCast=='*' && _typeAsk!=ASK_POINTER) 
				return;
				
			// c'est le cas d'un acces a un objet (*monPointer)
			if(!asCast && operatorCast=='*' && _typeAsk!=ASK_OBJECT)
				return;

			// c'est le cas d'un acces au pointer (&maVar)
			if(!asCast && operatorCast=='&' && _typeAsk!=ASK_POINTER)
				return;
				
			// est-ce une variable locale
			_currentVariable = _CPPParser->TypeOfVariable(extractString.String(),&pointerLevel);
	
			if(_currentVariable.Length()>0)
			{
				// chercher si on demande bien le bon niveau/type
				if(_typeAsk==ASK_METHOD)
					return;

				if(_typeAsk==ASK_OBJECT && pointerLevel>0)
					return;

				if(_typeAsk==ASK_POINTER && pointerLevel==0)
					return;
			
				// c'est une variable locale
				_varIsLocal = true;
			}
			else
			{
				// recuperer la chaine
				_currentVariable = extractString;
			}
		}
	}

	// bon ben on a pas d'erreur de parsing
	_parseWithoutError = true;

	// envoyer les info au AddOnHandler	
	SendInformationToHandler();	
}

/**** selectionner le type de demande ****/
int8 AddOnTextView::SelectTypeAsk(const char *currentText,int32 &start,int32 &end)
{
	int8		ask = -1;
	int32		length = 0;
	
	// determiner la longueur de la selection
	length = end - start;

	// une selection probablement une demande d'aide sur une fonction
	if (length != 0)
	{
		// recuperer la selection
		_currentSelection.Append(currentText+start,length);

		// donc c'est une demande d'aide sur une fonction
		ask = ASK_HELP_BEHAPPY;

		// se mettre comme si c'etait une autre demande
		end = start;

	}

	// déjà on regarde le dernier caractère tapé
	// et on cherche la fin du mot (nom de variable)
	switch(currentText[end-1])
	{
	case '.':
		end --;
		if(ask==-1)
			ask =  ASK_OBJECT;
		break;
	case '>':
		{
			end -= 2;
			if (currentText[end] != '-')
			{
				_StringView->SetText("> but not a -");
				return -1;
			}
			if(ask==-1)
				ask =  ASK_POINTER;
		}
		break;
	case ':':
		{
			end -= 2;
			if (currentText[end] != ':')
			{
				_StringView->SetText(": but not a :");
				return -1;
			}
			if(ask==-1)
				ask = ASK_METHOD;
		}
		break;
	default:
		{
			if(_askCompletion)
			{
				CPreferences	prefs(PREF_FILE_NAME,PREF_PATH_NAME);

				// verifier si la completion est desactivé
				prefs.Load();	
				ask = ASK_COMPLETION;
				if(prefs._useCompleteWord != B_CONTROL_ON)
				{
					_StringView->SetText("fonction to complete word disabled");
					ask = -1;
				}				
			}
		}
	}

	// annuler de toute maniere la completion
	_askCompletion = false;	

	// retablir la position
	start = end;

	return ask;
}

/**** on customise le dessin de la vue ****/
void AddOnTextView::Draw(BRect frame)
{
	BRect	bounds = Bounds();

	BView::Draw(frame);

	SetHighColor(156,156,156);
	StrokeLine(BPoint(bounds.left,bounds.bottom),BPoint(bounds.right,bounds.bottom));
}

/**** on attend un clique ****/
void AddOnTextView::MessageReceived(BMessage *message)
{

	switch (message->what)
	{
		// on doit reparser un fichier pour prendre en compte les mises a jour
		case UPDATE_FILE_MSG:
			UpdateFile();
			break;
		// on doit bloquer la fenetre
		case LOCK_MOTHER_WIN:
			// si on a trouve une classe on affiche la fenetre
			Looper()->PostMessage(ADDON_BUTTON_SELECT_MSG,_addOnButton);
			break;
		// on doit debloquer la fenetre
		// envoyer le pointer de la fenetre a la vue pour les deplacement
		case UNLOCK_MOTHER_WIN:
			Looper()->PostMessage(ADDON_BUTTON_UNSELECT_MSG,_addOnButton);
			break;
		case MOVE_WINDOW_PTR:
			Looper()->PostMessage(message);
			break;
		case ADD_DATAS_TEXT:
			GetAddText(message);
			break;
		case DISPLAY_BUFFER_MSG:
			DisplaySearchBuffer(message);
			break;
		case CALL_COMPTETE_WORD:
			_askCompletion = true;
			break;
		default:
			BView::MessageReceived(message);
	}
}

/**** on resize, attention le bouton aussi ****/
void AddOnTextView::FrameResized(float width,float height)
{
	_addOnButton->MoveTo(BPoint(width-14,3));
}

/**** demander au handler d'ouvrir la fenetre ****/
void AddOnTextView::SendInformationToHandler()
{
	BMessage		searchMessage(LAUNCH_SEARCH_MSG);
	BRect			winRect = ConvertToScreen(Frame());

	// on determine la taille de la fenetre
	winRect.SetRightBottom(BPoint(winRect.right-1,winRect.bottom+((Window()->Bounds()).Height() / 2)));
	
	// on rempli le message avec les infos : variables etc ... 
	searchMessage.AddRect(SHOW_WINDOW_RECT,winRect);
	searchMessage.AddPointer(MOTHER_VIEW,this);
	searchMessage.AddString(VARIABLE_NAME,_currentVariable);
	searchMessage.AddString(CLASSE_NAME,_currentClasse);
	searchMessage.AddString(SELECTION,_currentSelection);
	searchMessage.AddInt8(CS_ASK_TYPE,_typeAsk);
	searchMessage.AddInt32(RECURS_LEVEL,_extractLevel);
	searchMessage.AddBool(VARIABLE_LOCAL,_varIsLocal);
	
	// et on doit ajouter les variables locales de la fonction
	if(_typeAsk==ASK_COMPLETION)
	{
		const char *varName = _CPPParser->GetFirstVariableName();
		while (varName != NULL)
		{
			// sauvegarder le nom trouvé
			searchMessage.AddString(CS_NAME,varName);
			varName = _CPPParser->GetNextVariableName();
		}
	}

	// envoyer la commande
	be_app->PostMessage(&searchMessage);
}

/**** afficher les informations ****/
void AddOnTextView::DisplayInormations()
{
	CPreferences	prefs(PREF_FILE_NAME,PREF_PATH_NAME);
	BString			message("");

	// message standard
	message = "Parsing...";
	
	// afficher des infos supplementaire
	prefs.Load();	
	if(prefs._useInfoParsing == B_CONTROL_ON)
	{
		message << " Variable : < ";
		message << _currentVariable;
		message << " > Class : < " << _currentClasse;
		message << " > Selection : < " << _currentSelection << " > ";
		switch(_typeAsk)
		{
		case ASK_COMPLETION:
			message << "Request : < Complete Word >";
			break;
		case ASK_HELP_BEHAPPY:
			message << "Request : < Be Happy Display >";
			break;
		case ASK_METHOD:
			message << "Request : < Method >";
			break;
		case ASK_OBJECT:
			message << "Type : < Object >";
			break;
		case ASK_POINTER:
			message << "Type : < Pointer >";
			break;
		}
	}
	
	// gerer le cas d'erreur
	if(!_parseWithoutError)
		message << " (Parsing : Error)";

	// afficher le message
	_StringView->SetText(message.String());	
}

/**** recuperer la chaine a ajouter ****/
void AddOnTextView::GetAddText(BMessage *message)
{
	int32		modifiers = 0;
	int8		askType = -1;
	BString		text("");

	// en fonctions des touches de fonction on va initialiser
	// une variable qui determine ou se placer apres l'ajout de texte
	// et qui permet d'ajouter un ;

	// reinitialiser l'etat des touches
	_addTextOption = NO_ADD_TEXT_OPTION;

	// recuperer le texte a ajouter
	// et le type de demande
	message->FindInt8(CS_ASK_TYPE,&askType);
	message->FindString(SRING_TO_ADD,&text);

	// on ne gere pas lors de la completion
	// les touches speciales
	if(message->FindInt32("modifiers",&modifiers)==B_OK && text.Length()>0)
	{
		// ajouter un ; a la fin
		if(modifiers & B_RIGHT_SHIFT_KEY)
			_addTextOption |= ADD_ENDLINE_OPTION;

		// ne pas se placer dans les parentheses
		if(modifiers & B_RIGHT_CONTROL_KEY)
			_addTextOption |= PLACE_ON_END_LINE_OPTION;
	}

	// si on a du texte on appel l'addon
	// pour declancher l'ajout
	if(text.Length()>0)
		_currentText.SetTo(text);

	// on peux ne rien avoir dans le message on ajoute alors le contenu du buffer
	if(_currentText.Length()>0)
		Window()->PostMessage(_AddOnMessage);
}

/**** afficher le buffer de recherche ****/
void AddOnTextView::DisplaySearchBuffer(BMessage *message)
{
	BString		text("");

	if(message->FindString(DISPLAY_BUFFER,&text)!=B_OK)
		return;

	_currentText.SetTo(text);
	_StringView->SetText(_currentText.String());
}

/**** mettre a jour les données du fichier ****/
void AddOnTextView::UpdateFile()
{
	// si on a rien c'est un nouveau fichier
	if(_currentPathFile.Length()>0)
	{
		BMessage	updateFile(UPDATE_FILE_MSG);
	
		// envoyer le nom et chemin du fichier ouvert
		updateFile.AddString(FILE_NAME,_currentPathFile);
		be_app->PostMessage(&updateFile);
	}
}

/**** activation de la fenetre ****/
void AddOnTextView::WindowActivated(bool active)
{
	// on va rafraichir quand on desactive la fenetre
	// le message d'appel de l'addon doit etre valide
	if(active && _AddOnMessage!=NULL)
		return;
	
	BString		winTitle;
	
	// recuperer le nom de la fenetre pour voir si c'est header
	LockLooper();
	winTitle = Window()->Title();
	UnlockLooper();
	
	// trouver si c'est un *.h
	if(winTitle.Length()<3)
		return;
	
	winTitle.Remove(0,winTitle.CountChars()-2);
	if(winTitle != ".h")
		return;
	
	// ok on refresh	
	_askRefreshHParsing = true;
	Window()->PostMessage(_AddOnMessage);
}
