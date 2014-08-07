/*
 * Parseur c++ pour keyspy
 *
 * CSPVariable: objet qui représente une variable
 *
 */

#ifndef CSPVARIABLE_H
#define CSPVARIABLE_H

#include <String.h>

class CSPVariable
{
public:
	CSPVariable();
	~CSPVariable();
	
private:
	BString m_TypeName;
	BString m_Name;
	unsigned int m_PointerLevel;	// niveau de pointeur (nombre de * dans le type)
	unsigned int m_Pointer;		// pointeur vers la définition de la variable (relatif)
	
friend class CSPSourceFile;
};

#endif //CSPVARIABLE_H
