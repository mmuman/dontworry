/*
 * Test des classes CS
 * Application qui teste la détection de mise à jour d'un fichier header
 *
 *
 */

#ifndef PARMEMOIRE_H
#define PARMEMOIRE_H

#include <Application.h>

class CSLibrary;
class CSClass;
class CSVariable;
class CSMethod;

class ModificationsApp : public BApplication
{
public:
	ModificationsApp();
	
	virtual void ReadyToRun();

private:
	void DumpClass(CSLibrary*, const char*);
	void DumpClass(CSLibrary*,CSClass*);
	
	void DumpMemberVariable(CSVariable*);
	void DumpMethod(CSMethod*);
	
	void WriteZone(unsigned int zone);
};

#endif