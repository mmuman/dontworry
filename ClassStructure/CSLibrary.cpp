/* ClassStructure
 * 
 * Analyseur de code c++
 * par Sylvain Tertois - 2001
 *
 * CSLibrary - Classe d'API générale
 *
 * Cette classe sert à faire la liaison entre les classes CS et l'application cliente
 *
 * La fonctionnalité 'paths' (voir constructeur) n'est pas encore implémentée
 */


#include "CSLibrary.h"
#include "CSParser.h"
#include "CSClass.h"

CSLibrary::CSLibrary(BPositionIO *data, const char **paths)
{
	m_Data = data;
	m_InitCheck = B_ERROR;
	m_Parser = NULL;
	m_NameStrings = NULL;
	m_NamesList = NULL;
	m_NextLibrary = NULL;
	
	// on va faire quelques vérifications de routine sur le fichier
	unsigned int l_Pointer = 0, l_Type, l_Size, l_FileSize;
	
	if (!ChunkInfo(l_Pointer,&l_Type,&l_FileSize))
	{
		// il y a eu une erreur, c'est que le fichier doit être vide.
		// on va faire appel au constructeur de CSParser, qui sait créer un fichier
		// vide correct
		m_InitCheck  = BeginParse();
		return;
	}
	l_FileSize += g_ChunkHeaderSize;
	
	// on vérifie le type de fichier
	if (l_Type != g_CFile)
		return;
	
	// on va vérifier que tous les chunks sont bien là
	// pour cela on va tous les parcourir un à un
	unsigned int l_Chunks = 0;	// masque de bits. Chaque bit représente un chunk
	l_Pointer = g_ChunkHeaderSize;
	
	while (ChunkInfo(l_Pointer,&l_Type,&l_Size))
	{
		// sur quel chunk est-on tombé?
		switch (l_Type)
		{
			case g_CClassList:
				l_Chunks |= 0x01;
				break;
				
			case g_CClassNames:
				l_Chunks |= 0x02;
				break;
				
			case g_CNamesList:
				l_Chunks |= 0x04;
				break;
				
			default:
				break;
		}
		
		ChunkSkip(&l_Pointer);
	}
	
	// on est arrivé à la fin du fichier.
	// tous les chunks sont-ils bien là?
	if (l_Chunks != 0x07)
		return;
	
	// la taille du fichier est-elle bien correcte?
	if (l_FileSize != l_Pointer)
		return;
		
	// alors c'est bon...
	m_InitCheck = B_OK;
}

CSLibrary::~CSLibrary()
{
	// on détruit tous les objets qui pourraient être encore là
	delete m_Parser;
	delete m_NameStrings;
	delete m_NamesList;
}

bool CSLibrary::Empty()
{
	// on détruit tous les objets qui pourraient être encore là
	delete m_Parser;
	delete m_NameStrings;
	delete m_NamesList;
	m_Parser = NULL;
	m_NameStrings = NULL;
	m_NamesList = NULL;
	
	// on réduit le fichier à une taille nulle
	m_Data->SetSize(0);
	
	// ensuite on va faire appel au constructeur de CSParser, qui sait créer un fichier
	// vide correct
	return (BeginParse() == B_OK);
}

status_t CSLibrary::Parse(const char *path, bool directory, unsigned int recurse_dir, bool savePath)
{
	// si le parser n'est pas là, on l'invoque
	status_t l_InitCheck = BeginParse();
	if (l_InitCheck != B_OK)
		return l_InitCheck;
	
	// puis on effectue le parsing
	return m_Parser->Parse(path,directory,recurse_dir,savePath);
}

status_t CSLibrary::Parse(const char *text,unsigned int textLength)
{
	// si le parser n'est pas là, on l'invoque
	status_t l_InitCheck = BeginParse();
	if (l_InitCheck != B_OK)
		return l_InitCheck;
	
	// puis on effectue le parsing
	return m_Parser->Parse(text,textLength);
}

