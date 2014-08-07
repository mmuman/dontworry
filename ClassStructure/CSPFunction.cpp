/*
 * Parseur c++ pour keyspy
 *
 * CSPFunction: objet qui représente une fonction dans un source
 *
 */

#include "CSPFunction.h"
#include "CSPBlock.h"
#include "CSPVariable.h"

CSPFunction::CSPFunction()
{
	m_PointerToBegin = 0;
	m_Hidden = false;
}

CSPFunction::~CSPFunction()
{
	Empty();
}

void CSPFunction::Empty()
{
	int l_NBlocks = m_Blocks.CountItems();
	
	for(int i=0; i<l_NBlocks; i++)
	{
		delete (CSPBlock*)m_Blocks.RemoveItem((int32)0);
	}

	int l_NVars = m_Variables.CountItems();
	
	for(int i=0; i<l_NVars; i++)
	{
		delete (CSPVariable*)m_Variables.RemoveItem((int32)0);
	}
}
