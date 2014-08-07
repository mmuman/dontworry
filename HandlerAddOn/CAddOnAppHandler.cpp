/*******************/
/* AddOnAppHandler */
/*******************/

#include "CAddOnAppHandler.h"
#include "AddOnTextView.h"
#include "AddOnIconView.h"
#include "AddOnListInfo.h"
#include "AddOnConstantes.h"
#include "AddOnInfoParsing.h"
#include "CSContainer.h"
#include "CSLibrary.h"
#include "CSMethod.h"
#include "CSClass.h"
#include "CSVariable.h"
#include "CPreferences.h"
#include "CProject.h"
#include "HappyCommander.h"

#include <Application.h>
#include <Entry.h>
#include <Path.h>
#include <Directory.h>
#include <Window.h>
#include <String.h>
#include <File.h>
#include <FindDirectory.h>
#include <DataIO.h>
#include <Control.h>
#include <Node.h>
#include <NodeInfo.h>
#include <Bitmap.h>
#include <Locker.h>

#include <stdio.h>

//===============================================================
// Destructeur et constructeur de CAddOnAppHandler
//===============================================================

/**** constructeur ****/
CAddOnAppHandler::CAddOnAppHandler()
: BHandler(ADDON_HANDLER_NAME)
{
	_settings = new CPreferences(PREF_FILE_NAME,PREF_PATH_NAME);

	// doit-on afficher le splash-screen
	_settings->Load();
	_displaySplash = (_settings->_DisplaySplash == B_CONTROL_ON);

	// verifier si le type MIME est installé
	VerifyMIMEType();
	
	// verifier si on l'utlilise
	UpdatePrefsSettings();

	// Création du bloc mémoire
	// pour fichier CS des classes systemes
	_systemFile = NULL;
	
	// Création de la lib qui va contenir les autres
	_conteneur = new CSContainer();

	// Ajout de la lib systeme
	SystemFiles();

	// creation du HappyCommander !
	_beHappyCommand = new HappyCommander(BH_ADDON_NAME,BH_ADDON_FILE_NAME,"");

	// creation du locker
	_generalLocker = new BLocker("DontWorry-Locker");
}

/**** destructeur ****/
CAddOnAppHandler::~CAddOnAppHandler()
{
	int32		nbProject;
	int32		nbPath;

	// effacer les chemins systeme de la liste
	nbPath = _systemPath.CountItems();
	for(int32 index=0;index<nbPath;index++)
		delete (BString *)(_systemPath.RemoveItem((int32)0));
	
	// effacer les element CProject de la liste
	nbProject = _projectList.CountItems();
	for(int32 index=0;index<nbProject;index++)
		delete (CProject *)(_projectList.RemoveItem((int32)0));

	// destruction de la lib conteneur
	delete _conteneur;
	delete _systemFile;
	delete _settings;

	// on efface le HappyCommander
	delete _beHappyCommand;
	
	// liberer le locker
	delete _generalLocker;
}

/**** reception des messages ****/
void CAddOnAppHandler::MessageReceived(BMessage *message)
{
		switch(message->what)
		{
		// on change l'utilisation de l'addon (oui/non)
		case PREFS_CHANGED_MSG:
			UpdatePrefsSettings();
			break;
		// on creer un nouveau fichier
		case NEW_FILE_MSG:
		// on ouvre un fichier *.h ou *.cpp
		case OPEN_FILE_MSG:
			if(_useDontWorry)
				InstallAddOn(message);
			break;
		// on ferme un fichier *.h ou *.cpp
		case ADD_FILE_MSG:
			ParseNewProjectFile(message);
			break;
		// on retire un fichier du projet
		case REMOVE_FILE_MSG:
			UpdateProjectFile(message);			
			break;
		case UPDATE_FILE_MSG:
			UpdateFileOfProject(message);
			break;
		// lancer la recherche
		case LAUNCH_SEARCH_MSG:
			LaunchSearch(message);
			break;
		// fermer l'aide active
		case CLOSE_WINDOW_MSG:
			KillFillThread();
			break;
		// ouverture d'un projet
		case OPEN_PROJECT_FILE:
			OpenProject(message);
			break;
		// rafraichir la lib systeme
		case REFRESH_SYSLIB_MSG:
			RefreshSystemLib();
			break;
		// affichage dans behappy
		case BH_ADDON_SHOW:
			CallBeHappy(message);
			break;
		// reception de la fin du traitement du thread (cas de la completion)
		case DISPLAY_SEARCH_RESULT_MSG:
			ManageSearchResult();
			break;
		// rafraichir la CSLib du projet avec un header
		case REFRESH_HEADER_MSG:
			RefreshHeader(message);
			break;
		default:
			BHandler::MessageReceived(message);
		}
}

//==================================================================
// appel a la creation du handler par la vue settings 
// et metode d'installaion de l'addon dans les fenetres de code
// et installation de l'icone qui indique la presence de DontWorry
//==================================================================

/**** rechercher une fenetre de l'application d'apres son titre ****/
BWindow *CAddOnAppHandler::SearchWindowOfBeIDE(BString &name,bool entireName)
{
	// ok sinon on va chercher la fenetre du projet puis inserer
	// notre petite vue qui dessine l'icone
	int32		nbWindow = 0;
	int32		timeOut = 0;
	BWindow		*searchWindow = NULL;
	BWindow		*newWindow = NULL;
	BString		searchName("");
	bool		exit = false;

	//recherche de la fenetre du projet
	while(timeOut<30 && !exit)
	{
		// parcourir chaque fenetre
		nbWindow = be_app->CountWindows();
		for(int32 index = 0;index<nbWindow;index++)
		{
			searchWindow = be_app->WindowAt(index);	
			if(searchWindow!=NULL)
			{
				// bloquer la fenetre si elle ne l'est pas
				// on doit le faire pour pouvoir recuperer son titre
				if(!(searchWindow->IsLocked()))
					searchWindow->Lock();
				
				// recuperer le titre et comparer avec celui que l'on recherche
				searchName.SetTo(searchWindow->Title());
				if((searchName == name) || ((searchName.IFindFirst(name)!=B_ERROR) && !entireName))
				{
					// ok on a trouvé on sort
					newWindow = searchWindow;
					exit = true;		
				}
				
				// debloquer la fenetre si il le faut
				if(searchWindow->IsLocked())
					searchWindow->Unlock();
			}
		}
		// gerer un timeOut, c'est pour eviter le "while(1)"
		timeOut++;
	}

	// on a pas trouvé la fenetre, le temps de "timeout" est depassé
	if(timeOut==30)
		return NULL;

	return newWindow;
}

