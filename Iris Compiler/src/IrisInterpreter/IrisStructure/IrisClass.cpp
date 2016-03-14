#include "IrisInterpreter/IrisStructure/IrisClass.h"
#include "IrisInterpreter.h"
#include "IrisInterpreter/IrisNativeClasses/IrisClassBaseTag.h"
#include "IrisInterpreter/IrisNativeModules/IrisGC.h"
#include "IrisInterpreter/IrisStructure/IrisInterface.h"
#include "IrisInterpreter/IrisStructure/IrisMethod.h"
#include "IrisInterpreter/IrisStructure/IrisModule.h"
#include "IrisInterpreter/IrisStructure/IrisContextEnvironment.h"
#include "IrisFatalErrorHandler.h"
#include "IrisInterpreter/IrisNativeClasses/IrisUniqueString.h"

void IrisClass::_FunctionCollect(IrisInterface* pInterface, _InterfaceFunctionDeclareMap& mpFunctionDeclare) {
	// ����
	if (!pInterface)
		return;

	IrisInterface* pTmpInter = pInterface;

	// �Ӽ̳�����Ͷ˵Ľӿڿ�ʼ�������FunctionDeclare
	pTmpInter->m_iwlDecAddingLock.ReadLock();
	for (auto funcdec : pTmpInter->m_mpFunctionDeclareMap){
		// û�ҵ�����ӽ�ȥ��Ĭ�ϼ̳�����Ͷ˵Ľӿڷ������帲�Ǹ��ӿڵķ������壩
		if (mpFunctionDeclare.find(funcdec.first) == mpFunctionDeclare.end()){
			mpFunctionDeclare.insert(_InterfaceFunctionDeclarePair(funcdec.first, funcdec.second));
		}
	}
	pTmpInter->m_iwlDecAddingLock.ReadUnlock();

	// ��һ��
	pTmpInter->m_iwlInfAddingLock.ReadLock();
	for (auto inter : pTmpInter->m_mpInterfaces){
		_FunctionCollect(inter.second, mpFunctionDeclare);
	}
	pTmpInter->m_iwlInfAddingLock.ReadUnlock();
}

bool IrisClass::_FunctionAchieved() {
	_InterfaceFunctionDeclareMap mpFunctionDeclare;
	// �ݹ���ã������еĽӿڵ�FunctionDeclare���ŵ�map����
	m_iwlInterfaceAddingWLLock.ReadLock();
	for (auto inter : m_hsInterfaces){
		_FunctionCollect(inter.second, mpFunctionDeclare);
	}

	bool bResult = false;
	IrisMethod* pMethod = nullptr;
	// ��ʼ������
	for (auto funcdec : mpFunctionDeclare) {
		pMethod = GetMethod(IrisClass::SearchMethodType::InstanceMethod, funcdec.first, bResult);
		// ���û���ҵ�����ôֱ���˳�
		if (!pMethod) {
			m_iwlInterfaceAddingWLLock.ReadUnlock();
			return false;
		}
		else {
			// ����ҵ��˵��ǲ������ԣ������˳�
			auto pMethodDeclare = funcdec.second;
			if (pMethod->IsWithVariableParameter() != pMethodDeclare.m_bHaveVariableParameter
				|| pMethod->GetParameterAmount() != pMethodDeclare.m_nParameterAmount) {
				m_iwlInterfaceAddingWLLock.ReadUnlock();
				return false;
			}
		}
	}
	m_iwlInterfaceAddingWLLock.ReadUnlock();
	return true;
}

void IrisClass::_SearchModuleConstance(SearchVariableType eType, const string& strVariableName, IrisModule* pCurModule, IrisValue** pValue) {
	//����
	if (pCurModule == nullptr) {
		return;
	}
	
	bool bResult = false;
	if (eType == SearchVariableType::Constance) {
		auto& ivResult = pCurModule->GetCurrentModuleConstance(strVariableName, bResult);
		if (bResult) {
			*pValue = (IrisValue*)&ivResult;
			return;
		}
	}
	else {
		auto& ivResult = pCurModule->GetCurrentModuleClassVariable(strVariableName, bResult);
		if (bResult) {
			*pValue = (IrisValue*)&ivResult;
			return;
		}
	}

	for (auto module : pCurModule->GetModules()) {
		_SearchModuleConstance(eType, strVariableName, module.second, pValue);
		if (*pValue)
			return;
	}
}

