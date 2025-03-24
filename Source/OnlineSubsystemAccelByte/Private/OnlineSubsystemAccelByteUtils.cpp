#include "OnlineSubsystemAccelByteUtils.h"

#include "HAL/Platform.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "OnlineSubsystemAccelByte.h"
#include "Interfaces/OnlineUserInterface.h"
#include "Online.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemNames.h"
#include "OnlineSubsystemUtils.h"

FDelegateHandle FOnlineSubsystemAccelByteUtils::QueryUserHandle = FDelegateHandle();

FUniqueNetIdRef FOnlineSubsystemAccelByteUtils::GetUniqueIdFromString(FString UniqueIdString, bool bIsEncoded /*= true*/) {
	if (UniqueIdString.IsEmpty()) {
		return FUniqueNetIdAccelByteUser::Invalid();
	}

	if (!bIsEncoded) {
		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = UniqueIdString;
		return FUniqueNetIdAccelByteUser::Create(CompositeId);
	}

	return FUniqueNetIdAccelByteUser::Create(UniqueIdString);
}

FUniqueNetIdPtr FOnlineSubsystemAccelByteUtils::GetPlatformUniqueIdFromUniqueId(const FUniqueNetId& UniqueId)
{
	// Can't get a valid platform id if the unique id belongs to a different platform
	if (!IsPlayerOnSamePlatform(UniqueId)) return nullptr;

	if (UniqueId.GetType() == ACCELBYTE_USER_ID_TYPE)
	{
		FUniqueNetIdAccelByteUserRef AccelByteId = FUniqueNetIdAccelByteUser::CastChecked(UniqueId);
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
	FUniqueNetIdAccelByteUserRef UniqueId = FUniqueNetIdAccelByteUser::CastChecked(GetUniqueIdFromString(UniqueIdString));
	return IsPlayerOnSamePlatform(UniqueId.Get());
}

FString FOnlineSubsystemAccelByteUtils::GetAccelByteIdFromUniqueId(const FUniqueNetId& UniqueId)
{
	if (UniqueId.GetType() == ACCELBYTE_USER_ID_TYPE)
	{
		FUniqueNetIdAccelByteUserRef CompositeId = FUniqueNetIdAccelByteUser::CastChecked(UniqueId);
		return CompositeId->GetAccelByteId();
	}

	return TEXT("INVALID");
}

FString FOnlineSubsystemAccelByteUtils::GetPlatformNameFromUniqueId(const FUniqueNetId& UniqueId) 
{
	if (UniqueId.GetType() == ACCELBYTE_USER_ID_TYPE)
	{
		FUniqueNetIdAccelByteUserRef AccelByteId = FUniqueNetIdAccelByteUser::CastChecked(UniqueId);
		return AccelByteId->GetPlatformType();
	}

	return TEXT("NULL");
}

FString FOnlineSubsystemAccelByteUtils::GetPlatformIdStringFromUniqueId(const FUniqueNetId& UniqueId) 
{
	if (UniqueId.GetType() == ACCELBYTE_USER_ID_TYPE) 
	{
		FUniqueNetIdAccelByteUserRef AccelByteId = FUniqueNetIdAccelByteUser::CastChecked(UniqueId);
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
	FUniqueNetIdAccelByteUserRef UniqueIdObj = FUniqueNetIdAccelByteUser::Create(UniqueId);
	return GetDisplayName(LocalUserNum, UniqueIdObj, Delegate, DisplayName);
}

bool FOnlineSubsystemAccelByteUtils::GetDisplayName(int32 LocalUserNum, FUniqueNetIdPtr UniqueId, FOnGetDisplayNameComplete Delegate, FString DisplayName)
{
	if (!UniqueId.IsValid())
	{
		Delegate.ExecuteIfBound(DisplayName);
		return false;
	}

	IOnlineIdentityPtr IdentityInterface = ::Online::GetIdentityInterface(ACCELBYTE_SUBSYSTEM);
	FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (UniqueId.IsValid() && (LocalUserId.IsValid() && LocalUserId.Get() == UniqueId.Get())) 
	{
		Delegate.ExecuteIfBound(IdentityInterface->GetPlayerNickname(LocalUserId.ToSharedRef().Get()));
		return true;
	}
	
	IOnlineUserPtr UserInterface = ::Online::GetUserInterface(ACCELBYTE_SUBSYSTEM);
	TArray<FUniqueNetIdRef> IdsToQuery;
	IdsToQuery.Add(UniqueId.ToSharedRef());

	FOnQueryUserInfoCompleteDelegate OnQueryUserInfoCompleteDelegate = FOnQueryUserInfoCompleteDelegate::CreateStatic(FOnlineSubsystemAccelByteUtils::OnQueryUserInfoComplete, Delegate);
	QueryUserHandle = OnQueryUserInfoCompleteDelegate.GetHandle();
	UserInterface->AddOnQueryUserInfoCompleteDelegate_Handle(LocalUserNum, OnQueryUserInfoCompleteDelegate);
	UserInterface->QueryUserInfo(LocalUserNum, IdsToQuery);
	return true;
}

void FOnlineSubsystemAccelByteUtils::OnQueryUserInfoComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<TSharedRef<const FUniqueNetId>>& FoundIds, const FString& ErrorMessage, FOnGetDisplayNameComplete Delegate)
{
	if (bWasSuccessful)
	{
		for (const FUniqueNetIdRef& UniqueId : FoundIds)
		{
			IOnlineUserPtr UserInterface = ::Online::GetUserInterface(ACCELBYTE_SUBSYSTEM);
			TSharedPtr<FOnlineUser> UserInfo = UserInterface->GetUserInfo(LocalUserNum, UniqueId.Get());
			if (UserInfo.IsValid())
			{
				Delegate.ExecuteIfBound(UserInfo->GetDisplayName());
			}
			UserInterface->ClearOnQueryUserInfoCompleteDelegate_Handle(LocalUserNum, QueryUserHandle);
			QueryUserHandle.Reset();
			break;
		}
	}
}

FUniqueNetIdPtr FOnlineSubsystemAccelByteUtils::GetPlatformUniqueIdFromPlatformUserId(const FString& PlatformUserId)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::GetByPlatform();
	if (Subsystem == nullptr) {
		return nullptr;
	}

	const IOnlineIdentityPtr PlatformIdentityInterface = Subsystem->GetIdentityInterface();
	if (PlatformIdentityInterface.IsValid()) {
		return PlatformIdentityInterface->CreateUniquePlayerId(PlatformUserId);
	}

	return nullptr;
}

