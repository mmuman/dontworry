/*
 * Parseur c++ pour keyspy
 *
 * CSPTest: application pour tester la librairie CSP
 *
 */

#include "CSPTestApp.h"
#include "CSPSourceFile.h"

#include <File.h>

#include <stdio.h>

int main()
{
	CSPTestApp myApp;
	myApp.Run();
	return 0;
}

CSPTestApp::CSPTestApp()
	: BApplication("application/x-vnd.stertois-csptest")
{
}

CSPTestApp::~CSPTestApp()
{
}

void CSPTestApp::ReadyToRun()
{
	BFile l_File("/boot/home/Devel/ClassStructure/CSPSourceFile.cpp",B_READ_ONLY);
	
	off_t l_Size;
	l_File.GetSize(&l_Size);
	
	char *l_Buffer = new char[l_Size];
	l_File.Read(l_Buffer,l_Size);
	
	CSPSourceFile l_Source;
//	l_Source.Parse(l_Buffer,l_Buffer+l_Size);
	l_Source.Parse(l_Buffer,l_Buffer+2481);
	
	printf("**** quelques tests...\n");
	printf("Classe en cours:'%s'\n",l_Source.CurrentClass());
	printf("Fonction en cours: %s\n",l_Source.CurrentFunction());
	
	SearchVariable(l_Source,"toto");
	SearchVariable(l_Source,"titi");
	SearchVariable(l_Source,"i");
	SearchVariable(l_Source,"l_NBlocks");
	SearchVariable(l_Source,"l_Name1");
	SearchVariable(l_Source,"beginning");
	SearchVariable(l_Source,"pointer");
	
	// boucle pour afficher les variables
	const char *l_String = l_Source.GetFirstVariableName();
	while (l_String != NULL)
	{
		printf("variable: %s\n",l_String);
		l_String = l_Source.GetNextVariableName();
	}
	
	Quit();
}

void CSPTestApp::SearchVariable(CSPSourceFile &source,const char *name)
{
	printf("recherche de la variable '%s'...",name);
	
	const char *l_Type;
	unsigned int l_PointerLevel;
	
	l_Type = source.TypeOfVariable(name,&l_PointerLevel);
	if (l_Type == NULL)
		printf("pas trouvé\n");
	else
		printf("type: '%s', niveau de pointeur: %d\n",l_Type,l_PointerLevel);
}