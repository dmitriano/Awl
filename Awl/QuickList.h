#pragma once

#include <assert.h>

#include "Awl/SingleList.h"

namespace awl
{
	//template <class Link>
	class TForwardLink : public base_single_link<TForwardLink>
	{
	protected:
		TForwardLink() {}
	public:
		TForwardLink(TForwardLink * n) : base_single_link<TForwardLink>(n) {}
	};

	//template <class Link>
	class TBackwardLink : public base_single_link<TBackwardLink>
	{
	protected:
		TBackwardLink() {}
	public:
		TBackwardLink(TBackwardLink * n) : base_single_link<TBackwardLink>(n) {}
	};

	//! Double link consisting of two single links.
	/*! There is no Null of type quick_link, but there are two separate singly-lined lists, that can have different offset in their enclosing class,
		so we cannot make TForwardLink::pNext and TBackwardLink::pNext point to quick_link. Getting the address of the object by its member is illegal in C++ 17,
		so we should derive quick_link from two single links.
		There is not need to have basic_quick_link, because quick_link does not declare its own members like pNext and all the linking is actually done with the single links.*/
	class quick_link : public TForwardLink, public TBackwardLink
	{
	public:

		bool included() const
		{
			assert(ForwardLink::included() == BackwardLink::included());
			return ForwardLink::included();
		}

		void exclude()
		{
			quick_link * prev = static_cast<quick_link *>(this->BackwardLink::next());
			quick_link * next = static_cast<quick_link *>(this->ForwardLink::next());
			ForwardList::remove_after(prev);
			BackwardList::remove_after(next);
		}

		void safe_exclude()
		{
			if (included())
			{
				exclude();
			}
		}

		~quick_link()
		{
			safe_exclude();
		}

	protected:

		typedef TForwardLink ForwardLink;
		typedef TBackwardLink BackwardLink;

		//! The elements that are not Nulls are of type quick_link, but Nulls are ForwardLink and BackwardLink.
		typedef single_list<quick_link, ForwardLink> ForwardList;
		typedef single_list<quick_link, BackwardLink> BackwardList;

		quick_link() {}

		//! There should not be template parameter defaults in forward declaration.
		template <class T1, class DLink> friend class quick_list;

	public:

		quick_link(quick_link * next, quick_link * prev) : ForwardLink(next), BackwardLink(prev)
		{
		}
	};

	//! Doubly linked list consisting of two singly linked lists.
	template < class T, class DLink = quick_link>
	class quick_list
	{
	private:

		typedef typename DLink::ForwardLink ForwardLink;
		typedef typename DLink::BackwardLink BackwardLink;

		typedef single_list<T, ForwardLink> TForwardList;
		typedef single_list<T, BackwardLink> TBackwardList;

	public:

		typedef T * value_type;

		typedef typename TForwardList::iterator iterator;
		typedef typename TForwardList::const_iterator const_iterator;

		typedef typename TBackwardList::iterator reverse_iterator;
		typedef typename TBackwardList::const_iterator const_reverse_iterator;

		quick_list()
		{
		}

		quick_list(const quick_list& other) = delete;

		quick_list(quick_list&& other)
		{
			attach(other);
		}

		quick_list& operator = (const quick_list& other) = delete;

		quick_list& operator = (quick_list&& other)
		{
			if (this != &other)
			{
				//quick_list cannot free it resources, so it is supposed to be empty
				assert(empty());
				attach(other);
			}
			return *this;
		}

		T * front() { return Forward.front(); }
		const T * front() const { return Forward.front(); }

		T * back() { return Backward.front(); }
		const T * back() const { return Backward.front(); }

		iterator begin() { return Forward.begin(); }
		const_iterator begin() const { return Forward.begin(); }

		iterator end() { return Forward.end(); }
		const_iterator end() const { return Forward.end(); }

		reverse_iterator rbegin() { return Backward.begin(); }
		const_reverse_iterator rbegin() const { return Backward.begin(); }

		reverse_iterator rend() { return Backward.end(); }
		const_reverse_iterator rend() const { return Backward.end(); }

		//returns true if the list is empty
		bool empty() const { return Forward.empty(); }
		bool empty_or_contains_one() const { return Forward.empty_or_contains_one(); }
		bool contains_one() const { return Forward.contains_one(); }

		//Add... includes specified element to the list

		static void insert(iterator i, T * a) { insert_after(*i, a); }
		static void insert(reverse_iterator i, T * a) { insert_before(*i, a); }

		static void erase(iterator i) { remove(*i); }
		static void erase(reverse_iterator i) { remove(*i); }

		static void insert_after(quick_link * p, quick_link * a)
		{
			quick_link * next = static_cast<quick_link *>(p->ForwardLink::next());
			TForwardList::insert_after(p, a);
			TBackwardList::insert_after(next, a);
		}

		static void insert_before(quick_link * p, quick_link * a)
		{
			quick_link * prev = static_cast<quick_link *>(p->BackwardLink::next());
			TForwardList::insert_after(prev, a);
			TBackwardList::insert_after(p, a);
		}

		void push_front(T * a) { insert_after(static_cast<DLink *>(Forward.null()), a); }
		void push_back(T * a) { insert_before(static_cast<DLink *>(Forward.null()), a); }

		T * pop_front() { return remove(Forward.front()); }
		T * pop_back() { return remove(Backward.front()); }

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
			return Forward.size();
		}

	private:

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

		void attach(T * first, T * last)
		{
			Forward.attach(first, last);
			Backward.attach(last, first);
		}

		//! Excludes specified element from the list.
		static T * remove(T * a)
		{
			a->exclude();
			return a;
		}

		//! Forward and backward lists storing ForwardLink* and BackwardLink*, but not a quick_link*.
		TForwardList Forward;
		TBackwardList Backward;
	};
}
