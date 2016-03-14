#include "IrisDevelopUtil.h"
#include "IrisInterpreter/IrisStructure/IrisClass.h"
#include "IrisInterpreter/IrisStructure/IrisModule.h"
#include "IrisInterpreter.h"
#include "IrisInterpreter/IrisNativeClasses/IrisInteger.h"
#include "IrisInterpreter/IrisNativeClasses/IrisFloat.h"
#include "IrisInterpreter/IrisNativeClasses/IrisString.h"
#include "IrisInterpreter/IrisNativeClasses/IrisUniqueString.h"
#include "IrisInterpreter/IrisNativeModules/IrisGC.h"

#include <string>
using namespace std;
namespace IrisDevUtil {

	IrisThreadUniqueInfo * GetCurrentThreadInfo()
	{
		return IrisThreadManager::CurrentThreadManager()->GetThreadInfo(this_thread::get_id());
	}

	bool CurrentThreadIsMainThread()
	{
		return IrisThreadManager::CurrentThreadManager()->IsMainThread();
	}

	bool CheckClass(IrisValue & ivValue, const char* strClassName)
	{
		auto pClass = ivValue.GetIrisObject()->GetClass();
		auto& strName = pClass->GetInternClass()->GetClassName();
		return strName == strClassName;
	}

	void GroanIrregularWithString(const char* strIrregularString)
	{
		IrisInterpreter* pInterpreter = IrisInterpreter::CurrentInterpreter();
		IrisValue ivValue = CreateInstanceByInstantValue(strIrregularString);
		pInterpreter->RegistIrregular(ivValue);
	}

	int GetInt(IrisValue & ivValue)
	{
		return IrisInteger::GetIntData(ivValue);
	}

	double GetFloat(IrisValue & ivValue)
	{
		return IrisFloat::GetFloatData(ivValue);
	}

	const char* GetString(IrisValue & ivValue)
	{
		if (CheckClass(ivValue, "String")) {
			return IrisString::GetString(ivValue).c_str();
		}
		else {
			return IrisUniqueString::GetString(ivValue).c_str();
		}

	}

	IrisValue CallMethod(IrisValue & ivObj, IIrisValues * pParameters, const char* strMethodName)
	{
		return static_cast<IrisObject*>(ivObj.GetIrisObject())->CallInstanceFunction(strMethodName, nullptr, pParameters, CallerSide::Outside);
	}

	IrisValue CreateInt(int nInteger)
	{
		return CreateInstanceByInstantValue(nInteger);
	}

	IrisValue CreateFloat(double dDouble)
	{
		return CreateInstanceByInstantValue(dDouble);
	}

	IrisValue CreateString(const char* nInteger)
	{
		return CreateInstanceByInstantValue(nInteger);
	}

	IIrisClass * GetClass(const char* strClassPathName)
	{
		return IrisInterpreter::CurrentInterpreter()->GetIrisClass(strClassPathName)->GetExternClass();
	}

	IIrisModule * GetModule(const char * strClassPathName)
	{
		return IrisInterpreter::CurrentInterpreter()->GetIrisModule(strClassPathName)->GetExternModule();
	}

	IIrisInterface * GetInterface(const char * strClassPathName)
	{
		return IrisInterpreter::CurrentInterpreter()->GetIrisInterface(strClassPathName)->GetExternInterface();
	}

	const IrisValue & Nil()
	{
		return IrisInterpreter::CurrentInterpreter()->Nil();
	}

	const IrisValue & False()
	{
		return IrisInterpreter::CurrentInterpreter()->False();
	}

	const IrisValue & True()
	{
		return IrisInterpreter::CurrentInterpreter()->True();
	}

	void AddInstanceMethod(IIrisClass * pClass, const char * szMethodName, IrisNativeFunction pfFunction, size_t nParamCount, bool bIsWithVariableParameter)
	{
		pClass->GetInternClass()->AddInstanceMethod(new IrisMethod(szMethodName, pfFunction, nParamCount, bIsWithVariableParameter));
	}

	void AddClassMethod(IIrisClass * pClass, const char * szMethodName, IrisNativeFunction pfFunction, size_t nParamCount, bool bIsWithVariableParameter)
	{
		pClass->GetInternClass()->AddClassMethod(new IrisMethod(szMethodName, pfFunction, nParamCount, bIsWithVariableParameter));
	}

	void AddInstanceMethod(IIrisModule * pModule, const char * szMethodName, IrisNativeFunction pfFunction, size_t nParamCount, bool bIsWithVariableParameter)
	{
		pModule->GetInternModule()->AddInstanceMethod(new IrisMethod(szMethodName, pfFunction, nParamCount, bIsWithVariableParameter));
	}

	void AddClassMethod(IIrisModule * pModule, const char * szMethodName, IrisNativeFunction pfFunction, size_t nParamCount, bool bIsWithVariableParameter)
	{
		pModule->GetInternModule()->AddClassMethod(new IrisMethod(szMethodName, pfFunction, nParamCount, bIsWithVariableParameter));
	}

	void AddGetter(IIrisClass * pClass, const char * szInstanceVariableName, IrisNativeFunction pfFunction)
	{
		pClass->GetInternClass()->AddGetter(szInstanceVariableName, pfFunction);
	}

	void AddSetter(IIrisClass* pClass, const char * szInstanceVariableName, IrisNativeFunction pfFunction)
	{
		pClass->GetInternClass()->AddSetter(szInstanceVariableName, pfFunction);
	}

	void AddConstance(IIrisClass * pClass, const char * szConstanceName, const IrisValue & ivValue)
	{
		pClass->GetInternClass()->AddConstance(szConstanceName, ivValue);
	}

