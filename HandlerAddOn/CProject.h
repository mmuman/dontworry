/************/
/* CProject */
/************/

#ifndef _CPROJECT_H
#define _CPROJECT_H

#include <String.h>
#include <Path.h>
#include <List.h>
#include <Messenger.h>

class CSLibrary;
class BFile;
class BWindow;

class CProject
{
public:
	CProject(const char *projectName,const char *projectPath,BWindow *progressWindow);
	virtual ~CProject();

	void		AddPath(const char *path);
	void		AddFile(const char *file);
	void		RemoveFiles(BMessage *);
	BMessage	*FindNewFile(BMessage *);
	status_t	FindFile(const char *pathFile);
	void		BlinkIcon();
 	status_t InitCheck();
  	status_t Parse(bool directory = false, unsigned int recurse_dir = 1);

	inline	BString		Name()								{ return _projectName; }
	inline	CSLibrary	*Library()							{ return _CSUserLib; }
	inline	void		SetBlinkMessenger(BHandler *value)	{ _iconView = BMessenger(value); }

private:
	BString			_projectName;
	BPath			_projectFilePath;
	BList			_projectPath;
	BList			_projectFile;
	BFile			*_FileUserLib;
	CSLibrary		*_CSUserLib;
	BMessenger		_progressWindow;
	BMessenger		_iconView;
};

#endif