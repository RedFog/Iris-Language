#include "IrisInterpreter/IrisStructure/IrisMethod.h"
#include "IrisInterpreter/IrisNativeClasses/IrisMethodBase.h"
#include "IrisInterpreter/IrisStructure/IrisClass.h"
#include "IrisInterpreter/IrisStructure/IrisClosureBlock.h"
#include "IrisUnil/IrisIdentifier.h"
#include "IrisComponents/IrisParts/IrisFunctionHeader.h"
#include "IrisInterpreter/IrisStructure/IrisContextEnvironment.h"
#include "IrisUnil/IrisBlock.h"
#include "IrisUnil/IrisValues.h"
#include "IrisFatalErrorHandler.h"
#include "IrisInterpreter/IrisNativeModules/IrisGC.h"

IrisMethod::IrisMethod(const string& strMethodName, IrisNativeFunction pfNativeFunction, int nParameterAmount, bool bIsWithVariableParameter, MethodAuthority eAuthority) {
	m_strMethodName = strMethodName;
	m_eMethodType = MethodType::NativeMethod;
	m_uFunction.m_pfNativeFunction = pfNativeFunction;
	m_bIsWithVariableParameter = bIsWithVariableParameter;
	m_nParameterAmount = nParameterAmount;
	m_eAuthority = eAuthority;

	IrisClass* pMethodClass = nullptr;
	if (pMethodClass = IrisInterpreter::CurrentInterpreter()->GetIrisClass("Method")) {
		IrisValue ivValue = pMethodClass->CreateInstance(nullptr, nullptr);
		((IrisMethodBaseTag*)ivValue.GetIrisObject()->GetNativeObject())->SetMethod(this);
		m_pMethodObject = ivValue.GetIrisObject();
	}
}

IrisMethod::IrisMethod(const string& strMethodName, UserFunction* pUserFunction, MethodAuthority eAuthority) {
	m_strMethodName = strMethodName;
	m_eMethodType = MethodType::UserMethod;
	m_uFunction.m_pUserFunction = pUserFunction;
	m_bIsWithVariableParameter = (pUserFunction->m_strVariableParameter != "");
	m_eAuthority = eAuthority;

	IrisValue ivValue = IrisInterpreter::CurrentInterpreter()->GetIrisClass("Method")->CreateInstance(nullptr, nullptr);
	((IrisMethodBaseTag*)ivValue.GetIrisObject()->GetNativeObject())->SetMethod(this);
	m_pMethodObject = ivValue.GetIrisObject();
	m_nParameterAmount = pUserFunction->m_lsParameters.size();
}

IrisMethod::IrisMethod(const string & strMethodName, UserFunction* pUserFunction, MethodType eType, MethodAuthority eAuthority)
{
	m_strMethodName = strMethodName;
	m_eMethodType = eType;
	m_uFunction.m_pUserFunction = pUserFunction;
	m_bIsWithVariableParameter = false;
	m_eAuthority = eAuthority;

	IrisValue ivValue = IrisInterpreter::CurrentInterpreter()->GetIrisClass("Method")->CreateInstance(nullptr, nullptr);
	((IrisMethodBaseTag*)ivValue.GetIrisObject()->GetNativeObject())->SetMethod(this);
	m_pMethodObject = ivValue.GetIrisObject();
	m_nParameterAmount = pUserFunction->m_lsParameters.size();
}

bool IrisMethod::_ParameterCheck(IrisValues* pParameters) {
	// 参数检查
	// 如果有可变参数，那么实际参数的个数至少要大于等于形式参数
	if (pParameters) {
		if (m_bIsWithVariableParameter) {
			return pParameters->GetSize() >= (unsigned int) m_nParameterAmount;
		}
		// 如果没有可变参数，那么和实际参数必须要等于形式参数
		else {
			return pParameters->GetSize() == m_nParameterAmount;
		}
	}
	else {
		return m_nParameterAmount == 0;
	}
}

