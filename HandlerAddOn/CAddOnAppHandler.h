/*******************/
/* AddOnAppHandler */
/*******************/

#ifndef _ADDONAPPHANDLER_H
#define _ADDONAPPHANDLER_H

#include <Handler.h>
#include <List.h>
#include <String.h>
#include <Messenger.h>
#include <Rect.h>

class CSContainer;
class CSLibrary;
class BFile;
class BWindow;
class CSClass;
class CPreferences;
class CProject;
class CSVariable;
class HappyCommander;
class BLocker;

#define ADDON_HANDLER_NAME		"DontWorryHandler"

class CAddOnAppHandler : public BHandler
{
public:
	CAddOnAppHandler();
	virtual ~CAddOnAppHandler();

	virtual void	MessageReceived(BMessage *message);

	// trouver un projet dans la liste par son nom
	int32 		ItemFromProject(const char *name);
	CProject	*FindProjectFromFilePath(const char *pathFile);

private:
	CPreferences	*_settings;
	BList			_projectList;
	CSContainer		*_conteneur;
	CSLibrary		*_systemLibrary;
	BWindow			*_splashWindow;
	BLocker			*_generalLocker;
	// systeme
	BFile			*_systemFile;
	BList			_systemPath;
	// utiliser l'addon
	bool			_useDontWorry;
	HappyCommander	*_beHappyCommand;
	bool			_displaySplash;
	
// addon install
	void		InstallAddOn(BMessage *message);
	void		InstallIconAddOn(BString &name,CProject *currentProject);
	BWindow 	*SearchWindowOfBeIDE(BString &name,bool entireName);
// project and header management
	void		OpenProject(BMessage *message);
	CProject	*CreateProject(BString &ProjectName,BString &ProjectPath,BMessage *message);
	void		ParseProject(CProject *project);
	void		ParseNewProjectFile(BMessage *message);
	void		UpdateProjectFile(BMessage *message);
	void		UpdateFileOfProject(BMessage *message);
	void		RefreshHeader(BMessage *message);
// searching in CSLib
	void		DumpClass();
	void		DumpMethod();
	void		DumpMember();
	void		DumpAllClass();
	bool 		SearchMemberFromClass(CSClass *,const char *var,BString &,unsigned int &);
	bool		SearchMethodFromClass(CSClass *childClass,const char *methode,BString &className);
	void		LaunchSearch(BMessage *message);
	void		InitSearch(BMessage *message);
	void		InitCompletion(BMessage *message);
	void		ManageSearchResult();
// systeme CSLib
	void		SystemFiles();
	void		RefreshSystemLib();
	void		UpdatePrefsSettings();
// BeHappy
	void		CallBeHappy(BMessage *message);
	void		AskHelpForMethod(BMessage *message);
// MIME type
	void		VerifyMIMEType();
	
	// completion
	BList			_findList;
	BRect			_destWindow;
	BView			*_target;
	
	// function & variables relative au thread ou a la recherche
	CSClass			*_searchClass;
	uint			_filter;
	int8			_askType;
	BString			_searchString;
	BMessenger		_winMessenger;
	thread_id		_fillThreadId;
	int32			_typeMessage;
	
	void			CreateFillThread();
	static int32	FillThreadStub(void *obj);
	void			FillThread();
	void			KillFillThread();
	void			DoFathers(CSClass *);
};

#endif