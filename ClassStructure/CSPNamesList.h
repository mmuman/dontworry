/* ClassStructure
 * 
 * Analyseur de code c++
 * par Sylvain Tertois - 2001
 *
 * CSPNamesList - Gestion d'une liste de noms pour le parseur
 * C'est une extension de la classe CSFNamesList
 *
 */


 #ifndef CSPNAMESLIST_H
 #define CSPNAMESLIST_H
 
 #include "CSFNamesList.h"
 
 class CSPNamesList : public CSFNamesList
 {
 public:
 	CSPNamesList();
 	virtual ~CSPNamesList();
 	
 	virtual bool Init(CSFile *, unsigned int pointer);
 	
 	unsigned int AddName(const char *name, BMallocIO *strings,
 		unsigned int pclass=0xFFFFFFFF);
 		// ajoute le nom à la liste.
 		// si le nom existe déjà ne fait rien et rend juste un pointeur
 		// dans les deux cas, si pclass est différent de 0xFFFFFFFF, le pointeur vers la
 		// classe (dans le chunk 'LCLA') est mis à jour. On prend la valeur de pclass
 
 	bool Write(CSFile *,unsigned int *pointer);
 		// écrit le chunk 'LIST' dans le fichier donné en paramètres, à
 		// l'endroit indiqué
 	
 	void Offset(unsigned int from, int offset);
 		// décale tous les pointeurs après from (non compris) d'un décalage égal à offset
 		
 protected:
 	unsigned int m_MallocSize;
 		// taille allouée du buffer (en nombre d'éléments)
 };
 
 #endif //CSPNAMESLIST_H