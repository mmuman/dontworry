/**********************/
/* AddOnMessageFilter */
/**********************/

#include "AddOnWindowFilter.h"
#include "AddOnConstantes.h"
#include "AddOnTextView.h"
#include <Looper.h>
#include <InterfaceDefs.h>
#include <Message.h>
#include <Window.h>
#include <Messenger.h>

// debbuger
#include <stdio.h>

AddOnWindowFilter::AddOnWindowFilter(float height,AddOnTextView *textView)
: BMessageFilter(B_ANY_DELIVERY,B_ANY_SOURCE)
{
	_height = height;
	_textView = textView;
	_winParsing = NULL;
}

AddOnWindowFilter::~AddOnWindowFilter()
{}

filter_result AddOnWindowFilter::Filter(BMessage* message, BHandler **target)
{
	switch(message->what)
	{
	// recuperer le pointer de la fenetre de parsing
	case MOVE_WINDOW_PTR:
		if(message->FindPointer(WINDOW_RESULT_PTR,(void **)(&_winParsing))!=B_OK)
			_winParsing = NULL;
		break;
	case B_WINDOW_MOVED: // bouger la fenetre
	{
			// on doit alors bouger la fenetre de resultat si elle existe
			if(_winParsing!=NULL)
			{
				BPoint		newPosition;

				// on doit trouver la nouvelle position
				if(message->FindPoint("where",&newPosition)==B_OK)
				{
					newPosition.y += _height;
					_winParsing->MoveTo(newPosition);
				}
			}
		}
		break;
		case 0x5:	// sauver
		{
			BMessenger(_textView).SendMessage(UPDATE_FILE_MSG);
		}
		break;
	}

	return B_DISPATCH_MESSAGE;
}