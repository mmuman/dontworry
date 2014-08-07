/* ClassStructure
 * 
 * Analyseur de code c++
 * par Sylvain Tertois - 2001
 *
 * CSClass - Classe d'API
 *
 * Cette classe représente une classe. Elle n'est pas crée par l'application cliente, mais
 * par un objet CSLibrary. Par contre l'application peut utiliser ses méthodes pour
 * accéder aux différentes informations sur la classe
 *
 * Toutes les fonctions autour du typedef ne marchent pas pour l'instant
 * Les filtres 'public', 'protected' et 'private' ne marchent pas non plus
 */

#include "CSClass.h"
#include "CSLibrary.h"
#include "CSMethod.h"
#include "CSVariable.h"

#include <String.h>
#include <DataIO.h>

CSClass::CSClass(CSLibrary *library,unsigned int pointer)
{
	m_Library = library;
	m_InfoChunk = m_MetsChunk = m_MemberVarsChunk = 0;
	m_ClassStrings = NULL;
	m_IsTypeDef = false;
	
	// recherche de infos sur le chunk principal
	unsigned int l_Type,l_Size,l_ChunkEnd;
	if (!m_Library->ChunkInfo(pointer,&l_Type,&l_Size) || (l_Type != g_CClass))
	{
		// erreur... ne devrait pas arriver
		return;
	}
	l_ChunkEnd = pointer+l_Size;
	pointer += g_ChunkHeaderSize;
	
	// on parcourt tous les chunks les uns après les autres
	while (pointer< l_ChunkEnd)
	{
		if (!m_Library->ChunkInfo(pointer,&l_Type,&l_Size))	
		{
			// erreur... ne devrait pas arriver
			return;
		}
		
		// de quel chunk s'agit-il?
		switch(l_Type)
		{
		case g_CClassInfo:
			m_InfoChunk = pointer+g_ChunkHeaderSize;
			break;
		
		case g_CClassMethodsList:
			m_MetsChunk = pointer+g_ChunkHeaderSize;
			break;
		
		case g_CClassMemberVariables:
			m_MemberVarsChunk = pointer+g_ChunkHeaderSize;
			break;
		
		case g_CClassStrings:
			m_ClassStrings = new char[l_Size];
			m_Library->ChunkRead(&pointer,m_ClassStrings);
			break;
		
		default:
			break;
		}
		
		// on passe au chunk suivant
		if (l_Type != g_CClassStrings)
			m_Library->ChunkSkip(&pointer);
	}
}

CSClass::~CSClass()
{
	delete m_ClassStrings;
}

const char *CSClass::GetName()
{
	// lecture dans le chunk 'INFO' du pointeur vers le nom de la classe
	unsigned int l_NamePointer;
	m_Library->m_Data->ReadAt(m_InfoChunk,&l_NamePointer,4);
	
	return m_Library->GetClassName(l_NamePointer);
}

// fonctions typedef... pas encore écrit
CSClass *CSClass::GetRealClass()
{
	return NULL;
}

const char *CSClass::GetRealName()
{
	return NULL;
}

unsigned int CSClass::GetPointerLevel()
{
	return 0;
}

const char *CSClass::GetHeaderFile()
{
	// on va devoir parcourir un peu le chunk Info pour trouver l'info qu'on veut
	unsigned int l_PointerToHeader = m_InfoChunk+12;
	
	unsigned char l_TempByte;
	/// on lit le nombre de classes père
	m_Library->m_Data->ReadAt(m_InfoChunk+10,&l_TempByte,1);
	l_PointerToHeader += l_TempByte*4;
	
	/// on lit le nombre de classes amies
	m_Library->m_Data->ReadAt(m_InfoChunk+11,&l_TempByte,1);
	l_PointerToHeader += l_TempByte*4;
	
	// on vérifie qu'on n'est pas sorti du chunk info (si par exemple on a un fichier de version antérieure à la 6)
	unsigned int l_InfoChunkSize;
	m_Library->m_Data->ReadAt(m_InfoChunk-4,&l_InfoChunkSize,4);
	l_InfoChunkSize -= g_ChunkHeaderSize;
	if (l_PointerToHeader-m_InfoChunk+4 > l_InfoChunkSize)
		return NULL;
	
	// c'est bon! on va extraire le pointeur vers la chaine
	m_Library->m_Data->ReadAt(l_PointerToHeader,&l_PointerToHeader,4);
	
	// alors, elle existe cette chaîne?
	if (l_PointerToHeader == 0xffffffff)
		// non
		return NULL;
	else
		// oui
		return l_PointerToHeader+m_ClassStrings;
}	

