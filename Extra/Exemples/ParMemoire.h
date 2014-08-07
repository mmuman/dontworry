/*
 * Test des classes CS
 * Application qui parse les fichiers .h du projet et ressort la structure d'une classe,
 * en passant par un objet BMallocIO
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

class ParMemoireApp : public BApplication
{
public:
	ParMemoireApp();
	
	virtual void ReadyToRun();

private:
	void DumpClass(CSLibrary*, const char*);
	void DumpClass(CSLibrary*,CSClass*);
	
	void DumpMemberVariable(CSVariable*);
	void DumpMethod(CSMethod*);
	
	void WriteZone(unsigned int zone);
};

#endif