IrisContextEnvironment* IrisMethod::_CreateContextEnvironment(IrisObject* pCaller, IrisValues* pParameters, IrisContextEnvironment* pContextEnvrioment, bool& bIsGetNew) {

	IrisInterpreter* pInterpreter = IrisInterpreter::CurrentInterpreter();

	//IrisContextEnvironment* pNewEnvironment = new IrisContextEnvironment();
	IrisContextEnvironment* pNewEnvironment = pContextEnvrioment;

	bool bInitialize = false;
	bIsGetNew = false;
	if (!pNewEnvironment) {
		pNewEnvironment = new IrisContextEnvironment();
		pNewEnvironment->m_eType = IrisContextEnvironment::EnvironmentType::Runtime;
		pNewEnvironment->m_uType.m_pCurObject = pCaller;

		pNewEnvironment->m_pUpperContextEnvironment = IrisInterpreter::CurrentInterpreter()->GetCurrentContextEnvrionment();

		bInitialize = true;
		bIsGetNew = true;
	}

	//pNewEnvironment->m_eType = IrisContextEnvironment::EnvironmentType::Runtime;
	pNewEnvironment->m_uType.m_pCurObject = pCaller;
	pNewEnvironment->m_pCurMethod = this;

	// With Block and Without Block
	if (m_eMethodType == MethodType::UserMethod) {
		pNewEnvironment->m_pWithBlock = m_uFunction.m_pUserFunction->m_icsWithBlockCodes.m_pWholeCodes ? nullptr : &m_uFunction.m_pUserFunction->m_icsWithBlockCodes;
		pNewEnvironment->m_pWithoutBlock = m_uFunction.m_pUserFunction->m_icsWithoutBlockCodes.m_pWholeCodes ? nullptr : &m_uFunction.m_pUserFunction->m_icsWithoutBlockCodes;
	}

	// 如果该方法是UserFunction则将参数作为局部变量添加进来
	if (pParameters && m_eMethodType == MethodType::UserMethod) {
		auto it = pParameters->GetVector().begin();
		for (auto param : m_uFunction.m_pUserFunction->m_lsParameters) {
			pNewEnvironment->AddLocalVariable(param, *(it++));
		}
		// 如果有可变参数
		if (m_bIsWithVariableParameter) {
			// 新建一个数组
			IrisValues ivsValues;
			ivsValues.GetVector().assign(it, pParameters->GetVector().end());
			IrisValue ivArray = IrisInterpreter::CurrentInterpreter()->GetIrisClass("Array")->CreateInstance(&ivsValues, nullptr);
			pNewEnvironment->AddLocalVariable(m_uFunction.m_pUserFunction->m_strVariableParameter, ivArray);
		}
	}
	// ContextEnvironment入堆

	if (bInitialize) {
		IrisGC::CurrentGC()->AddContextEnvironmentSize();
		auto pEnvRegister = IrisDevUtil::GetCurrentThreadInfo()->m_pEnvrionmentRegister;
		if(pEnvRegister) {
			++pEnvRegister->m_nReferenced;
			IrisGC::CurrentGC()->ContextEnvironmentGC();
			--pEnvRegister->m_nReferenced;
		}
		else {
			IrisGC::CurrentGC()->ContextEnvironmentGC();
		}
		IrisInterpreter::CurrentInterpreter()->PushEnvironment();
		IrisInterpreter::CurrentInterpreter()->SetEnvironment(pNewEnvironment);
		IrisInterpreter::CurrentInterpreter()->AddNewEnvironmentToHeap(pNewEnvironment);
	}
	return pNewEnvironment;
}

