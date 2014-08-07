#ifndef MYAPP_H
#define MYAPP_H
/****************/
/* classe CSViewerApp */
/****************/

#include <Application.h>
#include <Window.h>

class CSViewerApp : public BApplication
{
public:
	CSViewerApp();
	virtual ~CSViewerApp();
	virtual	void	RefsReceived(BMessage *file);

protected:
	BWindow			*_mainWindow;

			void	InitMIMEType(); 	// verifier si le type mime existe
};

#endif