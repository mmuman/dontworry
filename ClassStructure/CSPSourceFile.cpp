/*
 * Parseur c++ pour keyspy
 *
 * CSPSourceFile: objet qui représente une source, et qui sait le parser (enfin en théorie ;-)
 *
 */

#include "CSPSourceFile.h"
#include "CSPFunction.h"
#include "CSPBlock.h"
#include "CSPVariable.h"

#include <String.h>
#include <ctype.h>

#ifdef DEBUG
#include <stdio.h>
#define _D1(x) printf(x)
#define _D2(x,y) printf(x,y)
#define _D3(x,y,z) printf(x,y,z)
#else
#define _D1(x)
#define _D2(x,y)
#define _D3(x,y,z)
#endif

CSPSourceFile::CSPSourceFile()
{
	m_CurrentFunction = NULL;
	m_CurrentBlock = NULL;
}

CSPSourceFile::~CSPSourceFile()
{
	Empty();
}

void CSPSourceFile::Empty()
{
	int l_NBlocks = m_LonelyBlocks.CountItems();
	
	for(int i=0; i<l_NBlocks; i++)
	{
		delete (CSPBlock*)m_LonelyBlocks.RemoveItem((int32)0);
	}

	int l_NFuncs = m_Functions.CountItems();
	
	for(int i=0; i<l_NFuncs; i++)
	{
		delete (CSPFunction*)m_Functions.RemoveItem((int32)0);
	}

	int l_NGlobVars = m_GlobalVariables.CountItems();
	
	for(int i=0; i<l_NGlobVars; i++)
	{
		delete (CSPVariable*)m_GlobalVariables.RemoveItem((int32)0);
	}
	
	m_CurrentFunction = NULL;
	m_CurrentBlock = NULL;
}

