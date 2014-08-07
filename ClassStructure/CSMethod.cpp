/* ClassStructure
 * 
 * Analyseur de code c++
 * par Sylvain Tertois - 2001
 *
 * CSMethod - Classe d'API
 *
 * Cette classe représente une méthode. Elle n'est pas crée par l'application cliente, mais
 * par un objet CSClass. Par contre l'application peut utiliser ses méthodes pour
 * accéder aux différentes informations sur la méthode
 *
 */

#include "CSMethod.h"
#include "CSLibrary.h"
#include "CSVariable.h"
#include "CSClass.h"

#include <DataIO.h>

CSMethod::CSMethod(CSClass *fromClass, unsigned int pointer)
{
	m_Class = fromClass;
	m_Chunk = pointer;
}

CSMethod::~CSMethod()
{
}

const char *CSMethod::GetName()
{
	// on va chercher les infos dans le chunk 'METH'
	unsigned int l_PointerToName;
	m_Class->m_Library->m_Data->ReadAt(m_Chunk,&l_PointerToName,4);
	
	return m_Class->m_ClassStrings+l_PointerToName;
}

bool CSMethod::IsVirtual()
{
	// on va chercher les infos dans le chunk 'METH'
	unsigned char l_Flags;
	m_Class->m_Library->m_Data->ReadAt(m_Chunk+9,&l_Flags,1);
	
	return ((l_Flags&2)!=0);
}

const char *CSMethod::GetReturnTypeName()
{
	// on va chercher les infos dans le chunk 'METH'
	unsigned int l_PointerToTypeName;
	m_Class->m_Library->m_Data->ReadAt(m_Chunk+4,&l_PointerToTypeName,4);
	
	return m_Class->m_Library->GetClassName(l_PointerToTypeName);
}

bool CSMethod::IsReturnTypeConst()
{
	// on va chercher les infos dans le chunk 'METH'
	unsigned char l_Flags;
	m_Class->m_Library->m_Data->ReadAt(m_Chunk+9,&l_Flags,1);
	
	return ((l_Flags&1)!=0);
}

bool CSMethod::IsReturnTypeRef()
{
	// on va chercher les infos dans le chunk 'METH'
	unsigned char l_Flags;
	m_Class->m_Library->m_Data->ReadAt(m_Chunk+9,&l_Flags,1);
	
	return ((l_Flags&4)!=0);
}


CSClass *CSMethod::GetReturnClass()
{
	return m_Class->m_Library->GetClass(GetReturnTypeName());
}

unsigned int CSMethod::GetReturnPointerLevel()
{
	// on va chercher les infos dans le chunk 'METH'
	unsigned char l_PointerLevel;
	m_Class->m_Library->m_Data->ReadAt(m_Chunk+8,&l_PointerLevel,1);
	
	return l_PointerLevel;
}

unsigned int CSMethod::GetZone()
{
	// on va chercher les infos dans le chunk 'METH'
	unsigned char l_Zone;
	m_Class->m_Library->m_Data->ReadAt(m_Chunk+11,&l_Zone,1);
	
	return l_Zone;
}

CSVariable *CSMethod::GetFirstArgument()
{
	// on va chercher le nombre d'arguments
	m_Class->m_Library->m_Data->ReadAt(m_Chunk+10,&m_NumArguments,1);
	
	m_NArgument = 0;

	return GetNextArgument();
}

CSVariable *CSMethod::GetNextArgument()
{
	// a-t-on fait tous les arguments?
	if (m_NArgument >= m_NumArguments)
		return NULL;	// oui
		
	// on construit le pointeur vers l'argument
	unsigned int l_PointerToArgument = m_Chunk+12+16*m_NArgument;
	
	// on incrémente le numéro de l'argument à voir
	m_NArgument++;
	
	// et on construit un objet pour représenter l'argument
	return new CSVariable(m_Class,l_PointerToArgument,false);
}