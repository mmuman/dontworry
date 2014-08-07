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
 * + les inline, static sont ignorés, ainsi que les passages par référence (&)
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

#include "CSParser.h"
#include "CSClass.h"			// pour les constances CSMT_...
#include "CSLibrary.h"		// pour la constante CSP_NOT_MODIFIED
#include <stdio.h>
#include <ctype.h>

#include <String.h>
#include <File.h>
#include <Directory.h>
#include <Entry.h>
#include <Path.h>
#include <NodeInfo.h>

//#define PRINT_PARSE_DEBUG

// macro pour le débug
#ifdef PRINT_PARSE_DEBUG
#define DBG(x) printf(x)
#define DBG2(x,y) printf(x,y)
#else
#define DBG(x) 
#define DBG2(x,y) 
#endif

CSParser::CSParser(BPositionIO *data)
{
	m_Data = data;
	m_InitCheck = B_ERROR;
	
	// on vérifie le header du fichier
	unsigned int l_Type,l_Size;
	if (!ChunkInfo(0,&l_Type,&l_Size)|| (l_Type != g_CFile))
	{
		// va pa...
		// on recrée un header propre
		m_EndClassChunk = 0;
		if (ChunkUpdate(&m_EndClassChunk,g_CFile,0) &&
			ChunkUpdate(&m_EndClassChunk,g_CClassList,0))
		{
			m_BegClassChunk = m_EndClassChunk;
			m_InitCheck = B_OK;
		}
	}
	else
	{
		// chargement des données contenues dans le fichier
		unsigned int l_Pointer = g_ChunkHeaderSize;
		m_EndClassChunk = 0;
		while (ChunkInfo(l_Pointer,&l_Type,&l_Size))
		{
			// on est tombés sur quel chunk?
			switch(l_Type)
			{
				case g_CClassList:	// liste de chunks 'CLAS'
				{
					// on repère le début du chunk
					m_BegClassChunk = l_Pointer+g_ChunkHeaderSize;
					
					// on va à la fin du chunk et on place le pointeur
					// m_EndClassChunk dessus
					if (!ChunkSkip(&l_Pointer))
						return;
					
					m_EndClassChunk = l_Pointer;
					break;
				}
				
				case g_CClassNames:	// espace de stockage des chaînes
							// de caractères pour les noms de classes
				{
					// on charge le chunk 'CLNM', dans m_ClassNamesData
					m_ClassNamesData.SetSize(l_Size);
			
					if (!ChunkRead(&l_Pointer,(void*)m_ClassNamesData.Buffer()))
						return;
					break;
				}
				
				case g_CNamesList:	// liste classée des noms de classes
				{
					// on charge la liste et on passe au chunk suivant
					if ((!m_ClassNamesList.Init(this,l_Pointer)) ||
						(!ChunkSkip(&l_Pointer)))
						return;
					break;
				}
				
				default:
				{
					// on saute simplement le chunk
					if (!ChunkSkip(&l_Pointer))
						return;
				}
			}

		}
		
		// si m_EndClassChunk n'a pas été positionné, c'est qu'on a pas trouvé le chunk 'LCLA' (boum)
		if (m_EndClassChunk == 0)
			return;
	}
	
	m_InitCheck = B_OK;
}

CSParser::~CSParser()
{
	// on va commencer par mettre à jour le chunk 'LCLA'
	// pour ça on va commencer par le chercher
	unsigned int l_Pointer = g_ChunkHeaderSize, l_Type=0, l_Size;
	if (!ChunkInfo(l_Pointer,&l_Type,&l_Size))
		// on ne devrait jamais arriver à ce point. On DOIT tomber sur le chunk 'LCLA' avant
		// la fin du fichier. Sinon c'est qu'il y a un gros problème...
		return;
			
	while (l_Type != g_CClassList)
	{
		// on passe au chunk suivant
		if ((!ChunkSkip(&l_Pointer)) || (!ChunkInfo(l_Pointer,&l_Type,&l_Size)))
			// même remarque que ci-dessus
			return;
	}
	
	// on est au début du chunk 'LCLA'. On peut mettre sa taille à jour
	if (!ChunkUpdate(&l_Pointer,g_CClassList,m_EndClassChunk-l_Pointer-g_ChunkHeaderSize))
		return;
	
	// écriture du chunk 'CLNM'
	if (!ChunkWrite(&l_Pointer,g_CClassNames,m_ClassNamesData.Buffer(),m_ClassNamesData.Seek(0,SEEK_END)))
		return;
	
	// et enfin du chunk 'LIST'
	if (!m_ClassNamesList.Write(this,&l_Pointer))
		return;
	
	// mise à jour du chunk principal
	unsigned int l_BegPointer=0;
	if (!ChunkUpdate(&l_BegPointer,g_CFile,l_Pointer-g_ChunkHeaderSize))
		return;
	
	// tout s'est bien passé!
	m_Data->SetSize(l_Pointer);
}

