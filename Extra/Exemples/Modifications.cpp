/*
 * Test primitif des classes CS
 *
 *
 */

#include <DataIO.h>
#include <Entry.h>
#include <Directory.h>
#include <Roster.h>
#include <Path.h>
#include <String.h>

#include <stdio.h>

#include "Modifications.h"
#include "CSLibrary.h"
#include "CSClass.h"
#include "CSMethod.h"
#include "CSVariable.h"

int main()
{
	(new ModificationsApp)->Run();
}


ModificationsApp::ModificationsApp()
	: BApplication("application/x-vnd.STertois-CS_Modifications")
{
}

void ModificationsApp::ReadyToRun()
{
	printf("Début du test.\n");
	
	// détermination du répertoire de l'application
	app_info l_MyAppInfo;
	GetAppInfo(&l_MyAppInfo);
	BEntry l_Application(&l_MyAppInfo.ref);
	BDirectory l_AppDir;
	l_Application.GetParent(&l_AppDir);
	
	
	printf("Création du bloc mémoire...");
	
	BMallocIO *l_MyFile = new BMallocIO;
	printf("OK\n");
	
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

	BPath l_MyPath(&l_AppDir,"Modifications.h");
	status_t l_Error;
	
	while(true)
	{
		l_Error = l_MyLibrary->Parse(l_MyPath.Path(),false,0,true);
		switch(l_Error)
		{
			case B_OK:
				printf("OK!\n");
				break;
			
			case CSP_NOT_MODIFIED:
				printf("Fichier non modifié\n");
				break;
			
			default:
				printf("Erreur!\n");
				break;
		}
			
		// restitution
		printf("Restitution.\n");
		
		DumpClass(l_MyLibrary,"ModificationsApp");
		
		printf("Appuyez sur une touche pour recommencer\n");
		getchar();
	}
		
	// destruction des objets
	printf("Desctruction...");
	delete l_MyLibrary;
	delete l_MyFile;
	printf("Fini!\n");
	
	Quit();
}

void ModificationsApp::DumpClass(CSLibrary *lib, const char *className)
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

void ModificationsApp::DumpClass(CSLibrary *lib, CSClass *theClass)
{
	// première ligne avec le nom de la classe
	const char *l_HeaderPath = theClass->GetHeaderFile();
	if (l_HeaderPath == NULL)
		printf("\nclass %s\n{\n",theClass->GetName());
	else
		printf("\nclass %s\t(%s)\n{\n",theClass->GetName(),l_HeaderPath);	
	
	// on va parcourir les méthodes
	CSMethod *l_Method = theClass->GetFirstMethod(CSMT_PUBLIC|CSMT_PROTECTED|CSMT_PRIVATE);
	while (l_Method != NULL)
	{
		DumpMethod(l_Method);
		
		delete l_Method;
		l_Method = theClass->GetNextMethod();
	} 
	
	// on va parcourir les variables membre
	printf("\n");
	CSVariable *l_Variable = theClass->GetFirstMember(CSMT_PROTECTED);
	while (l_Variable != NULL)
	{
		DumpMemberVariable(l_Variable);
		
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

void ModificationsApp::DumpMemberVariable(CSVariable *variable)
{
	// zone
	printf("\t");
	WriteZone(variable->GetZone());
	
	// type de variable
	printf(" %s ",variable->GetTypeName());
	
	// les '*'
	unsigned int l_PointerLevel = variable->GetPointerLevel();
	for (unsigned int i=0; i<l_PointerLevel; i++)
		printf("*");
	
	// le nom de la variable
	printf("%s;\n",variable->GetName());
}

void ModificationsApp::DumpMethod(CSMethod *method)
{
	// zone
	printf("\t");
	WriteZone(method->GetZone());
	
	// type de retour
	printf(" %s ",method->GetReturnTypeName());
	
	// les '*'
	unsigned int l_PointerLevel = method->GetReturnPointerLevel();
	for (unsigned int i=0; i<l_PointerLevel; i++)
		printf("*");
	
	// le nom de la méthode
	printf("%s(",method->GetName());
	
	// les arguments
	bool l_FirstArg = true;
	CSVariable *l_Argument = method->GetFirstArgument();
	while (l_Argument != NULL)
	{
		// le type de l'argument
		printf(l_FirstArg?" %s ":", %s ",l_Argument->GetTypeName());
		
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
		l_Argument = method->GetNextArgument();
		l_FirstArg = false;
	}
	
	// fin de la méthode
	printf(");\n");
}

void ModificationsApp::WriteZone(unsigned int zone)
{
	switch(zone)
	{
		case CSMT_PUBLIC:
			printf("public");
			break;
		
		case CSMT_PROTECTED:
			printf("protected");
			break;
			
		case CSMT_PRIVATE:
			printf("private");
			break;
		
		default:
			printf("unknown");
			break;
	}
}