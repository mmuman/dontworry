/* HappyCommander class
 *
 * Written by Sylvain Tertois (Sylvain.Tertois@fnac.net) to show how to use
 * BeHappy's scripting capabilities
 */

// This class installs the add-on in BeHappy's add-on folder, and provides
// methods to display a particuliar index or topic. The add-on must be in the
// application's folder to make the installation work

#ifndef HAPPYCOMMANDER_H
#define HAPPYCOMMANDER_H

#include "Messenger.h"

class HappyCommander
{
public:
	HappyCommander(const char *addOnName,const char *addOnFileName,const char *htmlIndex);
		// addOnName is the name of the add-on to use
		// htmlIndex is the name of the HTML file to open if there is an error
		// with BeHappy. (path relative to current application)
		// these pointers are stored in the class, the strings are NOT copied
	~HappyCommander(); 
	
	// installs the add-on:
	// this function checks if the add-on is installed. If not, it asks the user
	// if he wants it to be installed, and installs it if needed. 
	int InstallAddOn();
		// returns HC_ALREADY_INSTALLED if the add-on was already installed
		//         HC_OK if the add-on was successfully installed
		//         HC_NO_ADDON if the add-on was not found
		//         HC_INSTALL_ERROR if an error occured while trying to install
		//           the add-on
		//         HC_NO_BEHAPPY if BeHappy wasn't found
		//         HC_NO_MORE_ASKING if the user clicked the 'Don't ask anymore'
		//           button when asked if he wanted to install the add-on
		//         HC_REFUSED if the user didn't want to install the add-on
	
	// show a partiular index and/or topic
	// If the BeHappy window is already opened, this function just changes
	// it's topic or index. Otherwise a new window is opened
	int Show(const char *index = NULL, const char *topic = NULL);
		// returns HC_OK if the change was successfull
		//         HC_NO_BEHAPPY if BeHappy wasn't found (in that case the htmlIndex 
		//           file has been opened)
		//         HC_OLD_BEHAPPY BeHappy doesn't respond to scripting messages
		//           it must be an old version
		//         HC_NO_INDEX if the index wasn't found
		//         HC_NO_TOPIC if the topic wasn't found
		//         HC_ERROR if another error occured (if the add-on has been moved,
		//           as an example)
	
	// same function, with a father topic and child topic
	int Show(const char *index,const char *fatherTopic, const char *childTopic);
		// any argument can be NULL
	
	enum { HC_OK = 0, HC_ALREADY_INSTALLED, HC_NO_ADDON, HC_INSTALL_ERROR, 
		HC_NO_BEHAPPY, HC_NO_MORE_ASKING, HC_REFUSED, HC_OLD_BEHAPPY,
		HC_NO_INDEX, HC_NO_TOPIC, HC_ERROR };
		
	bool OpenHtmlIndex();
		// opens the html file... it's done when BeHappy is unavailable
		// returns true if the file was successfully opened
	
private:
	BMessenger bhWindow;	// messenger to the BeHappy window used by HappyCommander
	
	int CheckMessenger();	// checks that the messenger is valid. If not, opens a 
					// BeHappy window and sets the messenger to it.
	
	int ShowBookAndIndex(const char *index= NULL);
		// show a particular index
		// returns the same error constants than Show()
		
	// parameters
	const char *addOnName,*addOnFileName;
	const char *htmlIndex;
	bool addOnInstalled;
};

#endif //HAPPYCOMMANDER_H