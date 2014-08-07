/* ClassStructure
 * 
 * Analyseur de code c++
 * par Sylvain Tertois - 2001
 *
 * CSClass - Classe d'API
 *
 * Cette classe représente une classe. Elle n'est pas crée par l'application cliente, mais
 * par un objet CSLibrary. Par contre l'application peut utiliser ses méthodes pour
 * accéder aux différentes informations sur la classe
 *
 * Toutes les fonctions autour du typedef ne marchent pas pour l'instant
 * Les filtres 'public', 'protected' et 'private' ne marchent pas non plus
 */

#ifndef CSCLASS_H
#define CSCLASS_H

class CSLibrary;
class CSMethod;
class CSVariable;

enum csmembertype { 
	CSMT_PUBLIC = 0x01, 
	CSMT_PRIVATE = 0x02,
	CSMT_PROTECTED = 0x04
	};

class CSClass
{
private:
	CSClass(CSLibrary*,unsigned int pointer);

public:
	~CSClass();
	
	const char *GetName();
		// donne le nom de la classe
		// la chaîne retournée reste propriété de la classe. Elle ne doit pas être détruite
	
	// gestion des typedef
	inline bool IsTypeDef() {return m_IsTypeDef; }
		// true si le nom ci dessus est celui d'un typedef
	CSClass *GetRealClass();
		// essaie de trouver la vraie classe, dans le cas d'un typedef (NULL si pas trouvé)
		// rend NULL si la classe n'est pas un typedef
		// l'objet retourné devient propriété de l'appelant. Il doit le détruire après utilisation
	const char *GetRealName();
		// donne le nom véritable du type, dans le cas d'un typedef. Si ce n'est
		// pas un typedef, rend NULL.
		// la chaîne retournée reste propriété de la classe. Elle ne doit pas être détruite
	unsigned int GetPointerLevel();
		// dans le cas d'un typedef, donne le nombre de * entre le type défini et le type réel
	
	// chemin du fichier Header
	const char *GetHeaderFile();
		// Retourne le chemin vers le fichier header qui décrit la classe. Retourne NULL si le chemin 
		// n'est pas stocké.
		// la chaîne retournée reste propriété de la classe. Elle ne doit pas être détruite
		
	// les fonctions suivantes ne marchent que si cet objet représente bien une classe, et pas un
	// typedef
	// les objets retournées par ces méthodes deviennent propriété de l'appelant. Il doit les
	// détruire après utilisation
	// les objets de type CSMethod et CSVariable retourné ne sont valides que si l'objet CSClass
	// d'où ils proviennent est toujours là. Il vaut mieux détruire l'objet CSClass qu'après avoir
	// détruit tous les objets CSMethod et CSVariable qu'il a pu créer avec les méthodes suivantes.
	
	CSClass *GetFirstFather();
		// rend la première classe père (dans le cas d'un héritage)
	CSClass *GetNextFather();
		// rend les classes père suivantes
	
	bool IsFriend(const char *name);
		// rend true si la classe dont on donne le nom est une classe amie
	
	CSMethod *GetFirstMethod(unsigned int filter = CSMT_PUBLIC);
		// rend la première méthode qui satisfasse (??) au filtre donné
	CSMethod *GetNextMethod();
		// rend les méthodes suivantes
	CSMethod *FindMethod(const char *name, unsigned int index=0);
		// rend la index-ième méthode avec le nom name. NULL si pas trouvé
	
	CSVariable *GetFirstMember(unsigned int filter = CSMT_PUBLIC);
		// rend la première variable membre qui satisfasse (??) au filtre donné
	CSVariable *GetNextMember();
		// rend les variables membres suivantes
	CSVariable *FindMember(const char *name);
		// rend la variable membre avec le nom name, NULL si pas trouvé
	
	CSLibrary *GetLibrary();
		// retourne la librarie dans laquelle la classe est présente
		
private:
	bool m_IsTypeDef;
	
	// pointeurs internes sur les données intéressantes
	CSLibrary *m_Library;			// librarie qui nous a créé
	unsigned int m_InfoChunk;		// pointeur sur le chunk 'INFO'
	unsigned int m_MetsChunk;		// pointeur sur le chunk 'METS'
	unsigned int m_MemberVarsChunk;	// pointeur sur le chunk 'MVRS'
	char *m_ClassStrings;			// zone ou sont stockées les chaînes utilisées par la classe
	
	// compteurs/pointeurs et nombre totaux pour les méthodes First/Next
	/// compteurs/pointeurs (m_N*** et m_P***)
	unsigned int m_NFather;
	unsigned int m_PMethod;
	unsigned int m_NMemberVariable;
	
	/// nombre totaux (m_Num***s)
	unsigned char m_NumFathers;
	unsigned short m_NumMemberVariables;
	
	/// filtres utilisés en ce moment sur les zones (public/privées/protégées)
	unsigned char m_MethodFilter;
	unsigned char m_VariableFilter;
	
	friend class CSLibrary;
	friend class CSMethod;
	friend class CSVariable;
};

#endif