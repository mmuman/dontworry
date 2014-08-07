/* ClassStructure
 * 
 * Analyseur de code c++
 * par Sylvain Tertois - 2001
 *
 * CSFile - Classe générique d'accès au stockage des données
 *
 * Cette classe seule ne sert à rien. Les classes qui accèdent directement
 * au fichier héritent de celle-ci, qui fournit les fonctions de base pour
 * lire et écrire dans l'objet BPositionIO
 *
 */

#ifndef CSFILE_H
#define CSFILE_H

class BPositionIO;

class CSFile
{
public:
	virtual ~CSFile();
	
protected:
	BPositionIO *m_Data;
				
	// gestion des chunks
	// pointer est une variable qui représente une position: la position du
	// début du chunk (0 = début du fichier)
	// pointer est automatiquement mis à jour par la fonction: après l'appel
	// il pointe vers le chunk suivant
	// toutes les fonctions retournent true, sauf s'il y a eu une erreur de
	// lecture ou d'écriture
	
	/// demande d'informations sur le chunk (pointer n'est pas mis à jour)
	bool ChunkInfo(const unsigned int pointer, unsigned int *type, unsigned int *size);
	
	/// lecture d'un chunk
	bool ChunkRead(unsigned int *pointer, void *data);
		// data doit être un bloc déjà alloué, avec une taille suffisante
		// s'il y a une erreur de lecture, 
	
	/// sauter un chunk
	bool ChunkSkip(unsigned int *pointer);
	
	/// écrire un chunk
	bool ChunkWrite(unsigned int *pointer,unsigned int type, const void *data, unsigned int size);

	/// mettre à jour les informations d'un chunk
	bool ChunkUpdate(unsigned int *pointer, unsigned int type, unsigned int size);

	friend class CSFNamesList;
	friend class CSPNamesList;
};

// noms des chunks
const unsigned int g_CFile='CSFL';
const unsigned int g_CClassList='LCLA';
const unsigned int g_CClassNames='CLNM';
const unsigned int g_CNamesList='LIST';
const unsigned int g_CClass='CLAS';
const unsigned int g_CClassInfo='INFO';
const unsigned int g_CClassMethodsList='METS';
const unsigned int g_CClassMethod='METH';
const unsigned int g_CClassStrings='STRS';
const unsigned int g_CClassMemberVariables='MVRS';

// taille de l'entête d'un chunk
const unsigned int g_ChunkHeaderSize = 8;

#endif //CSFILE_H