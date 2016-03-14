#ifndef _H_IRISOBJECT_
#define _H_IRISOBJECT_

#include "IrisUnil/IrisValue.h"
#include "IrisUnil/IrisMemoryPool/IrisObjectMemoryPoolInterface.h"
#include "IrisUnil/IrisMemoryPool/IrisMemoryPoolDefines.h"

#include "IrisInterfaces/IIrisObject.h"


#include <string>
#include <unordered_map>
#include <mutex>
#include "IrisThread/IrisWLLock.h"

using namespace std;

class IrisMethod;
class IIrisClass;
class IIrisValues;
class IIrisContextEnvironment;

enum class CallerSide {
	Inside = 0,
	Outside,
};

#ifdef IR_USE_MEM_POOL
class IrisObject : public IIrisObject, public IrisObjectMemoryPoolInterface<IrisObject, POOLID_IrisObject>
#else
class IrisObject : public IIrisObject
#endif
{
private:
	typedef unordered_map<string, IrisMethod*> _MethodHash;
	typedef pair<string, IrisMethod*> _MethodPair;

	typedef unordered_map<string, IrisValue> _VariableHash;
	typedef pair<string, IrisValue> _VariablePair;

private:
	static unsigned int s_nMaxID;

private:
	IIrisClass* m_pClass = nullptr;
	_MethodHash m_mpInstanceMethods;
	_VariableHash m_mpInstanceVariables;
	void* m_pNativeObject = nullptr;

	int m_nObjectID = 0;

	bool m_bIsMaked = false;
	bool m_bLiteralObject = false;
	bool m_bLiteralObjectAssigned = false;

	bool m_bHashed = false;
	size_t m_nHash = 0;

	size_t m_nUsedCount = 0;
	size_t m_nFixCount = 0;

	bool m_bPermanent = false;

	recursive_mutex m_rmFixMutex;

	IrisWLLock m_iwlInstanceValueWLLock;

public:
	IrisObject();

	IrisValue CallInstanceFunction(const string& strFunctionName, IIrisContextEnvironment* pContextEnvironment, IIrisValues* ivsValues, CallerSide eSide, unsigned int nLineNumber = -1, int nBelongingFileIndex= - 1);

	inline bool IsUsed() { return m_nUsedCount > 0; }

	inline void Fix() { lock_guard<recursive_mutex> lgLock(m_rmFixMutex); ++m_nFixCount; }
	inline void Unfix() { lock_guard<recursive_mutex> lgLock(m_rmFixMutex); --m_nFixCount; }
	inline bool IsFixed() { return m_nFixCount > 0; }

	inline void SetPermanent(bool bFlag) { m_bPermanent = bFlag; }
	inline bool IsPermanent() { return m_bPermanent; }

	inline void SetHash(size_t nHash) { m_bHashed = true; m_nHash = nHash; }
	inline size_t GetHash() { return m_nHash; }
	inline bool Hashed() { return m_bHashed; }

	inline int GetObjectID() { return m_nObjectID; }
	inline IIrisClass* GetClass() { return m_pClass; }
	inline void SetClass(IIrisClass* pClass) { m_pClass = pClass; }

	const IrisValue& GetInstanceValue(const string& strInstanceValueName, bool& bResult);

	void AddInstanceValue(const string& strInstanceValueName, const IrisValue& ivValue);

	void Mark();
	inline void ClearMark() { m_bIsMaked = false; }

	inline void* GetNativeObject() { return m_pNativeObject; }
	inline void SetNativeObject(void* pNativeObject) { m_pNativeObject = pNativeObject; }
	
	~IrisObject();

	friend class IrisGC;
};

#endif