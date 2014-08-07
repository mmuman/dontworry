/* ClassStructure
 * 
 * Analyseur de code c++
 * par Sylvain Tertois - 2001
 *
 * CSPNamesList - Gestion d'une liste de noms pour le parseur
 * C'est une extension de la classe CSFNamesList
 *
 */

#include "CSPNamesList.h"
#include <stdlib.h>
#include <stdio.h>
#include "CSFile.h"

CSPNamesList::CSPNamesList()
{
	m_MallocSize=0;
}

CSPNamesList::~CSPNamesList()
{
}

bool CSPNamesList::Init(CSFile *file, unsigned int pointer)
{
	if (!CSFNamesList::Init(file,pointer))
		// erreur d'initialisation
		return false;
	else
	{
		// l'init s'est bien passée
		m_MallocSize = m_NumElements;
		return true;
	}
}

unsigned int CSPNamesList::AddName(const char *name, BMallocIO *strings, 
	unsigned int pclass)
{
	// d'abord on cherche si le nom est déjà là, et sinon à quel endroit on doit le mettre
	unsigned int l_Where;
	
	if (Search(name,strings->Buffer(),&l_Where))
	{
		// le nom exise déjà. On ne va pas l'ajouter
		/// on s'occupe quand même de pclass
		if (pclass != 0xFFFFFFFF)
			m_List[l_Where*2+1] = pclass;
	}
	else
	{
		// on ajoute la chaîne à la zone de mémoire faite pour ça
		off_t l_StringPointer = strings->Seek(0,SEEK_END);
		strings->Write(name,strlen(name)+1);
		
		// préparation de l'insertion du nom dans la liste
		unsigned int *l_PointSource = m_List, *l_PointDest;
		unsigned int *l_List = m_List;	// nouvelle liste (si on en alloue une autre)
		bool l_NewAlloc = false;	// true si on doit allouer un nouveau bloc
		
		// doit-on allouer un nouveau tampon?
		if (m_MallocSize <= m_NumElements)
		{
			l_NewAlloc = true;
			m_MallocSize += 32;
			l_List = new unsigned int [m_MallocSize*2*sizeof(unsigned int)];
			l_PointDest = l_List;
			
			// recopie des premiers éléments
			for (unsigned int i=0; i<l_Where; i++)
			{
				*l_PointDest++ = *l_PointSource++;
				*l_PointDest++ = *l_PointSource++;
			}
		}
		else
		{
			// non. Pas besoin de recopier les anciens éléments, on va juste
			// se mettre là où il faut
			l_PointSource += 2*l_Where;
			l_PointDest = l_PointSource;
		}
		
		// décalage des éléments suivants
		unsigned int *l_InsertPoint = l_PointDest;
		l_PointSource += (m_NumElements-l_Where)*2;
		l_PointDest += (m_NumElements-l_Where+1)*2;
		while (l_PointDest > l_InsertPoint+2)
		{
			*--l_PointDest = *--l_PointSource;
			*--l_PointDest = *--l_PointSource;
		}
		
		// écriture des valeurs
		*--l_PointDest = pclass;
		*--l_PointDest = l_StringPointer;
		
		// si on a alloué un nouveau tampon, il faut détruire l'ancien
		if (l_NewAlloc)
		{
			delete m_List;
			m_List = l_List;
		}
		
		// mise à jour du nombre d'elements
		m_NumElements++;
	} 
	
	// on retourne le pointeur vers le nom
	return m_List[l_Where*2];
}

bool CSPNamesList::Write(CSFile *file,unsigned int *pointer)
{
	return file->ChunkWrite(pointer,g_CNamesList,m_List,
		m_NumElements*2*sizeof(unsigned int));
}

void CSPNamesList::Offset( unsigned int from, int offset)
{
	// on parcourt toute la liste
	for (unsigned int l_Pointer = 0; l_Pointer<m_NumElements; l_Pointer++)
	{
		// la classe sur laquelle on est doit-elle être décalée?
		unsigned int l_ClassPtr = m_List[l_Pointer*2+1];
		if ((l_ClassPtr != 0xffffffff) && (l_ClassPtr > from))
			// on effectue le décalage
			m_List[l_Pointer*2+1] = l_ClassPtr+offset;
	}
}
	