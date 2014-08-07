/************************************/
/* classe de la chaine d'extraction */
/************************************/
#ifndef _ADDONEXTRACTEDSTRING_H
#define _ADDONEXTRACTEDSTRING_H

#include <SupportDefs.h>
#include <String.h>

class AddOnExtractedString
{
public:
	AddOnExtractedString();
	virtual	~AddOnExtractedString();
	
	BString		_string;
	char		_identifier;
	bool		_operatorAsCast;
	int32		_recursLevel;
};

#endif