CSClass *CSClass::GetFirstFather()
{
	// combien de pères?
	m_Library->m_Data->ReadAt(m_InfoChunk+10,&m_NumFathers,1);
	
	// initialisation
	m_NFather = 0;
	return GetNextFather();
}

CSClass *CSClass::GetNextFather()
{
	// est-on bien une classe?
	if (m_IsTypeDef)
		// non... on rend NULL
		return NULL;
		
	for (;;)
	{
	// est-on arrivé à la fin?
		if (m_NFather >= m_NumFathers)
			// oui... on rend NULL
			return NULL;
	
		// on récupère un pointeur vers le nom de la classe père
		unsigned int l_PointerToName;
		m_Library->m_Data->ReadAt(m_InfoChunk+12+4*m_NFather,&l_PointerToName,4);
		
		// puis une description de la classe
		CSClass *m_Father = m_Library->GetClass(l_PointerToName);
		
		// on augment le compteur
		m_NFather++;
		
		// si l'objet CSClass a été trouvé dans la librarie, on le rend
		if (m_Father != NULL)
			return (m_Father);
			
		// sinon on va lire le prochain
	}
}

bool CSClass::IsFriend(const char *name)
{
	// est-on bien une classe?
	if (m_IsTypeDef)
		return false;
	
	// on récupère le nombre de pères
	m_Library->m_Data->ReadAt(m_InfoChunk+10,&m_NumFathers,1);
	// et le nombre d'amis
	unsigned char l_NumFriends;
	m_Library->m_Data->ReadAt(m_InfoChunk+11,&l_NumFriends,1);
	
	BString l_Name(name);
	
	// boucle parmi les amis
	unsigned int l_PointerInChunk = m_InfoChunk+12+4*m_NumFathers;
	for (unsigned int i=0; i<l_NumFriends; i++)
	{
		unsigned int l_PointerToName;
		m_Library->m_Data->ReadAt(l_PointerInChunk,&l_PointerToName,4);
		l_PointerInChunk += 4;
		
		// est-ce le bon ami?
		if (l_Name == (m_Library->GetClassName(l_PointerToName)))
			// oui!
			return true;
	}
	
	// le nom n'a pas été trouvé dans la liste d'amis
	return false;
}

CSMethod *CSClass::GetFirstMethod(unsigned int filter)
{
	// initialisation
	m_PMethod = m_MetsChunk;
	m_MethodFilter = filter;
	return GetNextMethod();
}

CSMethod *CSClass::GetNextMethod()
{
	unsigned int l_Type,l_Size;
	unsigned char l_Zone;

	// on boucle tant qu'on a pas trouvé une méthode qui corresponde au filtre
	while (true)
	{	
		// on récupère les infos sur le chunk pour vérifier qu'il s'agit bien d'une méthode
		if (!m_Library->ChunkInfo(m_PMethod,&l_Type,&l_Size) || 
			(l_Type != g_CClassMethod))
			// c'est pas un chunk méthode... on est arrivé au bout de la liste
			return NULL;
	
		// c'est bien une méthode
		// on vérifie si ça rentre dans le filtre
		m_Library->m_Data->ReadAt(m_PMethod+g_ChunkHeaderSize+11,&l_Zone,1);
		if ((l_Zone & m_MethodFilter) != 0)
		{
			// c'est bon!
			CSMethod *l_Method = new CSMethod(this,m_PMethod+g_ChunkHeaderSize);
	
			// on se déplace à la méthode suivante, pour le prochain appel
			m_Library->ChunkSkip(&m_PMethod);
	
			// et on rend la méthode qu'on vient de créer
			return l_Method;
		}
		else
			// la méthode n'était pas dans la bonne zone
			// on passe à la suivante
			m_Library->ChunkSkip(&m_PMethod);
	}
}