// 按照类型的不同分别调用不同的函数（直接调用 or 解释运行）
IrisValue IrisMethod::Call(IrisValue& ivObject, IrisContextEnvironment* pContextEnvironment ,IrisValues* pParameters, unsigned int nLineNumber, int nBelongingFileIndex) {
	IrisValue ivValue = IrisInterpreter::CurrentInterpreter()->Nil();
	IrisValues ivsNormalPrameters;
	IrisValues ivsVariableValues;
	bool bHaveVariableParameters = false;

	// 参数检查错误
	// 特殊情况 new 不检查，丢给__format检查
	//if (!(m_strMethodName == "new")) {
		if (!_ParameterCheck(pParameters)) {
			// **Error**
			IrisFatalErrorHandler::CurrentFatalHandler()->ShowFatalErrorMessage(IrisFatalErrorHandler::FatalErrorType::ParameterNotFitIrregular, nLineNumber, nBelongingFileIndex, "Parameters of method " + m_strMethodName + " assigned is not fit.");
			return IrisInterpreter::CurrentInterpreter()->Nil();
		}
	//}

	// Getter Setter
	if (m_eMethodType == MethodType::GetterMethod) {
		string strValueName;
		strValueName.assign(m_strMethodName.begin() + 6, m_strMethodName.end());
		strValueName = "@" + strValueName;
		bool bResult = false;
		IIrisObject* pObject = ivObject.GetIrisObject();
		const IrisValue& ivResult = pObject->GetInstanceValue(strValueName, bResult);
		// 不存在则定义
		if (!bResult) {
			pObject->AddInstanceValue(strValueName, IrisInterpreter::CurrentInterpreter()->Nil());
			return IrisInterpreter::CurrentInterpreter()->Nil();
		}
		return ivResult;
	}
	else if (m_eMethodType == MethodType::SetterMethod) {
		string strValueName;
		strValueName.assign(m_strMethodName.begin() + 6, m_strMethodName.end());
		strValueName = "@" + strValueName;
		bool bResult = false;
		IIrisObject* pObject = ivObject.GetIrisObject();
		IrisValue& ivResult = (IrisValue&)pObject->GetInstanceValue(strValueName, bResult);
		IrisValue& ivParam = (*pParameters)[0];
		// 不存在则定义
		if (!bResult) {
			pObject->AddInstanceValue(strValueName, ivParam);
			return ivParam;
		}
		ivResult.SetIrisObject(ivParam.GetIrisObject());
		return ivParam;
	}

	bool bIsGetNew = false;
	IrisContextEnvironment* pNewEnvironment = _CreateContextEnvironment(static_cast<IrisObject*>(ivObject.GetIrisObject()), pParameters, pContextEnvironment, bIsGetNew);
	++pNewEnvironment->m_nReferenced;
	//pNewEnvironment->m_pUpperContextEnvironment = pContextEnvironment;
	
	//如果参数为空，直接调用
	if (!pParameters){
		if (m_eMethodType == MethodType::NativeMethod) {
			ivValue = m_uFunction.m_pfNativeFunction(ivObject, nullptr, nullptr, pNewEnvironment);
		}
		else {
			// 将参数加入环境中
			//ivValue = m_uFunction.m_pUserFunction->m_pBlock->Execute(pNewEnvironment).m_ivValue;
			IrisInterpreter* pInterpreter = IrisInterpreter::CurrentInterpreter();
			//IrisAM iaAM = pInterpreter->GetOneAM(iCoderPointer);
			pInterpreter->PushMethodDeepIndex(m_uFunction.m_pUserFunction->m_dwIndex);
			pInterpreter->RunCode(*m_uFunction.m_pUserFunction->m_icsBlockCodes.m_pWholeCodes, m_uFunction.m_pUserFunction->m_icsBlockCodes.m_nStartPointer, m_uFunction.m_pUserFunction->m_icsBlockCodes.m_nEndPointer, m_uFunction.m_pUserFunction->m_icsBlockCodes.m_nBelongingFileIndex);
			ivValue = IrisInterpreter::CurrentInterpreter()->GetCurrentResultRegister();
			pInterpreter->PopMethodTopDeepIndex();
		}
	}
	else {
		// 检查可变参数
		if ((int)pParameters->GetSize() > m_nParameterAmount) {
			bHaveVariableParameters = true;
			ivsVariableValues.GetVector().assign(pParameters->GetVector().begin() + m_nParameterAmount, pParameters->GetVector().end());
		}
		if (m_nParameterAmount > 0) {
			ivsNormalPrameters.GetVector().assign(pParameters->GetVector().begin(), pParameters->GetVector().begin() + m_nParameterAmount);
		}

		if (m_eMethodType == MethodType::NativeMethod) {
			if (bHaveVariableParameters) {
				if (m_nParameterAmount > 0) {
					ivValue = m_uFunction.m_pfNativeFunction(ivObject, &ivsNormalPrameters, &ivsVariableValues, pNewEnvironment);
				}
				else {
					ivValue = m_uFunction.m_pfNativeFunction(ivObject, nullptr, &ivsVariableValues, pNewEnvironment);
				}
			}
			else {
				if (m_nParameterAmount > 0) {
					ivValue = m_uFunction.m_pfNativeFunction(ivObject, &ivsNormalPrameters, nullptr, pNewEnvironment);
				}
				else {
					ivValue = m_uFunction.m_pfNativeFunction(ivObject, nullptr, nullptr, pNewEnvironment);
				}
			}
		}
		else {
			// 将参数加入环境中
			IrisInterpreter* pInterpreter = IrisInterpreter::CurrentInterpreter();
			
			pInterpreter->PushMethodDeepIndex(m_uFunction.m_pUserFunction->m_dwIndex);
			pInterpreter->RunCode(*m_uFunction.m_pUserFunction->m_icsBlockCodes.m_pWholeCodes, m_uFunction.m_pUserFunction->m_icsBlockCodes.m_nStartPointer, m_uFunction.m_pUserFunction->m_icsBlockCodes.m_nEndPointer, m_uFunction.m_pUserFunction->m_icsBlockCodes.m_nBelongingFileIndex);
			ivValue = IrisInterpreter::CurrentInterpreter()->GetCurrentResultRegister();
			pInterpreter->PopMethodTopDeepIndex();
		}
	}
	if (bIsGetNew) {
		IrisInterpreter::CurrentInterpreter()->PopEnvironment();
	}

	IrisInterpreter* pInterpreter = IrisInterpreter::CurrentInterpreter();
	if (pInterpreter->IrregularHappened()) {
		if(pParameters) {
			pInterpreter->PopStack(pParameters->GetVector().size());
		}
	}

	--pNewEnvironment->m_nReferenced;

	if (pNewEnvironment->m_pClosureBlock) {
		delete pNewEnvironment->m_pClosureBlock;
		pNewEnvironment->m_pClosureBlock = nullptr;
	}

	return ivValue;
}