/**** Installe l'AddOn dans la nouvelle fenetre ****/
void CAddOnAppHandler::InstallAddOn(BMessage *message)
{
	BString		winName("");
	BString		winPath("");
	entry_ref	entry;
	bool		entireName = true;
	BWindow		*newWindow = NULL;
	
	// si il n'y a pas de ref c'est un new (Untiteled ...)
	if(message->FindRef("refs",&entry)==B_OK)
	{
		BEntry		file(&entry);
		BPath		pathFile;
		
		// recuperer le nom et le chemin du fichier
		file.GetPath(&pathFile);
		winName.SetTo(entry.name);
		winPath.SetTo(pathFile.Path());
	}
	else
	{
		// c'est un nouveau, donc son nom va contenir "Untitled"
		winName.SetTo("Untitled ");
		winPath.SetTo("");
		entireName = false;
	}

	// rechercher la fenetre, si on a rien trouvé on sort
	newWindow = SearchWindowOfBeIDE(winName,entireName);
	if(newWindow==NULL)
		return;

	// ajouter l'addon que si elle n'y est pas deja
	newWindow->Lock();
	if(newWindow->FindView(ADDON_VIEW_NAME)==NULL)
		newWindow->AddChild(new AddOnTextView(winPath.String()));	
	newWindow->Unlock();
}

/**** Installe l'icone de l'AddOn dans la nouvelle fenetre projet ****/
void CAddOnAppHandler::InstallIconAddOn(BString &name,CProject *currentProject)
{
	// doit-on afficher l'icone dans le coin haut droit ?
	_settings->Load();	
	if(_settings->_useDontWorry == B_CONTROL_OFF)
		return;

	BWindow		*newWindow = NULL;

	// rechercher la fenetre, si on a rien trouvé on sort
	newWindow = SearchWindowOfBeIDE(name,true);
	if(newWindow==NULL)
		return;
	
	// ajouter l'icone a la fenetre du projet
	BView			*projectBar = NULL;
	
	newWindow->Lock();
	if((projectBar = newWindow->FindView("ProjectBar"))!=NULL)
	{
		AddOnIconView	*pictureView = NULL;
		BRect			frameBar;
	
		projectBar->ResizeBy(-20,0);		
		frameBar = projectBar->Frame();
		frameBar.left = frameBar.right + 1;
		frameBar.right += 19;
		pictureView = new AddOnIconView(BRect(frameBar));
		newWindow->AddChild(pictureView);
		
		// fixer la vue de l'icon
		currentProject->SetBlinkMessenger(pictureView);
	}	

	// debloquer la fenetre
	newWindow->Unlock();
}

//===============================================================
// Regeneration de la CSLibrary systeme et parsing des 
// repertoires d'un projet
//===============================================================

/**** Chercher les fichiers *.h pour un projet et creer la CSLibrary ****/
void CAddOnAppHandler::OpenProject(BMessage	*message)
{
	BString		name;
	BString		path;
	status_t	state;

	// le nom du projet et le chemin
	state = message->FindString(PROJECT_FILE_NAME,&name);
	state &= message->FindString(PROJECT_FILE_PATH,&path);
	if(state!=B_OK)
		return;

	// ouvrir la fenetre de chargement mais en petite taille
	// car si le pointeur est NULL c'est que ce n'est pas la
	// premiere ouverture

	// affichage de la fenetre de chargement
	_splashWindow = new AddOnInfoParsing(_displaySplash);
	_splashWindow->Show();
	
	// afficher un peu le splash-screen
	if(_displaySplash)
	{
		_displaySplash = false;
		snooze(500000);
	}
	
	// on ne va pas creer deux fois les informations que l'on connait
	CProject	*current = NULL;
	int32		index = 0;
		
	index = ItemFromProject(name.String());
	if(index>=0)
		current = (CProject *)(_projectList.ItemAt(index));
	else
		current = CreateProject(name,path,message);
	
	// installer l'icone dans la fenetre projet
	InstallIconAddOn(name,current);

	// on parse les fichiers du projet
	ParseProject(current);
	
	// ajouter les donnees du projet si c'est un nouveau
	if(index==-1)
		_conteneur->AddProject(current->Library());	

	// fermer la fenetre de chargement
	_splashWindow->PostMessage(B_QUIT_REQUESTED);
	_splashWindow = NULL;
}

/**** creer un nouveau projet ****/
CProject *CAddOnAppHandler::CreateProject(BString &ProjectName,BString &ProjectPath,BMessage *message)
{
	int32		index;
	BString		path("");
	BString		file("");
	CProject	*newProject = NULL;

	// Création du nouveau projet
	newProject = new CProject(ProjectName.String(),ProjectPath.String(),_splashWindow);
					
	// on retient les chemins du projet
	index = 0;
	while(message->FindString(PROJECT_PATH,index++,&path)==B_OK)
		newProject->AddPath(path.String());

	// on retient les fichiers du projet
	index = 0;
	while(message->FindString(FILE_PATHNAME,index++,&file)==B_OK)
		newProject->AddFile(file.String());
	
	// on retient les chemins du systeme
	index = 0;
	while(message->FindString(SYSTEM_PATH,index++,&path)==B_OK)
		_systemPath.AddItem(new BString(path));

	// ajouter notre projet a la liste
	_projectList.AddItem(newProject);
	
	// retourner le nouveau projet
	return newProject;	
}

/**** reouvrir un projet deja existant ****/
void CAddOnAppHandler::ParseProject(CProject *project)
{
	// on reparse les fichiers du projet 
	// il est possible que un fichier a ete editer hors de BeIDE !!
	// montrer qu'on parse
	project->BlinkIcon();
		
	// proteger le parsing
	_generalLocker->Lock();
	project->Parse();
	_generalLocker->Unlock();

	// ok c'est fini
	project->BlinkIcon();
}

