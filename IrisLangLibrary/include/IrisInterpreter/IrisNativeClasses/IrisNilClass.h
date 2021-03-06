#ifndef _H_IRISNILCLASS_
#define _H_IRISNILCLASS_

#include "IrisDevHeader.h"
#include "IrisIntegerTag.h"
#include "IrisNilClassTag.h"

class IrisNilClass : public IIrisClass
{
public:
	static IrisValue InitializeFunction(IrisValue& ivObj, IIrisValues* ivsValues, IIrisValues* ivsVariableValues, IIrisContextEnvironment* pContextEnvironment) {
		return ivObj;
	}

	static IrisValue GetName(IrisValue& ivObj, IIrisValues* ivsValues, IIrisValues* ivsVariableValues, IIrisContextEnvironment* pContextEnvironment) {
		IrisNilClassTag* pNil = IrisDevUtil::GetNativePointer<IrisNilClassTag*>(ivObj);
		return IrisDevUtil::CreateString(pNil->GetName().c_str());
	}

	static IrisValue LogicNot(IrisValue& ivObj, IIrisValues* ivsValues, IIrisValues* ivsVariableValues, IIrisContextEnvironment* pContextEnvironment) {
		return IrisDevUtil::True();
	}

public:
	void Mark(void* pNativeObjectPointer) {}

	const char* NativeClassNameDefine() const {
		return "NilClass";
	}

	IIrisClass* NativeSuperClassDefine() const {
		return IrisDevUtil::GetClass("Object");
	}

	int GetTrustteeSize(void* pNativePointer) {
		return sizeof(IrisNilClassTag);
	}

	void* NativeAlloc() {
		return new IrisNilClassTag();
	}

	void NativeFree(void* pNativePointer) {
		delete static_cast<IrisNilClassTag*>(pNativePointer);
	}

	void NativeClassDefine() {
		IrisDevUtil::AddInstanceMethod(this, "__format", InitializeFunction, 0, false);
		IrisDevUtil::AddInstanceMethod(this, "name", GetName, 0, false);
		IrisDevUtil::AddInstanceMethod(this, "!", LogicNot, 0, false);
	}

	IrisNilClass();
	~IrisNilClass();
};

#endif