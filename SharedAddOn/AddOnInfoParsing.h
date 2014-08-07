/*************************************/
/* fenetre de progression du parsing */
/*************************************/

#ifndef _ADDONINFOPARSING_H
#define _ADDONINFOPARSING_H

#include <Window.h>
#include <View.h>
#include <String.h>

class BBitmap;

class DisplayView : public BView
{
public:
	DisplayView(BRect frame,bool splashScreen,bool aboutBox);
	~DisplayView();	

	virtual void	Draw(BRect updateRect);

			void	SetNbFiles(float nbFiles);
	inline	void	IncreaseCounter()			{ _counter += _counterStep; }
	inline	void	InvalidateStatus()			{ Invalidate(_parsingRect); }
	
protected:
	BString		_displayText;
	bool		_splashScreen;
	bool		_aboutBox;
	BBitmap		*_screenBitmap;
	float		_nbFiles;
	float		_counter;
	float		_counterStep;
	BRect		_parsingRect;
};

class AddOnInfoParsing : public BWindow
{
public:
	AddOnInfoParsing(bool splashScreen = false, bool aboutBox = false);
	~AddOnInfoParsing();	

	virtual void	MessageReceived(BMessage *message);

protected:
	DisplayView	*_view;

	BRect		GetFrame(bool splashScreen,bool aboutBox);
};

#endif _ADDONINFOPARSING_H