FUniqueNetIdRef FOnlineSubsystemAccelByteUtils::GetAccelByteUserIdFromUniqueId(const FUniqueNetId& UniqueId)
{
	if (!UniqueId.IsValid())
	{
		return FUniqueNetIdAccelByteUser::Invalid();
	}
	
	if (UniqueId.GetType() == ACCELBYTE_USER_ID_TYPE)
	{
		return UniqueId.AsShared();
	}

	return FUniqueNetIdAccelByteUser::Invalid();
}

EAccelByteLoginType FOnlineSubsystemAccelByteUtils::GetAccelByteLoginTypeFromNativeSubsystem(const FName& SubsystemName)
{
	const FString SubsystemStr = SubsystemName.ToString();
	const UEnum* LoginTypeEnum = StaticEnum<EAccelByteLoginType>();
	if (SubsystemName.IsNone())
	{
		return EAccelByteLoginType::None;
	}

	if (SubsystemStr.Equals(TEXT("GDK"), ESearchCase::IgnoreCase) 
		|| SubsystemStr.Equals(TEXT("Live"), ESearchCase::IgnoreCase))
	{
		return EAccelByteLoginType::Xbox;
	}
	else if (SubsystemStr.Equals(TEXT("PS4"), ESearchCase::IgnoreCase)
		|| SubsystemStr.Equals(PS4_SUBSYSTEM.ToString(), ESearchCase::IgnoreCase))
	{
#if defined(WITH_CROSSGENSDK) && WITH_CROSSGENSDK
		return EAccelByteLoginType::PS4CrossGen;
#else
		return EAccelByteLoginType::PS4;
#endif
	}
	else if (SubsystemStr.Equals(TEXT("PS5"), ESearchCase::IgnoreCase))
	{
		return EAccelByteLoginType::PS5;
	}
	else if (SubsystemStr.Equals(TEXT("PSPC"), ESearchCase::IgnoreCase))
	{
		return EAccelByteLoginType::PSPC;
	}
	else if (SubsystemStr.Equals(TEXT("STEAM"), ESearchCase::IgnoreCase)
		|| SubsystemStr.Equals(STEAM_SUBSYSTEM.ToString(), ESearchCase::IgnoreCase))
	{
		return EAccelByteLoginType::Steam;
	}
	else if (SubsystemStr.Equals(TEXT("EOS"), ESearchCase::IgnoreCase)
		|| SubsystemStr.Equals(EOS_SUBSYSTEM.ToString(), ESearchCase::IgnoreCase))
	{
		return EAccelByteLoginType::EOS;
	}
	else if (SubsystemStr.Equals(TEXT("APPLE"), ESearchCase::IgnoreCase) 
		|| SubsystemStr.Equals(APPLE_SUBSYSTEM.ToString(), ESearchCase::IgnoreCase))
	{
		return EAccelByteLoginType::Apple;
	}
	else if (SubsystemStr.Equals(TEXT("GOOGLE"), ESearchCase::IgnoreCase)
		|| SubsystemStr.Equals(GOOGLE_SUBSYSTEM.ToString(), ESearchCase::IgnoreCase))
	{
		return EAccelByteLoginType::Google;
	}
	else if (SubsystemStr.Equals(TEXT("SWITCH"), ESearchCase::IgnoreCase)
		|| SubsystemStr.Equals(SWITCH_SUBSYSTEM.ToString(), ESearchCase::IgnoreCase))
	{
		return EAccelByteLoginType::Switch;
	}
	/* TODO: uncomment when able to authenticate GooglePlay using Server Auth Code
	else if (SubsystemStr.Equals(TEXT("GOOGLEPLAY"), ESearchCase::IgnoreCase)
		|| SubsystemStr.Equals(GOOGLEPLAY_SUBSYSTEM.ToString(), ESearchCase::IgnoreCase))
	{
		return EAccelByteLoginType::GooglePlayGames;
	}
	*/

	UE_LOG_AB(Warning, TEXT("Failed to convert subsystem '%s' to a usable login type for the AccelByte OSS! Most likely this subsystem is unsupported"), *SubsystemStr);
	return EAccelByteLoginType::None;
}

