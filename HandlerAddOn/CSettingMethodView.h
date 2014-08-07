//========================================================================
//	CSettingMethodView.h
//========================================================================	

#ifndef _MSETTINGMETHODVIEW_H
#define _MSETTINGMETHODVIEW_H

#include "MPlugInPrefsView.h"
#include <Entry.h>

class BMessage;
class BCheckBox;
class BBox;
class CPreferences;
class BBitmap;
class BButton;
class BMenuField;

class CSettingMethodView : public MPlugInPrefsView
{
public:
	CSettingMethodView(BRect inFrame,const char* inName,ulong inResizeMask,ulong inFlags);
	virtual	~CSettingMethodView();

	virtual void			AttachedToWindow();
	virtual void			DetachedFromWindow();
	virtual void			MessageReceived(BMessage* inMessage);
	virtual const char *	Title();
	virtual TargetT			Targets();
	virtual void			GetPointers(void*& outOld,void*& outNew,long& outLength);
	virtual void			DoSave();
	virtual void			DoRevert();
	virtual void			DoFactorySettings();
	virtual	bool			FilterKeyDown(ulong aKey);	
	virtual	bool			ProjectRequiresUpdate(UpdateType inType);
	virtual void			ValueChanged();
	virtual void			UpdateValues();

	inline virtual void			GetData(BMessage& inOutMessage) {}
	inline virtual void			SetData(BMessage& inMessage) {}

protected:
	CPreferences	*_prefs;
	BBox			*_legend;
	BCheckBox		*_useOption;
	BCheckBox		*_useProtection;
	BCheckBox		*_useInfoDisplay;
	BCheckBox		*_useParameters;
	BCheckBox		*_useCompWord;
	BCheckBox		*_displaySplash;
	BCheckBox		*_keySensitive;
	BCheckBox		*_downSearch;
	BButton			*_refreshSysLib;
	BMenuField		*_keyForBeHappy;
	BMenuField		*_keyForModifiers;
	int32			_useThisTool;
	int32			_keyForBeHappyIndex;
	int32			_keyModifiers;

	// images
	BBitmap		*_classesBitmap;
	BBitmap		*_metodesBitmap;
	BBitmap		*_variablesBitmap;
	BBitmap		*_protectedBitmap;
	BBitmap		*_privateBitmap;
	BBitmap		*_virtualBitmap;
	
	void			SetGrey(BView *);
	BBitmap			*GetBitmapFromIcon(const char *file);
	void			SetKeyForBeHappy(BMessage *msg);
	void			SetModifierKeyForBeHappy(BMessage *msg);
};

#endif
