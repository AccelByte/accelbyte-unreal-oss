#include "OnlineSubsystemAccelByteUtils.h"

#include "HAL/Platform.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "OnlineSubsystemAccelByte.h"
#include "Interfaces/OnlineUserInterface.h"
#include "Online.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemUtils.h"

FDelegateHandle FOnlineSubsystemAccelByteUtils::QueryUserHandle = FDelegateHandle();

TSharedRef<const FUniqueNetId> FOnlineSubsystemAccelByteUtils::GetUniqueIdFromString(FString UniqueIdString, bool bIsEncoded /*= true*/) {
	if (UniqueIdString.IsEmpty()) {
		return FUniqueNetIdAccelByteUser::Invalid();
	}

	if (!bIsEncoded) {
		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = UniqueIdString;
		return FUniqueNetIdAccelByteUser::Create(CompositeId).ToSharedRef();
	}

	return MakeShared<const FUniqueNetIdAccelByteUser>(UniqueIdString);
}

TSharedPtr<const FUniqueNetId> FOnlineSubsystemAccelByteUtils::GetPlatformUniqueIdFromUniqueId(const FUniqueNetId& UniqueId)
{
	// Can't get a valid platform id if the unique id belongs to a different platform
	if (!IsPlayerOnSamePlatform(UniqueId)) return nullptr;

	if (UniqueId.GetType() == ACCELBYTE_SUBSYSTEM)
	{
		TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(UniqueId.AsShared());
		return AccelByteId->GetPlatformUniqueId();
	}

	return nullptr;
}

bool FOnlineSubsystemAccelByteUtils::IsPlayerOnSamePlatform(const FUniqueNetId& UniqueId)
{
	FOnlineSubsystemAccelByte* AccelByteSubsystem = static_cast<FOnlineSubsystemAccelByte*>(IOnlineSubsystem::Get(ACCELBYTE_SUBSYSTEM));
	return (GetPlatformNameFromUniqueId(UniqueId) == AccelByteSubsystem->GetNativePlatformNameString() || GetPlatformNameFromUniqueId(UniqueId) == AccelByteSubsystem->GetSimplifiedNativePlatformName());
}

bool FOnlineSubsystemAccelByteUtils::IsPlayerOnSamePlatform(FString UniqueIdString) 
{
	TSharedRef<const FUniqueNetIdAccelByteUser> UniqueId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(GetUniqueIdFromString(UniqueIdString));
	return IsPlayerOnSamePlatform(UniqueId.Get());
}

FString FOnlineSubsystemAccelByteUtils::GetAccelByteIdFromUniqueId(const FUniqueNetId& UniqueId) {
	if (UniqueId.GetType() == ACCELBYTE_SUBSYSTEM) {
		TSharedRef<const FUniqueNetIdAccelByteUser> CompositeId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(UniqueId.AsShared());
		return CompositeId->GetAccelByteId();
	}

	return TEXT("INVALID");
}

FString FOnlineSubsystemAccelByteUtils::GetPlatformNameFromUniqueId(const FUniqueNetId& UniqueId) 
{
	if (UniqueId.GetType() == ACCELBYTE_SUBSYSTEM)
	{
		TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(UniqueId.AsShared());
		return AccelByteId->GetPlatformType();
	}

	return TEXT("NULL");
}

FString FOnlineSubsystemAccelByteUtils::GetPlatformIdStringFromUniqueId(const FUniqueNetId& UniqueId) 
{
	if (UniqueId.GetType() == ACCELBYTE_SUBSYSTEM) 
	{
		TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(UniqueId.AsShared());
		return AccelByteId->GetPlatformId();
	}
	
	return TEXT("INVALID");
}

FString FOnlineSubsystemAccelByteUtils::GetPlatformName()
{
	FOnlineSubsystemAccelByte* Subsystem = static_cast<FOnlineSubsystemAccelByte*>(IOnlineSubsystem::Get(ACCELBYTE_SUBSYSTEM));
	if (Subsystem == nullptr) {
		return TEXT("NULL");
	}

	return Subsystem->GetSimplifiedNativePlatformName();
}

bool FOnlineSubsystemAccelByteUtils::GetDisplayName(int32 LocalUserNum, FString UniqueId, FOnGetDisplayNameComplete Delegate, FString DisplayName /*= TEXT("")*/)
{
	TSharedRef<const FUniqueNetIdAccelByteUser> UniqueIdObj = MakeShared<const FUniqueNetIdAccelByteUser>(UniqueId);
	return GetDisplayName(LocalUserNum, UniqueIdObj, Delegate, DisplayName);
}

bool FOnlineSubsystemAccelByteUtils::GetDisplayName(int32 LocalUserNum, TSharedPtr<const FUniqueNetId> UniqueId, FOnGetDisplayNameComplete Delegate, FString DisplayName)
{
	if (!UniqueId.IsValid())
	{
		Delegate.ExecuteIfBound(DisplayName);
		return false;
	}

	IOnlineIdentityPtr IdentityInt = Online::GetIdentityInterface(ACCELBYTE_SUBSYSTEM);
	TSharedPtr<const FUniqueNetId> LocalUserId = IdentityInt->GetUniquePlayerId(LocalUserNum);
	if (UniqueId.IsValid() && (LocalUserId.IsValid() && LocalUserId.Get() == UniqueId.Get())) {
		Delegate.ExecuteIfBound(IdentityInt->GetPlayerNickname(LocalUserId.ToSharedRef().Get()));
		return true;
	}
	
	IOnlineUserPtr UserInt = Online::GetUserInterface(ACCELBYTE_SUBSYSTEM);
	TArray<TSharedRef<const FUniqueNetId>> IdsToQuery;
	IdsToQuery.Add(UniqueId.ToSharedRef());

	FOnQueryUserInfoCompleteDelegate OnQueryUserInfoCompleteDelegate = FOnQueryUserInfoCompleteDelegate::CreateStatic(FOnlineSubsystemAccelByteUtils::OnQueryUserInfoComplete, Delegate);
	QueryUserHandle = OnQueryUserInfoCompleteDelegate.GetHandle();
	UserInt->AddOnQueryUserInfoCompleteDelegate_Handle(LocalUserNum, OnQueryUserInfoCompleteDelegate);
	UserInt->QueryUserInfo(LocalUserNum, IdsToQuery);
	return true;
}