/**** Regeneration des ficiers CSLibrary pour le systeme ****/
void CAddOnAppHandler::SystemFiles()
{
	BString		optionPath;
	BPath		settingPath;

	// si le fichier n'existe pas on va le creer
	find_directory(B_USER_SETTINGS_DIRECTORY,&settingPath);
	optionPath.SetTo(settingPath.Path());
	optionPath << "/" << SETTING_PATH;
	
	// on doit creer le repertoir si il n'existe pas
	BDirectory	settingDirectory(optionPath.String());
	if(settingDirectory.InitCheck()!=B_OK)
		create_directory(optionPath.String(),0x1FF);

	// verifier si le fichier existe
	optionPath << "/" << "SystemHeaders.cs";
	_systemFile = new BFile(optionPath.String(),B_READ_ONLY);
	if(_systemFile->InitCheck() != B_OK)
	{
		_systemFile->SetTo(optionPath.String(),B_READ_WRITE|B_CREATE_FILE|B_ERASE_FILE);
		if(_systemFile->InitCheck() != B_OK)
			return;

		// renseigner le MIME Type
		BNode		fileNode(optionPath.String());
		BNodeInfo	nodeInfo(&fileNode);

		if(nodeInfo.InitCheck()==B_OK)
			nodeInfo.SetType("application/x-vnd.CKJ-Daixiwen-CSFile");

		// creer la lib systeme
		_systemLibrary = new CSLibrary(_systemFile);
		if (_systemLibrary->InitCheck() != B_OK)
			return;

		// proteger le parsing
		_generalLocker->Lock();
		_systemLibrary->Parse("/boot/develop/headers/be",true);
		_generalLocker->Unlock();
		
		// sauvegarder le fichier
		delete _systemLibrary;
		_systemFile->Unset();
	}
	
	// on ouvre a nouveau le fichier
	_systemFile->SetTo(optionPath.String(),B_READ_ONLY);
	if(_systemFile->InitCheck() != B_OK)
		return;
		
	_systemLibrary = new CSLibrary(_systemFile);
	if(_systemLibrary->InitCheck() == B_OK)
		_conteneur->AddSystemLib(_systemLibrary);
}

/**** Rafraichir la lib systeme ****/
void CAddOnAppHandler::RefreshSystemLib()
{
	// si la lib existe la vider
	if(_systemLibrary==NULL || _systemFile==NULL)
		return;

	BString		optionPath;
	BPath		settingPath;

	// si le repertoire n'existe pas on va le creer
	find_directory(B_USER_SETTINGS_DIRECTORY,&settingPath);
	optionPath.SetTo(settingPath.Path());
	optionPath << "/" << SETTING_PATH << "/SystemHeaders.cs";
	
	// vider la lib puis la remplir a nouveau	
	_systemFile->SetTo(optionPath.String(),B_READ_WRITE|B_CREATE_FILE|B_ERASE_FILE);
	if(_systemFile->InitCheck() != B_OK)
		return;

	// renseigner le MIME Type
	BNode		fileNode(optionPath.String());
	BNodeInfo	nodeInfo(&fileNode);

	if(nodeInfo.InitCheck()==B_OK)
		nodeInfo.SetType("application/x-vnd.CKJ-Daixiwen-CSFile");

	// vider l'actuelle CSlib systeme
	_systemLibrary->Empty();
	// proteger le parsing
	_generalLocker->Lock();
	_systemLibrary->Parse("/boot/develop/headers/be",true);
	_generalLocker->Unlock();
		
	// sauvegarder le fichier
	delete _systemLibrary;
	_systemFile->Unset();
	
	// on ouvre a nouveau le fichier
	_systemFile->SetTo(optionPath.String(),B_READ_ONLY);
	if(_systemFile->InitCheck() != B_OK)
		return;

	// recherger la lib systeme (rafraichie)
	_systemLibrary = new CSLibrary(_systemFile);
}

//===============================================================
// Creer la fenetre de resultat des recherches
//===============================================================

/**** lancer la recherche ****/
void CAddOnAppHandler::LaunchSearch(BMessage *message)
{
	int8	typeAsk;
	
	// trouver le type d'aide
	if(message->FindInt8(CS_ASK_TYPE,&typeAsk)!=B_OK)
		return;
		
	// ouvrir l'aide active
	switch(typeAsk)
	{
	case ASK_OBJECT:
	case ASK_POINTER:
	case ASK_METHOD:
		InitSearch(message);
		break;
	case ASK_COMPLETION:
		InitCompletion(message);
		break;
	// demande d'aide sur une methode
	case ASK_HELP_BEHAPPY:
		AskHelpForMethod(message);
		break;
	default:
		; // rien c'est une erreur
	}
}

