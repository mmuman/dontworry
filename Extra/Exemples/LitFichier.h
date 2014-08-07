/*
 * Test des classes CS
 * Application qui lit un fichier contenant la structure des classes CS
 *
 *
 */

#ifndef LITFICHIER_H
#define LITFICHIER_H

#include <Application.h>

class CSClass;
class CSLibrary;

class LitFichierApp : public BApplication
{
public:
	LitFichierApp();
	
	virtual void ReadyToRun();
	
private:
	void DumpClass(CSLibrary*, const char*);
	void DumpClass(CSLibrary*,CSClass*);
};

#endif