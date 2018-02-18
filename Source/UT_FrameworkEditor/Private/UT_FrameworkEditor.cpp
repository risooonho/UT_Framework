/************************************************************************/
/* UMBRA TOOLS                                                          */
/* Maxwell - Axel Clerget   											*/
/************************************************************************/

#include "UT_FrameworkEditor.h"

// Npc
#include "NpcEditor.h"
#include "NpcBehaviorBlueprint.h"
#include "NpcBehaviorGraphFactory.h"
#include "NpcBehaviorUtilities.h"
#include "NpcBehaviorBlueprintCompiler.h"

// StateMachine
#include "StateMachineEditor.h"
#include "StateMachineBlueprintCompiler.h"
#include "StateMachineBlueprint.h"
#include "StateMachineGraphFactory.h"

// Tasks
#include "NpcBehaviorTaskFactory_Wait.h"
#include "NpcBehaviorTaskFactory_MoveTo.h"
#include "NpcBehaviorTaskFactory_Restart.h"
#include "NpcBehaviorTaskFactory_RandomBranch.h"

// Styles
#include "FrameworkStyle.h"

// AssetTools
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "AssetTypeCategories.h"
#include "AssetTypeActions_NpcBehavior.h"
#include "AssetTypeActions_StateMachine.h"

// Debuggers
#include "GameplayDebugger.h"
#include "GDC_StateMachine.h"

#define LOCTEXT_NAMESPACE "FUT_FrameworkEditorModule"

void FUT_FrameworkEditorModule::StartupModule()
{
	// Styles
	FFrameworkStyle::Initialize();

	// Extenders 
	MenuExtensibilityManager = MakeShareable(new FExtensibilityManager());
	ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager());

	// Factories
	NpcBehaviorGraphFactory = MakeShareable(new FNpcBehaviorGraphFactory());
	StateMachineGraphFactory = MakeShareable(new FStateMachineGraphFactory());
	StateMachineGraphPinConnectionFactory = MakeShareable(new FStateMachineGraphPinConnectionFactory());
	FEdGraphUtilities::RegisterVisualNodeFactory(NpcBehaviorGraphFactory);
	FEdGraphUtilities::RegisterVisualNodeFactory(StateMachineGraphFactory);
	FEdGraphUtilities::RegisterVisualPinConnectionFactory(StateMachineGraphPinConnectionFactory);

	// Compilers
	IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
	KismetCompilerModule.GetCompilers().Add(this);

	// NpcBehaviorTasks
	this->RegisterNpcBehaviorTasks();

	// Debuggers
	#if WITH_GAMEPLAY_DEBUGGER
		//If the gameplay debugger is available, register the category and notify the editor about the changes
		IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
		GameplayDebuggerModule.RegisterCategory("UmbraFramework", IGameplayDebugger::FOnGetCategory::CreateStatic(&FGDC_StateMachine::MakeInstance), EGameplayDebuggerCategoryState::EnabledInGameAndSimulate);
		GameplayDebuggerModule.NotifyCategoriesChanged();
	#endif

	// AssetsActions
	this->RegisterAssetActions();
}

void FUT_FrameworkEditorModule::ShutdownModule()
{
	// Styles
	FFrameworkStyle::Shutdown();

	// Extenders
	MenuExtensibilityManager.Reset();
	ToolBarExtensibilityManager.Reset();

	// Factories
	FEdGraphUtilities::UnregisterVisualNodeFactory(NpcBehaviorGraphFactory);
	FEdGraphUtilities::UnregisterVisualNodeFactory(StateMachineGraphFactory);
	FEdGraphUtilities::UnregisterVisualPinConnectionFactory(StateMachineGraphPinConnectionFactory);

	// Debuggers
	#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
		GameplayDebuggerModule.UnregisterCategory("UmbraFramework");
	}
	#endif
}

TSharedRef<FAssetEditorToolkit> FUT_FrameworkEditorModule::CreateNpcEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UNpcBehaviorBlueprint* Blueprint)
{
	TSharedRef<FNpcEditor> NpcEditor(new FNpcEditor());
	NpcEditor->InitEditor(Mode, InitToolkitHost, Blueprint);

	return NpcEditor;
}

TSharedRef<FAssetEditorToolkit> FUT_FrameworkEditorModule::CreateStateMachineEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, UStateMachineBlueprint* Blueprint)
{
	TSharedRef<FStateMachineEditor> StateMachineEditor(new FStateMachineEditor());
	StateMachineEditor->InitEditor(Mode, InitToolkitHost, Blueprint);

	return StateMachineEditor;
}

void FUT_FrameworkEditorModule::RegisterAssetActions()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_NpcBehavior));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_StateMachine));
}

void FUT_FrameworkEditorModule::RegisterNpcBehaviorTasks()
{
	FNpcBehaviorUtilities::RegisterTask(MakeShareable(new FNpcBehaviorTaskFactory_Wait));
	FNpcBehaviorUtilities::RegisterTask(MakeShareable(new FNpcBehaviorTaskFactory_MoveTo));
	FNpcBehaviorUtilities::RegisterTask(MakeShareable(new FNpcBehaviorTaskFactory_Restart));
	FNpcBehaviorUtilities::RegisterTask(MakeShareable(new FNpcBehaviorTaskFactory_RandomBranch));
}

bool FUT_FrameworkEditorModule::CanCompile(const UBlueprint* Blueprint)
{
	return Cast<UStateMachineBlueprint>(Blueprint) != nullptr
		|| Cast<UNpcBehaviorBlueprint>(Blueprint) != nullptr;
}

void FUT_FrameworkEditorModule::Compile(UBlueprint* Blueprint, const FKismetCompilerOptions& CompileOptions, FCompilerResultsLog& Results, TArray<UObject *>* ObjLoaded)
{
	if (UStateMachineBlueprint* StateMachineBlueprint = Cast<UStateMachineBlueprint>(Blueprint))
	{
		FStateMachineBlueprintCompiler Compiler(StateMachineBlueprint, Results, CompileOptions, ObjLoaded);
		Compiler.Compile();
		check(Compiler.NewClass);
	}
	else if (UNpcBehaviorBlueprint* NpcBehaviorBlueprint = Cast<UNpcBehaviorBlueprint>(Blueprint))
	{
		FNpcBehaviorBlueprintCompiler Compiler(NpcBehaviorBlueprint, Results, CompileOptions, ObjLoaded);
		Compiler.Compile();
		check(Compiler.NewClass);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUT_FrameworkEditorModule, UT_FrameworkEditor)