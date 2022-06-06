// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"

namespace AccelByte
{
	
template <typename DelegateSignature, typename UserPolicy = FDefaultDelegateUserPolicy>
class TDelegateUtils
{
	static_assert(sizeof(DelegateSignature) == 0, "Expected a function signature for the delegate template parameter");
};

template <class ObjectType, ESPMode Mode>
class TSelfPtr;

template <typename InRetValType, typename... ParamTypes, typename UserPolicy>
class TDelegateUtils<TDelegate<InRetValType(ParamTypes...), UserPolicy>>
{
	using FuncType = InRetValType(ParamTypes...);
	typedef InRetValType RetValType;

public:
	template <typename UserClass, typename... VarTypes>
	UE_NODISCARD inline static TDelegate<RetValType(ParamTypes...), UserPolicy> CreateThreadSafeSelfPtr(TSelfPtr<UserClass, ESPMode::ThreadSafe> *InUserObjectRef, typename TMemFunPtrType<false, UserClass, RetValType(ParamTypes..., VarTypes...)>::Type InFunc, VarTypes... Vars)
	{
		static_assert(!TIsConst<UserClass>::Value, "Attempting to bind a delegate with a const object pointer and non-const member function.");

		TDelegate<RetValType(ParamTypes...), UserPolicy> Result;
		TBaseSPMethodDelegateInstance<false, UserClass, ESPMode::ThreadSafe, FuncType, UserPolicy, VarTypes...>::Create(Result, InUserObjectRef->GetInternalSP(), InFunc, Vars...);
		return Result;
	}
	template <typename UserClass, typename... VarTypes>
	UE_NODISCARD inline static TDelegate<RetValType(ParamTypes...), UserPolicy> CreateThreadSafeSelfPtr(TSelfPtr<UserClass, ESPMode::ThreadSafe> *InUserObjectRef, typename TMemFunPtrType<true, UserClass, RetValType(ParamTypes..., VarTypes...)>::Type InFunc, VarTypes... Vars)
	{
		TDelegate<RetValType(ParamTypes...), UserPolicy> Result;
		TBaseSPMethodDelegateInstance<true, const UserClass, ESPMode::ThreadSafe, FuncType, UserPolicy, VarTypes...>::Create(Result, InUserObjectRef->GetInternalSP(), InFunc, Vars...);
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

	template<typename,typename> friend class TDelegateUtils;

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