// fonctions utilitaires pour le parsing
/// recherche de mot
bool CSParser::ExtractWord(BString &word,const char **pointer, const char *endOfText, bool *isOperator)
{
	// n'est-on pas à la fin du fichier?
	if (*pointer >= endOfText)
		return false;
		
	// débord on vire les espaces du début, et les commentaires
	while(isspace(**pointer) || **pointer == '/')
	{
		// s'agit-il d'un commentaire?
		if (**pointer == '/')
		{
			// peut-être...
			
			// s'agit-il d'un commentaire en '//'?
			if (*(*pointer+1) == '/')
				// oui
				SkipToEndOfLine(pointer,endOfText);
			// ou d'un commentaire en '/*'?
			else if (*(*pointer+1) == '*')
			{
				// oui... recherche du */
				(*pointer)++;
				if (*pointer >= endOfText)
					return false;
				(*pointer)++;
				
				while ((**pointer != '*') || (*(*pointer+1) != '/'))
				{
					if (*pointer >= endOfText)
						return false;
					
					(*pointer)++;
				}
				*pointer += 2; 
			}
			else
			// non, ce n'est pas un commentaire. On est devant un authentique opérateur
				break;
				
			// n'est-on pas à la fin du fichier?
			if (*pointer >= endOfText)
				return false;
		}
		else
		{
			// c'est un espace tout bête... on passe au caractère suivant
			(*pointer)++;
			if (*pointer >= endOfText)
				return false;
		}
	}
		
	// maintenant pointer pointe sur un caractère utile
	// s'agit-il d'un mot?
	if (isalnum(**pointer) || **pointer=='_')
	{
		const char *l_Begin = *pointer;

		// oui, on va en chercher la fin
		while (isalnum(**pointer) || **pointer=='_')
		{
			(*pointer)++;
			if (*pointer >= endOfText)
				return false;
		}
		word.SetTo(l_Begin,*pointer-l_Begin);
		*isOperator = false;
		return true;
	}
	else
	// non
	{
		// est-ce une chaîne de caractères?
		if ((**pointer == '"') && (*(*pointer-1) != '\\') && (*(*pointer-1) != '\''))	// 39 c'est l'apostrophe
		{
			// oui
			const char *l_Begin = *pointer;
			SkipToEndOfString(pointer,endOfText);
			
			word.SetTo(l_Begin,*pointer-l_Begin);
			*isOperator = false;
			return true;
		}
		
		// non, c'est bien un opérateur
		word.SetTo(*pointer,1);
		(*pointer)++;
		*isOperator = true;
		return true;
	}			
}

/// va à la ligne suivante
void CSParser::SkipToEndOfLine(const char **pointer, const char *endOfText)
{
	while(**pointer != '\n')
	{
		if (*pointer >= endOfText)
			return;
		(*pointer)++;
	}
	(*pointer)++;
}

/// va à l'instruction suivante
void CSParser::SkipToNextInstruction(const char **pointer, const char *endOfText)
{
	bool l_IsOperator=false;
	BString l_Word;
	
	while (!l_IsOperator || l_Word != ";")
	{
		if (!ExtractWord(l_Word,pointer,endOfText,&l_IsOperator))
			return;
		
		if (l_IsOperator && l_Word == "\"")
			// on est tombés sur un ", donc une chaîne de caractères... on cherche la fin
			SkipToEndOfString(pointer,endOfText);
		else if (l_IsOperator && l_Word == "{")
			// on est au début d'un bloc logique... on va en chercher la fin
			SkipToClosingBracket(pointer,endOfText);
	}
}

/// va à la fin d'un bloc logique
void CSParser::SkipToClosingBracket(const char **pointer, const char *endOfText)
{
/*	bool l_IsOperator=false;
	BString l_Word;
	
	while (!l_IsOperator || l_Word != "}")
	{
		if (!ExtractWord(l_Word,pointer,&l_IsOperator))
			return;
		
		if (l_IsOperator && l_Word == "\"")
			// on est tombés sur un ", donc une chaîne de caractères... on cherche la fin
			SkipToEndOfString(pointer);
		else if (l_IsOperator && l_Word == "{")
			// on est un autre bloc logique à l'intérieur de celui-ci. On va le sauter aussi
			SkipToClosingBracket(pointer);
	}
*/
	while(**pointer != '}')
	{
		if (*pointer >= endOfText)
			return;
			
		char l_Char = *(*pointer)++;
		
		switch(l_Char)
		{
			case 0:
				(*pointer)--;
				return;
				break;
			
			case '"':
				SkipToEndOfString(pointer,endOfText);
				break;
			
			case '{':
				SkipToClosingBracket(pointer,endOfText);
				break;
				
			case '/':
				// peut-être un commentaire
				if (**pointer=='/')
					// oui, un commentaire sur une ligne
					SkipToEndOfLine(pointer,endOfText);
				else if (**pointer=='*')
				{
					// oui, un commentaire en /* */
					(*pointer)++;
					while (((**pointer) != '*') || (*(*pointer+1) != '/'))
					{
						if (*pointer >= endOfText)
							return;
						(*pointer)++;
					}
					*pointer += 2;
				}
				break;
		}
	}
	
	// on passe au caractère après le '}'
	(*pointer)++;
}

