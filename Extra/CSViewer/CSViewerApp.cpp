/***********************************************/ 
/* test d'utilisation du passage de parametres */ 
/***********************************************/
#include "CSViewerApp.h"
#include "CSViewerWindow.h"

#include <Mime.h>
#include <File.h>
#include <Bitmap.h>
#include <Message.h>
#include <Node.h>
#include <NodeInfo.h>

#include <iostream.h>

int main(int argc, char **argv) 
{ 
	CSViewerApp	testApp;
	
	testApp.Run();

	return 0; 
}

CSViewerApp::CSViewerApp()
: BApplication("application/x-vnd.CKJ-Daixiwen-CSViewer")
{
	// initialiser le type MIME
	InitMIMEType();
	
	// ouvrir la fenetre de l'appli
	_mainWindow = new CSViewerWindow(BRect(50,50,250,150));
	_mainWindow->Show();
}

CSViewerApp::~CSViewerApp()
{
}

// ouvrir l'appli avec un fichier
void CSViewerApp::RefsReceived(BMessage *file)
{
	entry_ref	refs;

	if(file->FindRef("refs",&refs)==B_OK)
	{
		BMessage	openFile('DATA');
			
		openFile.AddRef("refs",&refs);
		_mainWindow->PostMessage(&openFile);
	}
}

/**** gerer les types MIME ****/
void CSViewerApp::InitMIMEType()
{
	BMimeType	CSFileType("application/x-vnd.CKJ-Daixiwen-CSFile");
	
	// verifier la validite du type MIME
	if(CSFileType.IsValid())
	{
		// est-il installé, si non il faut l'installer
		if(!CSFileType.IsInstalled())
		{
			// renseigner le MIME Type
			BNode		fileNode("/boot/home/config/settings/AddOnBeIDE/Images/CS-MIME.tga");
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
			if(CSFileType.Install()!=B_OK)
				cout << "MIME type non installé\n";			
		}
	}
}