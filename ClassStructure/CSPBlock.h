/*
 * Parseur c++ pour keyspy
 *
 * CSPBlock: objet qui représente un bloc (de '{' à '}')
 *
 */

#ifndef CSPBLOCK_H
#define CSPBLOCK_H

#include <List.h>
class CSPFunction;

class CSPBlock
{
public:
	CSPBlock();
	~CSPBlock();
	
	void Empty();
	
	// retourne l'emplacement absolu du début du bloc
	unsigned int GetAbsolutePointerToBegin();

private:
	unsigned int m_PointerToBegin;	// pointeur relatif au bloc père ou à la fonction
	int m_Size;	// -1 si bloc pas fini, 0 si '}' seul sans '{' correspondant
	CSPFunction *m_InFunction;
	CSPBlock *m_InBlock;
	BList m_SubBlocks;
	BList m_Variables;

friend class CSPSourceFile;
};

#endif //CSPBLOCK_H