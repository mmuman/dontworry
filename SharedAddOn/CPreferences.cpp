#include "CPreferences.h"

#include <Application.h>
#include <FindDirectory.h>
#include <Path.h>
#include <String.h>
#include <Window.h>
#include <File.h>
#include <Roster.h>
#include <AppFileInfo.h>

#include <Application.h>
#include <Message.h>
#include <ScrollView.h>
#include <Bitmap.h>
#include <File.h>
#include <Resources.h>
// debbuger
#include <iostream.h>

CPreferences::CPreferences(const char *fileName, const char *filePath)
{
	BPath		path;
	BString		prefsFilePath;
	
	if((_status=find_directory(B_USER_SETTINGS_DIRECTORY,&path))!=B_OK)
		return;

	prefsFilePath=path.Path();
	if(filePath!=NULL)
	{
		if(filePath[0]!='/')
			prefsFilePath.Append("/");
		prefsFilePath.Append(filePath);
	}
	if(prefsFilePath.ByteAt(prefsFilePath.Length())!='/')
		prefsFilePath.Append("/");
	prefsFilePath.Append(fileName);

	_filePath=new char[prefsFilePath.Length()+1];
	strcpy(_filePath,prefsFilePath.String());

	// renseigner le chemin de la vue plugin
	// par rapport au chemin de BeIDE
	BRoster			roster;
	BPath			beidePath;
	entry_ref		fileRef;
	app_info		info;

	// attention on peux etre dans le cas du CSViewer
	if(roster.GetAppInfo("application/x-mw-BeIDE",&info)!=B_OK)
	{
		// charger depuis les ressources
		roster.GetAppInfo("application/x-vnd.CKJ-Daixiwen-CSViewer",&info);
		beidePath.SetTo(&(info.ref));
		_dontWorryPath = beidePath.Path();
	}
	else
	{	
		// on recupere le chemin de BeIDE
		beidePath.SetTo(&(info.ref));
		
		// on a un chemin valide
		beidePath.GetParent(&beidePath);
		_dontWorryPath = beidePath.Path();
		
		// on ajoute le reste	
		_dontWorryPath << "/plugins/Prefs_add_ons/DontWorryPlugin";
	}
}

CPreferences::~CPreferences()
{
	delete [] _filePath;
}

/**** charger les preferences ****/
status_t CPreferences::Load()
{
	BFile		file;

	if((_status=file.SetTo(_filePath,B_READ_ONLY))!=B_OK)
		return _status;
	
	// charger les donnees dans le message
	BMessage::MakeEmpty();
	_status = BMessage::Unflatten(&file);
	BMessage::FindInt32("use-keyspy",&_useDontWorry);
	BMessage::FindInt32("use-protection",&_useProtection);
	BMessage::FindInt32("use-info-parsing",&_useInfoParsing);
	BMessage::FindInt32("key-for-behappy",&_keyForBeHappy);
	BMessage::FindInt32("complete-word",&_useCompleteWord);
	BMessage::FindInt32("parameters",&_useParameters);
	BMessage::FindInt32("display-splash",&_DisplaySplash);
	BMessage::FindInt32("key-modifier",&_keyForModifiers);
	BMessage::FindInt32("key-sensitive",&_keySensitive);
	BMessage::FindInt32("down-search",&_downSearch);
				
	return _status;
}

/**** sauver les preferences ****/
status_t CPreferences::Save()
{
	BFile		file;
	status_t	status;

	if((status=file.SetTo(_filePath,B_CREATE_FILE|B_ERASE_FILE|B_WRITE_ONLY))!=B_OK)
		return status;
	
	// Sauver les donnees du message	
	BMessage::MakeEmpty();
	BMessage::AddInt32("use-keyspy",_useDontWorry);
	BMessage::AddInt32("use-protection",_useProtection);
	BMessage::AddInt32("use-info-parsing",_useInfoParsing);
	BMessage::AddInt32("key-for-behappy",_keyForBeHappy);
	BMessage::AddInt32("complete-word",_useCompleteWord);
	BMessage::AddInt32("parameters",_useParameters);
	BMessage::AddInt32("display-splash",_DisplaySplash);
	BMessage::AddInt32("key-modifier",_keyForModifiers);
	BMessage::AddInt32("key-sensitive",_keySensitive);
	BMessage::AddInt32("down-search",_downSearch);
	status = BMessage::Flatten(&file);
	
	return status;
}

/**** verifier si les prefs sont chargé ****/
status_t CPreferences::PreferencesLoaded()
{
	return _status;
}

/**** recuperer le chemin ****/
const char	*CPreferences::SettingPath()
{
	return	_filePath;
}

/**** verifier si on a bien la bonne touche (Fx + Modifier) ****/
bool CPreferences::AskKeyDontWorry(BMessage *message)
{
	int32 			key;
	int32			keyModifiers;
	status_t		state;

	// on doit trouver la touche Fx
	state = message->FindInt32("key", &key);
	state &= message->FindInt32("modifiers", &keyModifiers);
	if(state!=B_OK)
		return false;
	
	// attention key commence a 0 pour F1
	key--;
	if(key != _keyForBeHappy)	
		return false;

	// verifier maintenant le modifier (Ctrl/Alt/Shift)
	if(_keyForModifiers==0)
		return true;

	return (keyModifiers & _keyForModifiers);
}

/**** recuperer une image dans la ressource ****/
BBitmap *CPreferences::GetBitmap(const char *name)
{
	BResources		ressource;
	BBitmap 		*bitmap = NULL;
	size_t 			len = 0;
	status_t 		error;	
	BFile			file;
	BString			pluginName;
				
	// charger depuis les ressources
	// on peut ne pas trouver le fichier, essayer avec la be_app
	file.SetTo(_dontWorryPath.String(),B_READ_ONLY);
	if(file.InitCheck()!=B_OK)
		return NULL;
	
	ressource.SetTo(&file);
	const void *data = ressource.LoadResource('BBMP', name, &len);

	BMemoryIO stream(data, len);
	
	// charge l'image archivé
	BMessage archive;
	error = archive.Unflatten(&stream);
	if (error != B_OK)
		return NULL;

	// on va essayer de la recreer
	bitmap = new BBitmap(&archive);
	if(!bitmap)
		return NULL;

	// verifier que ca a marché
	if(bitmap->InitCheck() != B_OK)
	{
		delete bitmap;
		return NULL;
	}
	
	// tout c'est bien passé
	return bitmap;
}
