/************/
/* CProject */
/************/

#include "CProject.h"
#include "CSLibrary.h"
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <Window.h>
#include <Node.h>
#include <NodeInfo.h>

#include "AddOnConstantes.h"

// debug
//#include <iostream.h>

/**** constructeur ****/
CProject::CProject(const char *projectName,const char *projectPath,BWindow *progressWindow)
: _progressWindow(progressWindow)
{
	int32		posPoint;
	BString		CSFile("");
	BString		project("");
	
	// initialiser les vaiables
	_projectFilePath.SetTo(projectPath);
	_projectName.SetTo(projectName);
	_progressWindow = progressWindow;	

	// recuperer le nom du projet
	project.SetTo(projectName);
	if((posPoint = project.FindFirst('.'))!=B_ERROR)
		project.Truncate(posPoint);

	CSFile << _projectFilePath.Path() << "/";
	CSFile << project << ".cs";
	
	_FileUserLib = new BFile(CSFile.String(),B_READ_WRITE);
	// le fichier n'existe pas on le creer
	if(_FileUserLib->InitCheck()!= B_OK)
		_FileUserLib->SetTo(CSFile.String(),B_READ_WRITE|B_CREATE_FILE|B_ERASE_FILE);

	// renseigner le MIME Type
	BNode		fileNode(CSFile.String());
	BNodeInfo	nodeInfo(&fileNode);

	if(nodeInfo.InitCheck()==B_OK)
		nodeInfo.SetType("application/x-vnd.CKJ-Daixiwen-CSFile");

	// creer la CSLib et verifier sa validité
	_CSUserLib = new CSLibrary(_FileUserLib);
	if(_CSUserLib->InitCheck()!=B_OK)
		_CSUserLib->Empty();
}

/**** destructeur ****/
CProject::~CProject()
{
	// effacer les donnees de la BList
	int32		nbPath;
	int32		nbFile;
	
	nbPath = _projectPath.CountItems();
	for(int32 index=0;index<nbPath;index++)
		delete (BString *)(_projectPath.RemoveItem((int32)0));

	nbFile = _projectFile.CountItems();
	for(int32 index=0;index<nbFile;index++)
		delete (BString *)(_projectFile.RemoveItem((int32)0));

	if(_CSUserLib!=NULL)
		delete _CSUserLib;
	
	delete _FileUserLib;
}

/**** verifier si il est valide ****/
status_t CProject::InitCheck()
{
	if(_CSUserLib==NULL)
		return B_ERROR;
		
	return _CSUserLib->InitCheck();
}

/**** parser les fichiers header ****/
status_t CProject::Parse(bool directory, unsigned int recurse_dir)
{
	status_t	state = B_OK;

	// on doit avoir un pointer
	if(_CSUserLib==NULL)
		return B_ERROR;

	// verifier la validite de la lib
	if(InitCheck()!=B_OK)
		_CSUserLib->Empty();
	
	// creation de l'entry relative au projet
	BDirectory	projectPath(_projectFilePath.Path());

	// Recherche des fichiers header dans les repertoires du projet BeIDE
	BMessage	progressFiles(PROGRESS_START_MSG);
	int32		index;
	BEntry		headerFile;
	BString		path("");
	int32		nbFiles = 0;
	
	// le compter avant
	nbFiles = _projectPath.CountItems();
	
	// envoyer le total
	progressFiles.AddInt32(PROJECT_FILE_TOTAL,nbFiles);
	_progressWindow.SendMessage(&progressFiles);
	
	// puis les parser
	for(index=0;index<_projectPath.CountItems();index++)
	{
		path.SetTo(((BString *)(_projectPath.ItemAt(index)))->String());
		_progressWindow.SendMessage(PROGRESS_INCREASE_MSG);
		snooze(2500);
			
		state = _CSUserLib->Parse(path.String(),true,1,true);
		if(state == CSP_NOT_MODIFIED)
			state = B_OK;
	}

	return state;
}

/**** ajouter un chemin a la liste des chemins du projet ****/
void CProject::AddPath(const char *path)
{
	_projectPath.AddItem(new BString(path));
}

/**** ajouter un fichier a la liste ****/
void CProject::AddFile(const char *file)
{
	_projectFile.AddItem(new BString(file));
}

/**** trouver un nouveau fichier ****/
BMessage *CProject::FindNewFile(BMessage *fileList)
{
	int32		nbFiles;
	BString		newFile;
	BString		pathFile;
	bool		finded;
	int32		i,j;
	BMessage	*findedFiles;
	
	i = 0;
	findedFiles = new BMessage();
	nbFiles = _projectFile.CountItems();
	while(fileList->FindString(FILE_PATHNAME,i,&newFile)==B_OK)
	{
		// on regarde si on trouve le fichier
		j = 0;
		finded = false;
		while(j<nbFiles && !finded)
		{
			if(*((BString *)(_projectFile.ItemAt(j))) == newFile)
				finded = true;

			j++;
		}

		// si non c'est un nouveau, gagné !!
		if(!finded)
			findedFiles->AddString(FILE_PATHNAME,newFile);
		
		i++;
	}
	
	// on a rien trouvé de nouveau, c'est une erreur
	return	findedFiles;
}

/**** trouver le chemin d'un fichier ****/
status_t CProject::FindFile(const char *pathFile)
{
	int32		nbFiles;
	BString		listFile;
	
	nbFiles = _projectFile.CountItems();
	for(int32 i=0;i<nbFiles;i++)
	{
		listFile = *((BString *)_projectFile.ItemAt(i));
		if(listFile == pathFile)
			return	B_OK;
	}
	
	return	B_ERROR;
}

/**** mettre a jour la liste des fichiers quand on en enleve du projet ****/
void CProject::RemoveFiles(BMessage *fileList)
{
	int32		nbFiles;
	BString		file;
	bool		finded;
	int32		i,j;
		
	// vider la liste
	i = 0;
	nbFiles = _projectFile.CountItems();
	while(i<nbFiles)
	{
		j = 0;
		finded = false;
		while(!finded && fileList->FindString(FILE_PATHNAME,j,&file)==B_OK)
		{
			if(*((BString *)(_projectFile.ItemAt(i))) == file)
				finded = true;
				
			j++;
		}
		
		// on a pas trouve le fichier, il faut l'effacer
		if(!finded)
		{
			delete (BString *)(_projectFile.RemoveItem(i));
			nbFiles = _projectFile.CountItems();
		}
		else
			i++;
	}
}

/**** faire clignoter la vue de l'icon ****/
void CProject::BlinkIcon()
{
	// verifier que le messenger est valide
	if(!(_iconView.IsValid()))
		return;

	// envoyer la commande
	_iconView.SendMessage(BLINK_ICONVIEW_MSG);
}
