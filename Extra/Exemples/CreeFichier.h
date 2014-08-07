/*
 * Test des classes CS
 * Application qui crée un fichier contenant la structure des classes CS
 *
 *
 */

#ifndef CREEFICHIER_H
#define CREEFICHIER_H

#include <Application.h>

class CreeFichierApp : public BApplication
{
public:
	CreeFichierApp();
	
	virtual void ReadyToRun();
};

#endif