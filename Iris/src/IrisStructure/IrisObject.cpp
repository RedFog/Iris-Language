#include "IrisObject.h"
#include "IrisClass.h"
#include "IrisInterpreter.h"

unsigned int IrisObject::s_nMaxID = 0;

void IrisObject::Mark() {
	m_bIsMaked = true;
	m_pClass->Mark(m_pNativeObject);
	for (auto value : m_mpInstanceMethods) {
		value.second->m_pMethodObject->Mark();
	}
	for (auto value : m_mpInstanceVariables) {
		value.second.m_ivValue.GetObject()->Mark();
	}
}

IrisObject::IrisObject() {
	m_nObjectID = ++s_nMaxID;
}

const IrisValue& IrisObject::GetInstanceValue(const string& strInstanceValueName, bool& bResult) {
	if (m_mpInstanceVariables.find(strInstanceValueName) == m_mpInstanceVariables.end()){
		bResult = false;
		return IrisInterpreter::CurInstance()->Nil();
	}
	bResult = true;
	return m_mpInstanceVariables[strInstanceValueName].m_ivValue;
}

void IrisObject::AddInstanceValue(const string& strInstanceValueName, const IrisValue& ivValue){
	IrisVariable ivbVariable;
	ivbVariable.m_ivValue = ivValue;
	ivbVariable.m_strName = strInstanceValueName;
	m_mpInstanceVariables.insert(_VariablePair(strInstanceValueName, ivbVariable));
}

IrisValue IrisObject::CallInstanceFunction(const string& strFunctionName, IrisContextEnvironment* pContextEnvironment, IrisValues* ivsValues, CallerSide eSide) {
	// �����Լ���Instance Functions��Ѱ�Ҷ�Ӧ����
	IrisMethod* pMethod = nullptr;
	bool bIsCurClassMethod = false;
	if (!(m_mpInstanceMethods.find(strFunctionName) == m_mpInstanceMethods.end())) 
		pMethod = m_mpInstanceMethods[strFunctionName];
	else {
		pMethod = m_pClass->GetMethod(IrisClass::SearchMethodType::InstanceMethod, strFunctionName, bIsCurClassMethod);
	}

	if (pMethod) {
		IrisValue ivValue;
		ivValue.m_pObject = this;

		// �ڲ�����
		if (eSide == CallerSide::Inside) {
			// �����ڲ�����������
			if (bIsCurClassMethod) {
				return pMethod->Call(ivValue, pContextEnvironment, ivsValues);
			}
			// ������ø��෽��
			else {
				// ��ֹ����
				if (pMethod->m_eAuthority == IrisMethod::MethodAuthority::Personal) {
					// **Error**
					IrisInterpreter::CurInstance()->GroanFatalIrregular
						(IrisInterpreter::FatalIrregular::MethodAuthorityIrregular, IrisInterpreter::CurInstance()->GetExcutedLineNumber(), "Method of " + strFunctionName + " is Personal and cannot be called from derrived class " + m_pClass->GetClassName() + " .");
					return IrisInterpreter::CurInstance()->Nil();
				}
				else {
					return pMethod->Call(ivValue, pContextEnvironment, ivsValues);
				}
			}
		}
		// �ⲿ����
		else {
			// ֻ�ܵ���EveryOne�ķ���
			// ��ֹ����
			if (pMethod->m_eAuthority != IrisMethod::MethodAuthority::Everyone) {
				// **Error**
				IrisInterpreter::CurInstance()->GroanFatalIrregular
					(IrisInterpreter::FatalIrregular::MethodAuthorityIrregular, IrisInterpreter::CurInstance()->GetExcutedLineNumber(), "Method of " + strFunctionName + " is not Everyone and cannot be called outside the class " + m_pClass->GetClassName() + " .");
				return IrisInterpreter::CurInstance()->Nil();
			}
			else {
				return pMethod->Call(ivValue, pContextEnvironment, ivsValues);
			}
		}

	}
	
	// **Error**
	IrisInterpreter::CurInstance()->GroanFatalIrregular
		(IrisInterpreter::FatalIrregular::NoMethodIrregular, IrisInterpreter::CurInstance()->GetExcutedLineNumber(), "Method of " + strFunctionName + " not found in class " + m_pClass->GetClassName() + " .");
	return IrisInterpreter::CurInstance()->Nil();
}

IrisObject::~IrisObject() {
	this->m_pClass->NativeFree(m_pNativeObject);

	for (auto iter : m_mpInstanceMethods) {
		delete iter.second;
	}
}