// Parsing en lui même
// le parseur est une grosse machine d'états
// pas flemme je n'ai pas donné de noms à chaque étape. Norm	alement il devrait y avoir
// un schéma au format Gobe qui explique le fonctionnement de la machine.
void CSPSourceFile::Parse(const char *beginning, const char *end)
{
	BString l_Name1,l_Name2;		// chaînes avec les noms en cours de parsing
	BString l_Name3,l_Name4;		// pareil avec les paramètres de fonction
	BString l_ClassName;		// nom de classe identifié pour la fonction
	unsigned int l_State=1;		// numéro de l'état en cours
	const char *l_Pointer = beginning;
	unsigned int l_Recurs=0;		// compteur de récursivité pour les ( et )
	bool l_Func=false;			// flag qui sert à dire si ce qu'on est en train de regarder c'est une définition de fonction
	unsigned int l_PointerLevel=0;	// comptage du niveau de pointeur
	bool l_ParEmpty=true;		// flag qui sert à déterminer si les parenthèses sont vides
	BList l_ParamsList;			// liste des paramètres d'une fonction
	unsigned int l_Begin=0,l_Begin2=0;
						// pointeurs vers le début des objets en cours d'identification

	BString l_Word;			// mot identifié
	bool l_Operator;			// flag mis à true si le mot est un opérateur
	
	Empty();
		
	// machine à états
	while(ExtractWord(l_Word,&l_Pointer,end,&l_Operator))
	{
		if (l_Operator)
		{
			if (l_Word == "{")
			{
				// une exception: dans une déclaration de valeur par défaut, ce n'est pas un nouveau
				// bloc, juste une liste de valeurs
				if (l_State == 12)
				{
					SkipToClosingBracket(&l_Pointer,end);
					_D1("Sauté un {\n");
					continue;
				}
				_D2("{ (état était %d)\n",l_State);
				l_State = 1;
				
				// déclaration du début du bloc
				CSPBlock *l_NewBlock = new CSPBlock;
				
				// est-on dans un bloc père?
				if (m_CurrentBlock != NULL)
				{
					l_NewBlock->m_InBlock = m_CurrentBlock;
					l_NewBlock->m_InFunction = m_CurrentBlock->m_InFunction;
					l_NewBlock->m_PointerToBegin = l_Pointer-beginning-1-m_CurrentBlock->GetAbsolutePointerToBegin();
					m_CurrentBlock->m_SubBlocks.AddItem(l_NewBlock);
				}
				// dans une fonction alors?
				else if (m_CurrentFunction != NULL)
				{
					l_NewBlock->m_InFunction = m_CurrentFunction;
					l_NewBlock->m_PointerToBegin = l_Pointer-beginning-1-m_CurrentFunction->m_PointerToBegin;
					m_CurrentFunction->m_Blocks.AddItem(l_NewBlock);
				}
				else
				// non...
				{
					l_NewBlock->m_PointerToBegin = l_Pointer-beginning;
					m_LonelyBlocks.AddItem(l_NewBlock);
				}
				
				m_CurrentBlock = l_NewBlock;
				continue;
			}
			else if (l_Word == "}")
			{
				_D1("}\n");
				l_State = 1;
				
				// fin d'un bloc
				if (m_CurrentBlock != NULL)
				{
					// on fixe sa taille
					m_CurrentBlock->m_Size = l_Pointer-beginning-1-m_CurrentBlock->GetAbsolutePointerToBegin();
					
					// on met à jour le pointeur vers le bloc en cours
					m_CurrentBlock = m_CurrentBlock->m_InBlock;
				}
				else
				// on crée un bloc seul avec une taille 0
				{
					CSPBlock *l_NewBlock = new CSPBlock;
					l_NewBlock->m_PointerToBegin = l_Pointer-beginning;
					l_NewBlock->m_Size = 0;
					m_LonelyBlocks.AddItem(l_NewBlock);
				}
					
				continue;
			}
		}
		
		switch(l_State)
		{
			case 0:
				// on attend le ';'
				if (l_Operator && (l_Word == ";"))
					l_State = 1;
				break;
				
			case 1:
				// début
				if (l_Operator && l_Word=="#")
				{
					// directive de précompiation
					SkipToEndOfLine(&l_Pointer,end);
				}
				else if (!l_Operator)
				{
					// c'est un nom, on le stoque et on passe à l'étape suivante
					/// sauf si c'est "delete"
					if ((l_Word == "delete") || (l_Word == "if") || (l_Word == "for") || (l_Word == "while") || (l_Word == "return") || (l_Word == "else") || (l_Word == "switch") || (l_Word == "case") || (l_Word == "const"))
						// on ne fait rien
						l_State = 1;
					else
					{
						l_Name1 = l_Word;
						l_Name2 = "";
						l_PointerLevel = 0;
						l_Begin = l_Pointer-beginning;
						l_State = 2;
					}
				}
/*				else
					// c'est un opérateur... on va à la fin de l'instruction
					l_State = 0;
*/				break;
				
			case 2:
			case 3:
				// stoquage des noms
				if (l_Operator)
				{
					l_State = 0;		// si state n'a pas été changé, c'est
								// qu'on est dans un cas pas géré

					// on a donc un opérateur... c'est quoi?
					if (l_Name2 != "")
					{
						// ceux-ci ne marchent que si on a eu au moins 2 noms
						if (l_Word == ";")
						{
							l_State = 1;
							StoreVariable(l_Name1,l_Name2,l_PointerLevel,l_Begin);
							break;
						}
						else if (l_Word == ",")
						{
							l_State = 10;
							StoreVariable(l_Name1,l_Name2,l_PointerLevel,l_Begin);
							break;
						}
						else if (l_Word == "=")
						{
							l_State = 12;
							StoreVariable(l_Name1,l_Name2,l_PointerLevel,l_Begin);
							break;
						}
					}
					
					if (l_Word == ":")
					{
						l_ParEmpty = true;
						l_State = 30;
					}
					else if (l_Word == "(")
					{
						// s'il n'y a qu'un mot c'est une fonction, sinon ça peut être une variable
						if (l_Name2 == "")
						{
							l_Name2 = l_Name1;
							l_Name1 = "";
							l_Func = true;
						}
						else
							l_Func = false;
						
						// c'est une fonction sans classe
						l_ClassName = "";
						
						// on passe à l'exploration de ce qu'il y a entre les parenthèses
						l_ParEmpty = true;
						l_State = 34;
					}
					else if (l_Word == "*")
					{
						l_PointerLevel++;
						l_State = 2;
					}
					else if (l_Word == "[")
					{
						l_PointerLevel++;
						l_State = 3;
					}
					else if (l_Word == "]")
						l_State = 3;
				}
				else
				{
					// un nom... on stocke!
					if ((l_Word != "const") && (l_State==2))
					{
						if (l_Name2 == "")
							l_Name2 = l_Word;
						else
						{
							l_Name1 << " " << l_Name2;
							l_Name2 = l_Word;
						}
					}
				}
				break;
			
			case 10:
				// recherche de variables séparées par des ','
				if (!l_Operator)
				{
					// un nom... c'est une nouvelle variable
					StoreVariable(l_Name1,l_Word,l_PointerLevel,l_Begin);
					break;
				}
				else
				{
					l_State = 0;		// si state n'a pas été changé, c'est
								// qu'on est dans un cas pas géré
					// donc c'est un opérateur...
					
					if (l_Word == ";")
						l_State = 1;	// on revient au début
					else if (l_Word == ",")
						l_State = 10;	// on reste
					else if (l_Word == "=")
						l_State = 12;	// valeur par défaut
					else if (l_Word == "(")
					{
						l_Recurs = 1;
						l_State = 11;	// attente du '('
					}
					else if (l_Word == "*")
					{
						l_PointerLevel++;	// on augmente le niveau de pointeur
						l_State = 10;	// et on reste
					}
				}
				break;
			
			case 11:
				// attente du ')'
				if (l_Operator)
				{
					if (l_Word == ")")
						// trouvé!
						if (--l_Recurs == 0)
							l_State = 10;
					else if (l_Word == "(")
						// encore une autre parenthèse...
						l_Recurs++;
					else if (l_Word == ";")
						// erreur de syntaxe... on revient au début
						l_State = 1;
				}
				break;
			
			case 12:
				// déclaration de valeur par défaut
				if (l_Operator)
				{
					if (l_Word == ";")
						// fin de la ligne
						l_State = 1;
					else if (l_Word == ",")
						// variable suivante
						l_State = 10;
					else if (l_Word == "(")
					{
						// il faut attendre la ')' de fin
						l_Recurs = 1;
						l_State = 13;
					}
				}
				break;
			
			case 13:
				// attente du ')'
				if (l_Operator)
				{
					if (l_Word == ")")
						// trouvé!
						if (--l_Recurs == 0)
							l_State = 12;
					else if (l_Word == "(")
						// encore une autre parenthèse...
						l_Recurs++;
					else if (l_Word == ";")
						// erreur de syntaxe... on revient au début
						l_State = 1;
				}
				break;
			
			case 30:
				// peut-être un début de déclaration de fonction
				// on vire le type de retour (on s'en fout) et on garde uniquement le nom de la classe
				if (l_Name2 != "")
				{
					l_ClassName = l_Name2;
					l_Name1 = "";
					l_Name2 = "";
				}
				else
				{
					l_ClassName = l_Name1;
					l_Name1 = "";
				}
				
				if (l_Operator && l_Word == ":")
					l_State = 31;
				else
					// pas compris...
					l_State = 0;
				break;
			
			case 31:
				// peut-être un début de déclaraion de fonction
				if (!l_Operator)
				{
					// oui!!
					l_Name2 = l_Word;
					l_State = 33;
				}
				else if (l_Word == "~")
					// c'est un destructeur
					l_State = 32;
				else
					// pas compris
					l_State = 0;
				break;
			
			case 32:
				// desctucteur
				if (!l_Operator)
				{
					// oui, ça a bien l'air d'être le destructeur
					l_Name2 = "~";
					l_Name2 << l_Word;
					l_State = 33;
				}
				else
					// pas compris
					l_State = 0;
				break;

			case 33:
				// déclaration de fonction
				l_Func = true;
				if (l_Operator && (l_Word == "("))
				{
					// c'est bon
					l_State = 34;
				}
				else
					// pas compris
					l_State = 0;
				break;

			case 34:
				// on est entre la ( et la ) de la déclaration de fonction ou de variable
				if (!l_Operator)
				{
					if (l_Word != "const")
					{
						// c'est un nom, on passe à l'étape suivante
						l_Name3 = l_Word;
						l_PointerLevel = 0;
						l_ParEmpty = false;
						l_Begin2 = l_Pointer-beginning;
						l_State = 36;
					}
				}
				else
				{
					if (l_Word == ")")
					{
						// fin de la déclaration
						/// si les parenthèses étaient vides, ça doit être une fonction
						if (l_ParEmpty)
							l_Func = true;
							
						if (l_Func)
						{
							// on ne prend en compte la fonction que si on n'est pas dans un bloc
							if (m_CurrentBlock == NULL)
							{
								StoreFunction(l_ClassName,l_Name2,l_ParamsList,l_Begin);
								l_ParamsList.MakeEmpty();
							}
							else
								EmptyList(l_ParamsList);						
						}
						else
						{
							StoreVariable(l_Name1,l_Name2,l_PointerLevel,l_Begin);
							EmptyList(l_ParamsList);						
						}
						
						l_State = 35;
					}
					else if (l_Word == "(")
					{
						// une nouvelle parenthèse à l'intérieur, on va attendre qu'elle se ferme
						l_Recurs = 1;
						l_State = 37;
						l_ParEmpty = false;
					}
					else if (l_Word == ";")
						// késako?
					{
						EmptyList(l_ParamsList);
						l_State = 0;
					}
				}
				break;
			
			case 35:
				// fin de la déclaration de variable/fonction
				// s'il y a une , c'est qu'on déclare d'autres variables après
				if (!l_Func && l_Operator && (l_Word == ","))
					l_State = 10;
				else
					l_State = 0;
				break;
				
			case 36:
				// il y avait un nom, s'il y en a un deuxième à la suite, c'est que c'est une déclaration de fonction
				if (l_Operator)
				{
					// est-ce une étoile?
					if (l_Word == "*")
						l_PointerLevel++;
					// ou au moins un & ?
					else if (l_Word == "&")
					{
						// oui
					}
					else
					{
						// loupé... on revient au début de l'opérateur, et on revient à l'étape 34
						l_Pointer--;
						l_State = 34;
					}
				}
				else
				{
					if (l_Word != "const")
					{
						// il y a bien eu un deuxième nom
						l_Func = true;
						l_Name4 = l_Word;
						l_State = 38;		
					}
				}
				break;
			
			case 37:
				// attente du ')'
				if (l_Operator)
				{
					if (l_Word == ")")
						// trouvé!
						if (--l_Recurs == 0)
							l_State = 34;
					else if (l_Word == "(")
						// encore une autre parenthèse...
						l_Recurs++;
					else if (l_Word == ";")
						// erreur de syntaxe... on revient au début
						l_State = 1;
				}
				break;
			
			case 38:
				if (l_Operator)
				{
					// opérateur... étoile?
					if (l_Word == "*")
						// oui
						l_PointerLevel++;
					// ou un & ?
					else if (l_Word == "&")
					{
						// oui
					}
					else
					{
						// non... on déclare le paramètre
						StoreVariable(l_Name3,l_Name4,l_PointerLevel,l_Begin2,&l_ParamsList);
						
						// on revient sur l'opérateur et on passe repasse à l'étape 34
						l_Pointer--;
						l_State = 34;
					}
				}
				else
				{
					if (l_Word != "const")
					{
						// encore un nom...
						l_Name3 << " " << l_Name4;
						l_Name4 = l_Word;
					}
				}
				break;
				
			default:
				_D2("Etat inconnu: %d\n",l_State);
				l_State = 0;
		}
		
		if (l_State == 0)
		{
			// si le mot en cours est le ';' on passe à l'état 1 (début) sinon on reste à 0
			if (l_Operator && (l_Word == ";"))
				l_State = 1;
		}
	}	
}

