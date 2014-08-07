//========================================================================
//	CSettingMethodView.cpp
//========================================================================	

#include "AddOnConstantes.h"
#include "CSettingMethodView.h"
#include "PlugInPreferences.h"
#include "CMethodBuilder.h"
#include "CPreferences.h"

#include <Application.h>
#include <string.h>
#include <View.h>
#include <Box.h>
#include <ByteOrder.h>
#include <StringView.h>
#include <ScrollView.h>
#include <Window.h>
#include <Message.h>
#include <Control.h>
#include <CheckBox.h>
#include <Box.h>
#include <Bitmap.h>
#include <String.h>
#include <Button.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <MenuField.h>
#include <AppFileInfo.h>
#include <StringView.h>

// debug
//#include <iostream.h>

#if __POWERPC__
#pragma export on
#endif
extern "C" {
status_t MakeAddOnView(long inIndex, BRect inRect, MPlugInPrefsView*& ouView);
status_t MakeAddOnBuilder(long inIndex, MPlugInBuilder*& outBuilder);
}
#if __POWERPC__
#pragma export reset
#endif

const char *		PREFS_TITLE = "Language/DontWorry Tool";

// messages
const uint32		KEYSPY_PREFERENCES		= 'Uksp';
const uint32		USE_KEYSPY_MSG			= 'Uksm';
const uint32		USE_PROTECTION_MSG		= 'Upro';
const uint32		USE_INFODISPLAY_MSG		= 'Umip';
const uint32		KEY_FORBEHAPPY_MSG		= 'Kfbm';
const uint32		KEY_FORMODIFIER_MSG		= 'Kfmm';
const uint32		USE_PARAMETERS_MSG		= 'Upam';
const uint32		USE_AUTOCOMPLETE_MSG	= 'Uacm';
const uint32		USE_KEYSENSITIVE_MSG	= 'Ukse';
const uint32		USE_DOWNSEARCH_MSG		= 'Usdw';

// ---------------------------------------------------------------------------
//	MakeAddOnView
// ---------------------------------------------------------------------------

status_t MakeAddOnView(int32 inIndex,BRect inRect,MPlugInPrefsView*& outView)
{
	long		result = B_ERROR;

	if (inIndex == 0)
	{
		outView = new CSettingMethodView(inRect, "Methods-spy", 0, 0);
		result = B_NO_ERROR;
	}

	return result;	
}

// ---------------------------------------------------------------------------
//	MakeAddOnBuilder
// ---------------------------------------------------------------------------

status_t MakeAddOnBuilder(int32 inIndex,MPlugInBuilder*& outBuilder)
{
	status_t		result = B_ERROR;

	if (inIndex == 0)
	{
		outBuilder = new CMethodBuilder;
		result = B_NO_ERROR;
	}
	
	return result;	
}

// ---------------------------------------------------------------------------
//	CSettingMethodView
// ---------------------------------------------------------------------------
//	Constructor

CSettingMethodView::CSettingMethodView(BRect inFrame,const char* inName,ulong inResizeMask,ulong inFlags)
: MPlugInPrefsView(inFrame, inName, inResizeMask, inFlags)
{
	_prefs = NULL;
	_prefs = new CPreferences(PREF_FILE_NAME,PREF_PATH_NAME);
	_prefs->Load();	
}

/**** Destructeur ****/
CSettingMethodView::~CSettingMethodView()
{
	// liberer la memoire prise par les images
	delete		_classesBitmap;
	delete		_metodesBitmap;
	delete		_variablesBitmap;
	delete		_protectedBitmap;
	delete		_privateBitmap;
	delete		_virtualBitmap;

	// detruire les prefs
	delete _prefs;
}

/**** quand on detache la vue de la fenetre ****/
void CSettingMethodView::DetachedFromWindow()
{
	// on detache tout de meme la vue
	BView::DetachedFromWindow();
}

// ---------------------------------------------------------------------------
//	Title
// ---------------------------------------------------------------------------

const char *CSettingMethodView::Title()
{
	return PREFS_TITLE;
}

// ---------------------------------------------------------------------------
//	Targets
// ---------------------------------------------------------------------------

TargetT CSettingMethodView::Targets()
{
	return (kMWDefaults | kCurrentProject);
}

// ---------------------------------------------------------------------------
//	GetPointers
// ---------------------------------------------------------------------------
//	Provide the addresses and length of the new and old structs that hold
//	the data for this view.  If the data isn't held in a simple struct
//	then don't return any values.  If the values are returned then 
//	Revert will be handled automatically.

void CSettingMethodView::GetPointers(void*& outOld,void*& outNew,long&	outLength)
{
}

