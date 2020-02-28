#pragma once
#include <mutex>

/* ===== Interface ===== */

template <typename T>
class Pool
{
public:
	class Locker;
	class LockerPtr;

public:
	Pool(int capacity);
	Pool(const Pool& cpy) = delete;
	//Pool(Pool& mov) = delete;	// C4521
	~Pool();

public:
	Pool& operator=(const Pool& cpy) = delete;
	//Pool& operator=(Pool& mov) = delete;	// C4521

public:
	LockerPtr rent(int index, bool wait = true);	// 租用指定箱子
	LockerPtr rent_vacancy();						// 自动租用空箱子
	void give_back(LockerPtr& ptr);					// 归还箱子
	int capacity() const;							// 获得容量
	int count() const;								// 占用计数

private:
	int _capacity;
	Locker* _item_p;
};

template <typename T>
class Pool<T>::Locker
{
public:
	Locker() :_item_p(nullptr), _mut() {}
	Locker(const Locker& cpy) = delete;
	~Locker();

public:
	Locker& operator=(const Locker& cpy) = delete;

public:
	T* item_p();			// 获得物品指针
	T* replace(T* new_p);	// 替换物品指针
	void empty_out();		// 清空箱子，丢弃物品

private:
	T* _item_p;
	std::mutex _mut;

private:
	friend class Pool<T>;
	friend class Pool<T>::LockerPtr;
};

template <typename T>
class Pool<T>::LockerPtr
{
public:
	LockerPtr();
	LockerPtr(Pool<T>* pool_p, int pool_index, typename Pool<T>::Locker* locked_locker_p);
	LockerPtr(const LockerPtr& cpy) = delete;
	LockerPtr(LockerPtr&& mov);
	~LockerPtr();

public:
	LockerPtr& operator=(const LockerPtr& cpy) = delete;
	LockerPtr& operator=(LockerPtr&& mov);
	typename Pool<T>::Locker* operator->();		// 间访箱子
	operator bool();							// 判断空非

public:
	Pool<T>* pool_p() const;					// 获得归属的资源池指针
	int index() const;							// 获得归属的资源池序号
	typename Pool<T>::Locker* locker_p() const;	// 获得箱子指针

private:
	typename Pool<T>::Locker* _locker_p;
	Pool<T>* _pool_p;
	int _index;

private:
	friend class Pool<T>;
};

/* ===== Definitions - class Pool<T> ===== */

template<typename T>
Pool<T>::Pool(int capacity) :
	_capacity(capacity),
	_item_p(new Locker[capacity])
{}

template<typename T>
Pool<T>::~Pool()
{
	delete[] _item_p;
}

template<typename T>
typename Pool<T>::LockerPtr Pool<T>::rent(int index, bool wait)
{
	Locker* wanted_p = _item_p + index;
	if (wait)
		wanted_p->_mut.lock();
	else
		if (!wanted_p->_mut.try_lock())
			return LockerPtr(this, index, nullptr);
	return LockerPtr(this, index, wanted_p);
}

template<typename T>
typename Pool<T>::LockerPtr Pool<T>::rent_vacancy()
{
	for (int i = 0; i < _capacity; ++i)
	{
		Locker* wanted_p = _item_p + i;
		if (wanted_p->_mut.try_lock())
		{
			if (!wanted_p->_item_p)
			{
				return LockerPtr(this, i, wanted_p);
			}
			wanted_p->_mut.unlock();
		}
	}
	return LockerPtr(this, -1, nullptr);
}

template<typename T>
void Pool<T>::give_back(LockerPtr& ptr)
{
	ptr._locker_p->_mut.unlock();
	ptr._locker_p = nullptr;
}

template<typename T>
inline int Pool<T>::capacity() const
{
	return _capacity;
}

template<typename T>
int Pool<T>::count() const
{
	int count = 0;
	for (int i = 0; i < _capacity; ++i)
	{
		Locker& r = _item_p[i];
		if (r._mut.try_lock())
		{
			if (r._item_p)
				++count;
			r._mut.unlock();
		}
		else
			++count;
	}
	return count;
}

/* ===== Definitions - class Pool<T>::Locker */

template <typename T>
Pool<T>::Locker::~Locker()
{
	if (_item_p)
		delete _item_p;
}

template <typename T>
inline T* Pool<T>::Locker::item_p()
{
	return _item_p;
}

template <typename T>
T* Pool<T>::Locker::replace(T* new_p)
{
	T* temp = _item_p;
	_item_p = new_p;
	return temp;
}

template <typename T>
void Pool<T>::Locker::empty_out()
{
	if (_item_p)
	{
		delete _item_p;
		_item_p = nullptr;
	}
}

/* ===== Definitions - class Pool<T>::LockerPtr */

template <typename T>
Pool<T>::LockerPtr::LockerPtr() :
	_locker_p(nullptr),
	_pool_p(nullptr),
	_index(-1)
{}

template <typename T>
Pool<T>::LockerPtr::LockerPtr(Pool<T>* pool_p, int pool_index, typename Pool<T>::Locker* locked_locker_p) :
	_locker_p(locked_locker_p),
	_pool_p(pool_p),
	_index(pool_index)
{}

template <typename T>
Pool<T>::LockerPtr::LockerPtr(LockerPtr&& mov) :
	_locker_p(mov._locker_p),
	_pool_p(mov._pool_p),
	_index(mov._index)
{
	mov._locker_p = nullptr;
}

template <typename T>
Pool<T>::LockerPtr::~LockerPtr()
{
	if (_locker_p)
		_pool_p->give_back(*this);
}

template <typename T>
typename Pool<T>::LockerPtr& Pool<T>::LockerPtr::operator=(LockerPtr&& mov)
{
	this->~LockerPtr();
	_locker_p = mov._locker_p, mov._locker_p = nullptr;
	_pool_p = mov._pool_p;
	_index = mov._index;
	return *this;
}

template <typename T>
inline typename Pool<T>::Locker* Pool<T>::LockerPtr::operator->()
{
	return _locker_p;
}

template <typename T>
inline Pool<T>::LockerPtr::operator bool()
{
	return _locker_p;
}

template<typename T>
inline Pool<T>* Pool<T>::LockerPtr::pool_p() const
{
	return _pool_p;
}

template <typename T>
inline int Pool<T>::LockerPtr::index() const
{
	return _index;
}

template <typename T>
inline typename Pool<T>::Locker* Pool<T>::LockerPtr::locker_p() const
{
	return _locker_p;
}