void CSPSourceFile::StoreVariable(BString &type,BString &name, unsigned int pointerLevel, unsigned int pointer, BList *paramsList)
{
	CSPVariable *l_NewVariable = new CSPVariable;
	l_NewVariable->m_TypeName = type;
	l_NewVariable->m_Name = name;
	l_NewVariable->m_PointerLevel = pointerLevel;
	l_NewVariable->m_Pointer = pointer;
	
	// on stoque ça où?
	if (paramsList != NULL)
		paramsList->AddItem(l_NewVariable);
	else if (m_CurrentBlock != NULL)
		m_CurrentBlock->m_Variables.AddItem(l_NewVariable);
	else
		m_GlobalVariables.AddItem(l_NewVariable);
			
	if (pointerLevel != 0)
	{
		type << " ";
		type.Append('*',pointerLevel);
	}
	
	if (paramsList == NULL)
		_D3("variable: %s, %s\n",type.String(),name.String());
	else
		_D3("paramètre: %s, %s\n",type.String(),name.String());		
}

void CSPSourceFile::StoreFunction(BString &className,BString &name, BList &paramsList, unsigned int pointer)
{
	_D3("function: %s::%s()\n",className.String(),name.String());

	CSPFunction *l_NewFunction = new CSPFunction;
	l_NewFunction->m_ClassName = className;
	l_NewFunction->m_Name = name;
	l_NewFunction->m_PointerToBegin = pointer;
	l_NewFunction->m_Variables.AddList(&paramsList);
	// décalage des pointeurs pour les variables
	{
		unsigned int l_NVars = paramsList.CountItems();
		for (unsigned int i=0; i<l_NVars; i++)
			((CSPVariable*)paramsList.ItemAt(i))->m_Pointer -= pointer;	
	}
	
	m_Functions.AddItem(l_NewFunction);
	m_CurrentFunction = l_NewFunction;
	m_CurrentBlock = NULL;
}