// ---------------------------------------------------------------------------
//	MessageReceived
// ---------------------------------------------------------------------------
void CSettingMethodView::MessageReceived(BMessage *inMessage)
{
	switch(inMessage->what)
	{
	case USE_KEYSPY_MSG:
	case USE_PROTECTION_MSG:
	case USE_INFODISPLAY_MSG:
	case USE_PARAMETERS_MSG:
	case USE_AUTOCOMPLETE_MSG:
	case USE_KEYSENSITIVE_MSG:
	case USE_DOWNSEARCH_MSG:
		ValueChanged();
		break;
	case REFRESH_SYSLIB_MSG:
		be_app->PostMessage(inMessage);
		break;
	case KEY_FORBEHAPPY_MSG:
		SetKeyForBeHappy(inMessage);
		ValueChanged();
		break;
	case KEY_FORMODIFIER_MSG:
		SetModifierKeyForBeHappy(inMessage);
		ValueChanged();
		break;
	default:
		MPlugInPrefsView::MessageReceived(inMessage);
	}
}

/**** AttachedToWindow ****/
void CSettingMethodView::AttachedToWindow()
{
	BRect			bounds = Bounds();
	BBox			*box = NULL;

	// on attache tout de meme la vue
	BView::AttachedToWindow();

	// ajouter les composants graphique
	box = new BBox(bounds, "dontworry-settings");
	box->SetLabel("DontWorry Settings");
	box->SetFont(be_bold_font);
	AddChild(box);

	// initialisees les images
	_classesBitmap = _prefs->GetBitmap("classes");
	_metodesBitmap = _prefs->GetBitmap("metodes");
	_variablesBitmap = _prefs->GetBitmap("variables");
	_protectedBitmap = _prefs->GetBitmap("protected");
	_privateBitmap = _prefs->GetBitmap("private");
	_virtualBitmap = _prefs->GetBitmap("virtual");

	// les legendes
	_legend = new BBox(BRect(10,20,bounds.right-60,80),"Legend");
	_legend->SetLabel("Legend");
	box->AddChild(_legend);
	SetGrey(_legend);

	// ajouter les icones
	BView			*bitmapView = NULL;
	BStringView		*stringView = NULL;
	
	bitmapView = new BView(BRect(15,15,31,31),"classeBitmap",B_FOLLOW_NONE,B_WILL_DRAW);
	stringView = new BStringView(BRect(35,15,85,31),NULL,"Classes",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	_legend->AddChild(bitmapView);
	_legend->AddChild(stringView);
	bitmapView->SetViewBitmap(_classesBitmap);

	bitmapView = new BView(BRect(95,15,111,31),"metodeBitmap",B_FOLLOW_NONE,B_WILL_DRAW);
	stringView = new BStringView(BRect(115,15,165,31),NULL,"Methodes",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	_legend->AddChild(bitmapView);
	_legend->AddChild(stringView);
	bitmapView->SetViewBitmap(_metodesBitmap);

	bitmapView = new BView(BRect(175,15,191,31),"memberBitmap",B_FOLLOW_NONE,B_WILL_DRAW);
	stringView = new BStringView(BRect(195,15,285,31),NULL,"Members Attributs",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	_legend->AddChild(bitmapView);
	_legend->AddChild(stringView);
	bitmapView->SetViewBitmap(_variablesBitmap);

	bitmapView = new BView(BRect(15,35,25,45),"protectedBitmap",B_FOLLOW_NONE,B_WILL_DRAW);
	stringView = new BStringView(BRect(35,35,85,51),NULL,"Protected",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	_legend->AddChild(bitmapView);
	_legend->AddChild(stringView);
	bitmapView->SetViewBitmap(_protectedBitmap);

	bitmapView = new BView(BRect(95,35,105,45),"privateBitmap",B_FOLLOW_NONE,B_WILL_DRAW);
	stringView = new BStringView(BRect(115,35,165,51),NULL,"Private",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	_legend->AddChild(bitmapView);
	_legend->AddChild(stringView);
	bitmapView->SetViewBitmap(_privateBitmap);

	bitmapView = new BView(BRect(175,35,191,51),"virtualBitmap",B_FOLLOW_NONE,B_WILL_DRAW);
	stringView = new BStringView(BRect(195,35,285,51),NULL,"Virtual",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	_legend->AddChild(bitmapView);
	_legend->AddChild(stringView);
	bitmapView->SetViewBitmap(_virtualBitmap);

	// option pour l'utilisation de l'addon
	_useOption = new BCheckBox(BRect(10,85,bounds.right-15,100),"Use-Option","Use DontWorry",new BMessage(USE_KEYSPY_MSG));
	_useOption->SetTarget(this);
	box->AddChild(_useOption);
	SetGrey(_useOption);

	// option pour l'utilisation de la protection private/protected
	_useProtection = new BCheckBox(BRect(10,105,bounds.right-15,120),"Use-Protection","Show protected/private members.",new BMessage(USE_PROTECTION_MSG));
	_useProtection->SetTarget(this);
	box->AddChild(_useProtection);
	SetGrey(_useProtection);

	// afficher des informations complementaire sur le parsing
	_useInfoDisplay = new BCheckBox(BRect(10,125,bounds.right-15,140),"Use-Info-Display","Display more info about current parsing.",new BMessage(USE_INFODISPLAY_MSG));
	_useInfoDisplay->SetTarget(this);
	box->AddChild(_useInfoDisplay);
	SetGrey(_useInfoDisplay);

	// afficher des informations complementaire sur le parsing
	_useParameters = new BCheckBox(BRect(10,145,bounds.right-15,160),"Use-Parameters","Always add parameters of a method.",new BMessage(USE_PARAMETERS_MSG));
	_useParameters->SetTarget(this);
	box->AddChild(_useParameters);
	SetGrey(_useParameters);

	// afficher des informations complementaire sur le parsing
	_useCompWord = new BCheckBox(BRect(10,165,bounds.right-15,180),"Use-Comp-Word","Use auto complete variables.",new BMessage(USE_AUTOCOMPLETE_MSG));
	_useCompWord->SetTarget(this);
	box->AddChild(_useCompWord);
	SetGrey(_useCompWord);

	// afficher le splash-screen
	_displaySplash = new BCheckBox(BRect(10,185,bounds.right-15,200),"display-splash","Display Splash-Screen.",new BMessage(USE_AUTOCOMPLETE_MSG));
	_displaySplash->SetTarget(this);
	box->AddChild(_displaySplash);
	SetGrey(_displaySplash);

	// la recherche tient compte ou pas des Majuscules
	_keySensitive = new BCheckBox(BRect(10,205,bounds.right-15,220),"key-sensitive","Key Sensitive.",new BMessage(USE_KEYSENSITIVE_MSG));
	_keySensitive->SetTarget(this);
	box->AddChild(_keySensitive);
	SetGrey(_keySensitive);

	// commencer la recherche par le bas
	_downSearch = new BCheckBox(BRect(10,225,bounds.right-15,240),"search-down","Start Serach by Bottom.",new BMessage(USE_DOWNSEARCH_MSG));
	_downSearch->SetTarget(this);
	box->AddChild(_downSearch);
	SetGrey(_downSearch);

	// creation du menu pour la touche d'aide
	BPopUpMenu	*menu = NULL;
	BString		menuName;

	_keyForBeHappyIndex = 1;
	if(_prefs->PreferencesLoaded()==B_OK)
		_keyForBeHappyIndex = _prefs->_keyForBeHappy;
	
	menuName = "F";
	menuName << _keyForBeHappyIndex;
	if(_keyForBeHappyIndex==13)
		menuName = "Space";
	
	menu = new BPopUpMenu(menuName.String());	
	menu->AddItem(new BMenuItem("F1",new BMessage(KEY_FORBEHAPPY_MSG)));
	menu->AddItem(new BMenuItem("F2",new BMessage(KEY_FORBEHAPPY_MSG)));
	menu->AddItem(new BMenuItem("F3",new BMessage(KEY_FORBEHAPPY_MSG)));
	menu->AddItem(new BMenuItem("F4",new BMessage(KEY_FORBEHAPPY_MSG)));
	menu->AddItem(new BMenuItem("F5",new BMessage(KEY_FORBEHAPPY_MSG)));
	menu->AddItem(new BMenuItem("F6",new BMessage(KEY_FORBEHAPPY_MSG)));
	menu->AddItem(new BMenuItem("F7",new BMessage(KEY_FORBEHAPPY_MSG)));
	menu->AddItem(new BMenuItem("F8",new BMessage(KEY_FORBEHAPPY_MSG)));
	menu->AddItem(new BMenuItem("F9",new BMessage(KEY_FORBEHAPPY_MSG)));
	menu->AddItem(new BMenuItem("F10",new BMessage(KEY_FORBEHAPPY_MSG)));
	menu->AddItem(new BMenuItem("F11",new BMessage(KEY_FORBEHAPPY_MSG)));
	menu->AddItem(new BMenuItem("F12",new BMessage(KEY_FORBEHAPPY_MSG)));
	menu->AddItem(new BMenuItem("Space",new BMessage(KEY_FORBEHAPPY_MSG)));
	menu->SetTargetForItems(this);
	
	// touche clavier pour BeHappy
	_keyForBeHappy = new BMenuField(BRect(230,245,280,260),"help-key","",menu);
	_keyForBeHappy->SetDivider(0);
	box->AddChild(_keyForBeHappy);
	SetGrey(_keyForBeHappy);	
	
	// touche speciale Ctrl/Alt etc ...
	menuName.SetTo("");

	_keyModifiers = 32;
	if(_prefs->PreferencesLoaded()==B_OK)
		_keyModifiers = _prefs->_keyForModifiers;

	switch(_keyModifiers)
	{
		case B_SHIFT_KEY:
			menuName = "Shift";
			break;
		case B_COMMAND_KEY:
			menuName = "Ctrl";
			break;
		case B_OPTION_KEY:
			menuName = "Option";
			break;
		case B_MENU_KEY:
			menuName = "Menu";
			break;
		case 0:
			menuName = "None";
	}

	menu = new BPopUpMenu(menuName.String());	
	menu->AddItem(new BMenuItem("None",new BMessage(KEY_FORMODIFIER_MSG)));
	menu->AddItem(new BMenuItem("Shift",new BMessage(KEY_FORMODIFIER_MSG)));
	menu->AddItem(new BMenuItem("Ctrl",new BMessage(KEY_FORMODIFIER_MSG)));
	menu->AddItem(new BMenuItem("Option",new BMessage(KEY_FORMODIFIER_MSG)));
	menu->AddItem(new BMenuItem("Menu",new BMessage(KEY_FORMODIFIER_MSG)));
	menu->SetTargetForItems(this);
	
	// touche clavier pour BeHappy
	_keyForModifiers = new BMenuField(BRect(10,245,335,260),"modifier-key","Help Key & Complete word Key",menu);
	box->AddChild(_keyForModifiers);
	SetGrey(_keyForModifiers);

	// bloquer le menu si c'est space
	if(_keyForBeHappyIndex==13)
		_keyForModifiers->SetEnabled(false);
	
	// bouton pour rafraichir la lib system
	_refreshSysLib = new BButton(BRect(10,bounds.bottom-30,(bounds.right/2)-25,bounds.bottom-10),"refresh","Refresh SysLib",new BMessage(REFRESH_SYSLIB_MSG));
	_refreshSysLib->SetTarget(this);
	box->AddChild(_refreshSysLib);
	SetGrey(_refreshSysLib);	
	
	// info sur les auteurs he he nous en fait :-)
	BStringView	*creators;
	creators = new BStringView(BRect((bounds.right/2)-20,bounds.bottom-25,bounds.right-2,bounds.bottom-5),"info-autors","Created By Sylvain Tertois & Cedric Vincent");
	box->AddChild(creators);
	SetGrey(creators);	
	
	SetGrey(box);
		
	// mettre a jour les donnees
	UpdateValues();
}

/**** mettre a jour les donnees ****/
void CSettingMethodView::UpdateValues()
{
	if(_prefs->PreferencesLoaded()==B_OK)
	{
		_useOption->SetValue(_prefs->_useDontWorry);
		_useProtection->SetValue(_prefs->_useProtection);
		_useInfoDisplay->SetValue(_prefs->_useInfoParsing);
		_useCompWord->SetValue(_prefs->_useCompleteWord);
		_useParameters->SetValue(_prefs->_useParameters);
		_displaySplash->SetValue(_prefs->_DisplaySplash);
		_keySensitive->SetValue(_prefs->_keySensitive);
		_downSearch->SetValue(_prefs->_downSearch);
	}
}

// ---------------------------------------------------------------------------
//	ValueChanged
// ---------------------------------------------------------------------------
//	Notify the preferences window that one of the values in the view has 
//	changed.

void CSettingMethodView::ValueChanged()
{
	_prefs->_useDontWorry = _useOption->Value();
	_prefs->_useProtection = _useProtection->Value();
	_prefs->_useInfoParsing = _useInfoDisplay->Value();
	_prefs->_useCompleteWord = _useCompWord->Value();
	_prefs->_useParameters = _useParameters->Value();
	_prefs->_DisplaySplash = _displaySplash->Value();
	_prefs->_keyForBeHappy = _keyForBeHappyIndex;
	_prefs->_keyForModifiers = _keyModifiers;
	_prefs->_keySensitive = _keySensitive->Value();
	_prefs->_downSearch = _downSearch->Value();
	
	Window()->PostMessage(msgPluginViewModified);
}

// ---------------------------------------------------------------------------
//	DoSave
// ---------------------------------------------------------------------------
//	Called after the view's contents have been sent to the target.  If the
//	preferences for this view can be represented as a simple struct then
//	return valid values from the GetPointers function.  If the prefs are 
//	stored in some other way then this function should copy the new
//	prefs values to the old prefs values.  If valid values are returned
//	in GetPointers then this function won't be called.

void CSettingMethodView::DoSave()
{
	_prefs->_useDontWorry = _useOption->Value();
	_prefs->_useProtection = _useProtection->Value();
	_prefs->_useInfoParsing = _useInfoDisplay->Value();
	_prefs->_useCompleteWord = _useCompWord->Value();
	_prefs->_useParameters = _useParameters->Value();
	_prefs->_DisplaySplash = _displaySplash->Value();
	_prefs->_keyForBeHappy = _keyForBeHappyIndex;
	_prefs->_keyForModifiers = _keyModifiers;
	_prefs->_keySensitive = _keySensitive->Value();
	_prefs->_downSearch = _downSearch->Value();

	// souver les prefs
	_prefs->Save();

	// mettre a jour l'ajout de l'addon !!!
	be_app->PostMessage(PREFS_CHANGED_MSG);
}

// ---------------------------------------------------------------------------
//	DoRevert
// ---------------------------------------------------------------------------
//	Called when the revert button has been hit.  If valid values are returned
//	in GetPointers then this function won't be called.

void CSettingMethodView::DoRevert()
{
}

// ---------------------------------------------------------------------------
//	DoFactorySettings
// ---------------------------------------------------------------------------
//	Update all the fields in the view to reflect the factory settings, or 
//	built-in defaults.

void CSettingMethodView::DoFactorySettings()
{
}

// ---------------------------------------------------------------------------
//	FilterKeyDown
// ---------------------------------------------------------------------------
//	This function allows views to handle tab keys or any other special keys
//	before the keydown is passed to the target view.  Must return false if
//	the key is not to be passed to the target view.
//	We don't do anything special with keys in this plugin.

bool CSettingMethodView::FilterKeyDown(ulong aKey)
{
	return false;
}

/**** on retourne false pour ne pas obliger la recompilation ou le relinkage ****/
bool CSettingMethodView::ProjectRequiresUpdate(UpdateType inType)
{
	return false;
}

/**** dessiner la vue en gris ****/
void CSettingMethodView::SetGrey(BView* inView)
{
	inView->SetViewColor(kPrefsGray);
	inView->SetLowColor(kPrefsGray);
}

/**** recuperer l'icon pour s'en servir comme image ****/
BBitmap *CSettingMethodView::GetBitmapFromIcon(const char *file)
{
	BBitmap			*result = NULL;
	BAppFileInfo	fileinfo;
	BFile			picture;
	
	result = new BBitmap(BRect(0,0,31,31 ), B_CMAP8);
	picture.SetTo(file,B_READ_ONLY);
	if(picture.InitCheck()!=B_OK)
		return result;

	fileinfo.SetTo(&picture);
	fileinfo.GetIcon(result, B_LARGE_ICON);
	
	return result;
}

/**** mettre a jour l'option de selection pour BeHappy ****/
void CSettingMethodView::SetKeyForBeHappy(BMessage *msg)
{
	int32	index = -1;
	bool	spaceSelected = false;
	
	if(msg->FindInt32("index",&index)!=B_OK)
		return;
		
	_keyForBeHappyIndex = index +1;
	spaceSelected = (_keyForBeHappyIndex == 13);

	// gerer l'espace
	if(spaceSelected)
	{
		_keyModifiers = B_COMMAND_KEY;
		_keyForModifiers->Menu()->ItemAt(2)->SetMarked(true);
		_keyForModifiers->Menu()->ItemAt(2)->SetMarked(false);
	}
	// on bloque ou pas le menu
	_keyForModifiers->SetEnabled(!spaceSelected);
}

/**** mettre a jour l'option Ctrl/Alt/Shift ****/
void CSettingMethodView::SetModifierKeyForBeHappy(BMessage *msg)
{
	int32	index = -1;

	if(msg->FindInt32("index",&index)!=B_OK)
		return;

	switch(index)
	{
		case 0:
			_keyModifiers = 0;
			break;
		case 1:
			_keyModifiers = B_SHIFT_KEY;
			break;
		case 2:
			_keyModifiers = B_COMMAND_KEY;
			break;
		case 3:
			_keyModifiers = B_OPTION_KEY;
			break;
		case 4:
			_keyModifiers = B_MENU_KEY;
			break;
	}
}