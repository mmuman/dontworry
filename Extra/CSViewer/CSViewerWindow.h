#ifndef MYWINDOW_H
#define MYWINDOW_H
/*******************/
/* classe CSViewerWindow */
/*******************/

#include <Window.h>

class BView;
class BMenuBar;
class BMenu;
class CSClass;
class CSLibrary;
class BFile;
class BFilePanel;
class BStringView;

class CSViewerWindow : public BWindow
{
public:
	CSViewerWindow(BRect);
	virtual ~CSViewerWindow();
	
	virtual void MessageReceived(BMessage *);
	virtual bool QuitRequested();

protected:
	BFilePanel	*_filePanel;
	BView		*_supportView;
	BMenuBar	*_menuBar;
	BMenu		*_classMenu;
	CSClass		*_searchClass;
	CSLibrary	*_CSLib;
	BWindow		*_targetWindow;
	BFile		*_CSFile;
	BStringView	*_infos;
	uint		_filter;
	
	void	BuildMenu();
	void	OpenAndShowWindow(BMessage *msg);
	void	OpenFile(BMessage *msg);
	void	SearchDatas();
	void	DumpClass();
	void	DumpMethod();
	void	DumpMember();
	void	ResetAll();
};

#endif