const IrisValue& IrisClass::SearchConstance(const string& strConstName, bool& bResult) {
	bResult = true;
	IrisValue* pValue = nullptr;
	// ����˳���ࡢ��������ģ�顢����
	// ����
	//decltype(m_hsConstances)::iterator iCons;
	//if ((iCons = m_hsConstances.find(strConstName)) != m_hsConstances.end()){
	//	return iCons->second;
	//}

	auto& ivResultValue = GetCurrentClassConstance(strConstName, bResult);
	if (bResult) {
		return ivResultValue;
	}

	// ��������ģ��
	IrisClass* pCurClass = this;
	do{
		for (auto module : pCurClass->m_hsModules){
			_SearchModuleConstance(SearchVariableType::Constance, strConstName, module.second, &pValue);
			if (pValue){
				return *pValue;
			}
		}
		pCurClass = pCurClass->m_pSuperClass;
	} while (pCurClass);

	// ����
	pCurClass = m_pSuperClass;
	do{
		//if ((iCons = pCurClass->m_hsConstances.find(strConstName)) != pCurClass->m_hsConstances.end()){
		//	return iCons->second;
		//}
		auto& ivResultValue = pCurClass->GetCurrentClassConstance(strConstName, bResult);
		if (bResult) {
			return ivResultValue;
		}
		pCurClass = pCurClass->m_pSuperClass;
	} while (pCurClass);

	pValue = (IrisValue*)&IrisInterpreter::CurrentInterpreter()->GetConstance(strConstName, bResult);
	if (bResult) {
		return *pValue;
	}

	bResult = false;
	return IrisDev_Nil();
}

void IrisClass::AddInterface(IrisInterface* pInterface) {

	string strFullPath = "";
	IrisModule* pTmpModule = pInterface->m_pUpperModule;
	while (pTmpModule) {
		strFullPath = pTmpModule->GetModuleName() + "::" + strFullPath;
		pTmpModule = pTmpModule->GetUpperModule();
	}
	strFullPath += pInterface->GetInterfaceName();

	m_iwlInterfaceAddingWLLock.WriteLock();
	if (m_hsInterfaces.find(strFullPath) != m_hsInterfaces.end()) {
		m_hsInterfaces[strFullPath] = pInterface;
	}
	else {
		m_hsInterfaces.insert(_InterfacePair(strFullPath, pInterface));
	}
	m_iwlInterfaceAddingWLLock.WriteUnlock();
}

void IrisClass::AddModule(IrisModule* pModule) {

	string strFullPath = "";
	IrisModule* pTmpModule = pModule->GetUpperModule();
	while (pTmpModule) {
		strFullPath = pTmpModule->GetModuleName() + "::" + strFullPath;
		pTmpModule = pTmpModule->GetUpperModule();
	}
	strFullPath += pModule->GetModuleName();

	m_iwlModuleAddingWLLock.WriteLock();
	if (m_hsModules.find(strFullPath) != m_hsModules.end()) {
		m_hsModules[strFullPath] = pModule;
	}
	else {
		m_hsModules.insert(_ModulePair(strFullPath, pModule));
	}
	m_iwlModuleAddingWLLock.WriteUnlock();
}

