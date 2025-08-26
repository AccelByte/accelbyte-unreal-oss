// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteChatReportMessage.h"
#include "Api/AccelByteReportingApi.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteChatReportMessage"

FOnlineAsyncTaskAccelByteChatReportMessage::FOnlineAsyncTaskAccelByteChatReportMessage(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetId& InLocalUserId
	, const TSharedRef<FAccelByteChatMessage>& InMessage
	, const FString& InReason
	, const FString& InComment
	, FOnReportChatMessageComplete InCompletionDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Message(InMessage)
	, Reason(InReason)
	, Comment(InComment)
	, CompletionDelegate(InCompletionDelegate)
	, OnlineError(FOnlineError())
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteChatReportMessage::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	auto OnReportMessageSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsReportingSubmitResponse>>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteChatReportMessage::OnReportMessageSuccess);
	auto OnReportMessageErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteChatReportMessage::OnReportMessageError);
	
	FAccelByteModelsReportingSubmitDataChat ChatReportData{};

	FUniqueNetIdAccelByteUserRef ChatAuthorId = FUniqueNetIdAccelByteUser::CastChecked(Message->GetUserId());
	ChatReportData.UserId = ChatAuthorId->GetAccelByteId();
	
	ChatReportData.ChatTopicId = Message->GetTopicId();
	ChatReportData.ChatId = Message->GetChatId();
	ChatReportData.ChatCreatedAt = Message->GetTimestamp();
	ChatReportData.Reason = Reason;
	ChatReportData.Comment = Comment;

	API_FULL_CHECK_GUARD(Reporting, OnlineError);
	Reporting->SubmitChatReport(ChatReportData, OnReportMessageSuccessDelegate, OnReportMessageErrorDelegate);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatReportMessage::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s, ErrorMessage: %s"), LOG_BOOL_FORMAT(bWasSuccessful), *OnlineError.ErrorMessage.ToString());

	CompletionDelegate.ExecuteIfBound(UserId.ToSharedRef().Get(), bWasSuccessful, Message.Get(), ReportResponse);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatReportMessage::OnReportMessageSuccess(const FAccelByteModelsReportingSubmitResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	ReportResponse = Result;
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteChatReportMessage::OnReportMessageError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->GetAccelByteId());

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), FText::FromString(TEXT("report-chat-message-failed")));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to report chat message, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}

#undef ONLINE_ERROR_NAMESPACE
