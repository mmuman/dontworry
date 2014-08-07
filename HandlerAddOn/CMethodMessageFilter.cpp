/************************/
/* CMethodMessageFilter */
/************************/

#include <Application.h>
#include <Window.h>
#include <Handler.h>
#include "AddOnConstantes.h"
#include "CMethodMessageFilter.h"

/**** constructeur ****/
CMethodMessageFilter::CMethodMessageFilter(BHandler *redirect)
: BMessageFilter(B_ANY_DELIVERY,B_ANY_SOURCE),_redirect(redirect)
{}

/**** destructeur ****/
CMethodMessageFilter::~CMethodMessageFilter()
{}

/**** intercepter les messages et si ils nous concerne les envoyer a notre BHandler ****/	
filter_result CMethodMessageFilter::Filter(BMessage* message, BHandler **target)
{
	switch(message->what)
	{
	case NEW_FILE_MSG: // utilisation du menu "new"
	case OPEN_FILE_MSG:
	case PREFS_CHANGED_MSG:
	case UPDATE_FILE_MSG:
	case LAUNCH_SEARCH_MSG:
	case CLOSE_WINDOW_MSG:
	case REFRESH_SYSLIB_MSG:
	case BH_ADDON_SHOW:
	case REFRESH_HEADER_MSG:
		_redirect.SendMessage(message);
		break;
	default:
		;
	}

	return B_DISPATCH_MESSAGE;
}