const IrisValue& IrisClass::SearchClassVariable(const string& strClassVariableName, bool& bResult) {
	bResult = true;
	IrisValue* pValue = nullptr;
	// ����˳���ࡢ��������ģ�顢����
	// ����
	//decltype(m_hsClassVariables)::iterator iVariable;
	//if ((iVariable = m_hsClassVariables.find(strClassVariableName)) != m_hsClassVariables.end()) {
	//	auto& ivValue = iVariable->second;
	//	m_iwlClassVariableWLLock.ReadUnlock();
	//	return ivValue;
	//}

	auto& ivResultValue = GetCurrentClassConstance(strClassVariableName, bResult);
	if (bResult) {
		return ivResultValue;
	}

	// ��������ģ��
	IrisClass* pCurClass = this;
	do {
		for (auto& module : pCurClass->m_hsModules) {
			_SearchModuleConstance(SearchVariableType::ClassInstance, strClassVariableName, module.second, &pValue);
			if (pValue) {
				return *pValue;
			}
		}
		pCurClass = pCurClass->m_pSuperClass;
	} while (pCurClass);

	// ����
	pCurClass = m_pSuperClass;
	do {
		//if ((iVariable = pCurClass->m_hsClassVariables.find(strClassVariableName)) != pCurClass->m_hsClassVariables.end()) {
		//	auto& ivValue = iVariable->second;
		//	return ivValue;
		//}
		auto& ivResultValue = pCurClass->GetCurrentClassClassVariable(strClassVariableName, bResult);
		if (bResult) {
			return ivResultValue;
		}
		pCurClass = pCurClass->m_pSuperClass;
	} while (pCurClass);

	bResult = false;
	return IrisInterpreter::CurrentInterpreter()->Nil();
}

IrisValue IrisClass::CreateInstance(IIrisValues* ivsParams, IIrisContextEnvironment* pContexEnvironment) {
	// ����Ƿ���û��ʵ�ֵĽӿ�
	IrisValue ivValue;
	if (!m_bIsCompleteClass){
		if (!(m_bIsCompleteClass = _FunctionAchieved())) {
			// **Error**
			IrisFatalErrorHandler::CurrentFatalHandler()->ShowFatalErrorMessage(IrisFatalErrorHandler::FatalErrorType::ClassNotCompleteIrregular, 0, 0, "Class " + m_strClassName + " is still having some methods not defined but declared in the interfaces jointed.");
			return IrisInterpreter::CurrentInterpreter()->Nil();
		}
	}
	// ���ɶ���
	IrisObject* pObject = new IrisObject();
	pObject->SetClass(m_pExternClass);
	if (m_strClassName == "Object") {
		pObject->SetNativeObject(this);
		IrisGC::CurrentGC()->AddSize(sizeof(IrisObject));
	}
	else {
		pObject->SetNativeObject(m_pExternClass->NativeAlloc());
		IrisGC::CurrentGC()->AddSize(sizeof(IrisObject) + pObject->GetClass()->GetTrustteeSize(pObject->GetNativeObject()));
	}
	IrisGC::CurrentGC()->Start();
	ivValue.SetIrisObject(pObject);
	IrisDev_GetCurrentThreadInfo()->m_skTempNewObjectStack.push_back(pObject);
	pObject->CallInstanceFunction("__format", pContexEnvironment, static_cast<IrisValues*>(ivsParams), CallerSide::Outside);
	IrisDev_GetCurrentThreadInfo()->m_skTempNewObjectStack.pop_back();

	// ���¶��󱣴浽����
	IrisInterpreter::CurrentInterpreter()->AddNewInstanceToHeap(ivValue);

	return ivValue;
}

// Ϊ���ö����ṩ
//IrisValue IrisClass::CreateInstanceFromLiteral(char* pLiteral){
//	// ����Ƿ���û��ʵ�ֵĽӿ�
//	IrisValue ivValue;
//	// ���ɶ���
//	IrisObject* pObject = new IrisObject();
//	pObject->SetClass(this->GetExternClass());
//	pObject->SetNativeObject(m_pExternClass->GetLiteralObject(pLiteral));
//
//	IrisGC::CurrentGC()->AddSize(sizeof(IrisObject) + pObject->GetClass()->GetTrustteeSize(pObject->GetNativeObject()));
//	IrisGC::CurrentGC()->Start();
//
//	ivValue.SetIrisObject(pObject);
//	// ���¶��󱣴浽����
//	IrisInterpreter::CurrentInterpreter()->AddNewInstanceToHeap(ivValue);
//
//	return ivValue;
//}

