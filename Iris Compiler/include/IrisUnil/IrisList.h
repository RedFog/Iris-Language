#ifndef _H_IRISLIST_
#define _H_IRISLIST_
#include <functional>
#include <list>
using namespace std;
template<class T>
class IrisList {
private:
	list<T> m_lsList;
	int m_nSize = 0;

public:
	void Add(T tData) { m_lsList.push_back(tData); ++m_nSize; }
	void Clear() { m_lsList.clear(); }
	bool Ergodic(const function<bool(T&)>& fErgodicFunction) {
		for (auto& var : m_lsList) {
			if (!fErgodicFunction(var)) {
				return false;
			}
		}
		return true;
	}
	int GetSize() { return m_nSize; }
	IrisList() {}
	~IrisList() {}

	friend class IrisBlock;
	friend class IrisSwitchStatement;
	friend class IrisStatement;
	friend class IrisClosureBlock;
	friend class IrisFieldExpression;
};

#endif