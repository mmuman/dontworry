/*
 * Parseur c++ pour keyspy
 *
 * CSPSourceFile: objet qui représente une source, et qui sait le parser (enfin en théorie ;-)
 *
 */

#ifndef CSPSOURCEFILE_H
#define CSPSOURCEFILE_H

#include <List.h>

class CSPFunction;
class CSPBlock;
class BString;

class CSPSourceFile
{
public:
	CSPSourceFile();
	~CSPSourceFile();
	
	void Empty();

	// parsing
	void Parse(const char *begin, const char *end);
	
	// extraction d'informations
	// attention!! il ne faut pas toucher aux const char donnés. En particulier il ne faut pas détruire les chaînes
	const char *CurrentClass();		// rend "" si on n'est pas dans une fonction ou si on est dans une fonction globale
	const char *CurrentFunction();	// rend "" si on n'est pas dans une fonction
	const char *TypeOfVariable(const char *name, unsigned int *pointerLevel = NULL);
			// rend le type de la variable dont on donne le nom. NULL si elle n'est pas trouvé.
			// on cherche dans les listes suivantes:
			// * variables locales
			// * paramètres de la fonction
			// * variables globales
			// si pointerLevel n'est pas NULL, on met dedans le niveau de pointeur (le nombre de * dans le type)
	
	const char *GetFirstVariableName();
		// retourne le nom de la première variable. Rend NULL s'il n'y en a aucune. Le nom retourné reste
		// propriété de l'objet et ne doit être ni modifié ni détruit.
	const char *GetNextVariableName();
		// retourne le nom de la variable suivante. Rend NULL si on est arrivés à la fin de la liste. Le nom 
		// retourné reste propriété de l'objet et ne doit être ni modifié ni détruit.
		
private:
	BList m_Functions;
	BList m_LonelyBlocks;
	BList m_GlobalVariables;
	
	CSPFunction *m_CurrentFunction;
	CSPBlock *m_CurrentBlock;
	
	// objets qui servent aux fonctions GetFirst/Next/VariableName()
	CSPBlock *m_GFNVNBlock;	// bloc qu'on est en train d'explorer
	unsigned int m_GFNVNIndex;	// index dans le bloc
	bool m_GFNVInFunction;		// true: on est encore dans la fonction en cours, false: on fait les variables globales

// fonctions d'aide au parsing
	bool ExtractWord(BString &word, const char **pointer, const char *end, bool *isOperator);
		// va chercher un mot. Rend false si on est à la fin du fichier, true sinon
		// le mot est mis dans 'word'. isOperator est mis à true si le mot est en
		// fait un opérateur. Dans ce cas 'word' ne fait obligatoirement qu'un octet de long
	void SkipToEndOfLine(const char **pointer, const char *end);
		// va à la ligne suivante
	void SkipToNextInstruction(const char **pointer, const char *end);
		// va à l'instruction suivante (après un ';')
	void SkipToEndOfString(const char **pointer, const char *end);
		// va à la fin d'une chaîne de caractères
	void SkipToClosingBracket(const char **pointer, const char *end);
		// va à la fin d'un bloc logique

	const char *SearchVariable(BList &inlinst, const char *name, unsigned int *pointerLevel = NULL);
		
// fonctions diverses
	void EmptyList(BList &list);	// vide une liste de CSPVariables
	void StoreVariable(BString &type,BString &name, unsigned int l_PointerLevel, unsigned int pointer, BList *paramsList = NULL);
		// si paramsList est donné, la variable est stoquée dans la liste au lieu d'être dans la fonction en cours
		
	void StoreFunction(BString &className,BString &name, BList &paramsList, unsigned int pointer);
};

#endif //CSPSOURCEFILE_H