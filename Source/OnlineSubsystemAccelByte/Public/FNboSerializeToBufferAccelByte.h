// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByteTypes.h"
#if !(ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
#include "NboSerializer.h"
#else
#include "Online/NboSerializer.h"
#include "NboSerializerOSS.h"
#endif

class FNboSerializeToBufferAccelByte
#if !(ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1) 
	: public FNboSerializeToBuffer
#else
	: public FNboSerializeToBufferOSS
#endif
{
public:
	FNboSerializeToBufferAccelByte() 
		: FNboSerializeToBufferAccelByte(512)
	{
	}
	
	FNboSerializeToBufferAccelByte(uint32 Size) 
#if !(ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1) 
		: FNboSerializeToBuffer(Size) 
#else
		: FNboSerializeToBufferOSS(Size)
#endif
	{
	}

	friend inline FNboSerializeToBufferAccelByte& operator<<(FNboSerializeToBufferAccelByte& Ar, const FOnlineSessionInfoAccelByteV1& SessionInfo)
	{
		check(SessionInfo.GetHostAddr().IsValid());
		((FNboSerializeToBuffer&)Ar) << *SessionInfo.GetSessionId().ToString();
		((FNboSerializeToBuffer&)Ar) << *SessionInfo.GetHostAddr();
		return Ar;
	}

	friend inline FNboSerializeToBufferAccelByte& operator<<(FNboSerializeToBufferAccelByte& Ar, const FUniqueNetIdAccelByteUser& UniqueId)
	{
		((FNboSerializeToBuffer&)Ar) << UniqueId.UniqueNetIdStr;
		return Ar;
	}

	friend inline FNboSerializeToBufferAccelByte& operator<<(FNboSerializeToBufferAccelByte& Ar, const FUniqueNetIdAccelByteResource& UniqueId)
	{
		((FNboSerializeToBuffer&)Ar) << UniqueId.UniqueNetIdStr;
		return Ar;
	}
};

class FNboSerializeFromBufferAccelByte 
#if !(ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1) 
	: public FNboSerializeFromBuffer
#else
	: public FNboSerializeFromBufferOSS
#endif
{
public:
	FNboSerializeFromBufferAccelByte(uint8* Packet,int32 Length) 
#if !(ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1) 
		: FNboSerializeFromBuffer(Packet, Length)
#else
		: FNboSerializeFromBufferOSS(Packet, Length)
#endif
	{
	}
	
	friend inline FNboSerializeFromBufferAccelByte& operator>>(FNboSerializeFromBufferAccelByte& Ar, FOnlineSessionInfoAccelByteV1& SessionInfo)
	{
		check(SessionInfo.GetHostAddr().IsValid());
		TSharedRef<FUniqueNetIdAccelByteResource> Session = ConstCastSharedRef<FUniqueNetIdAccelByteResource>(SessionInfo.GetSessionIdRef());
		Ar >> *Session;
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
