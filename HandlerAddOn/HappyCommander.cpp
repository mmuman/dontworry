/* HappyCommander class
 *
 * Written by Sylvain Tertois (Sylvain.Tertois@fnac.net) to show how to use
 * BeHappy's scripting capabilities
 */

// constants
static const char *behappySignature = "application/x.vnd-STertois.BeHappy";
static const char *behappyAddOnPath = "Add-ons";
static const char *alertText = "Would you like to install MyApp's documentation for BeHappy?";
static const char *alertText2 = "Add-on successfully installed!";

#include "HappyCommander.h"
#include <Entry.h>
#include <Roster.h>
#include <Directory.h>
#include <Application.h>
#include <Alert.h>
#include <String.h>
#include <Path.h>

HappyCommander::HappyCommander(const char *an, const char *afn, const char *hi)
: addOnName(an),addOnFileName(afn),htmlIndex(hi)
{
	addOnInstalled = false;
}

HappyCommander::~HappyCommander()
{
}

int HappyCommander::InstallAddOn()
{
	// Search if BeHappy is there
	entry_ref behappyRef;
	if (be_roster->FindApp(behappySignature,&behappyRef) != B_OK)
		// it isn't here
		return HC_NO_BEHAPPY;

	// Find BeHappy's add-on folder
	BEntry behappyEntry(&behappyRef);
	BDirectory behappyAddOnDir;
	behappyEntry.GetParent(&behappyAddOnDir);
	behappyAddOnDir.SetTo(&behappyAddOnDir,behappyAddOnPath);
	if (behappyAddOnDir.InitCheck() != B_OK)
		return HC_NO_BEHAPPY;		// Add-ons folder not found...
	
	// Look if the add-on is here
	BEntry myAddon(&behappyAddOnDir,addOnFileName,true);
	if (myAddon.Exists())
	{
		addOnInstalled = true;
		return HC_ALREADY_INSTALLED;
	}
	
	// The add-on isn't in BeHappy's add-on folder...
	// We'll try and install it
	// first search the application's folder
	app_info myAppInfo;
	be_app->GetAppInfo(&myAppInfo);
	BEntry myAppEntry(&myAppInfo.ref);
	BDirectory myAppDir;
	myAppEntry.GetParent(&myAppDir);
	myAddon.SetTo(&myAppDir,addOnFileName,true);
	
	// so, is this add-on here?
	if (!myAddon.Exists())
		return HC_NO_ADDON;
	
	// ask the user if he wants to install the add-on
	BAlert *myAlert = new BAlert("Add-on installation",alertText,"Yes","No","Don't ask anymore",B_WIDTH_FROM_LABEL,B_IDEA_ALERT);
	switch(myAlert->Go())
	{
		case 1:
			return HC_REFUSED;
		
		case 2:
			return HC_NO_MORE_ASKING;
	}
	
	// the user wants to install the add-on
	if (myAddon.MoveTo(&behappyAddOnDir) == B_OK)
	{
		// everything worked!
		addOnInstalled = true;
		myAlert = new BAlert("Add-on installation",alertText2,"OK");
		myAlert->Go();
		return HC_OK;
	}
	else
		return HC_INSTALL_ERROR;
}

int HappyCommander::ShowBookAndIndex(const char *index)
{
	if (!addOnInstalled)
	{
		OpenHtmlIndex();
		return HC_NO_ADDON;
	}
		
	// first we must be shure the window is opened
	int error = CheckMessenger();
	if (error == HC_NO_BEHAPPY)
	{
		OpenHtmlIndex();
		return error;
	}
	if (error != HC_OK)
		return error;
	
	// the messenger is ok.
	// active the window
	BMessage activateMessage(B_SET_PROPERTY);
	activateMessage.AddSpecifier("Active");
	activateMessage.AddBool("data",true);
	bhWindow.SendMessage(&activateMessage);
	
	// check what book is opened
	BMessage messageG(B_GET_PROPERTY),reply;
	messageG.AddSpecifier("Book");
	bhWindow.SendMessage(&messageG,&reply);
	BString openedBook;
	if (reply.FindString("result",&openedBook) != B_OK)
	{
		// the reply isn't what's expected... it must be an old version of BeHappy
		OpenHtmlIndex();
		return HC_OLD_BEHAPPY;
	}
	
	BMessage messageS(B_SET_PROPERTY);
	// is the book opened the correct one?
	if (openedBook != addOnName)
	{
		// no... set it to the correct book
		messageS.AddSpecifier("Book");
		messageS.AddString("data",addOnName);
		bhWindow.SendMessage(&messageS,&reply);
		
		if ((reply.what != B_REPLY) || (reply.FindInt32("error") != B_OK))
			return HC_ERROR;
	}
	
	// now set the wanted index
	if (index != NULL)
	{
		// first get the current opened index
		messageG.MakeEmpty();
		messageG.AddSpecifier("Index");
		bhWindow.SendMessage(&messageG,&reply);
		BString openedIndex;
		if (reply.FindString("result",&openedIndex) != B_OK)
		{
			// the reply isn't what's expected...
			return HC_ERROR;
		}
		
		// if the index isn't what we want, open the new one
		if (openedIndex != index)
		{
			messageS.MakeEmpty();
			messageS.AddSpecifier("Index");
			messageS.AddString("data",index);
			bhWindow.SendMessage(&messageS,&reply);
	
			if ((reply.what != B_REPLY) || (reply.FindInt32("error") != B_OK))
				return HC_NO_INDEX;
		}
	}
	
	// if we've made it till here, it means that all went all right
	return HC_OK;
}

