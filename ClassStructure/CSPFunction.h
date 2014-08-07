/*
 * Parseur c++ pour keyspy
 *
 * CSPFunction: objet qui représente une fonction dans un source
 *
 */

#ifndef CSPFUNCTION_H
#define CSPFUNCTION_H

#include <String.h>
#include <List.h>

class CSPFunction
{
public:
	CSPFunction();
	~CSPFunction();
	
	void Empty();

private:
	BString m_ClassName;
	BString m_Name;
	BList m_Blocks;
	BList m_Variables;			// liste des paramètres de la fonction
	unsigned int m_PointerToBegin;
	bool m_Hidden;

friend class CSPSourceFile;
friend class CSPBlock;
};

#endif //CSPFUNCTION_H