/**** ouvrir la liste des methodes ****/
void CAddOnAppHandler::InitSearch(BMessage *message)
{
	BString			var;
	BString			classe;
	BString			classeName;
	CSClass			*resultClass = NULL;
	status_t		dataFind;
	int32			recursLevel;
	bool			varIsLocal;
	
	// initialiser
	_searchClass = NULL;
	_filter = CSMT_PUBLIC;
	_askType = 0;
	_typeMessage = ADD_DATAS_LIST;

	// on doit toujours trouver ces données
	dataFind = message->FindRect(SHOW_WINDOW_RECT,&_destWindow);
	dataFind &= message->FindPointer(MOTHER_VIEW,(void **)(&_target));
	dataFind &= message->FindString(VARIABLE_NAME,&var);
	dataFind &= message->FindString(CLASSE_NAME,&classe);
	dataFind &= message->FindInt8(CS_ASK_TYPE,&_askType);
	dataFind &= message->FindInt32(RECURS_LEVEL,&recursLevel);
	dataFind &= message->FindBool(VARIABLE_LOCAL,&varIsLocal);

	if(dataFind!=B_OK)
		return;
	
	// si var est egale a this on pend la classe courante
	if(var=="this")
		var = classe;

	//si la classe est la meme que dans la variable on change le filter		
	// on doit aussi avoir un recurslevel > 1 avec une metode object
	if(var==classe)
		_filter = CSMT_PUBLIC | CSMT_PROTECTED | CSMT_PRIVATE;		
	
	// proteger le parcour des CSLibs
	_generalLocker->Lock();
			
	// var est-il le nom d'une classe
	_searchClass = _conteneur->GetClass(var.String());

	// assigner la destination des messages du thread
	_winMessenger = BMessenger(this);

	// non donc ca doit etre une variable membre
	if(_searchClass == NULL)
	{
		resultClass = _conteneur->GetClass(classe.String());
		// attention on peut ne pas trouver la classe
		if(resultClass == NULL)
		{
			// deproteger le parcour des CSLibs
			_generalLocker->Unlock();
			
			return ;
		}

		BString			typeVar;
		unsigned int	pointerLevel = 0;
		
		// si non, on doit chercher si c'est une variable membre
		// et recuperer le niveau  (pointer/objet) de la variable
		if(SearchMemberFromClass(resultClass,var.String(),typeVar,pointerLevel))
		{
			CSClass			*varClass = NULL;
		
			// et verifier si la demande correspond
			// puis rechercher la classe du type
			if((pointerLevel>0 && _askType==ASK_POINTER) || (pointerLevel==0 && _askType==ASK_OBJECT))				
				if((varClass = _conteneur->GetClass(typeVar.String()))!=NULL)
					_searchClass = varClass;

			// si la variable est membre
			_filter = CSMT_PUBLIC | CSMT_PROTECTED | CSMT_PRIVATE;			
		}
	}

	// deproteger le parcour des CSLibs
	_generalLocker->Unlock();

	if(_searchClass != NULL)
	{
		// pour gerer le cas des include
		classeName = _searchClass->GetName();
		if(classeName==var && recursLevel<2 && _askType!=ASK_METHOD && !varIsLocal)
		{				
			delete _searchClass;
			_searchClass = NULL;
			
			return;
		}

		// lancer le thread de recherche
		CreateFillThread();
	}
}

/**** effectuer une recherche pour une completion ****/
void CAddOnAppHandler::InitCompletion(BMessage *message)
{
	BString			varName;
	BString			classe;
	status_t		dataFind;

	// initialiser
	_searchClass = NULL;
	_filter = CSMT_PUBLIC | CSMT_PROTECTED | CSMT_PRIVATE;		
	_askType = 0;
	_typeMessage = ADD_COMPLETION_DATAS;

	// on doit toujours trouver ces données
	dataFind = message->FindRect(SHOW_WINDOW_RECT,&_destWindow);
	dataFind &= message->FindPointer(MOTHER_VIEW,(void **)(&_target));
	dataFind &= message->FindString(VARIABLE_NAME,&_searchString);
	dataFind &= message->FindString(CLASSE_NAME,&classe);
	dataFind &= message->FindInt8(CS_ASK_TYPE,&_askType);
	
	// on doit tout avoir trouvé
	if(dataFind!=B_OK)
		return;

	// relire les prefs (elles peuvent avoir changées)
	_settings->Load();

	// proteger le parcour des CSLibs
	_generalLocker->Lock();
	
	_searchClass = _conteneur->GetClass(classe.String());
	
	// deproteger le parcour des CSLibs
	_generalLocker->Unlock();

	// attention on peut ne pas trouver la classe c'est une erreur dans ce cas
	if(_searchClass == NULL)
		return ;
	
	// assigner la destination des messages du thread
	_winMessenger = BMessenger(this);

	BMessage	*datas = NULL;
	int32		index = 0;
	int32		nbDatas = 0;

	// rechercher toutes les variables locales
	datas = new BMessage(_typeMessage);
	while(message->FindString(CS_NAME,index,&varName)==B_OK)
	{
		if((_settings->_keySensitive == B_CONTROL_ON && varName.FindFirst(_searchString)==0)
		|| (_settings->_keySensitive == B_CONTROL_OFF && varName.IFindFirst(_searchString)==0))
		{
			// remplir le message puis envoyer les infos
			datas->AddInt8(CS_TYPE,TYPE_VARIABLE);
			datas->AddString(CS_RETURN,"");		
			datas->AddString(CS_NAME,varName);
			datas->AddInt16(CS_PROTECTION,CSMT_PUBLIC);

			// mettre a jour le nombre
			nbDatas++;
		}
		index++;
	}
	// ajouter le nombre trouvé
	datas->AddInt32(NB_DATAS_TO_ADD,nbDatas);

	// on ajoute a la liste que si on a trouve un variable qui correspond
	if(nbDatas>0)
		_findList.AddItem(datas);
	else
		delete datas;
		
	// lancer le thread de recherche
	CreateFillThread();
}

/**** traiter le resultat de la recherche de la completion ****/
void CAddOnAppHandler::ManageSearchResult()
{
	BMessage	*addMessage = NULL;
	int32		nbListDatas = 0;
	int32		nbVar = 0;

	// verifier si quelquechose correspond au filtre
	nbListDatas = _findList.CountItems();
	if(nbListDatas==0)
		return;

	int32	nbCurrent = 0;
	// compter combien on a de donnée trouvée dans chaque message
	for(int32 index=0;index<nbListDatas;index++)
	{
		if(((BMessage *)(_findList.ItemAt(index)))->FindInt32(NB_DATAS_TO_ADD,&nbCurrent)==B_OK)
			nbVar += nbCurrent;
	}

	// si on en a trouvé plus d'un faut afficher la fenetre de selection
	if(nbVar>1)
	{
		// on ouvre la fenetre
		BWindow *winList = new AddOnListInfo(_destWindow,_target);
		if(winList->AddToSubset(_target->Window())!=B_OK)
		{
			delete winList;
			return;
		}
			
		// bloquer la fenetre
		BMessenger(_target).SendMessage(LOCK_MOTHER_WIN);

		// assigner la fenetre pour le thread
		winList->Show();		

		// fixer le messenger comme etant la nouvelle fenetre
		// de resultat
		_winMessenger = BMessenger(winList);

		// commencer l'animation
		_winMessenger.SendMessage(PROGRESS_START_MSG);
				
		for(int32 i=0;i<nbListDatas;i++)
			winList->PostMessage((BMessage *)(_findList.ItemAt(i)));
		
		// terminer l'animation
		BMessage	message(PROGRESS_STOP_MSG);
	
		message.AddInt8(CS_ASK_TYPE,_askType);
		_winMessenger.SendMessage(&message);

		// debloquer la fenetre
		BMessenger(_target).SendMessage(UNLOCK_MOTHER_WIN);
	}
	else
	{
		// si le filtre ne retourne q'un seul resultat on l'ajoute sans afficher la fenetre de choix
		BMessage	datas(ADD_DATAS_TEXT);
		BString		result("");

		// construire la chaine a retourner
		addMessage = (BMessage *)(_findList.ItemAt(0));
		if(addMessage->FindString(CS_NAME,&result)==B_OK)
		{
			// la variable a ajouter
			datas.AddInt8(CS_ASK_TYPE,_askType);
			datas.AddString(SRING_TO_ADD,result.String());

			// l'envoyer pour l'inserer
			BMessenger(_target).SendMessage(&datas);
		}
	}

	// vider la liste
	nbListDatas = _findList.CountItems();
	for(int32 i=0;i<nbListDatas;i++)
	{
		addMessage = (BMessage *)_findList.RemoveItem((int32)0);
		addMessage->MakeEmpty();
		delete addMessage;
	}
}

