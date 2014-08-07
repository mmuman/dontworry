/* ClassStructure
 * 
 * Analyseur de code c++
 * par Sylvain Tertois - 2001
 *
 * CSContainer - Classe de gestions de projets et d'objets CSLibrary
 *
 * Cette classe sert à gérer les relations entre les projets, les différentes librairies
 * et la librairie système
 *
 */
 
 #include "CSContainer.h"
 #include "CSLibrary.h"

CSContainer::CSContainer()
{
	m_SystemLib = NULL;
}

CSContainer::~CSContainer()
{
}

void CSContainer::AddSystemLib(CSLibrary *lib)
{
	m_SystemLib = lib;
	
	for(int i=0; i< m_ProjectsList.CountItems(); i++)
	{
		CSLibrary *l_Project = (CSLibrary*) m_ProjectsList.ItemAt(i);
		l_Project->m_NextLibrary = m_SystemLib;
	}
}

void CSContainer::AddProject( CSLibrary *lib)
{
	lib->m_NextLibrary = m_SystemLib;
	m_ProjectsList.AddItem(lib);
}

bool CSContainer::RemoveProject( CSLibrary *lib)
{
	return m_ProjectsList.RemoveItem(lib);
}

CSClass *CSContainer::GetClass(const char *name, bool system, CSLibrary *inProject)
{
	CSClass *l_Class = NULL;
	
	// on commence par la librairie système
	if (system && (m_SystemLib != NULL))
	{	
		l_Class = m_SystemLib->GetClass(name);
		if (l_Class != NULL)
			return l_Class;
	}
	
	// on cherche dans les autres
	if (inProject != NULL)
	{
		// on ne cherche que dans celle-là
		/// pour commencer on lui évite d'aller chercher dans la lib système, on la déjà fait
		inProject->m_NextLibrary = NULL;

		/// puis fait la recherche alle-même
		l_Class = inProject->GetClass(name);

		/// et on remet comme il faut la lib système
		inProject->m_NextLibrary = m_SystemLib;
		
		return l_Class;
	}
	else
	{
		// on va devoir faire toute la liste
		CSLibrary *l_Lib;
		for (int i=0; i< m_ProjectsList.CountItems(); i++)
		{
			l_Lib = (CSLibrary*)m_ProjectsList.ItemAt(i);
			
			// on ne fait pas de recherche dans la lib système, c'est déjà fait
			l_Lib->m_NextLibrary = NULL;
			l_Class = l_Lib->GetClass(name);
			l_Lib->m_NextLibrary = m_SystemLib;
			
			if (l_Class != NULL)
				return l_Class;
		}
	}
	
	// pas trouvé...
	return NULL;
}