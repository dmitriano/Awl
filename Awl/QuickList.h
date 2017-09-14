#pragma once

#include <assert.h>

#include "Awl/SingleList.h"

namespace awl
{
	class TForwardLink : public base_single_link<TForwardLink>
	{
	protected:
		TForwardLink() {}
	public:
		TForwardLink(TForwardLink * n) : base_single_link<TForwardLink>(n) {}
	};

	class TBackwardLink : public base_single_link<TBackwardLink>
	{
	protected:
		TBackwardLink() {}
	public:
		TBackwardLink(TBackwardLink * n) : base_single_link<TBackwardLink>(n) {}
	};

	//! Double link consisting of two single links.
	class quick_link : public TForwardLink, public TBackwardLink
	{
	public:

		bool included() const
		{
			return ForwardLink::included();// && BackwardLink::included();
		}

		void exclude()
		{
			quick_link * prev = static_cast<quick_link *>(static_cast<BackwardLink *>(this->BackwardLink::next()));
			quick_link * next = static_cast<quick_link *>(static_cast<ForwardLink *>(this->ForwardLink::next()));
			TForwardList::remove_after(prev);
			TBackwardList::remove_after(next);
		}

	protected:

		typedef TForwardLink ForwardLink;
		typedef TBackwardLink BackwardLink;

		typedef single_list<ForwardLink, ForwardLink> TForwardList;
		typedef single_list<BackwardLink, BackwardLink> TBackwardList;

		quick_link() {}

		//! There should not be template parameter defaults in forward declaration.
		template <class T1, class DLink> friend class quick_list;

	public:

		quick_link(TForwardLink * next, TBackwardLink * prev) : TForwardLink(next), TBackwardLink(prev)
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
			quick_link * next = static_cast<quick_link *>(static_cast<ForwardLink *>(p->ForwardLink::next()));
			TForwardList::insert_after(p, a);
			TBackwardList::insert_after(next, a);
		}

		static void insert_before(quick_link * p, quick_link * a)
		{
			quick_link * prev = static_cast<quick_link *>(static_cast<BackwardLink *>(p->BackwardLink::next()));
			TForwardList::insert_after(prev, a);
			TBackwardList::insert_after(p, a);
		}

		void push_front(T * a) { insert_after(static_cast<quick_link *>(Forward.null()), a); }
		void push_back(T * a) { insert_before(static_cast<quick_link *>(Forward.null()), a); }

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

		//forward and backward lists
		TForwardList Forward;
		TBackwardList Backward;
	};
}