//===============================================================
// Operation du thread de recherche
//===============================================================

/**** creer le thread ****/
void CAddOnAppHandler::CreateFillThread()
{
	thread_id	oldThread;

	// un seul thread a la fois !!
	oldThread = find_thread("DontWorry searching Thread...");
	if(oldThread!=B_NAME_NOT_FOUND)
		kill_thread(oldThread);

	// on lance le nouveau
	_fillThreadId = spawn_thread(FillThreadStub,"DontWorry searching Thread...",B_NORMAL_PRIORITY,this);
	resume_thread(_fillThreadId);
}

/**** fonction pour pouvoir acceder a la classe CAddOnAppHandler dans le thread ****/
int32 CAddOnAppHandler::FillThreadStub(void *obj)
{
	((CAddOnAppHandler *)obj)->FillThread();
	return 0;
}

/**** thread ****/
void CAddOnAppHandler::FillThread()
{
	// relire les prefs (elles peuvent avoir changées)
	_settings->Load();

	// proteger le parcour des CSLibs
	_generalLocker->Lock();

    // recuperer les donnees de la classe courante
    DumpAllClass();
    DumpClass();
    DumpMember();
    DumpMethod();       

	// on ne doit voir que les membres public de ces heritages
	_filter = CSMT_PUBLIC;
	if(_settings->_useProtection == B_CONTROL_ON)
		_filter = CSMT_PUBLIC | CSMT_PROTECTED | CSMT_PRIVATE;			

    // s'occuper des classe parentes
    DoFathers(_searchClass);

	// deproteger le parcour des CSLibs
	_generalLocker->Unlock();
	
	// on a fini, afficher les resultats
	_winMessenger.SendMessage(DISPLAY_SEARCH_RESULT_MSG);
}


/**** recherche des donnees d'une classe ****/
void CAddOnAppHandler::DoFathers(CSClass *childClass)
{

    // on boucle dans les fils de la classe
    _searchClass = childClass->GetFirstFather();

    while(_searchClass != NULL)
    {
        // on affiche la nouvelle classe

        DumpClass();
        DumpMember();
        DumpMethod();       
		
		// ajouter la liste mais inverser
		if(_askType!=ASK_COMPLETION)
		{
			BMessage datas(ADD_DATAS_LIST);
			datas.AddInt8(CS_TYPE,TYPE_UNDIFINED);
			_winMessenger.SendMessage(&datas);
		}
				
		// et on va faire pareil avec ses peres
        DoFathers(_searchClass);

        // passage au pire le suivant
        // mais on libere tout de meme la memoire prise par l'ancienne
        delete _searchClass;
        _searchClass = childClass->GetNextFather();
    }

    // on n'a plus besoin de la classe
    delete childClass;
}

/**** stop ****/
void CAddOnAppHandler::KillFillThread()
{
	// verifier que c'st pas locké !
	if(_generalLocker->IsLocked())
		_generalLocker->Unlock();

	kill_thread(_fillThreadId);
	_fillThreadId=-1;
}

//===============================================================
// obtenir des informations les classes/metodes/variables membres
//===============================================================

/**** chercher les infos d'une classe ****/
void CAddOnAppHandler::DumpClass()
{
	// completion on fait rien c'est deja fait dans
	// DumpAllClasses
	if(_askType==ASK_COMPLETION)
		return;

	BMessage	*datas = NULL;
	int32		nbDatas = 1;
	BString		name;

	// initialiser
	name.SetTo("");

	// on recupere le nom
	name = _searchClass->GetName();
	
	// creer le message
	datas = new BMessage(_typeMessage);

	// attention completion
	// dans ce cas on doit avoir le debut de la chaine
	// sinon on prend tout !
	datas->AddInt8(CS_TYPE,TYPE_CLASS);
	datas->AddString(CS_NAME,name);
	datas->AddInt8(CS_PROTECTION,CSMT_PUBLIC);
	
	// ajouter le nombre de donnees
	datas->AddInt32(NB_DATAS_TO_ADD,nbDatas);

	// ajouter a la liste le message
	_findList.AddItem(datas);
}

