/****************************************/
/* fenetre d'affichage des informations */
/****************************************/

#ifndef _ADDONLISTINFO_H
#define _ADDONLISTINFO_H

#include <Window.h>
#include <Messenger.h>
#include <List.h>

class BView;
class AddOnListView;
class AddOnProgressView;
class AddOnListItem;
class BMessage;
class BBitmap;

class AddOnListInfo : public BWindow
{
public:
	AddOnListInfo(BRect frame,BView *mother);
	~AddOnListInfo();	

	virtual void	MessageReceived(BMessage *message);
	virtual bool	QuitRequested();

protected:
	BMessenger			_motherMessenger;
	AddOnListView		*_listOfInfos;
	AddOnProgressView	*_progress;
	AddOnListItem		*_lastClass;
	int8				_typeRequested;
	// images
	BBitmap		*_classesBitmap;
	BBitmap		*_metodesBitmap;
	BBitmap		*_variablesBitmap;
	BBitmap		*_protectedBitmap;
	BBitmap		*_privateBitmap;
	BBitmap		*_virtualBitmap;

	void		AddDatasList(BMessage *message);
	void		AddCompletionDatas(BMessage *message);
	void		AddText(BMessage *message);

};

#endif _ADDONLISTINFO_H