void CSPSourceFile::EmptyList(BList &varsList)
{
	int l_NVars = varsList.CountItems();
	
	for(int i=0; i<l_NVars; i++)
	{
		delete (CSPVariable*)varsList.RemoveItem((int32)0);
	}
}

// extraction d'informations
const char *CSPSourceFile::CurrentClass()
{
	if (m_CurrentFunction == NULL)
		return "";
	else
		return m_CurrentFunction->m_ClassName.String();
}

const char *CSPSourceFile::CurrentFunction()
{
	if (m_CurrentFunction == NULL)
		return "";
	else
		return m_CurrentFunction->m_Name.String();
}

const char *CSPSourceFile::TypeOfVariable(const char *name,unsigned int *pointerLevel)
{
	const char *l_Type;
	
	// on commence par chercher dans les blocs
	CSPBlock *l_Block = m_CurrentBlock;
	while (l_Block != NULL)
	{
		l_Type = SearchVariable(l_Block->m_Variables,name,pointerLevel);
		if (l_Type != NULL)
			return l_Type;
		
		// on passe au bloc père
		l_Block = l_Block->m_InBlock;
	}
	
	// dans les paramètres de la fonction?
	if (m_CurrentFunction != NULL)
	{
		l_Type = SearchVariable(m_CurrentFunction->m_Variables,name,pointerLevel);
		if (l_Type != NULL)
			return l_Type;
	}
	
	// dans les variables globales
	l_Type = SearchVariable(m_GlobalVariables,name,pointerLevel);
	if (l_Type != NULL)
		return l_Type;
	else
		return "";
}

