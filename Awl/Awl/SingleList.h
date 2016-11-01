#pragma once

#include <iterator>

namespace awl
{
	template <class T>
	class single_link
	{
	public:

		single_link(T * n) : pNext(n) {}

		T * next() { return pNext; }

		const T * next() const { return pNext; }

	protected:

		T * pNext;

		single_link() : pNext(nullptr) {}

		bool included() const
		{
			return pNext != nullptr;
		}

		//! There should not be template parameter defaults in forward declaration.
		template <class T1, class DLink> friend class single_list;
	};

	//! The base class for list iterators.
	/*!	To satisfy iterator requirements, such as providing iterator_category member typedef, for example, the basic iterator derives from appropriate specialization
	of std::iterator.*/
	template <class T, class Link>
	class base_single_iterator : public std::iterator<std::forward_iterator_tag, T *>
	{
	public:

		base_single_iterator(T *p) : pCur(p) {}

		T * operator-> () const { return cur(); }

		T * operator* () const { return cur(); }

	protected:

		T * cur() const { return pCur; }

		void MoveNext() { pCur = pCur->Link::next(); }

	private:

		T * pCur;
	};

	template <class T, class Link>
	class single_iterator : public base_single_iterator<T, Link>
	{
	public:

		single_iterator(T *p) : base_single_iterator(p) {}

		single_iterator(const single_iterator & other) : single_iterator(*other) {}

		single_iterator & operator++ ()
		{
			MoveNext();

			return *this;
		}

		single_iterator operator++ (int)
		{
			single_iterator tmp = *this;

			MoveNext();

			return tmp;
		}

		bool operator == (const single_iterator & r) const
		{
			return cur() == r.cur();
		}

		bool operator != (const single_iterator & r)  const
		{
			return !(*this == r);
		}
	};

	template <class T, class Link>
	class const_single_iterator : public base_single_iterator<const T, Link>
	{
	public:

		const_single_iterator(const T *p) : base_single_iterator(p) {}

		const_single_iterator(const const_single_iterator & other) : const_single_iterator(*other) {}

		//! The only differece between single_iterator and const_single_iterator is that single_iterator can be converted to const_single_iterator but not vice versa.
		const_single_iterator(const single_iterator<T, Link> & other) : const_single_iterator(*other) {}

		const_single_iterator& operator++ ()
		{
			MoveNext();

			return *this;
		}

		const_single_iterator operator++ (int)
		{
			const_single_iterator tmp = *this;

			MoveNext();

			return tmp;
		}

		bool operator == (const const_single_iterator & r) const
		{
			return cur() == r.cur();
		}

		bool operator != (const const_single_iterator & r)  const
		{
			return !(*this == r);
		}
	};

	//! A singly linked list containing elements derived from single_link<T>.
	/*! Implementation of the list is based on the idea of holding some fake "null" element of type single_link<T> that goes before the first and after the last.
	Null element takes only sizeof(T*) bytes, but not sizeof(T). */
	template < class T, class Link = single_link<T> >
	class single_list
	{
	public:

		typedef T * value_type;

		typedef single_iterator<T, Link> iterator;
		typedef const_single_iterator<T, Link> const_iterator;

		single_list() : Null(null()) {}
		~single_list() {}

		T * front() { return Null.next(); }
		const T * front() const { return Null.next(); }

		iterator begin() { return front(); }
		const_iterator begin() const { return front(); }

		iterator end() { return null(); }
		const_iterator end() const { return null(); }

		static void insert(iterator i, T * a) { insert_after(i.prev(), a); }

		static void insert_after(T * p, T * a)
		{
			a->Link::pNext = p->Link::pNext;
			p->Link::pNext = a;
		}

		static T * remove_after(T * p)
		{
			T * r = p->Link::pNext;
			p->Link::pNext = r->Link::pNext;
			r->Link::pNext = nullptr;
			return r;
		}

		void push_front(T * a) { insert_after(null(), a); }

		T * pop_front() { return remove_after(null()); }

		bool empty() const { return front() == null(); }
		bool empty_or_contains_one() const { return front()->Link::pNext == null(); }
		bool contains_one() const { return !empty() && empty_or_contains_one(); }

		//void erase_after(T * p) { GC::destroy(remove_after(p));}

		//void erase_range(T * pre_first, T * post_last)
		//{
		//    iterator it = pre_first;
		//    while (it++ != post_last)
		//      erase_after(*it);
		//}

		//void erase_all() { erase_range(null(), null());}

		void clear() { Null.pNext = null(); }

	private:

		void attach(T * first, T * last)
		{
			Null.pNext = first;

			last->Link::pNext = null();
		}

		//SingList does not know its last element so it should be provided by QuickList

		void push_front(T * first, T * last)
		{
			T * old_first = front();

			Null.pNext = first;

			last->Link::pNext = old_first;
		}

		void push_back(T * first, T * last, T * old_last)
		{
			old_last->Link::pNext = first;

			last->Link::pNext = null();
		}

		T * null() { return (T *)&Null; }
		const T * null() const { return (T *)&Null; }

		Link Null;

		//! quick_list accesses null() function.
		template <class T1, class DLink> friend class quick_list;
	};
};
