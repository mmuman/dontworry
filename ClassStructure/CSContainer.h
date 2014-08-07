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

#ifndef CSCONTAINER_H
#define CSCONTAINER_H

class CSLibrary;
class CSClass;
#include <List.h>

class CSContainer
{
public:
	CSContainer();
	~CSContainer();
	
	void AddSystemLib(CSLibrary *lib);
		// définit lib comme nouvelle librairie système. La précédente librairie est
		// remplacée par celle ci, et n'est pas détruire
	void AddProject(CSLibrary *lib);
		// ajoute une librairie projet
	bool RemoveProject(CSLibrary *lib);
		// retire une librairie projet (true si la librairie a bien été effacée)
		
	CSClass *GetClass(const char *name, bool system = true, CSLibrary *inProject = NULL);

private:
	BList m_ProjectsList;
	CSLibrary *m_SystemLib;
};

#endif //CSCONTAINER_H
	
	
	