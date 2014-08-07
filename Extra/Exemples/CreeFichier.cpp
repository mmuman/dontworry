/*
 * Test primitif des classes CS
 *
 *
 */

#include <File.h>
#include <Entry.h>
#include <Directory.h>
#include <Roster.h>
#include <Path.h>
#include <String.h>

#include <stdio.h>

#include "CreeFichier.h"
#include "CSLibrary.h"

int main()
{
	(new CreeFichierApp)->Run();
}

CreeFichierApp::CreeFichierApp()
	: BApplication("application/x-vnd.STertois-CS_CreeFichier")
{
}

void CreeFichierApp::ReadyToRun()
{
	printf("Début du test.\n");
	
	// détermination du répertoire de l'application
	app_info l_MyAppInfo;
	GetAppInfo(&l_MyAppInfo);
	BEntry l_Application(&l_MyAppInfo.ref);
	BDirectory l_AppDir;
	l_Application.GetParent(&l_AppDir);
		
	printf("Création du fichier...");
	
	BFile *l_MyFile = new BFile(&l_AppDir,"CSLibrary.cs",B_READ_WRITE|B_CREATE_FILE|B_ERASE_FILE);
	if (l_MyFile->InitCheck() == B_OK)
		printf("OK\n");
	else
	{
		printf("Erreur!\n");
		return;
	}
	
	printf("Création de la librarie...");
	CSLibrary *l_MyLibrary = new CSLibrary(l_MyFile);
	if (l_MyLibrary->InitCheck() == B_OK)
		printf("OK\n");
	else
	{
		printf("Erreur!\n");
		return;
	}
	
	printf("Début du parsing.\n");
	
	// On passe au repertoire supérieur
	BEntry l_DirEntry;
	l_AppDir.GetEntry(&l_DirEntry);
	l_DirEntry.GetParent(&l_AppDir);

	// Recherche des fichiers header
	BEntry l_HeaderFile;
	l_AppDir.Rewind();
	while (l_AppDir.GetNextEntry(&l_HeaderFile) == B_OK)
	{
		// extraction du nom
		BPath l_HeaderPath;
		l_HeaderFile.GetPath(&l_HeaderPath);
		BString l_HeaderPathString(l_HeaderPath.Path());
		
		// s'agit-il d'un fichier header?
		l_HeaderPathString.Remove(0,l_HeaderPathString.CountChars()-2);
		if (l_HeaderPathString == ".h")
		{
			// oui
			printf("Parsing de %s...",l_HeaderPath.Leaf());
			
			// on demande un parsing dessus
			if (l_MyLibrary->Parse(l_HeaderPath.Path()) == B_OK)
				printf("OK!\n");
			else
				printf("Erreur!\n");
		}
	}

	printf("Encore une fois...\n");
	l_AppDir.Rewind();
	while (l_AppDir.GetNextEntry(&l_HeaderFile) == B_OK)
	{
		// extraction du nom
		BPath l_HeaderPath;
		l_HeaderFile.GetPath(&l_HeaderPath);
		BString l_HeaderPathString(l_HeaderPath.Path());
		
		// s'agit-il d'un fichier header?
		l_HeaderPathString.Remove(0,l_HeaderPathString.CountChars()-2);
		if (l_HeaderPathString == ".h")
		{
			// oui
			printf("Parsing de %s...",l_HeaderPath.Leaf());
			
			// on demande un parsing dessus
			if (l_MyLibrary->Parse(l_HeaderPath.Path()) == B_OK)
				printf("OK!\n");
			else
				printf("Erreur!\n");
		}
	}

	// destruction des objets -> enregistrement du fichier
	printf("Enregistrement...");
	delete l_MyLibrary;
	delete l_MyFile;
	printf("Fini!\n");
	
	Quit();
}