void IrisClass::_ClassModuleMethodSearch(SearchMethodType eSearchType, IrisClass* pCurClass, const string& strMethodName, IrisMethod** ppMethod) {
	for (auto module : pCurClass->m_hsModules) {
		_ModuleMethodSearch(eSearchType, strMethodName, module.second, ppMethod);
		// �ҵ����˳�
		if (*ppMethod)
			return;
	}
}

void IrisClass::_ModuleMethodSearch(SearchMethodType eSearchType, const string& strFunctionName, IrisModule* pCurModule, IrisMethod** ppMethod) {
	// ����
	if (!pCurModule)
		return;
	// ����ҵ�
	// �෽��
	IrisMethod* pMethod = nullptr;
	if (eSearchType == SearchMethodType::ClassMethod) {
		pMethod = pCurModule->GetCurrentModuleMethod(IrisModule::SearchMethodType::ClassMethod, strFunctionName);
	}
	// ʵ������
	else {
		pMethod = pCurModule->GetCurrentModuleMethod(IrisModule::SearchMethodType::InstanceMethod, strFunctionName);
	}
	if (pMethod) {
		*ppMethod = pMethod;
		return;
	}
	// �����ݹ����
	for (auto module : pCurModule->GetModules()) {
		_ModuleMethodSearch(eSearchType, strFunctionName, module.second, ppMethod);
		if (*ppMethod)
			break;
	}
}

IrisMethod* IrisClass::GetMethod(SearchMethodType eSearchType, const string& strMethodName, bool& bIsCurClassMethod) {
	// �����Ĳ���˳�򣺱��ࡢ��������ģ�顢���࣬������Ҳ����Ǿ����Ҳ�����
	// ��鱾��
	// ����ǵ����෽���������
	bIsCurClassMethod = false;
	IrisMethod* pResultMethod = nullptr;
	if (eSearchType == SearchMethodType::ClassMethod) {
		if (pResultMethod = GetCurrentClassMethod(eSearchType, strMethodName)) {
			bIsCurClassMethod = true;
			return pResultMethod;
		}
		else {
			// �����������ģ��
			IrisMethod* pMethod = nullptr;
			IrisClass* pCurClass = this;

			// �Լ̳�������������������ģ�����ɨ��
			while (pCurClass) {
				_ClassModuleMethodSearch(eSearchType, pCurClass, strMethodName, &pMethod);
				if (pMethod) {
					// �ҵ�����
					bIsCurClassMethod = true;
					return pMethod;
				}
				pCurClass = pCurClass->m_pSuperClass;
			}
			// ��û�ҵ���ֻ���ڸ��������
			IrisClass* pCurSuperClass = m_pSuperClass;
			while (pCurSuperClass) {
				pResultMethod = pCurSuperClass->GetCurrentClassMethod(eSearchType, strMethodName);
				if (pResultMethod) {
					bIsCurClassMethod = true;
					return pResultMethod;
				}
				pCurSuperClass = pCurSuperClass->m_pSuperClass;
			}
			// ���û���ҵ���ȥClass��������
			bool bResult = false;
			pMethod = IrisInterpreter::CurrentInterpreter()->GetIrisClass("Class")->GetMethod(eSearchType, strMethodName, bResult);
			return pMethod;
		}
	}
	// ����ǵ���ʵ�������������
	else{
		if (pResultMethod = GetCurrentClassMethod(eSearchType, strMethodName)) {
			bIsCurClassMethod = true;
			return pResultMethod;
		}
		else {
			// �����������ģ��
			IrisMethod* pMethod = nullptr;
			IrisClass* pCurClass = this;

			// �Լ̳�������������������ģ�����ɨ��
			while (pCurClass) {
				_ClassModuleMethodSearch(eSearchType, pCurClass, strMethodName, &pMethod);
				if (pMethod) {
					// �ҵ�����
					bIsCurClassMethod = true;
					return pMethod;
				}
				pCurClass = pCurClass->m_pSuperClass;
			}
			// ��û�ҵ���ֻ���ڸ��������
			IrisClass* pCurSuperClass = m_pSuperClass;
			while (pCurSuperClass) {
				pResultMethod = pCurSuperClass->GetCurrentClassMethod(eSearchType, strMethodName);
				if (pResultMethod) {
					bIsCurClassMethod = true;
					return pResultMethod;
				}
				pCurSuperClass = pCurSuperClass->m_pSuperClass;
			}
		}
	}

	// ����֮�������Ҳ�����
	return nullptr;
}

