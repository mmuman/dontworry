#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include <Message.h>
#include <String.h>

#define		C_DISPATCH_PREFS			'disp'
#define		PREF_FILE_NAME				"DontWorry.prefs"
#define		PREF_PATH_NAME				"AddOnBeIDE"

class BBitmap;

class CPreferences:public BMessage
{
protected:
	status_t	_status;
	char		*_filePath;

public:
				CPreferences(const char *fileName, const char *filePath=NULL);
				~CPreferences();
	
	// charger / sauver les prefs
	status_t	Load();
	status_t	Save();
	
	// verifier si les prefs sont chargées
	status_t	PreferencesLoaded();
	// recuperer le chemin
	const char	*SettingPath();
	// verifier si on a bien la bonne touche (Fx + Modifier)
	bool		AskKeyDontWorry(BMessage *);
	BBitmap		*GetBitmap(const char *name);
	
	// donnees
	int32	_useDontWorry;
	int32	_useProtection;
	int32	_useInfoParsing;
	int32	_keyForBeHappy;
	int32	_useCompleteWord;
	int32	_useParameters;
	int32	_DisplaySplash;
	int32	_keyForModifiers;
	int32	_keySensitive;
	int32	_downSearch;
	BString	_dontWorryPath;

};

#endif