int HappyCommander::Show(const char *index, const char *topic)
{	
	int error = ShowBookAndIndex(index);
	if (error != HC_OK)
		return error;
		
	BMessage messageS(B_SET_PROPERTY), reply;
	if (topic != NULL)
	{
		messageS.MakeEmpty();
		messageS.AddSpecifier("Topic");
		messageS.AddString("data",topic);
		bhWindow.SendMessage(&messageS,&reply);

		if ((reply.what != B_REPLY) || (reply.FindInt32("error") != B_OK))
			return HC_NO_TOPIC;
	}
	
	// if we've made it till here, it means that all went all right
	return HC_OK;
}

int HappyCommander::Show(const char *index, const char *fatherTopic, const char *childTopic)
{	
	int error = ShowBookAndIndex(index);
	if (error != HC_OK)
		return error;
		
	BMessage messageS(B_SET_PROPERTY), reply;
	if (fatherTopic != NULL)
	{
		messageS.MakeEmpty();
		messageS.AddSpecifier("FatherTopic");
		messageS.AddString("data",fatherTopic);
		bhWindow.SendMessage(&messageS,&reply);

		if ((reply.what != B_REPLY) || (reply.FindInt32("error") != B_OK))
			return HC_NO_TOPIC;
	}

	if (childTopic != NULL)
	{
		messageS.MakeEmpty();
		messageS.AddSpecifier("ChildTopic");
		messageS.AddString("data",childTopic);
		bhWindow.SendMessage(&messageS,&reply);

		if ((reply.what != B_REPLY) || (reply.FindInt32("error") != B_OK))
			return HC_NO_TOPIC;
	}
	
	// if we've made it till here, it means that all went all right
	return HC_OK;
}

int HappyCommander::CheckMessenger()
{
	// first see if the messenger still points to something
	if (bhWindow.IsValid())
		// it's ok!
		return HC_OK;
		
	// now, see if BeHappy is launched
	app_info behappyInfo;
	if(be_roster->GetAppInfo(behappySignature,&behappyInfo) != B_OK)
	{
		// it isn't... let's launch it
		BMessage initMessage('Hide');
		be_roster->Launch(behappySignature,&initMessage);
	}
	BMessenger happyMessenger(behappySignature);
	
	// if the messenger isn't initilalised, it means BeHappy isn't here
	if (!happyMessenger.IsValid())
		return HC_NO_BEHAPPY;
	
	// ask BeHappy to open a window
	BMessage openMessage('Open'),reply;
	openMessage.AddString("Name",addOnName);
	happyMessenger.SendMessage(&openMessage,&reply);
	
	// extract the messenger for the new opened window
	if (reply.what == B_NO_REPLY)
		// the old behappy doesn't send any reply
		return HC_OLD_BEHAPPY;
	if ((reply.what != B_REPLY) || (reply.FindMessenger("Messenger",&bhWindow) != B_OK))
		// there has been an error... add-on not here?
		return HC_ERROR;
	else
	{
		if (bhWindow.IsValid())
			return HC_OK;	// done!
		else
			return HC_ERROR;
	}
}

bool HappyCommander::OpenHtmlIndex()
{
	// find the application's folder
	app_info myInfo;
	be_app->GetAppInfo(&myInfo);
	BEntry myEntry(&myInfo.ref);
	BDirectory myDir;
	myEntry.GetParent(&myDir);
	BPath htmlFilePath(&myDir,htmlIndex);
	
	if (myEntry.SetTo(htmlFilePath.Path()) == B_OK)
		// open the file
	{
		entry_ref myRef;
		myEntry.GetRef(&myRef);
		return(be_roster->Launch(&myRef) == B_OK);
	}
	else
		// error while retreiving the application's directory
		return B_ERROR;
}

