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

#include "CSFile.h"
#include <DataIO.h>

CSFile::~CSFile()
{
}

// gestion des chunks
/// demande d'informations sur le chunk (pointer n'est pas mis à jour)
bool CSFile::ChunkInfo(const unsigned int pointer, unsigned int *type, unsigned int *size)
{
	if ((m_Data->ReadAt(pointer,type,4) != 4) ||
		(m_Data->ReadAt(pointer+4,size,4) != 4))
	{
		// erreur de lecture
		return false;
	}
	
	// la vraie taille est celle du chunk moins celle de l'entête
	*size -= g_ChunkHeaderSize;
	return true;
}

/// lecture d'un chunk
bool CSFile::ChunkRead(unsigned int *pointer, void *data)
{
	int l_size;
	if ((m_Data->ReadAt((*pointer)+4,&l_size,4) == 4) &&
				// lecture de la taille
		((int)(m_Data->ReadAt((*pointer)+g_ChunkHeaderSize,data,
			l_size-g_ChunkHeaderSize)) == (int)(l_size-g_ChunkHeaderSize)))
				// et des données
	{
		// mise à jour du pointeur
		*pointer += l_size;
		
		// tout a bien marché
		return true;
	}
	else
		// erreur de lecture (fin de fichier?)
		return false;
}
	
/// sauter un chunk
bool CSFile::ChunkSkip(unsigned int *pointer)
{
	unsigned int l_size;
	if (m_Data->ReadAt((*pointer)+4,&l_size,4) == 4)
	{
		// tout va bien
		*pointer += l_size;
		
		return true;
	}
	else
		// erreur de lecture (fin de fichier?)
		return false;
}
	
/// écrire un chunk
bool CSFile::ChunkWrite(unsigned int *pointer,unsigned int type, const void *data, unsigned int size)
{
	int l_ChunkSize = size+g_ChunkHeaderSize;
	
	if ((m_Data->WriteAt((*pointer),&type,4) == 4) &&
		(m_Data->WriteAt((*pointer)+4,&l_ChunkSize,4) == 4) &&
		(m_Data->WriteAt((*pointer)+g_ChunkHeaderSize,data,size) == (int)size))
	{
		// tout s'est bien passé
		(*pointer) += l_ChunkSize;
		return true;
	}
	else
		// erreur d'écriture
		return false;
}

/// mettre à jour les informations d'un chunk
bool CSFile::ChunkUpdate(unsigned int *pointer, unsigned int type, unsigned int size)
{
	unsigned int l_ChunkSize = size+g_ChunkHeaderSize;
	
	if ((m_Data->WriteAt((*pointer),&type,4) == 4) &&
		(m_Data->WriteAt((*pointer)+4,&l_ChunkSize,4) == 4))
	{
		// tout s'est bien passé
		(*pointer) += l_ChunkSize;
		return true;
	}
	else
		// erreur d'écriture
		return false;
}