IrisValue IrisMethod::CallMainMethod(IrisValues* pParameters, unsigned int nLineNumber, int nBelongingFileIndex) {
	IrisValue ivValue;

	IrisValues ivsNormalPrameters;
	IrisValues ivsVariableValues;
	bool bHaveVariableParameters = false;

	//if (!strBelongingFileName) {
	//	strBelongingFileName = &IrisDevUtil::GetCurrentThreadInfo()->m_strCurrentFileName;
	//}

	// 参数检查错误
	if (!_ParameterCheck(pParameters)) {
		// **Error**
		IrisFatalErrorHandler::CurrentFatalHandler()->ShowFatalErrorMessage(IrisFatalErrorHandler::FatalErrorType::ParameterNotFitIrregular, nLineNumber, nBelongingFileIndex, "Parameters of method " + m_strMethodName + " assigned is not fit.");
		return IrisInterpreter::CurrentInterpreter()->Nil();
	}
	bool bIsGetNew = false;
	IrisContextEnvironment* pNewEnvironment = _CreateContextEnvironment(nullptr, pParameters, IrisInterpreter::CurrentInterpreter()->GetCurrentContextEnvrionment(), bIsGetNew);	
	++pNewEnvironment->m_nReferenced;
	pNewEnvironment->m_pUpperContextEnvironment = nullptr;

	// 将参数加入环境中
	IrisInterpreter* pInterpreter = IrisInterpreter::CurrentInterpreter();
	
	pInterpreter->PushMethodDeepIndex(m_uFunction.m_pUserFunction->m_dwIndex);
	pInterpreter->RunCode(*m_uFunction.m_pUserFunction->m_icsBlockCodes.m_pWholeCodes, m_uFunction.m_pUserFunction->m_icsBlockCodes.m_nStartPointer, m_uFunction.m_pUserFunction->m_icsBlockCodes.m_nEndPointer, m_uFunction.m_pUserFunction->m_icsBlockCodes.m_nBelongingFileIndex);
	if (bIsGetNew) {
		IrisInterpreter::CurrentInterpreter()->PopEnvironment();
	}
	pInterpreter->PopMethodTopDeepIndex();

	if (pInterpreter->IrregularHappened()) {
		if (pParameters) {
			pInterpreter->PopStack(pParameters->GetVector().size());
		}
	}

	--pNewEnvironment->m_nReferenced;

	if (pNewEnvironment->m_pClosureBlock) {
		delete pNewEnvironment->m_pClosureBlock;
		pNewEnvironment->m_pClosureBlock = nullptr;
	}

	return IrisInterpreter::CurrentInterpreter()->GetCurrentResultRegister();
}

void IrisMethod::ResetObject() {
	IrisValue ivValue = IrisInterpreter::CurrentInterpreter()->GetIrisClass("Method")->CreateInstance(nullptr, nullptr);
	((IrisMethodBaseTag*)ivValue.GetIrisObject()->GetNativeObject())->SetMethod(this);
	m_pMethodObject = ivValue.GetIrisObject();
}

void IrisMethod::SetMethodAuthority(MethodAuthority eAuthority) {
	m_eAuthority = eAuthority;
}

IrisMethod::~IrisMethod() {
	if (m_eMethodType == MethodType::UserMethod){
		delete m_uFunction.m_pUserFunction;
	}
	//delete m_pMethodObject;
}

const string & IrisMethod::GetMethodName() {
	return m_strMethodName;
}

void IrisMethod::SetMethodName(const string & strMethodName) {
	m_strMethodName = strMethodName;
}

IrisMethod::MethodAuthority IrisMethod::GetAuthority() {
	return m_eAuthority;
}
