/* ClassStructure
 * 
 * Analyseur de code c++
 * par Sylvain Tertois - 2001
 *
 * CSFNamesList - Gestion d'une liste de noms de classes
 *
 */

#include "CSFNamesList.h"
#include "CSFile.h"
#include "String.h"

CSFNamesList::CSFNamesList()
{
	m_List = NULL;
	m_NumElements = 0;
}

CSFNamesList::~CSFNamesList()
{
	delete m_List;
}

bool CSFNamesList::Init(CSFile *file, unsigned int pointer)
{
	// on vérifie que c'est bien le chunk 'LIST' qui a été donné
	unsigned int l_Type,l_Size;
	
	if (!file->ChunkInfo(pointer,&l_Type,&l_Size))
		return false;
			// erreur lors du chargement du chunk
		
	if (l_Type == g_CNamesList)
	{
		// c'est bon!
		m_NumElements = (l_Size/sizeof(unsigned int)) >> 1;
		m_List = new unsigned int [m_NumElements << 1];
			
		return file->ChunkRead(&pointer,m_List);
	}
	else
		return false;
			// c'était pas le bon chunk		
}

unsigned int CSFNamesList::FindName(const char *name, const void *strings)
{
	// on appelle la fonction de recherche interne
	unsigned int l_Where;
	if (Search(name,strings,&l_Where))
		// trouvé
		return m_List[2*l_Where+1];
	else
		// pas trouvé
		return 0xFFFFFFFF;
}

// fonction interne de recherche
bool CSFNamesList::Search(const char *name, const void *strings, unsigned int *where)
{
	*where = 0;
	BString l_Name(name);
	
	// on procède par dichotomie pour trouver la chaîne
	unsigned int l_Start = 0;
	unsigned int l_End = m_NumElements;
	
	while(l_Start != l_End)
	{
		// on regarde ce qu'il y a au milieu
		*where = (l_End+l_Start)/2;
		int l_Comp = l_Name.Compare(((const char*)strings)+m_List[2*(*where)]);
		
		if (l_Comp == 0)
			return true;	// trouvé!
		
		if (l_Comp>0)
			// la chaîne à chercher est après
			l_Start = ++(*where);
		else
			// la chaîne à chercher est avant
			l_End = *where;
	}
	// on ne l'a pas trouvé... on se trouve juste après
	return false;	
}
