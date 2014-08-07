/* ClassStructure
 * 
 * Analyseur de code c++
 * par Sylvain Tertois - 2001
 *
 * CSVariable - Classe d'API
 *
 * Cette classe représente une variable membre, ou un argument dans une méthode.
 * Elle n'est pas crée par l'application cliente, mais par un objet CSClass ou CSMethod.
 * Par contre l'application peut utiliser ses méthodes pour accéder aux différentes 
 * informations sur la variable
 *
 */

#ifndef CSVARIABLE_H
#define CSVARIABLE_H

class CSClass;

class CSVariable
{
private:
	CSVariable(CSClass*,unsigned int pointer,bool memberVariable);

public:
	~CSVariable();
	
	const char *GetName();
		// rend le nom de la variable. NULL si pas de nom
		// la chaîne retournée reste propriété de la classe. Elle ne doit pas être détruite
	
	const char *GetTypeName();
		// donne le nom du type de la variable
		// la chaîne retournée reste propriété de la classe. Elle ne doit pas être détruite
		// le "const" éventuel n'est pas ajouté. Pour ça voir la méthode suivante
	
	bool IsConst();
		// retourne true si le type de la variable a un "const" devant
		
	bool IsRef();
		// retourne true si le type de la variable est une référence (uniquement un paramètre qui a un &)
		
	CSClass *GetClass();
		// donne une représentation du type de la variable
		// l'objet retourné devient propriété de l'appelant. Il doit le détruire après utilisation
	
	unsigned int GetPointerLevel();
		// donne le nombre de '*' entre le type de la variable et celle-ci
	
	const char *GetDefaultValue();
		// donne la valeur par défaut de l'argument (uniquement pour un argument
		// de méthode). NULL si pas de valeur par défaut
		// la chaîne retournée reste propriété de la classe. Elle ne doit pas être détruite

	unsigned int GetZone();
		// (variable membre uniquement)
		// retourne la zone (publique/privée/protégée) de la variable
	
private:
	CSClass *m_Class;		// classe à laquelle appartient la variable
	unsigned int m_Chunk;	// pointeur sur les données de la variable dans les chunks
	bool m_MemberVariable;	// true si c'est une variable membre
	
	friend class CSClass;
	friend class CSMethod;
};

#endif