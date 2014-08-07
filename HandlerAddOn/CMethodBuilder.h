//========================================================================
//	CMethodBuilder.h
//========================================================================	

#ifndef _MMETHODBUILDER_H
#define _MMETHODBUILDER_H

#include "MPlugInBuilder.h"
#include <List.h>
#include <Messenger.h>

class CAddOnAppHandler;

class CMethodBuilder : public MPlugInBuilder
{
public:
	CMethodBuilder();
	virtual ~CMethodBuilder();

protected:	
	virtual long GetToolName(MProject* inProject,char* outName,long inBufferLength,MakeStageT inStage,MakeActionT inAction);

	virtual const char *		LinkerName();
	virtual MakeStageT			MakeStages();
	virtual MakeActionT			Actions();
	virtual PlugInFlagsT		Flags();
	virtual ulong				MessageDataType();

	// return true if something changed in the settings
	virtual bool	ValidateSettings(BMessage& inOutMessage);
	virtual long	BuildPrecompileArgv(MProject& inProject,BList& inArgv,MFileRec& inFileRec);
	virtual long	BuildCompileArgv(MProject& inProject,BList& inArgv,MakeActionT inAction,MFileRec& inFileRec);
	virtual long	BuildPostLinkArgv(MProject& inProject,BList& inArgv,MFileRec& inFileRec);
	virtual bool	FileIsDirty(MProject& inProject,MFileRec& inFileRec,MakeStageT inStage,MakeActionT inAction,time_t inModDate);
	virtual long	ParseMessageText(MProject& inProject,const char* inText,BList& outList);
	virtual void	CodeDataSize(MProject& inProject,const char* inFilePath,long& outCodeSize,long& outDataSize);
	virtual long	GenerateDependencies(MProject& inProject,const char* inFilePath,BList& outList);
	virtual void	GetTargetFilePaths(MProject& inProject,MFileRec& inFileRec,BList& inOutTargetFileList);
	virtual void	ProjectChanged(MProject& inProject,ChangeT inChange);

public:
	// fonctions relatives au handler
	void	CreateAddOnHandler();
	void	DestroyAddOnHandler();
	
protected:
	BMessenger			_handler;
};

#endif