/**** chercher les metodes d'une classe ****/
void CAddOnAppHandler::DumpMethod()
{
	BMessage		*datas = NULL;
	int16			protection;
	int32			nbDatas = 0;
	BString			type;
	BString			name;
	BString			args;
	BString			className;
	bool			isVirtual;
		
	// creer le message
	datas = new BMessage(_typeMessage);
	
	// recuperer le nom de la classe
	className = _searchClass->GetName();
	
	// on va parcourir les méthodes
	CSVariable	*argument = NULL;
	CSMethod	*method = _searchClass->GetFirstMethod(_filter);
	while (method != NULL)
	{		
		// initialiser la chaine
		type.SetTo("");
		name.SetTo("");
		args.SetTo("");

		// verifier si c'est un const
		if(method->IsReturnTypeConst())
			type << "const ";

		// type de retour
		type << method->GetReturnTypeName() << " ";
		
		// les '*'
		unsigned int pointerLevel = method->GetReturnPointerLevel();
		for (unsigned int i=0; i<pointerLevel; i++)
			type << "*";

		// c'est peux etre une reference
		if(method->IsReturnTypeRef())
			type << "&";

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
			
			// verifier si c'est un const
			if(argument->IsConst())
				args << " const";
			
			// recuperer le type
			args << " " << argument->GetTypeName() << " ";
			
			// les '*'
			pointerLevel = argument->GetPointerLevel();
			for (unsigned int i=0; i<pointerLevel; i++)
				args << "*";
			
			// c'est peux etre une reference
			if(argument->IsRef())
				args << "&";

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
		
		// on a rien trouvé par defaut
		// attention a la completion
		if(_askType==ASK_METHOD
		|| ( _askType!=ASK_COMPLETION && (name.FindFirst("operator")==B_ERROR && name != className && name.FindFirst("~")==B_ERROR))
		|| (_askType==ASK_COMPLETION && (_settings->_keySensitive == B_CONTROL_ON && name.FindFirst(_searchString)==0)
		|| (_settings->_keySensitive == B_CONTROL_OFF && name.IFindFirst(_searchString)==0)))
		{
			// incrementer le nombre de data
			nbDatas++;
			
			// remplir avec les donnees
			datas->AddInt8(CS_TYPE,TYPE_METHODE);
			datas->AddString(CS_RETURN,type.String());
			datas->AddString(CS_NAME,name.String());
			datas->AddString(CS_PARAMETER,args.String());
			datas->AddInt16(CS_PROTECTION,protection);
			datas->AddBool(CS_VIRTUAL,isVirtual);
			}

		delete method;
		method = _searchClass->GetNextMethod();
	}

	// ajouter le nombre de donnees
	datas->AddInt32(NB_DATAS_TO_ADD,nbDatas);

	// envoyer le tout
	if(nbDatas>0)
		_findList.AddItem(datas);
	else
		delete datas;	
}

/**** chercher les variables membres d'une classe ****/
void CAddOnAppHandler::DumpMember()
{
	BMessage		*datas = NULL;
	int32			nbDatas = 0;
	int16			protection;
	BString			type;
	BString			name;
	
	// creer le message
	datas = new BMessage(_typeMessage);

	// on va parcourir les variables membres
	CSVariable *member = _searchClass->GetFirstMember(_filter);
	while (member != NULL)
	{		
		// initialiser la chaine
		type.SetTo("");
		name.SetTo("");

		// verifier si c'est un const
		if(member->IsConst())
			type << "const ";

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
				
		if(_askType!=ASK_COMPLETION
		|| (_settings->_keySensitive == B_CONTROL_ON && name.FindFirst(_searchString)==0)
		|| (_settings->_keySensitive == B_CONTROL_OFF && name.IFindFirst(_searchString)==0))
		{
			// incrementer le nombre de data
			nbDatas++;
			
			// remplir avec les donnees		
			datas->AddInt8(CS_TYPE,TYPE_VARIABLE);
			datas->AddString(CS_RETURN,type.String());		
			datas->AddString(CS_NAME,name.String());
			datas->AddInt16(CS_PROTECTION,protection);
		}

		// passer au suivant
		delete member;
		member = _searchClass->GetNextMember();
	}

	// ajouter le nombre de donnees
	datas->AddInt32(NB_DATAS_TO_ADD,nbDatas);
	
	// envoyer le tout
	if(nbDatas>0)
		_findList.AddItem(datas);
	else
		delete datas;
}

/**** chercher les infos des classes qui corresponde a la recherche ****/
void CAddOnAppHandler::DumpAllClass()
{
	// ne faire ca que pour la cmpletion
	if(_askType!=ASK_COMPLETION)
		return;

	BMessage	*datas = NULL;
	int32		nbDatas = 0;
	CSLibrary	*projectLib = NULL;
	BString		name;

	// creer le message
	datas = new BMessage(_typeMessage);

	// recuperer la lib de la classe (donc celle du projet)
	projectLib = _searchClass->GetLibrary();

	// recuperer les noms de classes qui corresponde au critere
	CSClass *currentClass = projectLib->GetFirstClass();
	while(currentClass!=NULL)
	{
		// on recupere le nom
		name = currentClass->GetName();
		
		// verifier que ca correspond au critere de recherche
		if((_settings->_keySensitive == B_CONTROL_ON && name.FindFirst(_searchString)==0)
		|| (_settings->_keySensitive == B_CONTROL_OFF && name.IFindFirst(_searchString)==0))
		{
			datas->AddInt8(CS_TYPE,TYPE_CLASS);
			datas->AddString(CS_NAME,name);
			datas->AddInt8(CS_PROTECTION,CSMT_PUBLIC);
		
			nbDatas++;
		}
		
		// passer au suivant
		delete currentClass;
		currentClass = projectLib->GetNextClass();
	}

	// faire la meme chose avec la lib systeme
	// recuperer les noms de classes qui corresponde au critere
	currentClass = _systemLibrary->GetFirstClass();
	while(currentClass!=NULL)
	{
		// on recupere le nom
		name = currentClass->GetName();
		
		// verifier que ca correspond au critere de recherche
		if((_settings->_keySensitive == B_CONTROL_ON && name.FindFirst(_searchString)==0)
		|| (_settings->_keySensitive == B_CONTROL_OFF && name.IFindFirst(_searchString)==0))
		{
			datas->AddInt8(CS_TYPE,TYPE_CLASS);
			datas->AddString(CS_NAME,name);
			datas->AddInt8(CS_PROTECTION,CSMT_PUBLIC);
		
			nbDatas++;
		}
		
		// passer au suivant
		delete currentClass;
		currentClass = _systemLibrary->GetNextClass();
	}

	
	// ajouter le nombre de donnees
	datas->AddInt32(NB_DATAS_TO_ADD,nbDatas);
	
	// envoyer le tout
	if(nbDatas>0)
		_findList.AddItem(datas);
	else
		delete datas;
}

/**** chercher si la variable proposée est membre de la classe ou des parentes ****/
bool CAddOnAppHandler::SearchMemberFromClass(CSClass *childClass,const char *var,BString &typeVar,unsigned int &pointerLevel)
{
	bool		state = false;
	CSClass		*current = NULL;
	CSVariable	*searchVar = NULL;

	state = ((searchVar = childClass->FindMember(var))!=NULL);
	if(state)
	{
			typeVar = searchVar->GetTypeName();
			pointerLevel = searchVar->GetPointerLevel();
	}
	else
	{

	    // on boucle dans les fils de la classe
    	current = childClass->GetFirstFather();

	    while(current!=NULL && !state)
    	{
        	// regarde si on trouve la variable dans les peres
			state = SearchMemberFromClass(current,var,typeVar,pointerLevel);

        	// passage au pire le suivant
     		current = childClass->GetNextFather();
    	}
    	
    	if(current!=NULL)
    		delete current;
	}
    // on n'a plus besoin de la classe
    delete childClass;
    
    return state; 
}