/// va à la fin d'une chaîne de caractères
void CSParser::SkipToEndOfString(const char **pointer, const char *endOfText)
{
	for (;;)
	{
		(*pointer)++;

		while(**pointer != '"')
		{
			if (*pointer >= endOfText)
				return;
		
			(*pointer)++;
		}

		// on vérifie que c'est pas un \" dans la chaîne
		if (*(*pointer-1) != '\\')
		{
			// on est bien à la fin de la chaîne
			(*pointer)++;
			return;
		}
		// c'était un \"... on continue la boucle infinie
	}
}

unsigned int CSParser::CreateTempString(const char *string,BMallocIO &space)
{
	unsigned int l_Index = space.Position();
	space.Write(string,strlen(string)+1);

	return l_Index;
}

// Parsing
status_t CSParser::Parse(const char *path, bool directory, unsigned int recurse_dirs, bool savePath)
{
	// si on ne veut faire que le parsing d'un fichier, on appelle directement DoFile
	if (directory == false)
		return DoFile(path,savePath);
	
	// bon, ben à priori c'est un répertoire...
	BDirectory l_Dir(path);
	
	// on boucle dans son contenu...
	status_t l_Error;
	BEntry l_Entry;
	while ((l_Error = l_Dir.GetNextEntry(&l_Entry,true)) == B_OK)
	{
		BPath l_Path;
		l_Entry.GetPath(&l_Path);
		// est-on tombés sur un répertoire?
		if (l_Entry.IsDirectory())
		{
			// oui... a-t-on le droit d'y aller?
			if (recurse_dirs>0)
			{
				// oui... on y va
				DBG2("On entre dans le répertoire %s\n",l_Path.Path());

				l_Error = Parse(l_Path.Path(),true,recurse_dirs-1,savePath);
				if (l_Error != B_OK)
					return l_Error;
			}
		}
		else
		{
			// non, c'est un fichier. Est-il du bon type?
			BFile l_File(&l_Entry,B_READ_ONLY);
			BNodeInfo l_NodeInfo(&l_File);
			char l_MimeType[B_MIME_TYPE_LENGTH+1];
			l_NodeInfo.GetType(l_MimeType);
			if (strcmp(l_MimeType,"text/x-source-code") == 0)
			{
				// c'est bien un fichier source. ça doit être un header!
				DBG2("On parse le fichier %s\n",l_Path.Path());
				DoFile(l_Path.Path(),savePath);
			}
			else
			{
				DBG2("Le fichier %s n'est pas du bon type\n",l_Path.Path());
			}
		}	
	}
	
	// pourquoi est-on sorti du while?
	// si c'est parce qu'on est arrivés à la fin du répertoire, c'est normal
	if (l_Error == B_ENTRY_NOT_FOUND)
		l_Error = B_OK;
		
	return l_Error;
}

status_t CSParser::DoFile(const char *path, bool savePath)
{
	BFile l_File(path,B_READ_ONLY);
	if (l_File.InitCheck() != B_OK)
		return l_File.InitCheck();
		
	// chargement du fichier
	off_t l_Size;
	l_File.GetSize(&l_Size);
	char *l_Text = new char[l_Size];
	if (l_File.Read(l_Text,l_Size) != l_Size)
	{
		delete l_Text;
		return B_ERROR;
	}

	// parsing proprement dit
	time_t l_ModificationTime=0;
	if (savePath)
		l_File.GetModificationTime(&l_ModificationTime);
		
	status_t l_Error = Parse(l_Text,l_Size,(savePath?path:NULL),l_ModificationTime);
	
	// on détruit les données qu'on avait chargé
	delete l_Text;
	return l_Error;
}	

status_t CSParser::Parse( const char *text, unsigned int textLength, const char *path,time_t modificationTime)
{
	const char *l_Pointer=text,*l_EndOfText = text+textLength;
	BString l_Word;
	bool l_IsOperator;
	status_t l_Error = B_OK;
	
	while (ExtractWord(l_Word,&l_Pointer,l_EndOfText,&l_IsOperator))
	{
		if (l_IsOperator && l_Word=="#")
		{
			// on est sur un #define, ou #include etc...
			// on passe à la ligne suivante
			SkipToEndOfLine(&l_Pointer,l_EndOfText);
			continue;
		}
		
		if (!l_IsOperator && l_Word=="class")
		{
			l_Error = DoClass(&l_Pointer,l_EndOfText,path,modificationTime);
			
			if (l_Error != B_OK)
				return l_Error;
			continue;
		}
		
		// sinon, ce n'est pas une nouvelle classe... on saute l'instruction
		if (!l_IsOperator)
		{
			SkipToNextInstruction(&l_Pointer,l_EndOfText);
			continue;
		}
	}
	
	return B_OK;
}

