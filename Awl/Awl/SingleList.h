#pragma once

namespace awl 
{

template <class T>
class single_link 
{
public :
  
  T * next;
  single_link(T * n) : next(n) {}

protected:
  
  single_link() : next(nullptr) {}

  bool included() const
  {
	  return next != nullptr;
  }
};

template <class T, class Link>
class single_iterator 
{
public:
  
    typedef T * value_type;

    single_iterator(T *p) : pPrev(p) {}

    single_iterator(const single_iterator & r) : pPrev(r.pPrev)
    {
    }

    T * operator-> () const { return cur();}

    T * operator* () const { return cur();}

    single_iterator& operator++ ()
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

    bool operator == (const single_iterator & r)
    {
      return pPrev == r.pPrev;
    }

    bool operator != (const single_iterator & r)
    {
      return !(*this == r);
    }

    T * prev() const { return pPrev;}

    T * cur() const { return pPrev->Link::next;}

private:
  
    void MoveNext() { pPrev = pPrev->Link::next;}

    T * pPrev;
};

template < class T, class Link = single_link<T> >
class single_list 
{
public :
  
    typedef T * value_type;

    typedef single_iterator<T, Link> iterator;
    typedef single_iterator<const T, Link> const_iterator;

    single_list() : m_null(null()) {}
    ~single_list() {}

    T * null() { return (T *)&m_null;}
    const T * null() const { return (T *)&m_null;}

    T * front() { return m_null.next;}
    const T * front() const { return m_null.next;}

    iterator begin() { return null();}
    const_iterator begin() const { return null();}

    //TSingleList can't access the last element
    //i != list.end() should be replaced with *i != list.null()
    
    //iterator end() { return iterator(back());}
    //const_iterator end() const { return const_iterator(back());}

    static void insert(iterator i, T * a) { insert_after(i.prev(), a);}

    static void insert_after(T * p, T * a)
    {
        a->Link::next = p->Link::next;
        p->Link::next = a;
    }

    static T * remove_after(T * p)
    {
        T * r = p->Link::next;
        p->Link::next = r->Link::next;
        r->Link::next = nullptr;
        return r;
    }

    void push_front(T * a) { insert_after(null(), a);}

    T * pop_front() { return remove_after(null());}

    bool empty() const { return front() == null();}
    bool has_one_or_less() const { return front()->Link::next == null();}
    bool has_one() const { return !empty() && has_one_or_less();}

    //void erase_after(T * p) { GC::destroy(remove_after(p));}

    //void erase_range(T * pre_first, T * post_last)
    //{
    //    iterator it = pre_first;
    //    while (it++ != post_last)
    //      erase_after(*it);
    //}

    //void erase_all() { erase_range(null(), null());}

    void clear() { m_null.next = null(); }

	void attach(T * first, T * last)
	{
		m_null.next = first;

		last->Link::next = null();
	}

	//SingList does not know its last element so it should be provided by QuickList

	void push_front(T * first, T * last)
	{
		T * old_first = front();
		
		m_null.next = first;

		last->Link::next = old_first;
	}

	void push_back(T * first, T * last, T * old_last)
	{
		old_last->Link::next = first;
		
		last->Link::next = null();
	}

protected:
  
    Link  m_null;
};

}; //namespace awl