IrisClass::IrisClass(const string& strClassName, IrisClass* pSuperClass, IrisModule* pUpperModule) : m_strClassName(strClassName), m_pSuperClass(pSuperClass), m_pUpperModule(pUpperModule) {
	if (IrisInterpreter::CurrentInterpreter()->GetIrisClass("Class")) {
		IrisValue ivValue = IrisInterpreter::CurrentInterpreter()->GetIrisClass("Class")->CreateInstance(nullptr, nullptr);
		((IrisClassBaseTag*)ivValue.GetIrisObject()->GetNativeObject())->SetClass(this);
		m_pClassObject = ivValue.GetIrisObject();
	}
}

IrisValue IrisClass::CallClassMethod(const string& strMethodName, IrisContextEnvironment* pContextEnvironment, IrisValues* ivParameters, CallerSide eSide, unsigned int nLineNumber, int nBelongingFileIndex) {
	bool bIsCrrentModuleMethod = false;

	//if (!strBelongingFileName) {
	//	strBelongingFileName = &IrisDev_GetCurrentThreadInfo()->m_strCurrentFileName;
	//}

	IrisMethod* pMethod = GetMethod(SearchMethodType::ClassMethod, strMethodName, bIsCrrentModuleMethod);
	if (pMethod) {
		// �ڲ�����
		if (eSide == CallerSide::Inside) {
			// ��ǰ�����û������
			if (bIsCrrentModuleMethod) {
				return pMethod->Call(IrisValue::WrapObjectPointerToIrisValue(m_pClassObject), pContextEnvironment, ivParameters);
			}
			// ��������ģ��/���Personal����
			else {
				if (pMethod->GetAuthority() == IrisMethod::MethodAuthority::Personal) {
					// **Error**
					IrisFatalErrorHandler::CurrentFatalHandler()->ShowFatalErrorMessage(IrisFatalErrorHandler::FatalErrorType::MethodAuthorityIrregular, nLineNumber, nBelongingFileIndex, "Method of " + strMethodName + " is PERSONAL and cannot be called from derrived class " + m_strClassName + ".");
					return IrisInterpreter::CurrentInterpreter()->Nil();
				}
				else {
					return pMethod->Call(IrisValue::WrapObjectPointerToIrisValue(m_pClassObject), pContextEnvironment, ivParameters);
				}
			}
		}
		// �ⲿ����
		else {
			// ֻ�ܵ���EveryOne�ķ���
			// ��ֹ����
			if (pMethod->GetAuthority() != IrisMethod::MethodAuthority::Everyone) {
				// **Error**
				IrisFatalErrorHandler::CurrentFatalHandler()->ShowFatalErrorMessage(IrisFatalErrorHandler::FatalErrorType::MethodAuthorityIrregular, nLineNumber, nBelongingFileIndex, "Method of " + strMethodName + " is not Everyone and cannot be called outside the class " + m_strClassName + ".");
				return IrisInterpreter::CurrentInterpreter()->Nil();
			}
			else {
				return pMethod->Call(IrisValue::WrapObjectPointerToIrisValue(m_pClassObject), pContextEnvironment, ivParameters);
			}
		}

		//return pMethod->Call(IrisValue::WrapObjectPointerToIrisValue(m_pClassObject), pContexEnvironment, ivParameters);
	}
	IrisFatalErrorHandler::CurrentFatalHandler()->ShowFatalErrorMessage(IrisFatalErrorHandler::FatalErrorType::NoMethodIrregular, nLineNumber, nBelongingFileIndex, "Method of " + strMethodName + " not found in class " + m_strClassName + ".");
	return IrisInterpreter::CurrentInterpreter()->Nil();
}