status_t CSParser::DoClass(const char **pointer, const char *endOfText, const char *headerFilePath, time_t modificationTime)
{
	DBG("\n\n****\nDébut de parsing de classe:");
	
	BString l_Word,l_ClassName;
	bool l_IsOperator;
	
	// blocs de mémoire qui vont stocker toutes les infos sur la classe
	BMallocIO l_CInfo,l_CMethods,l_CMemberVariables,l_CStrings,l_Friends;
	off_t l_CMethodsOffset=0;	// offset vers l'espace qui stocke la méthode en cours
	unsigned char l_NumFathers = 0, l_NumFriends = 0;
	unsigned short l_NumMemberVariables = 0;	
		// compteurs qui seront utilisés par la suite
	// ces deux variables servent à stocker temporairement des entiers sur 8 et 32 bits avant de les stocker dans le fichier
	unsigned int l_TempInt;
	unsigned char l_TempChar;
	
	// pointeur relatif vers le chemin du fichier (on utilisera ça si headerFilePath est NULL)
	unsigned int l_PointerToPath = 0xffffffff;
	
	// booléen qui sert à savoir si le fichier a été modifié depuis le dernier parsing
	bool l_FileModified = true;
	
	// on extrait le nom de la classe
	if (!ExtractWord(l_ClassName,pointer,endOfText,&l_IsOperator) || l_IsOperator)
		// pb. Soit on est à la fin du fichier, soit on est derrière un truc bizarre
	{
		DBG("Erreur: ne peut pas trouver le nom\n");
		return B_ERROR;
	}
	
	DBG2("nom: %s\n",l_ClassName.String());

	// on regarde si le fichier header a été modifié depuis le dernier parsing
	unsigned int l_ClassPointer = m_ClassNamesList.FindName(l_ClassName.String(),&m_ClassNamesData);
	if (l_ClassPointer != 0xffffffff)
	{
		// on va chercher la date dans le chunk INFO
		l_ClassPointer+= m_BegClassChunk+g_ChunkHeaderSize;
		unsigned int l_Type,l_Size;
		if ((ChunkInfo(l_ClassPointer,&l_Type,&l_Size) && (l_Type == g_CClassInfo)))
		{
			// on va se mettre au bon endroit pour lire la date de modification
			l_ClassPointer += g_ChunkHeaderSize;
			unsigned int l_ModTimePointer = l_ClassPointer+16;
			
			// on ajoute ce qu'il faut pour les classes père
			unsigned char l_TempByte;
			m_Data->ReadAt(l_ClassPointer+10,&l_TempByte,1);
			l_ModTimePointer += l_TempByte*4;
			
			// et les classes amies
			m_Data->ReadAt(l_ClassPointer+11,&l_TempByte,1);
			l_ModTimePointer += l_TempByte*4;
			
			// on vérifie qu'on n'est pas sorti du chunk info (si par exemple on a un fichier de version antérieure à la 6)
			if (l_ModTimePointer-l_ClassPointer+4 <= l_Size)
			{
				if(headerFilePath != NULL)
				{
					// on peut lire la date de modif!
					time_t l_ModTime;
					m_Data->ReadAt(l_ModTimePointer,&l_ModTime,4);
					
					// alors, faut-il faire la mise à jour?
					if (l_ModTime >= modificationTime)
						// non
						l_FileModified = false;
				}
				else
				{
					// on va lire l'ancien chemin du fichier, et prendre ça quand on stockera la classe
					m_Data->ReadAt(l_ModTimePointer-4,&l_PointerToPath,4);
				}
			}
		}
	}
	
	// mise en forme du chunk INFO
	l_TempInt = m_ClassNamesList.AddName(l_ClassName.String(),&m_ClassNamesData);
	l_CInfo.Write(&l_TempInt,4);
	l_TempInt = 0xffffffff;	// ce n'est pas un typedef
	l_CInfo.Write(&l_TempInt,4);
	l_TempInt = 0;		// on remplira ces valeurs plus tard
	l_CInfo.Write(&l_TempInt,4);
	
	// pareil pour le chunk MVRS (variables membres). On remplira ces 4 octets plus tard
	l_CMemberVariables.Write(&l_TempInt,4);
	
	// on cherche le début de la classe, et les classes dont elle hérite
	while (!l_IsOperator || l_Word != "{")
	{
		if (!ExtractWord(l_Word,pointer,endOfText,&l_IsOperator))
		{
			DBG("Erreur: fin prématurée de la déclaration\n");
			return B_ERROR;
		}
		
		if (!l_IsOperator && (l_Word != "public"))
		{
			DBG2("Hérite de: %s\n",l_Word.String());
			
			// héritage
			l_TempInt = m_ClassNamesList.AddName(l_Word.String(),&m_ClassNamesData);
			l_CInfo.Write(&l_TempInt,4);
			l_NumFathers++;
		}
		
		if (l_IsOperator && l_Word==";")
		{
			DBG("Simple déclaration, pas définition... dommage\n");
			return B_OK;
		}
		else
			if (!l_FileModified)
				// c'est une définition de classe, et le fichier n'a pas été modifié... on sort
				return CSP_NOT_MODIFIED;

		if (l_IsOperator && l_Word=="#")
		{
			// on est sur un #define, ou #include etc...
			// on passe à la ligne suivante
			SkipToEndOfLine(pointer,endOfText);
		}
	}
	
	// on est au début de la classe!!
	BString l_Type;		// cette chaîne va servir à retenir les types de variables
	BString l_Name;	// et celle-ci les noms (de variables ou de fonction)
	unsigned char l_PointerLevel = 0;
				// cet entier va stocker le niveau de pointeur
	unsigned char l_NumArguments = 0;
				// et celui-là le nombre d'arguments d'une fonction
	unsigned int l_State = 0;
				// et celui-là l'état en cours: 
				// 	0: on lit un nom de fonction/variable membre
				//	1: on lit un nom d'argument
				//	2: on lit la valeur par défaut de l'argument
				//	3: c'est la fin de la fonction (on attend un ; ou un {..} )
				//	4: on a lu un 'friend', on attend un 'class'
				//	5: on a lu un 'friend' puis un 'class', on attend le nom de la classe amie
				//	6: on a lu le nom d'une variable membre, on attend un ',' ou un ';'
	unsigned int l_Part = 0;
				// enfin celui-là la partie du nom de fonction ou d'argument qu'on 
				// est en train de retirer:
				//	0: on lit le type (de retour pour une fonction)
				//	1: on lit le nom
	unsigned char l_Zone = CSMT_PRIVATE;
				// pour savoir si on est en zone privée/pubique/protégée
	bool l_Virtual = false;
				// mis à true si la méthode qu'on lit en ce moment est virtuelle
	bool l_Const = false;	// mis à true si le type de retour ou de paramètre est constant
	bool l_Ref = false;	// mis à trus si le type de retour ou de paramètre est une référence
				
	// on boucle jusqu'à ce qu'on arrive à la fin de la classe
	while(!l_IsOperator || (l_Word != "}"))
	{
		if (!ExtractWord(l_Word,pointer,endOfText,&l_IsOperator))
		{
			DBG("Erreur:Fin prématurée...\n");
			return B_ERROR;
		}
		
		// petite exception pour les mots clefs public, protected et private
		// dans ce cas on note le changement de zone
		if (!l_IsOperator && (l_State==0))
		{
			if (l_Word == "public")
			{
				DBG("Passage en zone publique\n");
				l_Zone = CSMT_PUBLIC;
				continue;
			}
			else if (l_Word == "protected")
			{
				DBG("Passage en zone protégée\n");
				l_Zone = CSMT_PROTECTED;
				continue;
			}
			else if (l_Word == "private")
			{
				DBG("Passage en zone privée\n");
				l_Zone = CSMT_PRIVATE;
				continue;
			}
		}
		
		// autres mots à ignorer
		if (!l_IsOperator && ((l_Word == "static") || (l_Word == "inline")))
			continue;
		
		// si c'est un mot, on l'ajoute à ce qu'on est en train de stocker
		if (!l_IsOperator)
		{
			// gestion des mots spéciaux "virtual" et "const"
			if (l_Word == "virtual")
			{
				l_Virtual = true;
				continue;
			}
			if (l_Word == "const")
			{
				l_Const = true;
				continue;
			}

			switch (l_State)
			{
				case 0:
				case 1:
				case 2:
				case 3:
				{
					if (l_Part == 0)
					{
						// gestion des classes amies
						if (l_Word == "friend")
						{
							DBG("Class friend: 4\n");
							l_State = 4;
						}
						else
						{
							// on stocke le type
							l_Type += l_Word;
							
							// si on a stocké "unsigned", c'est que le vrai type va suivre
							// sinon c'est le nom qui va suivre
							if (l_Word == "unsigned")
								l_Type += " ";
							else
								l_Part = 1;
						}
					}
					else if (l_Part == 1)
					{
						// on stocke le nom
						l_Name += l_Word;
					}
				}
				break;
				
				// gestion des classes amies... on a vu un 'friend', a-ton un 'class'?
				case 4:
				{
					DBG2("Class friend: 5, trouvé '%s'\n",l_Word.String());
					if (l_Word == "class")
					{
						l_State = 5;
					}
					else
						l_State = 0;
				}
				break;
				
				// on a eu 'friend class', donc maintenant on a le nom de la classe amie
				case 5:
				{
					// on a un ami!!!!
					DBG2("Class friend: '%s'\n", l_Word.String());
				
					// on stoque le nom de cette classe amie
					l_TempInt = m_ClassNamesList.AddName(l_Word.String(),&m_ClassNamesData);
					l_Friends.Write(&l_TempInt,4);
					l_NumFriends++;
				}
				break;
				
				// dans les autres cas, on ne fait rien
				case 6:
				default:
					break;
			}
		}
		else
		{
			// ce n'est pas un mot. c'est un opérateur

			if (l_Word=="#")
			{
				// on est sur un #define, ou #include etc...
				// on passe à la ligne suivante
				SkipToEndOfLine(pointer,endOfText);
				continue;
			}
			
			//Si c'est un *, ça change le niveau de pointeur, sauf si on redéfinit l'opérateur
			if ((l_Word == "*") && ((l_Part == 0) || (l_Word != "operator")))
			{
				l_PointerLevel++;
				continue;
			}
			
			if (l_Word == "&")
			{
				l_Ref = true;
				continue;
			}
			
			// sinon on va voir si ça ne peut pas nous faire changer d'état
			bool l_Clear = false;	// à mettre à true s'il faut effacer Type,Name et PointerLevel
			switch(l_State)
			{
			case 0:
				// 	0: on lit un nom de fonction/variable membre
				if (l_Word == "~")
				{
					// ça doit être un destructeur
					l_Type = "~";
				}
				else if (l_Word == "[")
				{
					// c'est un tableau
					l_Part=2;	
						// on ne retient plus les mots qu'on va trouver, puisqu'ils
						// seront la taille du tableau
					l_PointerLevel++;
				}
				else if ((l_Word == ";") || (l_Word == "=") || (l_Word == ","))
				{
					// c'est une variable membre
					DBG2("Variable: %s",l_Name.String());
					DBG2(", type: %s",l_Type.String());
					if (l_Const)
						DBG(" (const)");
					DBG2(", niveau de pointeur: %d\n",(int)l_PointerLevel);
					
					// on stocke tout ça
					l_TempInt = m_ClassNamesList.AddName(l_Type.String(),&m_ClassNamesData);
					l_CMemberVariables.Write(&l_TempInt,4);
					/// nom
					if (l_Name != B_EMPTY_STRING)
						l_TempInt = CreateTempString(l_Name,l_CStrings);
					else
						l_TempInt = 0xffffffff;
					l_CMemberVariables.Write(&l_TempInt,4);
					
					// infos diverses
					/// niveau de pointeur
					l_CMemberVariables.Write(&l_PointerLevel,1);
					
					/// flag const (pas fait encore)
					l_TempChar = (l_Const?1:0);
					l_CMemberVariables.Write(&l_TempChar,1);
					
					/// zone
					l_CMemberVariables.Write(&l_Zone,1);
					
					/// réservé
					l_TempChar = 0;
					l_CMemberVariables.Write(&l_TempChar,1);
					
					l_NumMemberVariables++;
					
					// maintenant ce qu'il reste dépend de l'opérateur qu'on a vu
					if (l_Word == ";")
					{
						// retour à l'état initial
						l_Clear = true;
					}
					else if (l_Word == ",")
					{
						// il y aura une autre variable
						l_Name=B_EMPTY_STRING;
					}
					else if (l_Word == "=")
					{
						// on donne une valeur par défaut. on attend jusqu'au ',' ou ';' suivant
						l_State=6;
					}
				}
				else if (l_Word == "(")
				{
					// c'est une fonction
					
					// si c'est un constructeur ou un destructeur, il n'y a pas de
					// type de retour, et le parseur a pris le nom des fonctions
					// pour un type
					if (l_Name == B_EMPTY_STRING)
					{
						l_Name = l_Type;
						l_Type = B_EMPTY_STRING;
					}
						
					DBG2("Fonction: %s",l_Name.String());
					if (l_Virtual)
						DBG(" (virtual)");
					DBG2(", type de retour: %s",l_Type.String());
					if (l_Const)
						DBG(" (const)");
					if (l_Ref)
						DBG("(&)");
					DBG2(", niveau de pointeur: %d\n",l_PointerLevel);
					
					// on stocke tout ça
					/// création d'un nouveau chunk
					l_TempInt = g_CClassMethod;
					l_CMethods.Write(&l_TempInt,4);
						// taille encore inconnue
					l_TempInt = 0;
					l_CMethods.Write(&l_TempInt,4);
					
					/// stockage des autres informations
					l_TempInt = CreateTempString(l_Name,l_CStrings);
					l_CMethods.Write(&l_TempInt,4);
					l_TempInt = m_ClassNamesList.AddName(l_Type.String(),&m_ClassNamesData);
					l_CMethods.Write(&l_TempInt,4);
					l_CMethods.Write(&l_PointerLevel,1);
					/// const et virtual
					l_TempChar = (l_Const?1:0) | (l_Virtual?2:0) | (l_Ref?4:0);
					l_CMethods.Write(&l_TempChar,1);
					/// le nombre d'arguments on ne connait pas encore
					l_TempChar= 0;
					l_CMethods.Write(&l_TempChar,1);
					/// mais la zone on connait
					l_CMethods.Write(&l_Zone,1);
									
					// passage à l'état 1
					l_State = 1;
					l_Clear = true;
				}
					// gestion du nom spécial 'operator'
				else if ((l_Part == 1) && (l_Name.Compare("operator",8) == 0))
					l_Name += l_Word;
				break;
			
			case 1:
				//	1: on lit un nom d'argument
				{
					bool l_EndOfArgument = false;
					if (l_Word == ",")
					{
						// fin de l'argument, on passe au suivant
						l_EndOfArgument = true;
						l_Clear = true;
					}
					else if (l_Word == "=")
					{
						// il y a une valeur par défaut
						l_EndOfArgument = true;
						l_State = 2;
						l_Clear = true;
					}
					else if (l_Word == "[")
					{
						// c'est un tableau
						l_Part=2;	
							// on ne retient plus les mots qu'on va trouver, puisqu'ils
							// seront la taille du tableau
						l_PointerLevel++;
					}
					else if ((l_Word == ")") || (l_Word == ";") || (l_Word == "}"))
					{
						// c'est le dernier argument de la fonction
						if ((l_Name == "") && (l_Type == ""))
						{
							// en fait la fonction est vide
							DBG("  + fonction vide\n");
						}
						else
							l_EndOfArgument = true;
						
						// on passe en état 3 (fin de fonction)
						l_State = 3;
						l_Clear = true;
					}
					
					if (l_EndOfArgument)
					{
						DBG2("  + argument: %s",l_Name.String());
						DBG2(", type: %s",l_Type.String());
						if (l_Const)
							DBG(" (const)");
						if (l_Ref)
							DBG("(&)");
						DBG2(", niveau de pointeur: %d\n",l_PointerLevel);
						
						// on stocke les infos sur l'argument
						l_TempInt = m_ClassNamesList.AddName(l_Type.String(),&m_ClassNamesData);
						l_CMethods.Write(&l_TempInt,4);
						l_TempInt = CreateTempString(l_Name,l_CStrings);
						l_CMethods.Write(&l_TempInt,4);
						l_TempInt = 0xffffffff;	// la valeur par défaut sera réglée plus tard si nécessaire
						l_CMethods.Write(&l_TempInt,4);
						l_CMethods.Write(&l_PointerLevel,1);
						l_TempChar = (l_Const?1:0) | (l_Ref?2:0);
						l_CMethods.Write(&l_TempChar,1);
							// le reste est à zéro
						l_TempInt = 0;
						l_CMethods.Write(&l_TempInt,2);
						l_NumArguments++;
					}
				}
				break;
			
			case 2:
				//	2: on lit la valeur par défaut de l'argument
				if (l_Word == ",")
				{
					// on passe à l'argument suivant
					l_State = 1;
					l_Clear = true;
				}
				else if ((l_Word == ")") || (l_Word == ";") || (l_Word == "}"))
				{
					// c'est la fin de la fonction
					l_State = 3;
					l_Clear = true;
				} 
				else
				{
					// c'est un opérateur qui rentre dans le calcul de la valeur par défaut
					l_Name << l_Word;
				}
				
				if (l_Clear)
				{
					l_Type << l_Name;
					DBG2("   + valeur par défaut:%s\n",l_Type.String());
					
					// pour stocker la valeur par défaut il faut revenir un peu en arrière
					l_CMethods.Seek(-8,SEEK_CUR);
					l_TempInt = CreateTempString(l_Type,l_CStrings);
					l_CMethods.Write(&l_TempInt,4);
					l_CMethods.Seek(4,SEEK_CUR);
				}
				break;

			case 3:
				//	3: c'est la fin de la fonction (on attend un ; ou un {..} )
				if (l_Word == ";")
				{
					// c'est bon, on est à la fin... on repasse à l'état de départ
					l_State = 0;
					l_Clear = true;
				}
				else if (l_Word == "{")
				{
					// la fonction est définie ici... on va à la fin, et on repasse
					// à l'état de départ
					SkipToClosingBracket(pointer,endOfText);
					l_State = 0;
					l_Clear = true;
				}
				
				if (l_Clear)
				{
					// on est arrivé à la fin de la fonction, il faudrait remettre un peu d'ordre...
					off_t l_EndOfMethod = l_CMethods.Position();
					
					// mise à jour de la taille du chunk 'METH'
					l_TempInt = l_EndOfMethod - l_CMethodsOffset;
					l_CMethods.WriteAt(l_CMethodsOffset+4,&l_TempInt,4);
					
					// mise à jour du nombre d'arguments
					l_CMethods.WriteAt(l_CMethodsOffset+g_ChunkHeaderSize+10,&l_NumArguments,1);
					l_NumArguments = 0;
					
					// et enfin mise à jour de l'offset vers la méthode en cours de décryptage dans
					// le chunk 'METS'
					l_CMethodsOffset = l_EndOfMethod;
				}
				break;

			case 4:
			case 5:
				// on était en train de gérer un 'friend class'
				l_State = 0;
				DBG("Class friend: abandon\n");
				break;
				
			case 6:
				// on ne sort de cet état qu'avec un ',' ou un ';'
				if (l_Word == ",")
				{
					// il y aura d'autres variables du même type
					l_State=0;
					l_Name=B_EMPTY_STRING;
				}
				else if (l_Word == ";")
				{
					// c'est fini pour cette variable
					l_Clear = true;
					l_State = 0;
				}
				break;
				
			default:
				DBG("Erreur... je ne sais pas comment je suis arrivé là\n");
				break;
			}
			
			if (l_Clear)
			{
				l_Type = B_EMPTY_STRING;
				l_Name = B_EMPTY_STRING;
				l_PointerLevel = 0;
				l_Part = 0;
				l_Const = false;
				l_Virtual = false;
			}
		}			
		
	}	
	
	DBG("Fin de la visite de la classe\n");
	
	// mise à jour du chunk 'INFO'
	/// nombre de classes père
	l_CInfo.WriteAt(10,&l_NumFathers,1);
	
	/// et nombre d'amis
	l_CInfo.WriteAt(11,&l_NumFriends,1);
	
	/// les classes amies
	if (l_NumFriends != 0)
	{
		l_CInfo.Write(l_Friends.Buffer(),l_Friends.BufferLength());
	}
	
	/// le champ qui contient le chemin vers le fichier
	l_CInfo.Seek(0,SEEK_END);
	if (headerFilePath != NULL)
		l_PointerToPath = CreateTempString(headerFilePath,l_CStrings);
	l_CInfo.Write(&l_PointerToPath,4);
	
	/// la date de modification du fichier header
	l_CInfo.Write(&modificationTime,4);
	
	// mise à jour du chunk 'MVRS'
	l_CMemberVariables.WriteAt(0,&l_NumMemberVariables,2);
	
	// et enfin on écrit tous ces chunks dans le fichier, dans un gros chunk 'CLAS'
	/// quelle taille fait tout le chunk?
	unsigned int l_ChunkSize = l_CInfo.Position()+l_CMethods.Position()+l_CMemberVariables.Position()+l_CStrings.Position()+5*g_ChunkHeaderSize;
	
	/// on prépare l'écriture du chunk
	unsigned int l_Pointer =   ReadyToWriteClass(l_ClassName,l_ChunkSize);
	
	/// écriture du header
	l_TempInt = g_CClass;
	m_Data->Write(&l_TempInt,4);
	m_Data->Write(&l_ChunkSize,4);
	
	/// écriture du chunk 'INFO'
	l_TempInt = g_CClassInfo;
	m_Data->Write(&l_TempInt,4);
	l_TempInt = l_CInfo.Position()+g_ChunkHeaderSize;
	m_Data->Write(&l_TempInt,4);
	m_Data->Write(l_CInfo.Buffer(),l_TempInt-g_ChunkHeaderSize);
	
	/// écriture du chunk 'METS'
	l_TempInt = g_CClassMethodsList;
	m_Data->Write(&l_TempInt,4);
	l_TempInt = l_CMethods.Position()+g_ChunkHeaderSize;
	m_Data->Write(&l_TempInt,4);
	m_Data->Write(l_CMethods.Buffer(),l_TempInt-g_ChunkHeaderSize);

	/// écriture du chunk 'MVRS'
	l_TempInt = g_CClassMemberVariables;
	m_Data->Write(&l_TempInt,4);
	l_TempInt = l_CMemberVariables.Position()+g_ChunkHeaderSize;
	m_Data->Write(&l_TempInt,4);
	m_Data->Write(l_CMemberVariables.Buffer(),l_TempInt-g_ChunkHeaderSize);
	
	/// écriture du chunk 'STRS'
	l_TempInt = g_CClassStrings;
	m_Data->Write(&l_TempInt,4);
	l_TempInt = l_CStrings.Position()+g_ChunkHeaderSize;
	m_Data->Write(&l_TempInt,4);
	m_Data->Write(l_CStrings.Buffer(),l_TempInt-g_ChunkHeaderSize);
	
	// mise à jour de la liste de classes, pour qu'elle pointe vers le nouveau chunk 'CLAS' qu'on vient de créer
	m_ClassNamesList.AddName(l_ClassName.String(),&m_ClassNamesData,l_Pointer-m_BegClassChunk);

	return B_OK;
}