const char *CSPSourceFile::GetFirstVariableName()
{
	m_GFNVNBlock = m_CurrentBlock;
	m_GFNVNIndex = 0;
	m_GFNVInFunction = (m_CurrentFunction != NULL);
	return GetNextVariableName();
}

const char *CSPSourceFile::GetNextVariableName()
{
	// tout d'abord on cherche la liste de variables à utiliser
	BList *l_VariablesList;
	// est-ce qu'on est dans les variables locales?
	if (m_GFNVNBlock != NULL)
		l_VariablesList = &(m_GFNVNBlock->m_Variables);
	else
		// non... 
		if (!m_GFNVInFunction)
			// on prend les variables globales
			l_VariablesList = &m_GlobalVariables;
		else
			// on prend les paramètres de la fonction
			l_VariablesList = &m_CurrentFunction->m_Variables;
	
	// maintenant on regarde si on est à la fin de la liste ou pas
	if (m_GFNVNIndex >= (unsigned int)(l_VariablesList->CountItems()))
	{
		// oui, on est à la fin
		m_GFNVNIndex = 0;

		// est-ce qu'on est encore dans les variables locales?
		if (m_GFNVNBlock != NULL)
		{
			// oui, on va aller dans le bloc précédent
			m_GFNVNBlock = m_GFNVNBlock->m_InBlock;
			return GetNextVariableName();
		}
		else
		{
			// non. Si on était dans les paramètres de la fonction, il reste encore les variables locales
			if (m_GFNVInFunction)
			{
				m_GFNVInFunction = false;
				return GetNextVariableName();
			}
			else
				return NULL;
		}
	}
	
	// bon on est dans la bonne liste, et bien dedans
	CSPVariable *l_Variable = (CSPVariable*)(l_VariablesList->ItemAt(m_GFNVNIndex++));
	return l_Variable->m_Name.String();
}