void IrisClass::ResetAllMethodsObject() {
	for (auto classmethod : m_hsClassMethods) {
		classmethod.second->ResetObject();
	}

	for (auto instancemethod : m_hsInstanceMethods) {
		instancemethod.second->ResetObject();
	}
}

IrisMethod* IrisClass::GetCurrentClassMethod(SearchMethodType eSearchType, const string& strMethodName) {
	IrisMethod* pResultMethod = nullptr;
	if (eSearchType == SearchMethodType::ClassMethod) {
		m_iwlClassMethodWLLock.ReadLock();
		decltype(m_hsClassMethods)::iterator iMethod;
		if ((iMethod = m_hsClassMethods.find(strMethodName)) == m_hsClassMethods.end()) {
			pResultMethod = nullptr;
		}
		else {
			pResultMethod = iMethod->second;
		}
		m_iwlClassMethodWLLock.ReadUnlock();
	}
	else {
		m_iwlInstanceMethodWLLock.ReadLock();
		decltype(m_hsInstanceMethods)::iterator iMethod;
		if ((iMethod = m_hsInstanceMethods.find(strMethodName)) == m_hsInstanceMethods.end()) {
			pResultMethod = nullptr;
		}
		else {
			pResultMethod = iMethod->second;
		}
		m_iwlInstanceMethodWLLock.ReadUnlock();
	}
	return pResultMethod;
}

const IrisValue & IrisClass::GetCurrentClassClassVariable(const string & strVariableName, bool & bResult)
{
	decltype(m_hsClassVariables)::iterator iVariable;
	m_iwlClassClassVariableWLLock.ReadLock();
	if ((iVariable = m_hsClassVariables.find(strVariableName)) != m_hsClassVariables.end()) {
		auto& ivResult = iVariable->second;
		bResult = true;
		m_iwlClassClassVariableWLLock.ReadUnlock();
		return ivResult;
	}
	else {
		bResult = false;
		m_iwlClassClassVariableWLLock.ReadUnlock();
		return IrisDev_Nil();
	}
}

const IrisValue & IrisClass::GetCurrentClassConstance(const string & strConstanceName, bool & bResult)
{
	decltype(m_hsConstances)::iterator iVariable;
	m_iwlClassConstanceWLLock.ReadLock();
	if ((iVariable = m_hsConstances.find(strConstanceName)) != m_hsConstances.end()) {
		auto& ivResult = iVariable->second;
		m_iwlClassConstanceWLLock.ReadUnlock();
		bResult = true;
		return ivResult;
	}
	else {
		bResult = false;
		m_iwlClassConstanceWLLock.ReadUnlock();
		return IrisDev_Nil();
	}
}

void IrisClass::AddSetter(const string& strProperName, IrisNativeFunction pfMethod) {
	string strMethodName = strProperName;
	strMethodName.assign(strMethodName.begin() + 1, strMethodName.end());
	strMethodName = "__set_" + strMethodName;
	IrisMethod* pMethod = new IrisMethod(strMethodName, pfMethod, 1, false);
	pMethod->SetMethodName(strMethodName);

	m_iwlInstanceMethodWLLock.WriteLock();
	decltype(m_hsInstanceMethods)::iterator iMethod;
	if ((iMethod = m_hsInstanceMethods.find(pMethod->GetMethodName())) != m_hsInstanceMethods.end()) {
		delete iMethod->second;
		m_hsInstanceMethods[pMethod->GetMethodName()] = pMethod;
	}
	else {
		m_hsInstanceMethods.insert(_MethodPair(pMethod->GetMethodName(), pMethod));
	}
	m_iwlInstanceMethodWLLock.WriteUnlock();
}

