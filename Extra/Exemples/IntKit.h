/*
 * Test des classes CS
 * Application qui parse les fichiers .h du kit Interface et ressort la structure d'une classe,
 * en passant par un objet BMallocIO
 *
 *
 */

#ifndef PARMEMOIRE_H
#define PARMEMOIRE_H

#include <Application.h>

class CSLibrary;
class CSClass;

class IntKitApp : public BApplication
{
public:
	IntKitApp();
	
	virtual void ReadyToRun();

private:
	void DumpClass(CSLibrary*, const char*);
	void DumpClass(CSLibrary*,CSClass*);
};

#endif