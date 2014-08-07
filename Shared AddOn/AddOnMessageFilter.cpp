/**********************/
/* AddOnMessageFilter */
/**********************/

#include "AddOnMessageFilter.h"
#include "AddOnConstantes.h"
#include <Looper.h>
#include <Messenger.h>
#include <InterfaceDefs.h>

#include "CPreferences.h"

// debbuger
//#include <iostream.h>

AddOnMessageFilter::AddOnMessageFilter(BMessage *messageToSend,BHandler *textView)
: BMessageFilter(B_ANY_DELIVERY,B_ANY_SOURCE),_messageToSend(*messageToSend),_textView(textView)
{
	// pointer sur les preferences
	_prefs = new CPreferences(PREF_FILE_NAME,PREF_PATH_NAME);
}

AddOnMessageFilter::~AddOnMessageFilter()
{
	// liberer la memoire
	delete _prefs;
}

filter_result AddOnMessageFilter::Filter(BMessage* message, BHandler **target)
{
	if(message->what == '_KYD')
	{
		int8 	letter=0;

		message->FindInt8("byte",&letter);

		switch(letter)
		{
		case '.':
		case '>':
		case ':':
			Looper()->PostMessage(&_messageToSend);
			break;			
		case B_FUNCTION_KEY:	// pour la completion ou l'appel de l'addon
			{
				// les preferences doivent etre valide
				if(_prefs==NULL)
					break;
				
				// verifier si c'est la bonne touche
				_prefs->Load();
				if(_prefs->AskKeyDontWorry(message))
				{
					// on demande la completion
					_textView.SendMessage(CALL_COMPTETE_WORD);
					Looper()->PostMessage(&_messageToSend);

					return B_SKIP_MESSAGE;	
				}
			}
		}
	}
	
	// renvoyer le message a son destinataire
	return B_DISPATCH_MESSAGE;
}