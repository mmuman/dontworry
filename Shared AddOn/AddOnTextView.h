/*
 * Vue incructée dans la fenêtre texte
 *
 */

#ifndef ADDONTEXTVIEW_H
#define ADDONTEXTVIEW_H

#include <View.h>
#include <StringView.h>
#include <String.h>

class MTextAddOn;
class AddOnButton;
class CSPSourceFile;
class AddOnTextExtractor;

class AddOnTextView : public BView
{
public:
	AddOnTextView(const char *pathFile);
	~AddOnTextView();
	
	virtual	void	Draw(BRect frame);
	virtual void	AttachedToWindow();
	virtual void	DetachedFromWindow();
	virtual void	MessageReceived(BMessage* message);
	virtual void	FrameResized(float width,float height);
	virtual	void	WindowActivated(bool active);
	// appelée quand l'utilisateur a appuyé sur une touche intéressante
	void AddOnCalled(MTextAddOn*);

	inline	BMessage	*AddOnMessage()		{ return _AddOnMessage; }

private:
	BString				_currentVariable;
	BString				_currentText;
	BString				_currentClasse;
	BString				_currentPathFile;
	BString				_currentSelection;
	BStringView			*_StringView;
	AddOnButton			*_addOnButton;
	BMessage			*_AddOnMessage;
	CSPSourceFile		*_CPPParser;
	int8				_typeAsk;
	bool				_askCompletion;
	bool				_askRefreshHParsing;
	AddOnTextExtractor	*_extractor;
	bool				_parseWithoutError;
	int32				_extractLevel;
	bool				_varIsLocal;
	int8				_addTextOption;
	
	void		Parse(const char *currentText,int32 start,int32 end);
	void		SendInformationToHandler();
	void		GetAddText(BMessage *);
	void		DisplaySearchBuffer(BMessage *);
	int8		SelectTypeAsk(const char *currentText,int32 &start,int32 &end);
	void		DisplayInormations();
	void		UpdateFile();
};

#endif ADDONTEXTVIEW_H