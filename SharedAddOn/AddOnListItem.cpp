#include "AddOnListItem.h"
#include "AddOnConstantes.h"
#include "CPreferences.h"

#include <View.h>
#include <Message.h>
#include <Control.h>

/**** constructeur ****/
AddOnListItem::AddOnListItem (int8 type,BBitmap *image,BMessage *message,int32 indexMessage,BBitmap *state,BBitmap *iSvirtual) : BListItem() 
{
	// extraire les données
	if(message->FindString(CS_RETURN,indexMessage,&_returnType)!=B_OK)
		_returnType.SetTo("");
	// extraire le nom
	if(message->FindString(CS_NAME,indexMessage,&_name)!=B_OK)
		_name.SetTo("");
	//extraire les parametres
	if(message->FindString(CS_PARAMETER,indexMessage,&_parameter)!=B_OK)
		_parameter.SetTo("");
	
	_image = image;
	_type = type;
	_state = state;
	_isVirtual = iSvirtual;
}

/**** destructeur ****/
AddOnListItem::~AddOnListItem()
{
}

/**** dessiner l'item ****/
void AddOnListItem::DrawItem(BView *owner,BRect frame,bool complete)
{

	if(IsSelected())
	{
		switch(_type)
		{
		case TYPE_CLASS:
			owner->SetHighColor(200,200,255,255);
			break;
		case TYPE_METHODE:
			owner->SetHighColor(255,255,170,255);
			break;
		case TYPE_VARIABLE:
			owner->SetHighColor(255,190,190,255);
			break;
		}
	}
	else
		owner->SetHighColor(255,255,255,255);		
	
	owner->FillRect(BRect(12,frame.top,frame.right,frame.bottom));
	owner->SetLowColor(owner->HighColor());		

	// dessiner le type de donnees
	if(_image!=NULL)
	{
		owner->SetDrawingMode(B_OP_ALPHA);
		owner->DrawBitmap(_image,BPoint(frame.left+1,frame.top+1));
		owner->SetDrawingMode(B_OP_COPY);
	}
		
	frame.left += 15;
	// dessiner si c'est virtuel
	if(_isVirtual!=NULL)
	{
		owner->SetDrawingMode(B_OP_ALPHA);
		owner->DrawBitmap(_isVirtual,BPoint(frame.left+3,frame.top+1));
		owner->SetDrawingMode(B_OP_COPY);
	}	
	
	// dessiner l'etat de protection
	if(_state!=NULL)
	{
		owner->SetDrawingMode(B_OP_ALPHA);
		owner->DrawBitmap(_state,BPoint(frame.left+1,frame.top+6));
		owner->SetDrawingMode(B_OP_COPY);
	}

	frame.left += 20;
	owner->SetHighColor(0,0,0,0);
	owner->DrawString(_returnType.String(),BPoint(frame.left,frame.top+13));
	frame.left += owner->StringWidth(_returnType.String());
	owner->DrawString(_name.String(),BPoint(frame.left,frame.top+13));
	frame.left += owner->StringWidth(_name.String());
	if(_type==TYPE_METHODE)
	{
		owner->DrawString("(",BPoint(frame.left,frame.top+13));	
		frame.left += owner->StringWidth("(");
		owner->DrawString(_parameter.String(),BPoint(frame.left,frame.top+13));	
		frame.left += owner->StringWidth(_parameter.String());
		owner->DrawString(")",BPoint(frame.left,frame.top+13));	
	}
}

/**** definir la taille de l'item ****/
void AddOnListItem::Update(BView *owner, const BFont *font)
{
	SetHeight(17);
}

/**** retourner le nom de la variable ****/
BString AddOnListItem::Name(int8 askType,bool forBeHappy,bool forceNoParameters)
{
	BString			result;
	CPreferences	prefs(PREF_FILE_NAME,PREF_PATH_NAME);

	// charger les prefs
	prefs.Load();	

	result = _name;
	switch(_type)
	{
	case TYPE_CLASS:
	case TYPE_VARIABLE:
		// on ne fait rien (pour l'instant)
		break;
	case TYPE_METHODE:
		// ouvrir la parenthese des parametres
		result << "(";
		if(!forBeHappy && (askType==ASK_METHOD || prefs._useParameters == B_CONTROL_ON) && !forceNoParameters)
			result << _parameter.String();
		
		// fermer la parenthese des parametres
		result << ")";
		break;
	}
	
	return result;
}