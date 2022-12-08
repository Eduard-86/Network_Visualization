#pragma once

#include "Runtime/Engine/Classes/Engine/Engine.h"

/* 
 *************************************************************************************************/

/**
 * Basic UE_LOG but with replication to screen, and Format wrapped in TEXT out of box
 * @param Verbosity Can be omitted, defaulting to Verbose
 */
#define COMBO_LOG(CategoryName, Verbosity, Format, ...)                                            \
    {                                                                                              \
        UE_LOG(CategoryName, __COMBO_LOG_VERBOSITY_##Verbosity, TEXT(Format), __VA_ARGS__);        \
        __COMBO_LOG_VIEWPORT(__COMBO_LOG_VERBOSITY_##Verbosity, TEXT(Format), __VA_ARGS__);        \
    }

#define COMBO_LOG_FUNC(CategoryName, Verbosity, Format, ...) \
	COMBO_LOG(CategoryName, Verbosity, "%s: " Format, *FString(__func__), __VA_ARGS__)

/* 
 *************************************************************************************************/

#define __COMBO_LOG_VERBOSITY_            Verbose
#define __COMBO_LOG_VERBOSITY_Fatal       Fatal
#define __COMBO_LOG_VERBOSITY_Error       Error
#define __COMBO_LOG_VERBOSITY_Warning     Warning
#define __COMBO_LOG_VERBOSITY_Display     Display
#define __COMBO_LOG_VERBOSITY_Log         Log
#define __COMBO_LOG_VERBOSITY_Verbose     Verbose
#define __COMBO_LOG_VERBOSITY_VeryVerbose VeryVerbose
#define __COMBO_LOG_VERBOSITY_All         All

/*
 *************************************************************************************************/

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
#    define __COMBO_LOG_VIEWPORT(Verbosity, Format, ...)\
        {\
            if (GEngine)\
            {\
                FColor Color;\
                if constexpr (ELogVerbosity::Verbosity < ELogVerbosity::Warning)\
                {\
                    Color = FColor::Red;\
                }\
                else if constexpr (ELogVerbosity::Verbosity == ELogVerbosity::Warning)\
                {\
                    Color = FColor::Yellow;\
                }\
                else\
                {\
                    Color = FColor::Silver;\
                }\
                GEngine->AddOnScreenDebugMessage(\
                    -1,\
                    7,\
                    Color,\
                    __ComboLog_MakeMessage(Format, __VA_ARGS__)\
                );\
            }\
        }
#else
#    define __COMBO_LOG_VIEWPORT(Verbosity, Format, ...)
		// Nope. Won't display anything. Check GEngine->AddOnScreenDebugMessage insides
		//
        // {\
        //     if constexpr (ELogVerbosity::Verbosity == ELogVerbosity::Fatal)\
        //     {\
        //         if (GEngine)\
        //         {\
        //             GEngine->AddOnScreenDebugMessage(\
        //                 -1,\
        //                  7,\
        //                  FColor::Red,\
        //                  __ComboLog_MakeMessage(Format, __VA_ARGS__)\
        //             );\
        //         }\
        //     }\
        // }
#endif

/* 
 *************************************************************************************************/

/**
 * @remark Force inline to prevent cross-module dependencies
 */
FORCEINLINE FString __ComboLog_MakeMessage(const TCHAR* Fmt, ...)
{
    const auto OutMsgSize = 8192; /*LogMacros.cpp::17*/

    TArray<TCHAR> OutLogMessage;
    OutLogMessage.Reserve(OutMsgSize);
    GET_VARARGS(OutLogMessage.GetData(), OutMsgSize, OutMsgSize - 1, Fmt, Fmt);
    //OutLogMessage still thinking it's empty - feed it's contents to the string instead
    return FString(OutLogMessage.GetData());
}

/* 
 *************************************************************************************************/

// A useless but an epic ass crack, check the line calling the lambda
//
//#    define COOL_LOG(CategoryName, Verbosity, FormattedMessage)                                 \
//        {                                                                                       \
//            static_assert(TIsDerivedFrom<TRemoveReference<decltype(FormattedMessage)>::Type,    \
//                                         FString>::Value,                                       \
//                          "Message must be an FString.");                                       \
//            static_assert((ELogVerbosity::Verbosity & ELogVerbosity::VerbosityMask) <           \
//                                  ELogVerbosity::NumVerbosity &&                                \
//                              ELogVerbosity::Verbosity > 0,                                     \
//                          "Verbosity must be constant and in range.");                          \
//            CA_CONSTANT_IF((ELogVerbosity::Verbosity & ELogVerbosity::VerbosityMask) <=         \
//                               ELogVerbosity::COMPILED_IN_MINIMUM_VERBOSITY &&                  \
//                           (ELogVerbosity::Warning & ELogVerbosity::VerbosityMask) <=           \
//                               FLogCategory##CategoryName::CompileTimeVerbosity)                \
//            {                                                                                   \
//                UE_LOG_EXPAND_IS_FATAL(Verbosity,                                               \
//                                       PREPROCESSOR_NOTHING,                                    \
//                                       if (!CategoryName.IsSuppressed(ELogVerbosity::Verbosity))\
//                {                                                                               \
//                    auto UE_LOG_noinline_lambda = [](const auto & LCategoryName,                \
//                                                     const auto & LFormat,                      \
//                                                     const auto &... UE_LOG_Args) FORCENOINLINE \
//                    {                                                                           \
//                        TRACE_LOG_MESSAGE(LCategoryName, Verbosity, LFormat, UE_LOG_Args...)    \
//                        UE_LOG_EXPAND_IS_FATAL(                                                 \
//                            Verbosity,                                                          \
//                            {                                                                   \
//                                FMsg::Logf_Internal(UE_LOG_SOURCE_FILE(__FILE__),               \
//                                                    __LINE__,                                   \
//                                                    LCategoryName.GetCategoryName(),            \
//                                                    ELogVerbosity::Verbosity,                   \
//                                                    LFormat,                                    \
//                                                    UE_LOG_Args...);                            \
//                                _DebugBreakAndPromptForRemote();                                \
//                                FDebug::ProcessFatalError();                                    \
//                            },                                                                  \
//                            {                                                                   \
//                                FMsg::Logf_Internal(nullptr,                                    \
//                                                    0,                                          \
//                                                    LCategoryName.GetCategoryName(),            \
//                                                    ELogVerbosity::Verbosity,                   \
//                                                    LFormat,                                    \
//                                                    UE_LOG_Args...);                            \
//                            })                                                                  \
//                    };                                                                          \
//                    UE_LOG_noinline_lambda(CategoryName, *( TCHAR(*)[1] )(*FormattedMessage));  \
//                    UE_LOG_EXPAND_IS_FATAL(Verbosity, CA_ASSUME(false);, PREPROCESSOR_NOTHING)  \
//                }                                                                               \
//            }                                                                                   \
//            __COOL_LOG_VIEWPORT(Verbosity, FormattedMessage)                                    \
//        }