/**** chercher la classe a qui appartient une methode ****/
bool CAddOnAppHandler::SearchMethodFromClass(CSClass *childClass,const char *methode,BString &className)
{
	bool		state = false;
	CSClass		*current = NULL;

	CSMethod *findMethode = childClass->FindMethod(methode);
	if(findMethode!=NULL)
	{
		className = childClass->GetName();
		state = true;
		delete findMethode;
	}
	else
	{

	    // on boucle dans les fils de la classe
    	current = childClass->GetFirstFather();

	    while(current!=NULL && !state)
    	{
        	// regarde si on trouve la variable dans les peres
			state = SearchMethodFromClass(current,methode,className);

        	// passage au pire le suivant
     		current = childClass->GetNextFather();
    	}
    	
    	if(current!=NULL)
    		delete current;
	}
    // on n'a plus besoin de la classe
    delete childClass;
    
    return state; 
}

//===============================================================
// obtenir des informations sur les projets en cours
//===============================================================

/**** retrouver un projet par son nom ****/
int32 CAddOnAppHandler::ItemFromProject(const char *name)
{
	int32		nbProject;
	BString		projectName("");

	// comparer le nom du projet
	nbProject = _projectList.CountItems();
	for(int32 index=0;index<nbProject;index++)
	{
		projectName.SetTo(((CProject *)(_projectList.ItemAt(index)))->Name());
		if(projectName == name)
			return index;
	}
	
	return -1;
}

/**** trouver le projet a qui appartient un fichier + son chemin ****/
CProject *CAddOnAppHandler::FindProjectFromFilePath(const char *pathFile)
{
	int32		nbProject;
	CProject	*currentProject;

	nbProject = _projectList.CountItems();
	for(int32 index=0;index<nbProject;index++)
	{
		currentProject = (CProject *)(_projectList.ItemAt(index));
		if(currentProject->FindFile(pathFile)==B_OK)
			return currentProject;
	}

	return NULL;
}

/**** Mettre a jour la liste des fichiers d'un projet ****/
void CAddOnAppHandler::UpdateProjectFile(BMessage *message)
{
	BString		projectName;
	int32		item;

	// si on trrouve le nom du projet ca vient du CMethodBuilder
	if(message->FindString(PROJECT_FILE_NAME,&projectName)!=B_OK)
		return;

	if((item = ItemFromProject(projectName.String()))==-1)
		return;

	CProject	*current;
	
	// projet
	current = (CProject *)(_projectList.ItemAt(item));
	current->RemoveFiles(message);
}		


//===============================================================
// Parser un fichier si on l'ajoute ou on le ferme
//===============================================================

/**** Mettre a jour les infos si on a un nouveau fichier header ****/
void CAddOnAppHandler::ParseNewProjectFile(BMessage *message)
{
	BString			projectName;

	// si on trrouve le nom du projet ca vient du CMethodBuilder
	if(message->FindString(PROJECT_FILE_NAME,&projectName)!=B_OK)
		return;

	int32	item = ItemFromProject(projectName.String());
	if(item == -1)
		return;

	BString		newFile;
	BString		headerPathString(newFile);
	BMessage	*findedFile = NULL;
	CProject	*currentProject;
	int32		i;
		
	currentProject = (CProject *)(_projectList.ItemAt(item));
	findedFile = currentProject->FindNewFile(message);
	
	// proteger le parsing
	_generalLocker->Lock();

	// attention on doit avoir trouvé un ou plusieurs fichiers
	i=0;
	while(findedFile->FindString(FILE_PATHNAME,i,&newFile)==B_OK)
	{	
		// oui donc on verifie si c'est un header (oui, on le parse)
		headerPathString.SetTo(newFile);
		if(headerPathString.CountChars()>2)
		{
			headerPathString.Remove(0,headerPathString.CountChars()-2);
			if(headerPathString == ".h")
				currentProject->Library()->Parse(newFile.String(),false,0,true);
		
		}
		// au prochain
		i++;
	}
	_generalLocker->Unlock();

	// on detruit le message
	delete findedFile;
}

/**** Mettre a jour les infos si un fichier header a change (apres sauvegarde) ****/
void CAddOnAppHandler::UpdateFileOfProject(BMessage *message)
{
	BString		pathFile;

	// on recupere le chemin du fichier
	// c'est peut etre un nouveau fichier, il est traité dans l'ajout
	if(message->FindString(FILE_NAME,&pathFile)!=B_OK)
		return;

	// oui donc on verifie si c'est un header (oui, on va le parser)
	BString		headerPathString(pathFile);

	headerPathString.Remove(0,headerPathString.CountChars()-2);
	if (headerPathString != ".h")
		return;

	// est-ce a un des projets
	CProject	*currentProject = FindProjectFromFilePath(pathFile.String());
	
	if(currentProject==NULL)
		return ;

	// faire clignoter l'icone
	currentProject->BlinkIcon();

	// on parse, penser a proteger le parsing
	_generalLocker->Lock();
	currentProject->Library()->Parse(pathFile.String(),false,0,true);	
	_generalLocker->Unlock();

	// retablir l'icone
	currentProject->BlinkIcon();
}