const char *CSPSourceFile::SearchVariable(BList &inlist, const char *name, unsigned int *pointerLevel)
{
	unsigned int l_NVars = inlist.CountItems();
	
	for (unsigned int i=0; i<l_NVars; i++)
	{
		CSPVariable *l_Variable = (CSPVariable*)inlist.ItemAt(i);
		if (l_Variable->m_Name == name)
		{
			if (pointerLevel != NULL)
				*pointerLevel = l_Variable->m_PointerLevel;
				
			return l_Variable->m_TypeName.String();
		}
	}
	
	return NULL;
}

// fonctions utilitaires pour le parsing
/// recherche de mot
bool CSPSourceFile::ExtractWord(BString &word,const char **pointer, const char *end, bool *isOperator)
{
	// n'est-on pas à la fin du fichier?
	if (*pointer == end)
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
				SkipToEndOfLine(pointer,end);
			// ou d'un commentaire en '/*'?
			else if (*(*pointer+1) == '*')
			{
				// oui... recherche du */
				(*pointer)++;
				if (*pointer == end)
					return false;
				(*pointer)++;
				
				while ((**pointer != '*') || (*(*pointer+1) != '/'))
				{
					if (*pointer == end)
						return false;
					
					(*pointer)++;
				}
				*pointer += 2; 
			}
			else
			// non, ce n'est pas un commentaire. On est devant un authentique opérateur
				break;
			
			// n'est-on pas à la fin du fichier?
			if (*pointer == end)
				return false;
		}
		else
		{
			// c'est un espace tout bête... on passe au caractère suivant
			(*pointer)++;
			if (*pointer == end)
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
			if (**pointer==0)
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
			SkipToEndOfString(pointer,end);
			
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
void CSPSourceFile::SkipToEndOfLine(const char **pointer, const char *end)
{
	while(**pointer != '\n')
	{
		if (*pointer == end)
			return;
		(*pointer)++;
	}
	(*pointer)++;
}

/// va à l'instruction suivante
void CSPSourceFile::SkipToNextInstruction(const char **pointer, const char *end)
{
	bool l_IsOperator=false;
	BString l_Word;
	
	while (!l_IsOperator || l_Word != ";")
	{
		if (!ExtractWord(l_Word,pointer,end,&l_IsOperator))
			return;
		
		if (l_IsOperator && l_Word == "\"")
			// on est tombés sur un ", donc une chaîne de caractères... on cherche la fin
			SkipToEndOfString(pointer,end);
		else if (l_IsOperator && l_Word == "{")
			// on est au début d'un bloc logique... 
			// on recule d'un pour que le '{' soit bien repéré
		{
			(*pointer)--;
			return;
		}
	}
}

/// va à la fin d'une chaîne de caractères
void CSPSourceFile::SkipToEndOfString(const char **pointer, const char *end)
{
	for (;;)
	{
		(*pointer)++;

		while(**pointer != '"')
		{
			if (*pointer == end)
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

/// va à la fin d'un bloc logique
void CSPSourceFile::SkipToClosingBracket(const char **pointer, const char *end)
{
	while(**pointer != '}')
	{
		if (*pointer == end)
			return;
			
		char l_Char = *(*pointer)++;
		
		switch(l_Char)
		{
			case '"':
				SkipToEndOfString(pointer,end);
				break;
			
			case '{':
				SkipToClosingBracket(pointer,end);
				break;
				
			case '/':
				// peut-être un commentaire
				if (**pointer=='/')
					// oui, un commentaire sur une ligne
					SkipToEndOfLine(pointer,end);
				else if (**pointer=='*')
				{
					// oui, un commentaire en /* */
					(*pointer)++;
					while (((**pointer) != '*') || (*(*pointer+1) != '/'))
						if (*(*pointer)++ == 0)
							return;
					*pointer += 2;
				}
				break;
		}
	}
	
	// on passe au caractère après le '}'
	(*pointer)++;
}