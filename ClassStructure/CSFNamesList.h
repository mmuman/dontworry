/* ClassStructure
 * 
 * Analyseur de code c++
 * par Sylvain Tertois - 2001
 *
 * CSFNamesList - Gestion d'une liste de noms de classes
 *
 */

#ifndef CSFNAMESLIST_H
#define CSFNAMESLIST_H

class CSFile;
#include <DataIO.h>

class CSFNamesList
{
public:
	CSFNamesList();
	virtual ~CSFNamesList();
	
	virtual bool Init(CSFile*,unsigned int pointer);
		// charge la liste contenue dans le fichier donné
		// pointer doit être la position du chunk 'LIST'
		// rend true si la liste a bien pu être chargée, false sinon
		
	unsigned int FindName(const char *name, const void *strings);
	inline unsigned int FindName(const char *name, const BMallocIO *strings)
		{ return FindName(name,strings->Buffer()); }
		// retrouve un nom dans la liste, et donne un pointeur vers le chunk 'CLAS'
		// qui lui correspond, dans le chunk 'LCLA'
		// 0xFFFFFFFF si non trouvé
		// le second argument est un pointeur vers la zone de mémoire ou a été
		// chargée le chunk 'CLNM'
	
protected:
	// données sur le buffer mémoire dans lequelle la liste est stockée
	unsigned int *m_List;
	unsigned int m_NumElements;
	
	// méthode interne de recherche, utilisée par FindName
	bool Search(const char *name, const void *strings, unsigned int *where);
		// rend true si le nom a été trouvé, et met le numéro de l'élément dans where
		// rend false si le nom n'a pas été trouvé, et met le numéro du nom immédiatement suivant dans l'alphabet dans where
};

#endif //CSFNAMESLIST_H