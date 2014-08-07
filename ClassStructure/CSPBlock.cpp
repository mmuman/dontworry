/*
 * Parseur c++ pour keyspy
 *
 * CSPBlock: objet qui représente un bloc (de '{' à '}')
 *
 */

#include "CSPBlock.h"
#include "CSPFunction.h"
#include "CSPVariable.h"

CSPBlock::CSPBlock()
{
	m_PointerToBegin = 0;
	m_Size = -1;
	m_InFunction = NULL;
	m_InBlock = NULL;
}

CSPBlock::~CSPBlock()
{
	Empty();
}

void CSPBlock::Empty()
{
	int l_NBlocks = m_SubBlocks.CountItems();
	
	for(int i=0; i<l_NBlocks; i++)
	{
		delete (CSPBlock*)m_SubBlocks.RemoveItem((int32)0);
	}

	int l_NVars = m_Variables.CountItems();
	
	for(int i=0; i<l_NVars; i++)
	{
		delete (CSPVariable*)m_Variables.RemoveItem((int32)0);
	}
}

unsigned int CSPBlock::GetAbsolutePointerToBegin()
{
	// est-on le fils d'un autre bloc?
	if (m_InBlock != NULL)
		// oui
		return m_InBlock->GetAbsolutePointerToBegin()+m_PointerToBegin;
	// sinon est-on le bloc principal d'une fonction?
	else if (m_InFunction != NULL)
		// oui
		return m_InFunction->m_PointerToBegin+m_PointerToBegin;
	else
	// ben sinon c'est le pointeur du bloc qui est absolu
		return m_PointerToBegin;
}
