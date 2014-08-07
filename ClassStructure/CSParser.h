/* Début de parseur de fichiers .h
 *
 * Ce parseur analyse un fichier header, à la recherche de définitions des 
 * classes et stocke les informations trouvées dans un objet BPositionIO
 *
 *
 * Pour l'instant le parseur est très simplifié:
 * + n'utiliser qu'avec des syntaxes simples
 * + ne gère pas les define, include et autres #machin
 * + ne gère pas les typedef
 * + ne reconnait pas les #define sur plusieurs lignes avec des \
 * + les const, virtual, inline, static sont ignorés, ainsi que les passages par référence (&)
 * + ignore les classes amies (friend)
 * + n'a pas été essayé avec des tableaux (définis par [] ) comportement incertain dans ce cas
 * + les classes imbriquées ne sont pas gérées (pareil, comportement incertain dans ce cas)
 * + sûrement d'autres trucs que je n'ai pas vu
 */

/* Utilisation de la classe:
 * Au moment où on doit faire un parsing, créer un objet CSParser en lui
 * donnant en argument l'objet BPositionIO que l'on veut utiliser pour
 * stocker les informations.
 * Ensuite on fait un appel à Parse() pour chaque fichier .h que l'on veut
 * parser.
 * Enfin on détruit l'objet CSParser.
 * ON NE DOIT PAS LIRE L'OBJET BPOSITIONIO AVANT QUE L'OBJET CSPARSER SOIT
 * DETRUIT
 */

/* La classe suppose que le fichier donné est dans un format valide. (en gros
 * que les chunks se suivent bien et qu'aucune dimension ne soit farfelue)
 * Le comportement est indéterminé si ce n'est pas le cas. Il y a de grandes
 * chances que ça fasse boum.
 */
 
#ifndef CSPARSER_H
#define CSPARSER_H

#include "CSFile.h"
#include "CSPNamesList.h"

#include <List.h>
#include <SupportDefs.h>
#include <DataIO.h>
#include <String.h>

class CSParser : public CSFile
{
public:
	CSParser(BPositionIO *data);
	virtual ~CSParser();
	
	inline status_t InitCheck() { return m_InitCheck; }
	
	// Parsing
	status_t Parse(const char *path, bool directory=false, unsigned int recurse_dirs = 5, bool savePath = false);
		// path: chemin du fichier ou dossier à explorer
		// directory: si true, 'path' est le chemin vers un dossier. Il faut en faire tous les fichiers .h
		// recurse_dirs: si directory est true, profondeur de la recherche, en nombre de dossiers
		// savePath: si true, le chemin du fichier header est stocké avec la classe
 	status_t Parse(const char *text, unsigned int textLength,const char *path=NULL, time_t modificationTime=0);
 		// même version, sur un texte en mémoire. Laisser path et modificationTime à NULL et 0
 		
private:
	// listes qui stockent des trucs penant le parsing
	/// Liste de noms
	CSPNamesList m_ClassNamesList;

	/// et bloc de mémoire qui stocke les chaînes de caractères
	BMallocIO m_ClassNamesData;
	
	// pointeur vers le début des données du chunk 'LCLA'
	unsigned int m_BegClassChunk;
	// pareil vers la fin
	unsigned int m_EndClassChunk;
	
	// erreur à l'intialisation
	status_t m_InitCheck;

	// fonctions utilitaires pour le parsing
	status_t DoFile(const char *path, bool savePath);
		// fait le parsing sur un fichier
		// si savePath est vrai, on stocke le chemin vers le fichier header avec la classe
		
	bool ExtractWord(BString &word, const char **pointer, const char *endOfText, bool *isOperator);
		// va chercher un mot. Rend false si on est à la fin du fichier, true sinon
		// le mot est mis dans 'word'. isOperator est mis à true si le mot est en
		// fait un opérateur. Dans ce cas 'word' ne fait obligatoirement qu'un octet de long
	void SkipToEndOfLine(const char **pointer, const char *endOfText);
		// va à la ligne suivante
	void SkipToNextInstruction(const char **pointer, const char *endOfText);
		// va à l'instruction suivante (après un ';')
	void SkipToEndOfString(const char **pointer, const char *endOfText);
		// va à la fin d'une chaîne de caractères
	void SkipToClosingBracket(const char **pointer, const char *endOfText);
		// va à la fin d'un bloc logique (fermé par un '}')
	unsigned int CreateTempString(const char *string,BMallocIO &space);
	inline unsigned int CreateTempString(BString &string,BMallocIO &space)
		{ return CreateTempString(string.String(),space); }
		// met une chaîne de caractères dans l'espace temporaire donné, et retourne
		// l'offset vers cette chaîne
	
	// fonctions de parsing sur des objets particuliers
	status_t DoClass(const char **pointer, const char *endOfText, const char *headerFilePath,time_t modificationTime);
	
	// fonction qui prépare le stockage d'une classe (gestion des doubles etc...)
	unsigned int ReadyToWriteClass(BString &className, unsigned int chunkSize);
		// retourne la position du début de l'écriture
};

#endif //CSPARSER