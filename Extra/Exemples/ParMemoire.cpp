/*
 * Test primitif des classes CS
 *
 *
 */

//#define USE_BSTRING_H
// si cette constante est définie, on va regarder la classe BString, au lieu des classes CS

#include <DataIO.h>
#include <Entry.h>
#include <Directory.h>
#include <Roster.h>
#include <Path.h>
#include <String.h>

#include <stdio.h>

#include "ParMemoire.h"
#include "CSLibrary.h"
#include "CSClass.h"
#include "CSMethod.h"
#include "CSVariable.h"

int main()
{
	(new ParMemoireApp)->Run();
}

ParMemoireApp::ParMemoireApp()
	: BApplication("application/x-vnd.STertois-CS_ParMemoire")
{
}

void ParMemoireApp::ReadyToRun()
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

#ifndef USE_BSTRING_H
	// on va au répertoire père
	BEntry l_DirEntry;
	l_AppDir.GetEntry(&l_DirEntry);
	l_DirEntry.GetParent(&l_AppDir);
	
	// et encore au père
	l_AppDir.GetEntry(&l_DirEntry);
	l_DirEntry.GetParent(&l_AppDir);
	l_AppDir.SetTo(&l_AppDir,"ClassStructure");

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
			if (l_MyLibrary->Parse(l_HeaderPath.Path(),false,5,false) == B_OK)
			{
				printf("OK! on refait...\n");
				if (l_MyLibrary->Parse(l_HeaderPath.Path(),false,5,true) == B_OK)
					printf("OK!\n");
				else
					printf("Erreur!\n");
			}
			else
				printf("Erreur!\n");
		}
	}

#else // USE_BSTRING_H
	if (l_MyLibrary->Parse("/boot/develop/headers/be/support/String.h",false,0,false) == B_OK)
		printf("OK!\n");
	else
		printf("Erreur!\n");
		
#endif //USE_BSTRING_H

	// restitution
	printf("Restitution.\n");
	
	DumpClass(l_MyLibrary,"BString");
	DumpClass(l_MyLibrary,"CSPNamesList");

#ifndef USE_BSTRING_H
	printf("Recherche de méthodes et variables membre dans CSPNamesList\n");
	CSClass *l_Class = l_MyLibrary->GetClass("CSPNamesList");
	if (l_Class == NULL)
	{
		printf("Erreur... classe non trouvée!\n");
	}
	else
	{
		printf("Recherche de la méthode Init:");
		CSMethod *l_Method = l_Class->FindMethod("Init");
		if (l_Method == NULL)
			printf("Non trouvée...\n");
		else
		{
			DumpMethod(l_Method);
			delete l_Method;
		}
		printf("Recherche de la méthode Gaga:");
		l_Method = l_Class->FindMethod("Gaga");
		if (l_Method == NULL)
			printf("Non trouvée...\n");
		else
		{
			DumpMethod(l_Method);
			delete l_Method;
		}
		
		printf("Recherche de la variable m_MallocSize:");
		CSVariable *l_Variable = l_Class->FindMember("m_MallocSize");
		if (l_Variable == NULL)
			printf("Non trouvée...\n");
		else
		{
			DumpMemberVariable(l_Variable);
			delete l_Variable;
		}
		printf("Recherche de la variable m_Gaga:");
		l_Variable = l_Class->FindMember("m_Gaga");
		if (l_Variable == NULL)
			printf("Non trouvée...\n");
		else
		{
			DumpMemberVariable(l_Variable);
			delete l_Variable;
		}
		delete l_Class;
	}
#endif //USE_BSTRING_H
	printf("Dump de la première classe\n");
	DumpClass(l_MyLibrary,l_MyLibrary->GetFirstClass());
	printf("Dump de la seconde classe\n");
	DumpClass(l_MyLibrary,l_MyLibrary->GetNextClass());
	
	unsigned int i=2;
	while(l_MyLibrary->GetNextClass() != NULL)
		i++;
	
	printf("Il y a en tout %d classes\n",i);
	
	// destruction des objets
	printf("Desctruction...");
	delete l_MyLibrary;
	delete l_MyFile;
	printf("Fini!\n");
	
	Quit();
}

void ParMemoireApp::DumpClass(CSLibrary *lib, const char *className)
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

void ParMemoireApp::DumpClass(CSLibrary *lib, CSClass *theClass)
{
	// première ligne avec le nom de la classe
	const char *l_HeaderPath = theClass->GetHeaderFile();
	if (l_HeaderPath == NULL)
		printf("\nclass %s\n{\n",theClass->GetName());
	else
		printf("\nclass %s\t(%s)\n{\n",theClass->GetName(),l_HeaderPath);	
	
	// on va parcourir les méthodes
	CSMethod *l_Method = theClass->GetFirstMethod(CSMT_PUBLIC|CSMT_PROTECTED);
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

void ParMemoireApp::DumpMemberVariable(CSVariable *variable)
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

void ParMemoireApp::DumpMethod(CSMethod *method)
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

	// la ref
	if (method->IsReturnTypeRef())
		printf("&");
	
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
		
		// la ref
		if (l_Argument->IsRef())
			printf("&");
				
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

void ParMemoireApp::WriteZone(unsigned int zone)
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