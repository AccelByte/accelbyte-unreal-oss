// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "NboSerializer.h"

class FNboSerializeToBufferAccelByte : public FNboSerializeToBuffer
{
public:
	FNboSerializeToBufferAccelByte() : FNboSerializeToBuffer(512) 
	{
	}
	
	FNboSerializeToBufferAccelByte(uint32 Size) : FNboSerializeToBuffer(Size) 
	{
	}

	friend inline FNboSerializeToBufferAccelByte& operator<<(FNboSerializeToBufferAccelByte& Ar, const FOnlineSessionInfoAccelByte& SessionInfo)
	{
		check(SessionInfo.GetHostAddr().IsValid());
		Ar << *SessionInfo.GetSessionId().ToString();
		Ar << *SessionInfo.GetHostAddr();
		return Ar;
	}

	friend inline FNboSerializeToBufferAccelByte& operator<<(FNboSerializeToBufferAccelByte& Ar, const FUniqueNetIdAccelByteUser& UniqueId)
	{
		Ar << UniqueId.UniqueNetIdStr;
		return Ar;
	}

	friend inline FNboSerializeToBufferAccelByte& operator<<(FNboSerializeToBufferAccelByte& Ar, const FUniqueNetIdAccelByteResource& UniqueId)
	{
		Ar << UniqueId.UniqueNetIdStr;
		return Ar;
	}
};

class FNboSerializeFromBufferAccelByte : public FNboSerializeFromBuffer
{
public:
	FNboSerializeFromBufferAccelByte(uint8* Packet,int32 Length) : FNboSerializeFromBuffer(Packet,Length) 
	{
	}
	
	friend inline FNboSerializeFromBufferAccelByte& operator>>(FNboSerializeFromBufferAccelByte& Ar, FOnlineSessionInfoAccelByte& SessionInfo)
	{
		check(SessionInfo.GetHostAddr().IsValid());
		Ar >> *SessionInfo.GetSessionIdRef();
		Ar >> *SessionInfo.GetHostAddr();
		return Ar;
	}
	
	friend inline FNboSerializeFromBufferAccelByte& operator>>(FNboSerializeFromBufferAccelByte& Ar, FUniqueNetIdAccelByteUser& UniqueId)
	{
		Ar >> UniqueId.UniqueNetIdStr;
		return Ar;
	}

	friend inline FNboSerializeFromBufferAccelByte& operator>>(FNboSerializeFromBufferAccelByte& Ar, FUniqueNetIdAccelByteResource& UniqueId)
	{
		Ar >> UniqueId.UniqueNetIdStr;
		return Ar;
	}
};
