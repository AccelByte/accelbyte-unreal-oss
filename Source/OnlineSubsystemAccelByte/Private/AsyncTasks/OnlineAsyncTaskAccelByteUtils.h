// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"

namespace AccelByte
{
	
#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
template <typename DelegateSignature, typename UserPolicy = FDefaultDelegateUserPolicy>
#else
template <typename DelegateSignature>
#endif
class TDelegateUtils
{
	static_assert(sizeof(DelegateSignature) == 0, "Expected a function signature for the delegate template parameter");
};

template <class ObjectType, ESPMode Mode>
class TSelfPtr;

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
#define DELEGATE_TEMPLATE_TYPE TDelegate<InRetValType(ParamTypes...), UserPolicy>
template <typename InRetValType, typename... ParamTypes, typename UserPolicy>
class TDelegateUtils<DELEGATE_TEMPLATE_TYPE>
#else
#define DELEGATE_TEMPLATE_TYPE TBaseDelegate<InRetValType, ParamTypes...>
template <typename InRetValType, typename... ParamTypes>
class TDelegateUtils<DELEGATE_TEMPLATE_TYPE>
#endif
{
	using FuncType = InRetValType(ParamTypes...);
	typedef InRetValType RetValType;

public:
	template <typename UserClass, typename... VarTypes>
	UE_NODISCARD inline static DELEGATE_TEMPLATE_TYPE CreateThreadSafeSelfPtr(TSelfPtr<UserClass, ESPMode::ThreadSafe> *InUserObjectRef, typename TMemFunPtrType<false, UserClass, RetValType(ParamTypes..., VarTypes...)>::Type InFunc, VarTypes... Vars)
	{
		static_assert(!TIsConst<UserClass>::Value, "Attempting to bind a delegate with a const object pointer and non-const member function.");

		DELEGATE_TEMPLATE_TYPE Result;
#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
		TBaseSPMethodDelegateInstance<false, UserClass, ESPMode::ThreadSafe, FuncType, UserPolicy, VarTypes...>::Create(Result, InUserObjectRef->GetInternalSP(), InFunc, Vars...);
#else
		TBaseSPMethodDelegateInstance<false, UserClass, ESPMode::ThreadSafe, FuncType, VarTypes...>::Create(Result, InUserObjectRef->GetInternalSP(), InFunc, Vars...);
#endif
		return Result;
	}
	template <typename UserClass, typename... VarTypes>
	UE_NODISCARD inline static DELEGATE_TEMPLATE_TYPE CreateThreadSafeSelfPtr(TSelfPtr<UserClass, ESPMode::ThreadSafe> *InUserObjectRef, typename TMemFunPtrType<true, UserClass, RetValType(ParamTypes..., VarTypes...)>::Type InFunc, VarTypes... Vars)
	{
		DELEGATE_TEMPLATE_TYPE Result;
#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
		TBaseSPMethodDelegateInstance<true, const UserClass, ESPMode::ThreadSafe, FuncType, UserPolicy, VarTypes...>::Create(Result, InUserObjectRef->GetInternalSP(), InFunc, Vars...);
#else
		TBaseSPMethodDelegateInstance<true, const UserClass, ESPMode::ThreadSafe, FuncType, VarTypes...>::Create(Result, InUserObjectRef->GetInternalSP(), InFunc, Vars...);
#endif
		return Result;
	}
};

/**
 * Self Reference to this ObjectType
 * Use this when WeakPtr needed and the object lifecycle is not managed by SharedPointer
 * WARNING: Don't use this with TSharedFromThis because it could messed up with the reference
 */
template <class ObjectType, ESPMode Mode>
class TSelfPtr
{
public:
	TSelfPtr()
	{
		static_assert(!TIsDerivedFrom<ObjectType, TSharedFromThis<ObjectType, ESPMode::ThreadSafe>>::IsDerived, "ObjectType should not derived from TSharedFromThis");
		static_assert(!TIsDerivedFrom<ObjectType, TSharedFromThis<ObjectType, ESPMode::NotThreadSafe>>::IsDerived, "ObjectType should not derived from TSharedFromThis");
	}
	
	virtual ~TSelfPtr()
	{
		ThisPtr.Reset();
		checkf(!bHasReference, TEXT("the shared pointer still has reference to this"));
	}

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
	template<typename,typename> friend class TDelegateUtils;
#else
	template<typename> friend class TDelegateUtils;
#endif

private:
	/**
	 * Return self Shared Pointer, must store the returned value to weak pointer, otherwise the reference will be dangling
	 * This function must be private to avoid wrong usage
	 */
	TSharedRef< ObjectType, Mode > GetInternalSP()
	{
		if (!bHasReference)
		{
			ObjectType* Ptr = StaticCast<ObjectType*>(this);
			ThisPtr = MakeShareable(Ptr, [this](ObjectType *) { bHasReference = false; });
			bHasReference = true;
		}
		return ThisPtr.ToSharedRef();
	}
	TSharedPtr<ObjectType, Mode> ThisPtr;
	bool bHasReference{false};
};

}