void FOnlineSubsystemAccelByteUtils::OnQueryUserInfoComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<TSharedRef<const FUniqueNetId>>& FoundIds, const FString& ErrorMessage, FOnGetDisplayNameComplete Delegate)
{
	if (bWasSuccessful)
	{
		for (const TSharedRef<const FUniqueNetId>& UniqueId : FoundIds)
		{
			IOnlineUserPtr UserInt = Online::GetUserInterface(ACCELBYTE_SUBSYSTEM);
			TSharedPtr<FOnlineUser> UserInfo = UserInt->GetUserInfo(LocalUserNum, UniqueId.Get());
			if (UserInfo.IsValid())
			{
				Delegate.ExecuteIfBound(UserInfo->GetDisplayName());
			}
			UserInt->ClearOnQueryUserInfoCompleteDelegate_Handle(LocalUserNum, QueryUserHandle);
			QueryUserHandle.Reset();
			break;
		}
	}
}

TSharedPtr<const FUniqueNetId> FOnlineSubsystemAccelByteUtils::GetPlatformUniqueIdFromPlatformUserId(const FString& PlatformUserId)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::GetByPlatform();
	if (Subsystem == nullptr) {
		return nullptr;
	}

	const IOnlineIdentityPtr PlatformIdentityInt = Subsystem->GetIdentityInterface();
	if (PlatformIdentityInt.IsValid()) {
		return PlatformIdentityInt->CreateUniquePlayerId(PlatformUserId);
	}

	return nullptr;
}

TSharedRef<const FUniqueNetId> FOnlineSubsystemAccelByteUtils::GetAccelByteUserIdFromUniqueId(const FUniqueNetId& UniqueId)
{
	if (!UniqueId.IsValid())
	{
		return FUniqueNetIdAccelByteUser::Invalid();
	}
	
	if (UniqueId.GetType() == ACCELBYTE_SUBSYSTEM)
	{
		return UniqueId.AsShared();
	}

	return FUniqueNetIdAccelByteUser::Invalid();
}

EAccelByteLoginType FOnlineSubsystemAccelByteUtils::GetAccelByteLoginTypeFromNativeSubsystem(const FName& SubsystemName)
{
	const FString SubsystemStr = SubsystemName.ToString();
	const UEnum* LoginTypeEnum = StaticEnum<EAccelByteLoginType>();
	if (SubsystemStr.Equals(TEXT("GDK"), ESearchCase::IgnoreCase) || SubsystemStr.Equals(TEXT("Live"), ESearchCase::IgnoreCase))
	{
		return EAccelByteLoginType::Xbox;
	}
	else if (SubsystemStr.Equals(TEXT("PS4"), ESearchCase::IgnoreCase))
	{
		return EAccelByteLoginType::PS5;
	}
	else if (SubsystemStr.Equals(TEXT("PS5"), ESearchCase::IgnoreCase))
	{
		return EAccelByteLoginType::PS5;
	}
	else if (SubsystemStr.Equals(TEXT("STEAM"), ESearchCase::IgnoreCase))
	{
		return EAccelByteLoginType::Steam;
	}

	UE_LOG_AB(Warning, TEXT("Failed to convert subsystem '%s' to a usable login type for the AccelByte OSS! Most likely this subsystem is unsupported"), *SubsystemStr);
	return EAccelByteLoginType::None;
}

TMap<FString, FString> FOnlineSubsystemAccelByteUtils::UserPlatformMaps;
TMap<FString, FString> FOnlineSubsystemAccelByteUtils::UserJoinCached;
TMap<FString, FString> FOnlineSubsystemAccelByteUtils::UserDisconnectedCached;

void FOnlineSubsystemAccelByteUtils::AddUserPlatform(const FString& UserId, const FString PlatformName)
{
	UserPlatformMaps.Add(UserId, PlatformName);
}

FString FOnlineSubsystemAccelByteUtils::GetUserPlatform(const FString& UserId)
{
	if(UserPlatformMaps.Contains(UserId))
	{
		return UserPlatformMaps[UserId];
	}
	return TEXT("NULL");
}

void FOnlineSubsystemAccelByteUtils::AddUserJoinTime(const FString& UserId, const FString Value)
{
	UserJoinCached.Add(UserId, Value);
}

FString FOnlineSubsystemAccelByteUtils::GetUserJoinTime(const FString& UserId)
{
	if(UserJoinCached.Contains(UserId))
	{
		return UserJoinCached[UserId];
	}
	return FDateTime::Now().ToIso8601();
}

void FOnlineSubsystemAccelByteUtils::AddUserDisconnectedTime(const FString& UserId, const FString Value)
{
	UserDisconnectedCached.Add(UserId, Value);
}

FString FOnlineSubsystemAccelByteUtils::GetUserDisconnectedTime(const FString& UserId)
{
	if(UserDisconnectedCached.Contains(UserId))
	{
		return UserDisconnectedCached[UserId];
	}
	return FString();
}

