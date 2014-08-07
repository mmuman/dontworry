/*
 * Parseur c++ pour keyspy
 *
 * CSPTest: application pour tester la librairie CSP
 *
 */


#ifndef CSPTEST_H
#define CSPTEST_H

#include <Application.h>
class CSPSourceFile;

class CSPTestApp : public BApplication
{
public:
	CSPTestApp();
	~CSPTestApp();
	
	virtual void ReadyToRun();
	
	void SearchVariable(CSPSourceFile &sourceFile,const char *name);
};
#endif //CSPTEST_H