#pragma once

#include "Awl/SingleList.h"

namespace awl
{
	template <class T, class Link>
	class base_quick_iterator
	{
	public:

		typedef T * value_type;

		base_quick_iterator(T *p) : pCur(p) {}

		T * operator-> () const { return cur(); }

		T * operator* () const { return cur(); }

	protected:

		T * cur() const { return pCur; }

		void MoveNext() { pCur = pCur->Link::next; }

	private:

		T * pCur;
	};

	template <class T, class Link>
	class quick_iterator : public base_quick_iterator<T, Link>
	{
	public:

		quick_iterator(T *p) : base_quick_iterator(p) {}

		quick_iterator(const quick_iterator & other) : quick_iterator(*other) {}

		quick_iterator& operator++ ()
		{
			MoveNext();

			return *this;
		}

		quick_iterator operator++ (int)
		{
			quick_iterator tmp = *this;

			MoveNext();

			return tmp;
		}

		bool operator == (const quick_iterator & r) const
		{
			return cur() == r.cur();
		}

		bool operator != (const quick_iterator & r)  const
		{
			return !(*this == r);
		}
	};

	template <class T, class Link>
	class const_quick_iterator : public base_quick_iterator<const T, Link>
	{
	public:

		const_quick_iterator(const T *p) : base_quick_iterator(p) {}

		const_quick_iterator(const const_quick_iterator & other) : const_quick_iterator(*other) {}

		//! The only differece between quick_iterator and const_quick_iterator is that quick_iterator can be converted to const_quick_iterator but not vice versa.
		const_quick_iterator(const quick_iterator<T, Link> & other) : const_quick_iterator(*other) {}

		const_quick_iterator& operator++ ()
		{
			MoveNext();

			return *this;
		}

		const_quick_iterator operator++ (int)
		{
			const_quick_iterator tmp = *this;

			MoveNext();

			return tmp;
		}

		bool operator == (const const_quick_iterator & r) const
		{
			return cur() == r.cur();
		}

		bool operator != (const const_quick_iterator & r)  const
		{
			return !(*this == r);
		}
	};

	template <class T>
	class TForwardLink : public single_link<T>
	{
	protected:
		TForwardLink() {}
	public:
		TForwardLink(T * n) : single_link<T>(n) {}
	};

	template <class T>
	class TBackwardLink : public single_link<T>
	{
	protected:
		TBackwardLink() {}
	public:
		TBackwardLink(T * n) : single_link<T>(n) {}
	};

	template <class T>
	class quick_link : public TForwardLink<T>, public TBackwardLink<T>
	{
	public:

		typedef TForwardLink<T> ForwardLink;
		typedef TBackwardLink<T> BackwardLink;

	protected:

		typedef single_list<T, ForwardLink> TForwardList;
		typedef single_list<T, BackwardLink> TBackwardList;

		quick_link() {}

		bool included() const
		{
			return ForwardLink::included() && BackwardLink::included();
		}

		void exclude()
		{
			T * prev = this->BackwardLink::next;
			T * next = this->ForwardLink::next;
			TForwardList::remove_after(prev);
			TBackwardList::remove_after(next);
		}

		template <class T1, class DLink> friend class quick_list;
		//friend class quick_list<T, quick_link<T>>;

	public:

		quick_link(T * next, T * prev) : TForwardLink<T>(next), TBackwardLink<T>(prev)
		{
		}
	};

	template < class T, class DLink = quick_link<T>> //previously the default was removed because it already defaulted in above forward declaration
	class quick_list
	{
	private:

		typedef typename DLink::ForwardLink ForwardLink;
		typedef typename DLink::BackwardLink BackwardLink;

		typedef single_list<T, ForwardLink> TForwardList;
		typedef single_list<T, BackwardLink> TBackwardList;

	public:

		typedef T * value_type;

		typedef quick_iterator<T, ForwardLink> iterator;
		typedef const_quick_iterator<T, ForwardLink> const_iterator;

		typedef quick_iterator<T, BackwardLink> reverse_iterator;
		typedef const_quick_iterator<T, BackwardLink> const_reverse_iterator;

		T * front() { return Forward.front(); }
		const T * front() const { return Forward.front(); }

		T * back() { return Backward.front(); }
		const T * back() const { return Backward.front(); }

		iterator begin() { return Forward.front(); }
		const_iterator begin() const { return Forward.front(); }

		iterator end() { return iterator(Forward.null()); }
		const_iterator end() const { return const_iterator(Forward.null()); }

		reverse_iterator rbegin() { return Backward.front(); }
		const_reverse_iterator rbegin() const { return Backward.front(); }

		reverse_iterator rend() { return Backward.null(); }
		const_reverse_iterator rend() const { return Backward.null(); }

		//returns true if the list is empty
		bool empty() const { return Forward.empty(); }
		bool has_one_or_less() const { return Forward.has_one_or_less(); }
		bool has_one() const { return Forward.has_one(); }

		//Add... includes specified element to the list

		static void insert(iterator i, T * a) { insert_before(*i, a); }
		static void insert(reverse_iterator i, T * a) { insert_after(*i, a); }

		static void remove(iterator i) { remove(*i); }
		static void remove(reverse_iterator i) { remove(*i); }

		//static void erase(iterator i) { erase(*i);}
		//static void erase(reverse_iterator i) { erase(*i);}

		static void insert_after(T * p, T * a)
		{
			T * next = p->ForwardLink::next;
			TForwardList::insert_after(p, a);
			TBackwardList::insert_after(next, a);
		}

		static void insert_before(T * p, T * a)
		{
			T * prev = p->BackwardLink::next;
			TForwardList::insert_after(prev, a);
			TBackwardList::insert_after(p, a);
		}

		void push_front(T * a) { insert_after(Forward.null(), a); }
		void push_back(T * a) { insert_before(Forward.null(), a); }

		//excludes specified element from the list

		static T * remove(T * a)
		{
			a->exclude();
			return a;
		}

		T * pop_front() { return remove(Forward.front()); }
		T * pop_back() { return remove(Backward.front()); }

		//void erase(T * a) { GC::destroy(remove(a));}

		void attach(T * first, T * last)
		{
			Forward.attach(first, last);
			Backward.attach(last, first);
		}

		void attach(quick_list & src)
		{
			if (src.empty())
			{
				clear();
			}
			else
			{
				T * first = src.front();
				T * last = src.back();

				attach(first, last);

				src.clear();
			}
		}

		void push_front(quick_list & src)
		{
			if (!src.empty())
			{
				T * first = src.front();
				T * last = src.back();

				T * old_last = front(); //the last element in the backward list

				Forward.push_front(first, last);
				Backward.push_back(last, first, old_last);

				src.clear();
			}
		}

		void push_back(quick_list & src)
		{
			if (!src.empty())
			{
				T * first = src.front();
				T * last = src.back();

				T * old_last = back(); //the last element in the forward list

				Forward.push_back(first, last, old_last);
				Backward.push_front(last, first);

				src.clear();
			}
		}

		void clear()
		{
			Forward.clear();
			Backward.clear();
		}

		size_t size() const
		{
			size_t count = 0;

			for (const_iterator i = begin(); i != end(); ++i)
			{
				++count;
			}

			return count;
		}

	private:

		//forward and backward lists
		TForwardList Forward;
		TBackwardList Backward;
	};
};
