#pragma once

#include <iterator>

namespace awl
{
	//! Base class for all the single links. Objects of a class can be included into multiple lists by deriving from multiple base_single_link classes.
	/*! Link template parameter is the class pNext points to, but not this points to, but static_cast<Link *>(this) is always correct.
		Casting pNext to T * is allowed by C++ standad only if pNext points to a subobject of type T, if pNext points to Link that actually is not a subobject of T,
		the behavior of static_cast<T *>(pNext) is undefined. But casting this to T * makes sense and can be done by an iterator to access the objects it designates.
		With this type of the link we can have a list of objects of different types derived from the same type of the link, but of cause typical lists do not allow this.*/
	template <class Link>
	class base_single_link
	{
	public:

		base_single_link(Link * n) : pNext(n) {}

		bool included() const
		{
			return pNext != nullptr;
		}

	protected:

		base_single_link() : pNext(nullptr) {}

		Link * next() { return pNext; }

		const Link * next() const { return pNext; }

		Link * pNext;

	private:

		template <class T1, class Link1> friend class base_single_iterator;
		template <class T1, class Link1> friend class single_list;
	};

	//! If objects of a class included to only one list, single_link can be used by default.
	class single_link : public base_single_link<single_link>
	{
	private:

		typedef base_single_link<single_link> Base;

	public:

		using Base::Base;

		template <class T1, class Link1> friend class base_single_iterator;
		template <class T1, class Link1> friend class single_list;
	};

	//! The base class for list iterators. All the object in the should be of the same type T derived from Link.
	/*!	To satisfy iterator requirements, such as providing iterator_category member typedef, for example, the basic iterator derives from appropriate specialization
		of std::iterator.*/
	template <class T, class Link>
	class base_single_iterator : public std::iterator<std::forward_iterator_tag, T *>
	{
	public:

		base_single_iterator(Link *p) : pCur(p) {}

		T * operator-> () const { return cur(); }

		T * operator* () const { return cur(); }

	protected:

		//! Results in undefined behavior if the iterator is end().
		T * cur() const { return static_cast<T *>(pCur); }

		void MoveNext() { pCur = pCur->Link::next(); }

	private:

		Link * pCur;
	};

	template <class T, class Link>
	class single_iterator : public base_single_iterator<T, Link>
	{
	public:

		single_iterator(Link *p) : base_single_iterator<T, Link>(p) {}

		single_iterator(const single_iterator & other) : single_iterator(*other) {}

		single_iterator & operator++ ()
		{
			this->MoveNext();

			return *this;
		}

		single_iterator operator++ (int)
		{
			single_iterator tmp = *this;

			this->MoveNext();

			return tmp;
		}

		bool operator == (const single_iterator & r) const
		{
			return this->cur() == r.cur();
		}

		bool operator != (const single_iterator & r)  const
		{
			return !(*this == r);
		}
	};

	template <class T, class Link>
	class const_single_iterator : public base_single_iterator<const T, const Link>
	{
	public:

		const_single_iterator(const Link *p) : base_single_iterator<const T, const Link>(p) {}

		const_single_iterator(const const_single_iterator & other) : const_single_iterator(*other) {}

		//! The only differece between single_iterator and const_single_iterator is that single_iterator can be converted to const_single_iterator but not vice versa.
		const_single_iterator(const single_iterator<T, Link> & other) : const_single_iterator(*other) {}

		const_single_iterator& operator++ ()
		{
			this->MoveNext();

			return *this;
		}

		const_single_iterator operator++ (int)
		{
			const_single_iterator tmp = *this;

			this->MoveNext();

			return tmp;
		}

		bool operator == (const const_single_iterator & r) const
		{
			return this->cur() == r.cur();
		}

		bool operator != (const const_single_iterator & r)  const
		{
			return !(*this == r);
		}
	};

	//! A singly linked list containing elements derived from single_link<T>.
	/*! Implementation of the list is based on the idea of holding some fake "null" element of type single_link<T> that goes before the first and after the last.
		Null element takes only sizeof(T*) bytes, but not sizeof(T). */
	template <class T, class Link = single_link>
	class single_list
	{
	public:

		typedef T * value_type;

		typedef single_iterator<T, Link> iterator;
		typedef const_single_iterator<T, Link> const_iterator;

		single_list() : Null(null()) {}

		single_list(const single_list& other) = delete;

		single_list(single_list&& other) = delete;

		~single_list() {}

		single_list& operator = (const single_list& other) = delete;

		single_list& operator = (single_list&& other) = delete;

		//! Results in undfined behavior if the list is empty.
		T * front() { return static_cast<T *>(Null.next()); }
		const T * front() const { return static_cast<const T *>(Null.next()); }

		//! begin() does not cast Null.next() to T *, so it can return a valid end().
		iterator begin() { return Null.next(); }
		const_iterator begin() const { return Null.next(); }

		iterator end() { return null(); }
		const_iterator end() const { return null(); }

		static void insert(iterator i, T * a) { insert_after(i.prev(), a); }

		void push_front(T * a) { insert_after(null(), a); }

		T * pop_front() { return remove_after(null()); }

		bool empty() const { return front() == null(); }
		bool empty_or_contains_one() const { return front()->Link::pNext == null(); }
		bool contains_one() const { return !empty() && empty_or_contains_one(); }

		void clear() { Null.pNext = null(); }

		//! Returns the count of elements in the list.
		size_t size() const
		{
			size_t count = 0;

			//We do not need to do dereferencing here, so we do not use std::for_each
			//std::for_each(begin(), end(), [&count](const T *) { ++count; });

			for (const_iterator i = begin(); i != end(); ++i)
			{
				++count;
			}

			return count;
		}

	private:

		//! One or both the parameters can be end(), so they are not T*.
		static void insert_after(Link * p, Link * a)
		{
			a->Link::pNext = p->Link::pNext;
			p->Link::pNext = a;
		}

		//! The parameter can be end(), so it is not T*.
		static Link * remove_after(Link * p)
		{
			Link * r = p->Link::pNext;
			p->Link::pNext = r->Link::pNext;
			r->Link::pNext = nullptr;
			return r;
		}

		void attach(Link * first, Link * last)
		{
			Null.pNext = first;

			last->Link::pNext = null();
		}

		//SingList does not know its last element so it should be provided by QuickList

		void push_front(Link * first, Link * last)
		{
			Link * old_first = front();

			Null.pNext = first;

			last->Link::pNext = old_first;
		}

		void push_back(Link * first, Link * last, Link * old_last)
		{
			old_last->Link::pNext = first;

			last->Link::pNext = null();
		}

		Link * null() { return &Null; }
		const Link * null() const { return &Null; }

		Link Null;

		//! quick_list accesses null() function.
		template <class T1, class DLink> friend class quick_list;
		template <class Dlink> friend class basic_quick_link;
		friend class quick_link;
	};
}