CSVariable *CSClass::GetFirstMember(unsigned int filter)
{
	// combien de variables membre?
	m_Library->m_Data->ReadAt(m_MemberVarsChunk,&m_NumMemberVariables,2);
	
	// initialisation
	m_NMemberVariable = 0;
	m_VariableFilter = filter;
	return GetNextMember();
}

CSVariable *CSClass::GetNextMember()
{
	// est-on autre chose qu'une classe?
	if (m_IsTypeDef)
		// oui... on rend NULL
		return NULL;

	// on boucle tant qu'on a pas trouvé une variable dans la bonne zone
	while(true)
	{
		// est-on arrivé à la fin?
		if (m_NMemberVariable >= m_NumMemberVariables)
			// oui, on a pas trouvé de nouvelle variable, on rend NULL
			return NULL;
			
		// on crée un pointeur vers la structure qui représente la variable
		unsigned int l_PointerToVariable = m_MemberVarsChunk+4+m_NMemberVariable*12;

		// on augmente le compteur
		m_NMemberVariable++;
		
		// alors, est-on dans la bonne zone?
		unsigned char l_Zone;
		m_Library->m_Data->ReadAt(l_PointerToVariable+10,&l_Zone,1);
		
		if ((l_Zone & m_VariableFilter) != 0)
			// oui, on crée l'objet CSVariable qui va bien
			return new CSVariable(this,l_PointerToVariable,true);
		// non, on continue à chercher
	}
}

CSVariable *CSClass::FindMember(const char *name)
{
	// est-on bien une classe?
	if (m_IsTypeDef)
		return NULL;
		
	// on fait un objet BString avec le nom, c'est mieux pour la comparaison
	BString l_Name(name);

	// combien de variables membre?
	m_Library->m_Data->ReadAt(m_MemberVarsChunk,&m_NumMemberVariables,2);
	
	// on boucle dans les variables membres jusqu'à tomber sur la bonne
	unsigned int l_PointerToVariable = m_MemberVarsChunk+4;
	for (unsigned short i=0; i<m_NumMemberVariables; i++)
	{
		// on récupère l'offset vers le nom
		unsigned int l_PointerToName;
		m_Library->m_Data->ReadAt(l_PointerToVariable+4,&l_PointerToName,4);
		
		// est-ce la bonne variable?
		if (l_Name == (m_ClassStrings+l_PointerToName))
			// oui, on a trouvé!
			return new CSVariable(this,l_PointerToVariable,true);

		// on va vers le prochain
		l_PointerToVariable += 12;
	}
	
	// la variable membre n'a pas été trouvée
	return NULL;
}

CSMethod *CSClass::FindMethod(const char *name, unsigned int index)
{
	// initialisation
	m_PMethod = m_MetsChunk;

	// on fait un objet BString avec le nom, c'est mieux pour la comparaison
	BString l_Name(name);

	// on boucle dans les méthodes jusqu'à ce qu'on tombe sur la bonne
	/// recherche des infos sur le prochain chunk
	unsigned int l_Type,l_Size;
	if (!m_Library->ChunkInfo(m_PMethod,&l_Type,&l_Size))
		return NULL;
	/// condition de boucle: le chunk doit être du bon type
	while (l_Type == g_CClassMethod)
	{
		// on vérifie si le nom est bon
		/// chargement de l'offset vers le nom
		unsigned int l_PointerToName;
		m_Library->m_Data->ReadAt(m_PMethod+g_ChunkHeaderSize,&l_PointerToName,4);
		/// et comparaison
		if (l_Name == (m_ClassStrings+l_PointerToName))
			// on l'a trouvée!!
			if (index-- == 0)
				return new CSMethod(this,m_PMethod+g_ChunkHeaderSize);
			
		// on passe au chunk prochain
		m_Library->ChunkSkip(&m_PMethod);
		if (!m_Library->ChunkInfo(m_PMethod,&l_Type,&l_Size))
			return NULL;
	}
	
	// pas trouvé
	return NULL;
}

CSLibrary *CSClass::GetLibrary()
{
	return m_Library;
}
