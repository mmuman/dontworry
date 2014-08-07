/*******************************/
/* classe d'extraction du text */
/*******************************/
#ifndef _ADDONTEXTEXTRACTOR_H
#define _ADDONTEXTEXTRACTOR_H

#include <SupportDefs.h>
#include <String.h>
#include <List.h>

class AddOnExtractedString;

class AddOnTextExtractor
{
public:
	AddOnTextExtractor();
	virtual	~AddOnTextExtractor();
	
	status_t				Extract(const char *source,int32 position,bool completeWord);
	AddOnExtractedString	*Extracted();

protected:
	// pour la recursivité
	const char		*_text;
	int32			_position;
	bool			_completeWord;
	int32			_nbRecursExtract;
	// chaine
	BList			_extractedList;
	int32			_nbUsedItems;

		void		AddItem(BString &string,char identifier,bool asCast);
		status_t	RecursExtract(bool firstCall);
};

#endif
