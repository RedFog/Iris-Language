#ifndef _H_IRISTHREADUTIL_
#define _H_IRISTHREADUTIL_

#include "IrisUnil/IrisValue.h"
#include "IrisInterpreter/IrisStructure/IrisContextEnvironment.h"
#include "IrisUnil/IrisStack.h"
#include "IrisUnil/IrisHeap.h"
#include "IrisUnil/IrisStack.h"

#include <unordered_map>
#include <vector>
#include <list>
using namespace std;

struct IrisThreadUniqueInfo {
private:
	typedef unordered_map<string, IrisValue> _ValueMap;
	typedef pair<string, IrisValue> _ValuePair;
	
	typedef vector<IrisContextEnvironment*> _EnvironmentStack;
	typedef vector<unsigned int> _LoopDeepStack;
	typedef vector<IrisValue> _ValueStack;
	typedef vector<bool> _BooleanStack;
	typedef vector<IrisObject*> _ObjectStack;
	typedef unordered_set<IrisContextEnvironment*> _EnvironmentHeap;

public:
	
	//string m_strCurrentFileName = "";
	int m_nCurrentFileIndex = -1;

	//IrisHeap m_hpHeap;
	IrisStack m_stStack;

	_EnvironmentHeap m_ehEnvironmentHeap;
	_EnvironmentStack m_skEnvironmentStack;				      // �߳�������ջ
	_LoopDeepStack m_skDeepStack;							  // �߳����ջ
	_LoopDeepStack m_skMethodDeepStack;					      // �̷߳������ջ
	_ValueStack m_skCounterRegister;						  // �߳�Counterջ
	_ValueStack m_skTimerRegister;							  // �߳�Timerջ
	_BooleanStack m_skUnimitedLoopFlag;						  // �߳�UnitedLoopFlagջ
	_ValueStack m_skVessleRegister;							  // �߳�����ջ
	_ValueStack m_skIteratorRegister;						  // �߳�Iteratorջ
	_ObjectStack m_skTempNewObjectStack;  					  // �߳�����������ʱջ

	IrisValue m_ivResultRegister;							  // ����Ĵ���
	IrisValue m_ivCounterRegister;							  // �����Ĵ���
	IrisValue m_ivTimerRegister;							  // �����Ĵ���
	IrisValue m_ivVessleRegister;							  // for�����������Ĵ���
	IrisValue m_ivIteratorRegister;							  // for���������Ĵ���
	IrisValue m_ivCompareRegister;							  // switch���ȽϼĴ���
	list<string> m_lsFieldsRegister;						  // ��Ĵ���
	bool m_bUnimitedLoopFlagRegister = false;				  // ����ѭ����־�Ĵ���
	IrisContextEnvironment* m_pEnvrionmentRegister = nullptr; // �����ļĴ���
	IrisClosureBlock* m_pClosureBlockRegister = nullptr;	  // ��Ĵ���

	IrisValue m_ivIrregularObjectRegister;					  // �쳣״̬����Ĵ���
	bool m_bIrregularHappenedRegister = false;				  // �쳣�����Ĵ���
	bool m_bFatalErrorHappendRegister = false;			      // FatalError�����Ĵ���
};

#endif