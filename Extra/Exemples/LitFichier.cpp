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

#include "LitFichier.h"

#include "CSLibrary.h"
#include "CSClass.h"
#include "CSMethod.h"
#include "CSVariable.h"

int main()
{
	(new LitFichierApp)->Run();
}

LitFichierApp::LitFichierApp()
	: BApplication("application/x-vnd.STertois-CS_LitFichier")
{
}

void LitFichierApp::ReadyToRun()
{
	printf("Début du test.\n");
	
	// détermination du répertoire de l'application
	app_info l_MyAppInfo;
	GetAppInfo(&l_MyAppInfo);
	BEntry l_Application(&l_MyAppInfo.ref);
	BDirectory l_AppDir;
	l_Application.GetParent(&l_AppDir);
	
	
	printf("Ouverture du fichier...");
	
	BFile *l_MyFile = new BFile(&l_AppDir,"CSLibrary.cs",B_READ_ONLY);
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
	
	DumpClass(l_MyLibrary,"BApplication");
	DumpClass(l_MyLibrary,"CSPNamesList");
	
	// destruction des objets -> fermeture du fichier
	printf("Fermeture...");
	delete l_MyLibrary;
	delete l_MyFile;
	printf("Fini!\n");
	
	Quit();
}

void LitFichierApp::DumpClass(CSLibrary *lib, const char *className)
{
	// on cherche si une description de la classe existe dans la blibliothèque
	CSClass *l_Class = lib->GetClass(className);
	
	if (l_Class == NULL)
		printf("\n\nClasse %s non trouvée.\n",className);
	else
	{
		DumpClass(lib,l_Class);
		delete l_Class;
	}
}

void LitFichierApp::DumpClass(CSLibrary *lib, CSClass *theClass)
{
	// première ligne avec le nom de la classe
	printf("\nclass %s\n{\n",theClass->GetName());
	
	// on va parcourir les méthodes
	CSMethod *l_Method = theClass->GetFirstMethod();
	while (l_Method != NULL)
	{
		// type de retour
		printf("\t%s%s%s ",l_Method->IsVirtual()?"virtual ":"",
			l_Method->IsReturnTypeConst()?"const ":"",
			l_Method->GetReturnTypeName());
		
		// les '*'
		unsigned int l_PointerLevel = l_Method->GetReturnPointerLevel();
		for (unsigned int i=0; i<l_PointerLevel; i++)
			printf("*");
		
		// le nom de la méthode
		printf("%s(",l_Method->GetName());
		
		// les arguments
		bool l_FirstArg = true;
		CSVariable *l_Argument = l_Method->GetFirstArgument();
		while (l_Argument != NULL)
		{
			// le type de l'argument
			BString l_TypeName;
			if (l_Argument->IsConst())
				l_TypeName << "const " << l_Argument->GetTypeName();
			else
				l_TypeName << l_Argument->GetTypeName();
			printf(l_FirstArg?" %s ":", %s ",l_TypeName.String());
			
			// les '*'
			l_PointerLevel = l_Argument->GetPointerLevel();
			for (unsigned int i=0; i<l_PointerLevel; i++)
				printf("*");
			
			// le nom de l'argument, s'il est là
			const char *l_Text = l_Argument->GetName();
			if (l_Text != NULL)
				printf(l_Text);
			
			// la valeur par défault, si elle est là
			l_Text = l_Argument->GetDefaultValue();
			if (l_Text != NULL)
				printf(" = %s",l_Text);
		
			delete l_Argument;
			l_Argument = l_Method->GetNextArgument();
			l_FirstArg = false;
		}
		
		// fin de la méthode
		printf(");\n");
		
		delete l_Method;
		l_Method = theClass->GetNextMethod();
	} 
	
	// on va parcourir les variables membre
	printf("\n");
	CSVariable *l_Variable = theClass->GetFirstMember();
	while (l_Variable != NULL)
	{
		// type de variable
		printf("\t%s%s ",l_Variable->IsConst()?"const ":"",
			l_Variable->GetTypeName());
		
		// les '*'
		unsigned int l_PointerLevel = l_Variable->GetPointerLevel();
		for (unsigned int i=0; i<l_PointerLevel; i++)
			printf("*");
		
		// le nom de la variable
		printf("%s;\n",l_Variable->GetName());
		
		// on passe à la suivante
		delete l_Variable;
		l_Variable = theClass->GetNextMember();
	}
	
	// fin de la classe
	printf("}\n\n");
	
	// recherche de classes pères
	CSClass *l_Father = theClass->GetFirstFather();
	while (l_Father != NULL)
	{
		printf("%s hérite de %s:\n",theClass->GetName(),l_Father->GetName());
		DumpClass(lib,l_Father);
		
		delete l_Father;
		l_Father = theClass->GetNextFather();
	}
}
