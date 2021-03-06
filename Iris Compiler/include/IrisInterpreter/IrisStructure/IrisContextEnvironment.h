#ifndef _H_IRISCONTENXENVIRIONMENT_
#define _H_IRISCONTENXENVIRIONMENT_
#include "IrisObject.h"
#include "IrisComponents/IrisVirtualCodeStructures.h"
#include "IrisUnil/IrisMemoryPool/IrisObjectMemoryPoolInterface.h"
#include "IrisUnil/IrisMemoryPool/IrisMemoryPoolDefines.h"
#include "IrisUnil/IrisCodeSegment.h"
#include "IrisThread/IrisWLLock.h"
#include "IrisInterfaces/IIrisContextEnvironment.h"

#include <list>
#include <string>
#include <unordered_map>
using namespace std;

class IrisClosureBlock;
class IrisBlock;
class IrisMethod;
class IrisObject;
class IrisClass;
class IrisModule;
class IrisInterface;

class IIrisClosureBlock;

#ifdef IR_USE_MEM_POOL
class IrisContextEnvironment : public IIrisContextEnvironment, public IrisObjectMemoryPoolInterface<IrisContextEnvironment, POOLID_IrisContextEnvironment>
#else
class IrisContextEnvironment : public IIrisContextEnvironment
#endif
{
private:
	typedef unordered_map<string, IrisValue> _VariableMap;
	typedef pair<string, IrisValue> _VariablePair;

public:
	enum class EnvironmentType {
		ClassDefineTime = 0,
		ModuleDefineTime,
		InterfaceDefineTime,
		Runtime,
	};

	union {
		IrisObject* m_pCurObject = nullptr;
		IrisClass* m_pClass;
		IrisModule* m_pModule;
		IrisInterface* m_pInterface;
	} m_uType;

	struct IrregularProcessProgram {
		IrisCodeSegment m_icsOrderCodes;
		IrisCodeSegment m_icsServeCodes;
		IrisCodeSegment m_icsIgnoreCodes;
	};

public:
	_VariableMap m_mpVariables;
	EnvironmentType m_eType = EnvironmentType::Runtime;

	IrisContextEnvironment* m_pUpperContextEnvironment = nullptr;
	bool m_bIsClosure = false;
	bool m_bIsThreadMainContext = false;

	IrisClosureBlock* m_pClosureBlock = nullptr;

	IrisCodeSegment* m_pWithBlock = nullptr;
	IrisCodeSegment* m_pWithoutBlock = nullptr;

	IrisMethod* m_pCurMethod = nullptr;

	int m_nReferenced = 0;
	int m_nThreadReferenced = 0;

	IrisWLLock m_iwlVariableLock;

public:

	inline IIrisClosureBlock* GetClosureBlock();
	inline void SetClosureBlock(IIrisClosureBlock* pBlock);
	inline IIrisContextEnvironment* GetUpperContextEnvrioment();

	const IrisValue& GetVariableValue(const string& strVariableName, bool& bResult);
	void AddLocalVariable(const string& strVariableName, const IrisValue& ivValue);

	IrisContextEnvironment();
	~IrisContextEnvironment();
};

#endif