/**** Mettre a jour la CSLib par rapport a une modification d'un header (pas de sauvegarde) ****/
void CAddOnAppHandler::RefreshHeader(BMessage *message)
{
	BString		headerText;
	BString		pathFile;

	// on recupere le chemin du fichier
	// c'est peut etre un nouveau fichier, il est traité dans l'ajout
	if(message->FindString(FILE_NAME,&pathFile)!=B_OK)
		return;

	// oui donc on verifie si c'est un header (oui, on va le parser)
	BString		headerPathString(pathFile);

	// le header doit avoir plus de 2 lettre
	if(headerPathString.CountChars()<3)
		return;
	
	// et etre de la forme "*.h"
	headerPathString.Remove(0,headerPathString.CountChars()-2);
	if (headerPathString != ".h")
		return;

	// trouver le text
	if(message->FindString(HEADER_TEXT,&headerText)!=B_OK)
		return;

	// est-ce a un des projets
	CProject	*currentProject = FindProjectFromFilePath(pathFile.String());
	
	if(currentProject==NULL)
		return;

	// faire clignoter l'icone
	currentProject->BlinkIcon();
	
	// on parse, penser a proteger le parsing
	_generalLocker->Lock();
	currentProject->Library()->Parse(headerText.String(),(unsigned int)(headerText.CountChars()));	
	_generalLocker->Unlock();

	// retablir l'icone
	currentProject->BlinkIcon();
}


//===============================================================
// Gerer la mise a jour des prefs
//===============================================================

void CAddOnAppHandler::UpdatePrefsSettings()
{
	_settings->Load();	
	if(_settings->PreferencesLoaded()!=B_OK)
		return;

	_useDontWorry = (_settings->_useDontWorry == B_CONTROL_ON);
}

//===============================================================
// Gestion de BeHappy
//===============================================================

/**** demande de positionnement de BeHappy ****/
void CAddOnAppHandler::CallBeHappy(BMessage *message)
{
	BString		className;
	BString		methodeName;
	status_t	state = B_ERROR;
		
	state = message->FindString(CS_TYPE,&className);
	state &= message->FindString(CS_NAME,&methodeName);
	if(state!=B_OK)
		return;

	switch(_beHappyCommand->InstallAddOn())
	{
	case HappyCommander::HC_REFUSED:
	case HappyCommander::HC_NO_MORE_ASKING:
	case HappyCommander::HC_NO_ADDON:
	case HappyCommander::HC_INSTALL_ERROR:
		return;
	}

	_beHappyCommand->Show("By Kit",className.String(),methodeName.String());
}

/**** demande d'aide sur une methode (dans le code) ****/
void CAddOnAppHandler::AskHelpForMethod(BMessage *message)
{
	BString			var;
	BString			classe;
	BString			selection;
	status_t		dataFind;
	CSClass			*currentClass = NULL;

	// initialiser
	_searchClass = NULL;
	_filter = CSMT_PUBLIC | CSMT_PROTECTED | CSMT_PRIVATE;		
	_askType = -1;

	// on doit toujours trouver ces données
	dataFind = message->FindRect(SHOW_WINDOW_RECT,&_destWindow);
	dataFind &= message->FindPointer(MOTHER_VIEW,(void **)(&_target));
	dataFind &= message->FindString(VARIABLE_NAME,&var);
	dataFind &= message->FindString(CLASSE_NAME,&classe);
	dataFind &= message->FindString(SELECTION,&selection);
	dataFind &= message->FindInt8(CS_ASK_TYPE,&_askType);

	// on doit tout avoir trouvé
	if(dataFind!=B_OK)
		return;

	// verifier que c'est bien une demande d'aide sur une methode
	if(_askType!=ASK_HELP_BEHAPPY)
		return;

	// on doit alors forcement trouver la classe dans laquelle on est
	currentClass = _conteneur->GetClass(classe.String());
	if(currentClass == NULL)
		return ;
		
	// si var est vide c'est une methode de la classe courente ou parente
	// a cette derniere
	if(var.Length()==0)
	{		
		// on se base alors sur la classe courante
		_searchClass = currentClass;
	}
	else
	{
		// si var n'est pas vide c'est soit une classe soit une variable membre
		// et la methode appartient a cette derniere
		_searchClass = _conteneur->GetClass(var.String());
		if(_searchClass == NULL)
		{ 
			// sinon c'est une variable membre
			BString			typeVar;
			unsigned int	pointerLevel = 0;
		
			// si non, on doit chercher si c'est une variable membre
			// et recuperer le niveau  (pointer/objet) de la variable
			if(SearchMemberFromClass(currentClass,var.String(),typeVar,pointerLevel))
			{		
				// et verifier si la demande correspond
				// puis rechercher la classe du type
				CSClass *varClass = _conteneur->GetClass(typeVar.String());
				if(varClass!=NULL)
					_searchClass = varClass;
			}
		}
	}

	if(_searchClass!=NULL)
	{
		BString		className;

		// chercher a qui appartient la methode
		if(!SearchMethodFromClass(_searchClass,selection.String(),className))
			return;

		// on appel alors BeHappy si on a trouvé la classe
		if(className.Length()>0)
		{
			// vider le message
			message->MakeEmpty();
		
			// ajouter les parenthese a la methode
			selection << "()";
		
			message->AddString(CS_TYPE,className);
			message->AddString(CS_NAME,selection);
	
			// appel BeHappy
			CallBeHappy(message);
		}
	}	
}

/**** gerer les types MIME ****/
void CAddOnAppHandler::VerifyMIMEType()
{
	BMimeType	CSFileType("application/x-vnd.CKJ-Daixiwen-CSFile");

	// verifier la validite du type MIME
	if(CSFileType.IsValid())
	{
		// est-il installé, si non il faut l'installer
		if(!CSFileType.IsInstalled())
		{
			// renseigner le MIME Type
			BNode		fileNode(_settings->_dontWorryPath.String());
			BNodeInfo	nodeInfo(&fileNode);

			if(nodeInfo.InitCheck()!=B_OK)
				return;

			// recuperer les icones de l'application
			BBitmap largeIcon(BRect(0,0,31,31), B_CMAP8);
			BBitmap miniIcon(BRect(0,0,15,15), B_CMAP8);
	
			nodeInfo.GetIcon(&largeIcon, B_LARGE_ICON);
			nodeInfo.GetIcon(&miniIcon, B_MINI_ICON);
			
			// definir les icones du type
			// l'application prefere
			// l'extension de type de fichier supporté
			BMessage	extension;
			
			extension.AddString("extensions","cs");
			CSFileType.SetIcon(&largeIcon,B_LARGE_ICON);
			CSFileType.SetIcon(&miniIcon,B_MINI_ICON);			
			CSFileType.SetPreferredApp("application/x-vnd.CKJ-Daixiwen-CSViewer");
			CSFileType.SetFileExtensions(&extension);
			
			// l'installer
			CSFileType.Install();
		}
	}
}