void IrisClass::AddGetter(const string& strProperName, IrisNativeFunction pfMethod) {
	string strMethodName = strProperName;
	strMethodName.assign(strMethodName.begin() + 1, strMethodName.end());
	strMethodName = "__get_" + strMethodName;
	IrisMethod* pMethod = new IrisMethod(strMethodName, pfMethod, 0, false);
	pMethod->SetMethodName(strMethodName);

	m_iwlInstanceMethodWLLock.WriteLock();
	decltype(m_hsInstanceMethods)::iterator iMethod;
	if ((iMethod = m_hsInstanceMethods.find(pMethod->GetMethodName())) != m_hsInstanceMethods.end()) {
		delete iMethod->second;
		m_hsInstanceMethods[pMethod->GetMethodName()] = pMethod;
	}
	else {
		m_hsInstanceMethods.insert(_MethodPair(pMethod->GetMethodName(), pMethod));
	}
	m_iwlInstanceMethodWLLock.WriteUnlock();
}

void IrisClass::ResetNativeObject() {
	IrisValue ivValue = IrisInterpreter::CurrentInterpreter()->GetIrisClass("Class")->CreateInstance(nullptr, nullptr);
	((IrisClassBaseTag*)ivValue.GetIrisObject()->GetNativeObject())->SetClass(this);
	m_pClassObject = static_cast<IrisObject*>(ivValue.GetIrisObject());
}


 void IrisClass::ClearInvolvingModules() {
	m_iwlModuleAddingWLLock.WriteLock();
	m_hsModules.clear();
	m_iwlModuleAddingWLLock.WriteUnlock();
}

 void IrisClass::ClearJointingInterfaces() {
	m_iwlInterfaceAddingWLLock.WriteLock();
	m_hsInterfaces.clear();
	m_iwlInterfaceAddingWLLock.WriteUnlock();
}

 void IrisClass::AddConstance(const string& strConstName, const IrisValue& ivInitialValue) {
	m_iwlClassConstanceWLLock.WriteLock();
	m_hsConstances.insert(_VariablePair(strConstName, ivInitialValue));
	m_iwlClassConstanceWLLock.WriteUnlock();
}

 void IrisClass::AddClassMethod(IrisMethod* pMethod) {
	decltype(m_hsClassMethods)::iterator iMethod;
	m_iwlClassMethodWLLock.WriteLock();
	if ((iMethod = m_hsClassMethods.find(pMethod->GetMethodName())) != m_hsClassMethods.end()) {
		//delete iMethod->second;
		m_hsClassMethods[pMethod->GetMethodName()] = pMethod;
	}
	else {
		m_hsClassMethods.insert(_MethodPair(pMethod->GetMethodName(), pMethod));
	}
	m_iwlClassMethodWLLock.WriteUnlock();
}

 void IrisClass::AddInstanceMethod(IrisMethod* pMethod) {
	decltype(m_hsInstanceMethods)::iterator iMethod;
	m_iwlInstanceMethodWLLock.WriteLock();
	if ((iMethod = m_hsInstanceMethods.find(pMethod->GetMethodName())) != m_hsInstanceMethods.end()) {
		//delete iMethod->second;
		m_hsInstanceMethods[pMethod->GetMethodName()] = pMethod;
	}
	else {
		m_hsInstanceMethods.insert(_MethodPair(pMethod->GetMethodName(), pMethod));
	}
	m_iwlInstanceMethodWLLock.WriteUnlock();
}

 void IrisClass::AddClassVariable(const string& strClassVariableName) {
	m_iwlClassClassVariableWLLock.WriteLock();
	m_hsClassVariables.insert(_VariablePair(strClassVariableName, IrisInterpreter::CurrentInterpreter()->Nil()));
	m_iwlClassClassVariableWLLock.WriteUnlock();
}

 void IrisClass::AddClassVariable(const string& strClassVariableName, const IrisValue& ivInitialValue) {
    m_iwlClassClassVariableWLLock.WriteLock();
	m_hsClassVariables.insert(_VariablePair(strClassVariableName, ivInitialValue));
	m_iwlClassClassVariableWLLock.WriteUnlock();
}

IrisClass::~IrisClass() {
	for (auto iter : m_hsInstanceMethods){
		delete iter.second;
	}
	for (auto iter : m_hsClassMethods){
		delete iter.second;
	}
	delete m_pExternClass;
}