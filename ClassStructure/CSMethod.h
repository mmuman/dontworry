/* ClassStructure
 * 
 * Analyseur de code c++
 * par Sylvain Tertois - 2001
 *
 * CSMethod - Classe d'API
 *
 * Cette classe représente une méthode. Elle n'est pas crée par l'application cliente, mais
 * par un objet CSClass. Par contre l'application peut utiliser ses méthodes pour
 * accéder aux différentes informations sur la méthode
 *
 */

#ifndef CSMETHOD_H
#define CSMETHOD_H

class CSClass;
class CSVariable;

class CSMethod
{
private:
	CSMethod(CSClass*,unsigned int pointer);

public:
	~CSMethod();
	
	const char *GetName();
		// rend le nom de la méthode
		// la chaîne retournée reste propriété de la classe. Elle ne doit pas être détruite
	bool IsVirtual();
		// retourne true si la méthode est virtuelle
		
	const char *GetReturnTypeName();
		// donne le nom du type de la variable de retour
		// la chaîne retournée reste propriété de la classe. Elle ne doit pas être détruite
		// le "const" éventuel n'est pas ajouté au nom (voir la méthode suivante)	
	
	bool IsReturnTypeConst();
		// retourne true si le type de la variable de retour a un "const"
	
	bool IsReturnTypeRef();
		// retourne true si le type de la variable de retour est une référence (a un &)
		
	CSClass *GetReturnClass();
		// donne une représentation du type de la variable de retour
		// l'objet retourné devient propriété de l'appelant. Il doit le détruire après utilisation

	unsigned int GetReturnPointerLevel();
		// donne le nombre de '*' entre le type de la variable de retour et celle-ci
	
	unsigned int GetZone();
		// retourne la zone de la méthode (privée/publique/protégée)
		 
	CSVariable *GetFirstArgument();
		// retourne un objet CSVariable qui décrit le premier argument, NULL s'il
		// n'y en a pas.
		// l'objet retourné devient propriété de l'appelant. Il doit le détruire après utilisation

	CSVariable *GetNextArgument();
		// retourne un objet CSVariable qui décrit un argument suivent, NULL si
		// on est arrivés au bout
		// l'objet retourné devient propriété de l'appelant. Il doit le détruire après utilisation

private:
	// quelques données
	CSClass *m_Class;		// pointeur vers la classe à laquelle appartient cette méthode
	unsigned int m_Chunk;	// pointeur vers le chunk 'METH' qui décrit la méthode
	
	// compteur de l'argument en cours
	unsigned int m_NArgument;
	
	// nombre total d'arguments
	unsigned char m_NumArguments;
	
	friend class CSClass;
};
			
#endif