EAccelBytePlatformType FOnlineSubsystemAccelByteUtils::GetAccelBytePlatformTypeFromAuthType(const FString& InAuthType)
{
	if (InAuthType.Equals(TEXT("STEAM"), ESearchCase::IgnoreCase))
	{
		return EAccelBytePlatformType::Steam;
	}
	else if (InAuthType.Equals(TEXT("PS4"), ESearchCase::IgnoreCase))
	{
#if defined(WITH_CROSSGENSDK) && WITH_CROSSGENSDK
		return EAccelBytePlatformType::PS4CrossGen;
#else
		return EAccelBytePlatformType::PS4;
#endif
	}
	else if (InAuthType.Equals(TEXT("PS5"), ESearchCase::IgnoreCase))
	{
		return EAccelBytePlatformType::PS5;
	}
	else if (InAuthType.Equals(TEXT("APPLE"), ESearchCase::IgnoreCase))
	{
		return EAccelBytePlatformType::Apple;
	}
	else if (InAuthType.Equals(TEXT("GOOGLE"), ESearchCase::IgnoreCase) 
		|| InAuthType.Equals(TEXT("GOOGLEPLAYGAMES"), ESearchCase::IgnoreCase))
	{
		return EAccelBytePlatformType::GooglePlayGames;
	}
	else if (InAuthType.Equals(TEXT("PSPC"), ESearchCase::IgnoreCase))
	{
		return EAccelBytePlatformType::PSPC;
	}
	else if (InAuthType.Equals(TEXT("LIVE"), ESearchCase::IgnoreCase) 
		|| InAuthType.Equals(TEXT("GDK"), ESearchCase::IgnoreCase))
	{
		return EAccelBytePlatformType::Live;
	}
	else if (InAuthType.Equals(TEXT("EPICGAMES"), ESearchCase::IgnoreCase) 
		|| InAuthType.Equals(TEXT("EOS"), ESearchCase::IgnoreCase))
	{
		return EAccelBytePlatformType::EpicGames;
	}
	else if (InAuthType.Equals(TEXT("DEVICE"), ESearchCase::IgnoreCase))
	{
		return EAccelBytePlatformType::Device;
	}
	return EAccelBytePlatformType::None;
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

FString FOnlineSubsystemAccelByteUtils::GetLocalTimeOffsetFromUTC()
{
#if !UE_BUILD_SHIPPING
	FString TimeZoneOverrideValue;
	if (FParse::Value(FCommandLine::Get(), TEXT("-TimeZoneOverride="), TimeZoneOverrideValue))
	{
		return TimeZoneOverrideValue;
	}
#endif

	// Get current time locally as well as in UTC to calculate an offset for timezone
	const FDateTime UtcTime = FDateTime::UtcNow();
	const FDateTime LocalTime = FDateTime::Now();

	// Begin by checking if we are not on the same day as UTC, so that we can either advance the UTC time or local time by
	// a day to get an accurate measurement of hours behind/ahead of UTC
	int32 UtcHour = UtcTime.GetHour();
	int32 LocalHour = LocalTime.GetHour();
	if (UtcTime.GetDay() != LocalTime.GetDay())
	{
		if (UtcTime > LocalTime) UtcHour += 24;
		else if (LocalTime > UtcTime) LocalHour += 24;
	}

	// Now calculate the difference in hours, and create a local offset format timezone, or UTC if no difference
	const int32 HourDifferenceFromUtc = LocalHour - UtcHour;
	const int32 MinuteDifferenceFromUtc = LocalTime.GetMinute() - UtcTime.GetMinute();
	FString LocalOffsetTimezone;
	if (HourDifferenceFromUtc == 0 && MinuteDifferenceFromUtc == 0)
	{
		LocalOffsetTimezone = TEXT("UTC");
	}
	else
	{
		// Getting the sign manually since we need plus or minus
		FString LocalOffsetSign;
		if (HourDifferenceFromUtc >= 0)
		{
			LocalOffsetSign = TEXT("+");
		}
		else
		{
			LocalOffsetSign = TEXT("-");
		}

		// Converting both hour and minute offset to a string to allow for prepending a leading zero
		// #NOTE Using abs for difference values as we already have this from the offset sign
		FString HourString = FString::FromInt(FMath::Abs(HourDifferenceFromUtc));
		if (HourString.Len() == 1)
		{
			HourString = TEXT("0") + HourString;
		}

		FString MinuteString = FString::FromInt(FMath::Abs(MinuteDifferenceFromUtc));
		if (MinuteString.Len() == 1)
		{
			MinuteString = TEXT("0") + MinuteString;
		}

		// Finally, set the LocalOffsetTimezone value to our formatted string
		LocalOffsetTimezone = FString::Printf(TEXT("%s%s:%s"), *LocalOffsetSign, *HourString, *MinuteString);
	}

	UE_LOG_AB(Verbose, TEXT("Calculated local time offset from UTC: %s"), *LocalOffsetTimezone);
	return LocalOffsetTimezone;
}

FString FOnlineSubsystemAccelByteUtils::GetStringFromStringTable(const FString& StringTable, const int32 Key)
{
	const FString StringKey = FString::Printf(TEXT("%d"), Key);
	return FText::FromStringTable(*StringTable, StringKey).ToString();
}

EAccelBytePlatformType FOnlineSubsystemAccelByteUtils::GetCurrentAccelBytePlatformType(const FName& NativeSubsystemName)
{
	auto LoginType = GetAccelByteLoginTypeFromNativeSubsystem(NativeSubsystemName);
	auto PlatformType = ConvertOSSTypeToAccelBytePlatformType(LoginType);
	
	//To retain original behavior
	if (PlatformType == EAccelBytePlatformType::None)
	{
		PlatformType = EAccelBytePlatformType::Device;
	}
	return PlatformType;
}

bool FOnlineSubsystemAccelByteUtils::IsValidLocalUserNum(const int32& InLocalUserNum)
{
	return InLocalUserNum >= 0 && InLocalUserNum < MAX_LOCAL_PLAYERS;
}
