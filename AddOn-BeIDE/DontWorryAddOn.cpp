//	Test add-on.

#include "AddOnConstantes.h"
#include "AddOnTextView.h"
#include "MTextAddOn.h"
#include <Window.h>
#include <View.h>

#include <TextView.h>

#if __POWERPC__
#pragma export on
#endif
extern "C" {
__declspec(dllexport) status_t perform_edit(MTextAddOn *addon);
}

#if __POWERPC__
#pragma export reset
#endif
/* Public interface for the add-on */
long perform_edit(MTextAddOn *addon)
{
	BWindow			*editWindow = addon->Window();
	AddOnTextView	*l_AddOnView;

	// la nouvelle vue est-elle installée?
	// si non! ben on utilise peut etre pas l'addon
	if ((l_AddOnView = (dynamic_cast<AddOnTextView *>(editWindow->FindView(ADDON_VIEW_NAME)))) == NULL)
		return B_NO_ERROR;	
	else
		l_AddOnView->AddOnCalled(addon);
	
	return B_NO_ERROR;
}