	void AddConstance(IIrisModule * pModule, const char * szConstanceName, const IrisValue & ivValue)
	{
		pModule->GetInternModule()->AddConstance(szConstanceName, ivValue);
	}

	void AddClassVariable(IIrisClass * pClass, const char * szVariableName, const IrisValue & ivValue)
	{
		pClass->GetInternClass()->AddClassVariable(szVariableName, ivValue);
	}

	void AddClassVariable(IIrisModule * pClass, const char * szVariableName, const IrisValue & ivValue)
	{
		pClass->GetInternModule()->AddClassVariable(szVariableName, ivValue);
	}

	void AddModule(IIrisClass * pClass, IIrisModule * pTargetModule)
	{
		pClass->GetInternClass()->AddModule(pTargetModule->GetInternModule());
	}

	void AddModule(IIrisModule * pModule, IIrisModule * pTargetModule)
	{
		pModule->GetInternModule()->AddModule(pTargetModule->GetInternModule());
	}

	IIrisClass * GetNativeClass(const IrisValue & ivValue)
	{
		return nullptr;
	}

	IIrisModule * GetNativeModule(const IrisValue & ivValue)
	{
		return nullptr;
	}

	IIrisInterface * GetNativeInterface(const IrisValue & ivValue)
	{
		return nullptr;
	}

	IrisValue CreateInstance(IIrisClass * pClass, IIrisValues * ivsParams, IIrisContextEnvironment * pContexEnvironment)
	{
		return pClass->GetInternClass()->CreateInstance(ivsParams, pContexEnvironment);
	}

	IrisValue CreateInstanceByInstantValue(const char * szString)
	{
		auto pClass = GetClass("String");
		IrisValue ivValue;
		IrisObject* pObject = new IrisObject();
		pObject->SetClass(pClass);
		IrisStringTag* pString = new IrisStringTag(szString);
		pObject->SetNativeObject(pString);
		ivValue.SetIrisObject(pObject);

		IrisGC::CurrentGC()->AddSize(sizeof(IrisObject) + pObject->GetClass()->GetTrustteeSize(pObject->GetNativeObject()));
		IrisGC::CurrentGC()->Start();

		// ���¶��󱣴浽����
		IrisInterpreter::CurrentInterpreter()->AddNewInstanceToHeap(ivValue);
		return ivValue;
	}

	IrisValue CreateInstanceByInstantValue(double dFloat)
	{
		auto pClass = GetClass("Float");
		IrisValue ivValue;
		IrisObject* pObject = new IrisObject();
		pObject->SetClass(pClass);
		IrisFloatTag* pFloat = new IrisFloatTag(dFloat);
		pObject->SetNativeObject(pFloat);
		ivValue.SetIrisObject(pObject);

		IrisGC::CurrentGC()->AddSize(sizeof(IrisObject) + pObject->GetClass()->GetTrustteeSize(pObject->GetNativeObject()));
		IrisGC::CurrentGC()->Start();

		// ���¶��󱣴浽����
		IrisInterpreter::CurrentInterpreter()->AddNewInstanceToHeap(ivValue);
		return ivValue;
	}

	IrisValue CreateInstanceByInstantValue(int nInteger)
	{
		auto pClass = GetClass("Integer");
		IrisValue ivValue;
		IrisObject* pObject = new IrisObject();
		pObject->SetClass(pClass);
		IrisIntegerTag* pInteger = new IrisIntegerTag(nInteger);
		pObject->SetNativeObject(pInteger);
		ivValue.SetIrisObject(pObject);

		IrisGC::CurrentGC()->AddSize(sizeof(IrisObject) + pObject->GetClass()->GetTrustteeSize(pObject->GetNativeObject()));
		IrisGC::CurrentGC()->Start();

		// ���¶��󱣴浽����
		IrisInterpreter::CurrentInterpreter()->AddNewInstanceToHeap(ivValue);
		return ivValue;
	}

	//IrisValue CreateFloatInstanceByInstantValue(char * szLiterral)
	//{
	//	return IrisValue();
	//}
	//
	//IrisValue CreateIntegerInstanceByInstantValue(char * szLiterral)
	//{
	//	return IrisValue();
	//}
	//
	//IrisValue CreateStringInstanceByInstantValue(char * szLiterral)
	//{
	//	return IrisValue();
	//}

	IrisValue CreateUniqueStringInstanceByUniqueIndex(size_t nIndex)
	{
		IrisValue ivValue;
		bool bResult = false;
		ivValue = IrisUniqueString::GetUniqueString(nIndex, bResult);
		if (bResult) {
			return ivValue;
		}

		auto pClass = GetClass("UniqueString");

		IrisObject* pObject = new IrisObject();
		pObject->SetClass(pClass);
		pObject->SetPermanent(true);
		IrisUniqueStringTag* pString = new IrisUniqueStringTag(IrisCompiler::CurrentCompiler()->GetUniqueString(nIndex, IrisCompiler::CurrentCompiler()->GetCurrentFileIndex()));
		pObject->SetNativeObject(pString);
		ivValue.SetIrisObject(pObject);

		IrisGC::CurrentGC()->AddSize(sizeof(IrisObject) + pObject->GetClass()->GetTrustteeSize(pObject->GetNativeObject()));
		IrisGC::CurrentGC()->Start();

		// ���¶��󱣴浽����
		IrisInterpreter::CurrentInterpreter()->AddNewInstanceToHeap(ivValue);

		IrisUniqueString::AddUniqueString(nIndex, ivValue);
		return ivValue;
	}

	bool IrregularHappened()
	{
		return IrisInterpreter::CurrentInterpreter()->IrregularHappened();
	}

	bool FatalErrorHappened()
	{
		return IrisInterpreter::CurrentInterpreter()->FatalErrorHappened();
	}
}