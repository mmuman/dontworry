#include "CSViewerWindow.h"
#include "AddOnListInfo.h"
#include "CSLibrary.h"
#include "CSMethod.h"
#include "CSClass.h"
#include "CSVariable.h"
#include "AddOnConstantes.h"

#include <Application.h>
#include <View.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <String.h>
#include <FilePanel.h>
#include <File.h>
#include <StringView.h>

const uint32	U_LOAD_MSG			= 'Ulom';
const uint32	U_PANEL_OPEN_MSG	= 'Uopm';
const uint32	U_SELECT_CLASS_MSG	= 'Usem';

CSViewerWindow::CSViewerWindow(BRect frame)
: BWindow(frame,"CS Viewer",B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
	// positionner les filtres
	_filter = CSMT_PUBLIC | CSMT_PROTECTED | CSMT_PRIVATE;			
	
	// et initialiser le reste
	_searchClass = NULL;
	_CSLib = NULL;
	_CSFile = NULL;
	
	// la vue de support
	_supportView = new BView(Bounds(),"CS-Drag-View", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	_supportView->SetViewColor(219,219,219);

	// pour avoir des infos
	_infos = new BStringView(BRect(5,25,200,50),NULL,"Drag here a CS file...",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);

	BuildMenu();
	
	AddChild(_supportView);
	_supportView->AddChild(_menuBar);
	_supportView->AddChild(_infos);

	// pour ouvrir un fichier
	BMessenger	winMessenger(this);
	_filePanel = new BFilePanel(B_OPEN_PANEL,&winMessenger,NULL,B_FILE_NODE,false,new BMessage(U_PANEL_OPEN_MSG),NULL,true);
}

CSViewerWindow::~CSViewerWindow()
{
	if(_CSLib!=NULL)
	{
		delete _CSLib;
		delete _CSFile;
	}

	delete _filePanel;
}

/**** construire le menu ****/
void CSViewerWindow::BuildMenu()
{
	BMenu	*menu = NULL;
	
	// Main menu
	_menuBar = new BMenuBar(Bounds(),"Main menu");
	
	// File Menu
	menu = new BMenu("File");	
	menu->AddItem(new BMenuItem("Load CS File...",new BMessage(U_LOAD_MSG),'L'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Quit",new BMessage(B_QUIT_REQUESTED),'Q'));
	_menuBar->AddItem(menu);
	
	// Class menu
	_classMenu = new BMenu("Class");
	_classMenu->AddItem(new BMenuItem("Nothing",NULL));
	_menuBar->AddItem(_classMenu);
}

void CSViewerWindow::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
	case U_LOAD_MSG:
		_filePanel->Show();
		break;
	case U_PANEL_OPEN_MSG:
		OpenFile(msg);
		break;
	case U_SELECT_CLASS_MSG:
		OpenAndShowWindow(msg);
		break;
	case 'DATA':
		// c'est peut etre un drag & drop d'un fichier
		OpenFile(msg);
		break;
	default:
		BWindow::MessageReceived(msg);
	}
}

/**** reinitialiser le tout ****/
void CSViewerWindow::ResetAll()
{
	if(_CSLib!=NULL)
	{
		delete _CSLib;
		delete _CSFile;
	}

	_CSLib = NULL;
	// vider le menu
	int32	nbItems = _classMenu->CountItems();
	for(int32 i=0;i<nbItems;i++)
		delete _classMenu->RemoveItem((int32)0);
}

/**** ouvrir un fichier cs ****/
void CSViewerWindow::OpenFile(BMessage *msg)
{
	entry_ref	fileRef;

	if ((msg->FindRef("refs",&fileRef)) == B_OK);
	{
		_infos->SetText("Ouverture fichier");
		
		// initialiser
		ResetAll();

		_CSFile = new BFile(&fileRef,B_READ_ONLY);
		
		_CSLib = new CSLibrary(_CSFile);
		if(_CSLib->InitCheck()!=B_OK)
		
		{
			_infos->SetText("Erreur de Check() sur la Lib !");
			ResetAll();
			_classMenu->AddItem(new BMenuItem("Nothing",NULL));

			return;
		}
				
		DumpClass();
	}
}

/**** ouvrir la liste des methodes ****/
void CSViewerWindow::OpenAndShowWindow(BMessage *msg)
{
	BRect	bounds;
	int32	index = 0;
	BString	className("");
	
	if((msg->FindInt32("index",&index))!=B_OK)
		return;

	BMenuItem	*currentItem = NULL;
	currentItem = _classMenu->ItemAt(index);
	className = currentItem->Label();
		
	// si on a une classe on affiche les infos
	_searchClass = _CSLib->GetClass(className.String());
	if(_searchClass==NULL)
	{
		_infos->SetText("Classe introuvable !");
		return;
	}

	// on doit toujours trouver ces données
	bounds = Bounds();
	bounds.OffsetBy(100,100);
	
	// on ouvre la fenetre
	BWindow *winList = new AddOnListInfo(bounds,_supportView);

	// assigner la fenetre pour le thread
	_targetWindow = winList;
	_targetWindow->Lock();
	_targetWindow->SetFeel(B_NORMAL_WINDOW_FEEL);
	_targetWindow->SetTitle(className.String());
	_targetWindow->SetLook(B_DOCUMENT_WINDOW_LOOK);
	_targetWindow->SetFlags(0);
	_targetWindow->Unlock();
	_targetWindow->Show();		

	BMessage	datas(ADD_DATAS_LIST);
	int32		nbDatas = 1;
	// envoyer le nom de la classe
	datas.AddInt8(CS_TYPE,TYPE_CLASS);
	datas.AddString(CS_NAME,className);
	datas.AddInt8(CS_PROTECTION,CSMT_PUBLIC);
	
	// ajouter le nombre de donnees
	datas.AddInt32(NB_DATAS_TO_ADD,nbDatas);
	
	BMessenger(_targetWindow).SendMessage(&datas);

	// afficher les infos
	SearchDatas();
}

//==========================================================
// acces au methodes et variables
//==========================================================
/**** recherche des infos ****/
void CSViewerWindow::SearchDatas()
{
	BMessenger		datasTarget(_targetWindow);
	
	// commencer l'animation
	datasTarget.SendMessage(PROGRESS_START_MSG);

    // recuperer les donnees de la classe courante
    DumpMember();
	DumpMethod();       

	// terminer l'animation
	datasTarget.SendMessage(PROGRESS_STOP_MSG);

	// liberer la memoire
	delete _searchClass;
}

/**** chercher les infos d'une classe ****/
void CSViewerWindow::DumpClass()
{
	const char		*buffer = NULL;
	BString			name("");
	int32			nb = 0;
	
	// on va parcourir les variables membres
	CSClass *classCS = _CSLib->GetFirstClass();
	while (classCS != NULL)
	{		
		// initialiser la chaine
		name.SetTo("");
		nb++;

		// type de retour
		buffer = classCS->GetName();
		if(buffer==NULL)
			break;
		name = buffer;
		
		// ajouter la classe au menu
		_classMenu->AddItem(new BMenuItem(name.String(),new BMessage(U_SELECT_CLASS_MSG)));
		name << " trouvé (" << nb << ")";
		_infos->SetText(name.String());
				
		delete classCS;
		classCS = _CSLib->GetNextClass();
	}

	// afficher le nombre de classes trouvées
	name.SetTo("");
	name << nb << " classes trouvées ";
	_infos->SetText(name.String());
}

/**** chercher les metodes d'une classe ****/
void CSViewerWindow::DumpMethod()
{
	BMessage		datas(ADD_DATAS_LIST);
	BMessenger		windowList(_targetWindow);
	int16			protection;
	int32			nbDatas = 0;
	BString			type;
	BString			name;
	BString			args;
	bool			isVirtual;
		
	// on va parcourir les méthodes
	CSVariable	*argument = NULL;
	CSMethod	*method = _searchClass->GetFirstMethod(_filter);
	while (method != NULL)
	{		
		// initialiser la chaine
		type.SetTo("");
		name.SetTo("");
		args.SetTo("");

		// type de retour
		type << method->GetReturnTypeName() << " ";
		
		// les '*'
		unsigned int pointerLevel = method->GetReturnPointerLevel();
		for (unsigned int i=0; i<pointerLevel; i++)
			type << "*";

		// le nom de la méthode
		name = method->GetName();
		
		// recuperer la protection public/protected/private
		protection = method->GetZone();

		// est-ce une metode virtuelle
		isVirtual = method->IsVirtual();		

		// les arguments
		bool firstArg = true;
		argument = method->GetFirstArgument();
		while (argument != NULL)
		{
			// le type de l'argument
			if(!firstArg)
				args << ",";
			
			// recuperer le type
			args << " " << argument->GetTypeName() << " ";
			
			// les '*'
			pointerLevel = argument->GetPointerLevel();
			for (unsigned int i=0; i<pointerLevel; i++)
				args << "*";
			
			// le nom de l'argument, s'il est là
			args << argument->GetName();
			
			// la valeur par défault, si elle est là
			if(argument->GetDefaultValue()!=NULL)
				args << " = " << argument->GetDefaultValue();

			delete argument;
			argument = method->GetNextArgument();
			
			// ce n'est plus le premier
			firstArg = false;
		}
		
		// incrementer le nombre de data
		nbDatas++;
			
		// remplir avec les donnees
		datas.AddInt8(CS_TYPE,TYPE_METHODE);
		datas.AddString(CS_RETURN,type.String());
		datas.AddString(CS_NAME,name.String());
		datas.AddString(CS_PARAMETER,args.String());
		datas.AddInt16(CS_PROTECTION,protection);
		datas.AddBool(CS_VIRTUAL,isVirtual);

		delete method;
		method = _searchClass->GetNextMethod();
	}

	// ajouter le nombre de donnees
	datas.AddInt32(NB_DATAS_TO_ADD,nbDatas);

	// envoyer le tout
	if(nbDatas>0)
		windowList.SendMessage(&datas);
	
	// vider le message	
	datas.MakeEmpty();
}

/**** chercher les variables membres d'une classe ****/
void CSViewerWindow::DumpMember()
{
	BMessage		datas(ADD_DATAS_LIST);
	BMessenger		windowList(_targetWindow);
	int32			nbDatas = 0;
	int16			protection;
	BString			type;
	BString			name;
	
	// on va parcourir les variables membres
	CSVariable *member = _searchClass->GetFirstMember(_filter);
	while (member != NULL)
	{		
		// initialiser la chaine
		type.SetTo("");
		name.SetTo("");

		// type de retour
		type << member->GetTypeName() << " ";
		
		// les '*'
		unsigned int pointerLevel = member->GetPointerLevel();
		for (unsigned int i=0; i<pointerLevel; i++)
			type << "*";
		
		// le nom de la méthode
		name = member->GetName();

		// recuperer la protection public/protected/private
		protection = member->GetZone();
				
		// incrementer le nombre de data
		nbDatas++;
			
		// remplir avec les donnees		
		datas.AddInt8(CS_TYPE,TYPE_VARIABLE);
		datas.AddString(CS_RETURN,type.String());		
		datas.AddString(CS_NAME,name.String());
		datas.AddInt16(CS_PROTECTION,protection);

		// passer au suivant
		delete member;
		member = _searchClass->GetNextMember();

	}

	// ajouter le nombre de donnees
	datas.AddInt32(NB_DATAS_TO_ADD,nbDatas);
	
	// envoyer le tout
	if(nbDatas>0)
		windowList.SendMessage(&datas);
	
	// vider le message	
	datas.MakeEmpty();
}

/**** Quitter l'appli ****/
bool CSViewerWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return	true;
}