CSClass *CSLibrary::GetClass(const char *name)
{
	// si le parser est là, on l'enlève
	if (EndParse() != B_OK)
		return NULL;
	
	// on commence par chercher le nom de classe
	unsigned int l_OffsetToClass = m_NamesList->FindName(name, m_NameStrings);
	
	if (l_OffsetToClass == 0xffffffff)
	{
		// pas trouvé... peut-on chercher dans une autre bibliothèque?
		if (m_NextLibrary != NULL)
			return m_NextLibrary->GetClass(name);
		else
			return NULL;	// non
	}
	else
		return new CSClass(this,m_ClassListChunk+l_OffsetToClass+g_ChunkHeaderSize);	
			// trouvé!
}

void CSLibrary::AddLibrary(CSLibrary *library)
{
	// on ajoute la bibliothèque à la liste
	if (m_NextLibrary == NULL)
		m_NextLibrary = library;
	else
		m_NextLibrary->AddLibrary(library);
}

status_t CSLibrary::BeginParse()
{
	// le parseur est-il là?
	if (m_Parser == NULL)
	{
		// non
		// on commence par enlever de la mémoire les données sur les noms de classe,
		// qui vont maintenant être invalides
		delete m_NameStrings;
		delete m_NamesList;
		m_NameStrings = NULL;
		m_NamesList = NULL;
		
		// puis création du parser
		m_Parser = new CSParser(m_Data);
		
		return m_Parser->InitCheck();
	}
	else
	{
		// il est déjà là, tout va bien
		return B_OK;
	}
}

status_t CSLibrary::EndParse()
{
	// la parser est-il là?
	if (m_Parser != NULL)
	{
		// oui. On l'enlève
		delete m_Parser;
		m_Parser = NULL;
	}
	
	// les données sur les noms de classes sont-elles là?
	if (m_NameStrings == NULL)
	{
		// non, on va devoir les charger
		// on parcourt les chunks jusqu'à ce qu'on tombe dessus
		unsigned int l_Pointer = g_ChunkHeaderSize;
		unsigned int l_Type,l_Size;
		
		while(ChunkInfo(l_Pointer,&l_Type,&l_Size))
		{
			switch(l_Type)
			{
				case g_CClassList:
					// c'est la liste des classes. On retient l'offset
					m_ClassListChunk = l_Pointer;
					ChunkSkip(&l_Pointer);
					break;
					
				case g_CClassNames:
					// ce sont les chaînes de caractères pour les noms
					// de classe.
					// on va les charger
					m_NameStrings = new char[l_Size];
					if (!ChunkRead(&l_Pointer,m_NameStrings))
					{
						// il y a eu une erreur de lecture
						delete m_NameStrings;
						m_NameStrings = NULL;
					}
					break;
				
				case g_CNamesList:
					m_NamesList = new CSFNamesList;
					if (!m_NamesList->Init(this,l_Pointer))
					{
						// il y a eu une erreur de lecture
						delete m_NamesList;
						m_NamesList = NULL;
					}
					ChunkSkip(&l_Pointer);
					break;
				
				default:
					ChunkSkip(&l_Pointer);
					break;
			}
		}
		
		// a-t-on tout chargé sans erreur?
		if ((m_NameStrings == NULL) || (m_NamesList == NULL))
		{
			// il y a eu une erreur, ou un chunk n'a pas été chargé
			delete m_NameStrings;
			delete m_NamesList;
			m_NameStrings = NULL;
			m_NamesList = NULL;
			
			return B_ERROR;
		}
	}
	
	// tout va bien
	return B_OK;
} 

// fonctions de débug
CSClass *CSLibrary::GetFirstClass()
{
	// si le parser est là, on l'enlève
	if (EndParse() != B_OK)
		return NULL;

	m_DebugPointer = m_ClassListChunk+g_ChunkHeaderSize;
	return GetNextClass();
}

CSClass *CSLibrary::GetNextClass()
{
	// on retire des infos sur le chunk
	unsigned int l_Type, l_Size;
	if (!ChunkInfo(m_DebugPointer,&l_Type,&l_Size) || (l_Type != g_CClass))
		return NULL;
	
	CSClass *l_newClass = new CSClass(this,m_DebugPointer);
	
	ChunkSkip(&m_DebugPointer);
	
	return l_newClass;
}
