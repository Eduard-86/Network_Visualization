#include "PathColorSynchronizer/Public/PathColorSynchronizer.h"

inline void FPathColorSynchronizer::ShutdownModule( )
{
	if (!FPaths::FileExists(GEditorPerProjectIni))
		return;

	const int32 ProjectPathEnding = GEditorPerProjectIni.Find(TEXT("/Saved/Config/"), ESearchCase::CaseSensitive);
	if (ProjectPathEnding == INDEX_NONE)
		return;

	TArray<FString> PathColors;
	GConfig->GetSection(TEXT("PathColor"), PathColors, GEditorPerProjectIni);
	if (PathColors.Num( ) == 0)
		return;

	const FString DefaultEditorPerProjectIni =
			GEditorPerProjectIni.Left(ProjectPathEnding) / "Config/DefaultEditorPerProjectUserSettings.ini";
	if (!FPaths::FileExists(DefaultEditorPerProjectIni))
		return;

	// Could just Read(), but this operation may have undesired side effects
	FString DefaultConfigFile;
	FFileHelper::LoadFileToString(DefaultConfigFile, *DefaultEditorPerProjectIni);
	FConfigFile File;
	File.ProcessInputFileContents(DefaultConfigFile);

	FConfigSection * PathColorSection = File.FindOrAddSection(TEXT("PathColor"));
	PathColorSection->Empty(PathColors.Num( ));
	for (const FString & PathColor : PathColors)
	{
		int32      SplitterPos;
		const bool bFound = PathColor.FindChar('=', SplitterPos);
		check(bFound);

		const FString Key = PathColor.Left(SplitterPos);
		const FString Value = PathColor.Right(PathColor.Len() - SplitterPos - 1);

		PathColorSection->Add(*Key, Value);
	}
	File.Dirty = true;
	File.NoSave = false;
	File.Write(DefaultEditorPerProjectIni, false);
}

IMPLEMENT_MODULE(FPathColorSynchronizer, PathColorSynchronizer)
