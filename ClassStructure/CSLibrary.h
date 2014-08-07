/* ClassStructure
 * 
 * Analyseur de code c++
 * par Sylvain Tertois - 2001
 *
 * CSLibrary - Classe d'API générale
 *
 * Cette classe sert à faire la liaison entre les classes CS et l'application cliente
 *
 * La fonctionnalité 'paths' (voir constructeur) n'est pas encore implémentée
 */
 
 #ifndef CSLIBRARY_H
 #define CSLIBRARY_H
 
 #include "CSFile.h"
 #include <SupportDefs.h>
 
 class CSParser;
 class CSFNamesList;
 class CSClass;
 
 class CSLibrary : public CSFile
 {
 public:
 	CSLibrary(BPositionIO *data, const char **paths = NULL);
 		// data est le fichier (BFile) ou la zone mémoire (BMallocIO) utilisé pour
 		// stocker l'information sur les classes
 		// paths est un tableau de chemins d'accès pour les headers
 	virtual ~CSLibrary();
 	
 	inline status_t InitCheck() { return m_InitCheck; }
 		// vérifie si la construction s'est bien passée
 	
 	bool Empty();
 		// retire toutes les informations de classes stockées
 	
 	// Parsing
  	status_t Parse(const char *path, bool directory = false, unsigned int recurse_dir = 5, bool savePath=false);
 		// parse un fichier header.
		// path: chemin du fichier ou dossier à explorer
		// directory: si true, 'path' est le chemin vers un dossier. Il faut en faire tous les fichiers .h
		// recurse_dirs: si directory est true, profondeur de la recherche, en nombre de dossiers
		// savePath: si true, le chemin du fichier header est stocké avec la classe
  	status_t Parse(const char *text, unsigned int textLength);
  		// même chose sur un texte stocké en mémoire
 	
 	// Exploitation
 	CSClass *GetClass(const char *name);
 		// donne les informations sur une classe dont on donne le nom
		// l'objet retourné devient propriété de l'appelant. Il doit le détruire après utilisation

	void AddLibrary(CSLibrary *library);
		// ajoute une bibliothèque dans la liste de classes à chercher
		
	// Debug
	CSClass *GetFirstClass();
		// retourne la première classe de la librairie, NULL s'il y en a aucune.
		// l'objet retourné devient propriété de l'appelant. Il doit le détruire après utilisation

	CSClass *GetNextClass();
		// retourne la classe suivante de la librairie, NULL si on est arrivés au bout.
		// l'objet retourné devient propriété de l'appelant. Il doit le détruire après utilisation

private:
	status_t m_InitCheck;
	
	// gestion de l'objet CSParser
	CSParser *m_Parser;
	status_t BeginParse();
	status_t EndParse();
	
	// accès aux noms de classes (pour les fonctions de l'API)
	inline const char *GetClassName(unsigned int pointer)  
		{ return m_NameStrings+pointer; }
	inline CSClass *GetClass(unsigned int pointer)
		{ return GetClass(GetClassName(pointer)); }
	
	// stockage des noms de classes
	char *m_NameStrings;
	CSFNamesList *m_NamesList;
	
	// pointeur sur le chunk 'LCLA'
	unsigned int m_ClassListChunk;
	
	// classes amies: l'API
	friend class CSClass;
	friend class CSMethod;
	friend class CSVariable;
	
	// bibliothèque suivante pour faire les recherches
	CSLibrary *m_NextLibrary;
	
	// pointeur pour les deux fonctions de debug GetFirstClass et GetNextClass
	unsigned int m_DebugPointer;
	
	friend class CSContainer;
};

#define CSP_NOT_MODIFIED B_ERRORS_END+1
	// code d'erreur retourné par le parseur quand le fichier .h n'avait pas été modifié depuis le dernier parsing dessus
#endif
