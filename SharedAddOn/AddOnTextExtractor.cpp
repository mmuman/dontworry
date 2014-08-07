/*******************************/
/* classe d'extraction du text */
/*******************************/
#include "AddOnTextExtractor.h"
#include "AddOnExtractedString.h"

#include <InterfaceDefs.h>

#include <iostream.h>

/**** constructeur ****/
AddOnTextExtractor::AddOnTextExtractor()
{
	// initialiser
	_nbUsedItems = 0;
}

/**** destructeur ****/
AddOnTextExtractor::~AddOnTextExtractor()
{
	int32	nbItems;
	
	nbItems = _extractedList.CountItems();
	for(int32 i=0;i<nbItems;i++)
		delete (AddOnExtractedString *)(_extractedList.RemoveItem((int32)0));
}

/**** extraire les variables metodes etc ****/
status_t AddOnTextExtractor::Extract(const char *source,int32 position,bool completeWord)
{
	// initialiser
	_text = source;
	_position = position;
	_completeWord = completeWord;
	
	// on a plus aucun item
	// compteur de recursivité a zero
	_nbUsedItems = 0;
	_nbRecursExtract = 0;
	
	// lancer l'extraction
	return RecursExtract(true);
}

/**** fonction recursive d'extraction ****/
status_t AddOnTextExtractor::RecursExtract(bool firstCall)
{
	BString		extracted("");
	status_t	result = B_OK;
	bool		exit = false;
	char		lastValid = '\0';
	char		findedOperator = '\0';
	bool		isCast = false;
	bool		valideChar = true;

	// positionner result en fonction du  niveau de recursivité
	if(!firstCall)
		result = B_ERROR;

	// augmenter le niveau de recursivité
	_nbRecursExtract++;

	// on peux sortir du while si la position est
	// au debut du texte, ce qui es une erreur
	while(!exit)
	{
		// passer au caractere suivant
		_position--;		
		
		// traiter l'erreur de position
		if(_position==0)
		{
			result = B_ERROR;
			break;
		}

		switch((uint8)(_text[_position]))
		{
		// ignorer les caracteres non pris en compte par le compilateur
		case B_ENTER:
		case B_TAB:
		case B_SPACE:
			exit = (_completeWord || firstCall);	
			continue;
			break;
		// operateur unaire
		// operateur binaire
		// divers
		// dans ces deux cas on vide la chaine si on est pas en
		// completion, sinon on sort
		// cast, acces a l'objet ou operateur de multiplication
		case '*':
			isCast = true;
		// reference
		case '&':
			findedOperator = _text[_position];
		case '+':
		case '-':
		case '<':
		case '>':
		case '.':
		case ':':
		case '\\':
		// cas des commentaire
		case '/':
			{
			if(lastValid=='/')
				exit=true;
			}
		case ',':
		case '=':
		case '!':
		case '|':
			{
				// en compeltion on sort au premier caractere non
				// valide pour un nom (de meme pour le premier appel de la fonction)
				if(_completeWord || firstCall)
					exit = true;
			}
			break;
		// si on a une chaine de caractere on en tiens pas compte
		// tout ce qui est dedans n'est pas valide
		case '"':
			valideChar = !valideChar;
			break;
		// on doit absolument sortir (c'est pas la peine de continuer)
		// dans ces cas on arrive a la fin de la ligne
		// precedante ou on termine le bloc entre parenthese que l'on analyse
		case '(':
		case '[':
			{
				// tout est bien, on a trouvé la parenthese fermante
				result = B_OK;
				
				// on peux etre tombé sur un operateur
				if(findedOperator=='&'|| findedOperator=='*')				
					isCast = false;					
				
				// on sort de toute maniere
				exit = true;
			}
			break;
		// on est sur la fin d'une ligne d'instruction
		// ou une fin de bloc C/C++
		// ou ce sont des caracteres speciaux
		case '#':
		case '{':
		case '}':
		case ';':
			exit = true;
			break;
		// prochain bloc nouvelle parenthese (ou crochets)
		case ')':
		case ']':
			{
				AddItem(extracted,findedOperator,isCast);
				extracted = "";
				result &= RecursExtract(false);
			}
			break;
		// dans ce cas on retient le caractere
		// si il est valide
		default:
			if(valideChar)
				extracted.Prepend(_text[_position],1);
		}
		
		// retenir le dernier caractere valid pour le compilateur
		lastValid = _text[_position];
	}

	// ajouter ce qu'il reste si on a quelque chose
	// on doit tout de meme avoir respecter les parentheses
	if(result==B_OK)
		AddItem(extracted,findedOperator,isCast);

	// retourner le resultat
	return result;
}

/**** recuperer ma chaine trouvée ****/
AddOnExtractedString *AddOnTextExtractor::Extracted()
{
	return (AddOnExtractedString *)(_extractedList.ItemAt(_nbUsedItems - 1));
}

/**** ajouter un item ****/
void AddOnTextExtractor::AddItem(BString &string,char identifier,bool asCast)
{
	// verifier que la chaine n'est pas vide
	if(string.Length()==0)
		return;
	
	AddOnExtractedString	*newItem = NULL;
	bool					isNew = false;

	// verifier que l'item n'existe pas deja
	newItem = (AddOnExtractedString *)(_extractedList.ItemAt(_nbUsedItems));
	isNew = (newItem==NULL);
	if(isNew)
		newItem = new AddOnExtractedString();

	// ajouter la chaine extraite
	newItem->_string = string;
	newItem->_identifier = identifier;
	newItem->_operatorAsCast = asCast;	
	newItem->_recursLevel = _nbRecursExtract;

	if(isNew)
		_extractedList.AddItem(newItem);

	// un de plus
	_nbUsedItems++;
}