unsigned int CSParser::ReadyToWriteClass(BString &className, unsigned int chunkSize)
{
	// la classe existe-t'elle déjà?
	unsigned int l_ClassPosition = m_ClassNamesList.FindName(className.String(),&m_ClassNamesData);
	if (l_ClassPosition == 0xffffffff)
	{
		// elle n'existe pas
		l_ClassPosition = m_EndClassChunk;
		m_Data->Seek(l_ClassPosition,SEEK_SET);
		
		// la zone liste de classes grandit
		m_EndClassChunk += chunkSize;
	}
	else
	{
		// elle existe...
		// bon, ben on va devoir y aller
		// déjà on récupère la vraie position dans m_Data
		l_ClassPosition += m_BegClassChunk;
		
		// quelle est la taille actuelle de la classe?
		unsigned int l_CurrentChunkSize;
		m_Data->ReadAt(l_ClassPosition+4,&l_CurrentChunkSize,4);
		
		// si la classe est déjà à la fin du buffer, ou si elle garde la même taille, on n'a rien à faire
		if ((l_ClassPosition+l_CurrentChunkSize >= m_EndClassChunk) ||
		 (l_CurrentChunkSize == chunkSize))
		 {
			m_Data->Seek(l_ClassPosition,SEEK_SET);
			if (l_CurrentChunkSize != chunkSize)
			{
				// c'est qu'on est à la fin du buffer.
				// si la taille de la classe a diminué... il faut diminuer la taille de la zone
				m_EndClassChunk = l_ClassPosition+chunkSize;
			}
		}
		else
		{
			// on va devoir déplacer les données situées après la classe
			/// allocation du buffer
			/// calcul de la quantité de données à déplacer
			unsigned int l_BufferLength = m_EndClassChunk-l_ClassPosition-l_CurrentChunkSize;
			char *l_Buffer = new char[l_BufferLength];
			
			/// lecture des données
			m_Data->ReadAt(l_ClassPosition+l_CurrentChunkSize,l_Buffer,l_BufferLength);
			
			/// écriture à leur nouvelle place
			m_Data->WriteAt(l_ClassPosition+chunkSize,l_Buffer,l_BufferLength);
			m_EndClassChunk = l_ClassPosition+chunkSize+l_BufferLength;
			
			/// destruction du buffer
			delete l_Buffer;
			
			// on se place au bon endroit dans le buffer
			m_Data->Seek(l_ClassPosition,SEEK_SET);
			
			// il faut décaler tous les pointeurs
			m_ClassNamesList.Offset(l_ClassPosition,chunkSize-l_CurrentChunkSize);
		}
	}
	
	return l_ClassPosition;
}
