/* ClassStructure
 * 
 * Analyseur de code c++
 * par Sylvain Tertois - 2001
 *
 * CSVariable - Classe d'API
 *
 * Cette classe représente une variable membre, ou un argument dans une méthode.
 * Elle n'est pas crée par l'application cliente, mais par un objet CSClass ou CSMethod.
 * Par contre l'application peut utiliser ses méthodes pour accéder aux différentes 
 * informations sur la variable
 *
 */

#include "CSVariable.h"
#include "CSClass.h"
#include "CSLibrary.h"

#include <DataIO.h>

CSVariable::CSVariable(CSClass *fromClass, unsigned int pointer, bool memberVariable)
{
	m_Class = fromClass;
	m_Chunk = pointer;
	m_MemberVariable = memberVariable;
}

CSVariable::~CSVariable()
{
}

const char *CSVariable::GetName()
{
	// on va chercher les infos dans le chunk
	unsigned int l_PointerToName;
	m_Class->m_Library->m_Data->ReadAt(m_Chunk+4,&l_PointerToName,4);
	
	if (l_PointerToName == 0xffffffff)
		// pas de nom
		return NULL;
	else
		return m_Class->m_ClassStrings+l_PointerToName;
}
	
const char *CSVariable::GetTypeName()
{
	// on va chercher les infos dans le chunk
	unsigned int l_PointerToTypeName;
	m_Class->m_Library->m_Data->ReadAt(m_Chunk,&l_PointerToTypeName,4);
	
	return m_Class->m_Library->GetClassName(l_PointerToTypeName);
}

bool CSVariable::IsConst()
{
	// on va chercher les infos dans le chunk 'METH' ou 'MVRS'
	unsigned char l_Flags;
	m_Class->m_Library->m_Data->ReadAt(
		m_Chunk + (m_MemberVariable?9:13),&l_Flags,1);
	
	return ((l_Flags&1)!=0);
}

bool CSVariable::IsRef()
{
	// uniquement pour un paramètre
	if (m_MemberVariable)
		return false;
		
	// on va chercher les infos dans le chunk 'METH'
	unsigned char l_Flags;
	m_Class->m_Library->m_Data->ReadAt(
		m_Chunk + 13,&l_Flags,1);
	
	return ((l_Flags&2)!=0);
}

CSClass *CSVariable::GetClass()
{
	return m_Class->m_Library->GetClass(GetTypeName());
}
	
unsigned int CSVariable::GetPointerLevel()
{
	// on va chercher les infos dans le chunk 'METH' ou 'MVRS'
	unsigned char l_PointerLevel;
	m_Class->m_Library->m_Data->ReadAt(
		m_Chunk + (m_MemberVariable?8:12),&l_PointerLevel,1);
	
	return l_PointerLevel;
}
	
const char *CSVariable::GetDefaultValue()
{
	if (m_MemberVariable)
		return NULL;	// pas de valeur par défaut pour les variables membre
	else
	{
		// on va chercher les infos dans le chunk
		unsigned int l_PointerToValue;
		m_Class->m_Library->m_Data->ReadAt(m_Chunk+8,&l_PointerToValue,4);
		
		if (l_PointerToValue == 0xffffffff)
			// pas de valeur
			return NULL;
		else
			return m_Class->m_ClassStrings+l_PointerToValue;
	}
}

unsigned int CSVariable::GetZone()
{
	if (!m_MemberVariable)
		return 0;
		// cette fonction n'a pas de sens si on l'appelle sur un argument de fonction
		
	// on va chercher les infos dans le chunk 'MVRS'
	unsigned char l_Zone;
	m_Class->m_Library->m_Data->ReadAt(m_Chunk+10,&l_Zone,1);
			
	return l_Zone;
}