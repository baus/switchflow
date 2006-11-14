//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_UTIL_POOLED_LIST_HPP
#define SF_UTIL_POOLED_LIST_HPP

#include <list>
#include <functional>

#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/reverse_iterator.hpp>
#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>

namespace switchflow{
namespace util{  

  template<typename T>
  class pooled_list{
  public:

    //
    // The following types are for standard conformance
    typedef T value_type;
    typedef const T* const_pointer;
    typedef const T& const_reference;
    typedef T* pointer;
    typedef T& reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

  
  private:
    //
    // The following types are for convenience of the implementation.
    typedef char* raw_data_type;
    typedef std::list<raw_data_type> raw_data_ptr_list;  
    typedef typename std::list<raw_data_type>::iterator raw_data_ptr_list_it;
    typedef typename std::list<raw_data_type>::const_iterator raw_data_ptr_list_const_it;

    //
    // Functor to convert iterator to underlying datatype to type T.
    class reinterpret_raw_data_ptr
    {
    public:
    
      typedef T& result_type;

      T& operator()(raw_data_type& raw_data) const
        {
          return *reinterpret_cast<T*>(raw_data);
        }

      T& operator()(raw_data_type const & raw_data) const
        {
          return *reinterpret_cast<T*>(raw_data);
        }    
    };

    //
    // Functor which enables Binary_predicates which operate on type T to be applied
    // to the underlying data type.
    template<typename binary_predicate>
    class apply_binary_predicate
    {
    public:
      apply_binary_predicate(binary_predicate pred):pred_(pred){}
      bool operator()(const raw_data_type& l, const raw_data_type& r) const
        {
          return pred_(*reinterpret_cast<T*>(l),
                       *reinterpret_cast<T*>(r));

        }
    
    private:
      binary_predicate pred_;
    };

  public:

    typedef boost::transform_iterator<reinterpret_raw_data_ptr, typename raw_data_ptr_list::iterator, T&, T > iterator;
    typedef boost::transform_iterator<reinterpret_raw_data_ptr, typename raw_data_ptr_list::const_iterator, T const&, T > const_iterator;

    typedef boost::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef boost::reverse_iterator<iterator> reverse_iterator;

    //
    // Public class to create pools to which pooled_lists are attached.
    class pool:boost::noncopyable
    {
    public:
      friend class pooled_list;
    
      pool(std::size_t size):free_list_(size), size_(size)
        {
          raw_data_ptr_list_it cur;
          raw_data_ptr_list_it end = free_list_.end();
        
          for(cur = free_list_.begin(); cur != end; ++cur){
            *cur = new char[sizeof(T)];
          }
        }
    
      ~pool()
        {
          raw_data_ptr_list_it cur = free_list_.begin();
          raw_data_ptr_list_it end = free_list_.end();
          for(; cur != end; ++cur){
            delete [] *cur;
          }
        }
    
      bool empty() const
        {
          return free_list_.empty();
        }
    
      std::size_t elements_remaining() const
        {
          return free_list_.size();
        }

      std::size_t max_size() { return size_; }
    private:
      raw_data_ptr_list free_list_;
      std::size_t size_;
    };

    pooled_list(pool& free_pool ):p_pool_(&free_pool)
      {
      }

    pooled_list(pool& free_pool, size_type n, const value_type& value=value_type()):p_pool_(&free_pool)
      {
        this->assign(n, value);
      }

    pooled_list(const pooled_list<T>& x):p_pool_(x.p_pool_)
      {
        this->insert(this->begin(), x.begin(), x.end());
      }

    template<typename Input_iterator> pooled_list(Input_iterator first, Input_iterator last, pool& free_pool):p_pool_(&free_pool)
      {
        this->insert(this->begin(), first, last);
      }
  
    ~pooled_list()
      {
        clear();
      }

    iterator insert(iterator pos, const T& value)
      {
        if(!this->p_pool_->empty()){
          // use in place new on the raw data.
          new( &(*p_pool_->free_list_.begin())[0] ) T(value);
          active_list_.splice(pos.base(), p_pool_->free_list_, p_pool_->free_list_.begin());
        }
        else{
          throw std::bad_alloc();
        }
        return --pos;
      }

    template<typename Input_iterator> void insert(iterator pos, Input_iterator first, Input_iterator last)
      {
        for(;first != last; ++first){
          this->insert(pos, *first);
        }
      }

    void insert(iterator pos, size_type n, const value_type &x)
      {
        for (; n > 0; --n){
          this->insert(pos, x);
        }
      }

    iterator erase(iterator pos)
      {
        iterator itr = pos;
        ++pos;
        T& item = *itr;
        // explicitly destruct item
        item.~T();
        p_pool_->free_list_.splice( p_pool_->free_list_.begin(), active_list_, itr.base());
        return pos;
      }

    void swap(pooled_list<T> &x)
      {
        active_list_.swap(x.active_list_);
        std::swap(this->p_pool_, x.p_pool_);
      }

    bool empty() const
      {
        return active_list_.empty();
      }
  
  
    void erase(iterator first, iterator last)
      {
        while(first != last){
          first = this->erase(first);
        }
      }
  
    void clear()
      {
        this->erase(this->begin(), this->end());
      }

  
    void assign(size_type n, const value_type& value=value_type())
      {
        this->clear();
        this->insert(this->begin(), n, value);
      }

    template<typename input_iterator>
    void assign(input_iterator first, input_iterator last)
      {
        this->clear();
        this->insert(this->begin(), first, last);
      }
    
    const_reference back() const
      {
        const_iterator tmp = this->end();
        --tmp;
        return *tmp;
      }

    reference back()
      {
        const_iterator tmp = this->end();
        --tmp;
        return *tmp;
      }

    const_reference front() const
      {
        return *begin();
      }

    reference front()
      {
        return *begin();
      }

  
    iterator begin()
      {
        return boost::make_transform_iterator<reinterpret_raw_data_ptr, raw_data_ptr_list_it>(active_list_.begin());
      }
  
    iterator end()
      {
        return boost::make_transform_iterator<reinterpret_raw_data_ptr, raw_data_ptr_list_it>(active_list_.end());
      }
  
    const_iterator begin() const
      {
        return boost::make_transform_iterator<reinterpret_raw_data_ptr, raw_data_ptr_list_const_it>(active_list_.begin());
      }

    const_iterator end() const
      {
        return boost::make_transform_iterator<reinterpret_raw_data_ptr, raw_data_ptr_list_const_it>(active_list_.end());
      }

    reverse_iterator rbegin()
      {
        return boost::make_reverse_iterator(this->end());
      }

    const_reverse_iterator rbegin() const
      {
        return boost::make_reverse_iterator(this->end());
      }

    reverse_iterator rend()
      {
        return boost::make_reverse_iterator(this->begin());
      }

    const_reverse_iterator rend() const
      {
        return boost::make_reverse_iterator(this->begin());
      }
  
    size_type max_size() const
      {
        return this->p_pool_->max_size();
      }


    pooled_list<T>& operator=(const pooled_list& x)
      {
        // This temporarily drains the pool of x.size() elements.
        // The operation might fail here, but this the only way to provide strong exception safety.
        pooled_list<T> temp(x);
    
        this->swap(temp);

        // On destruction of temp, temp elements are released back to the pool.
      }

    void pop_back()
      {
        if(!empty()){
          iterator temp = this->end();
          --temp;
          this->erase(temp);
        }
      }

    void pop_front()
      {
        if(!empty()){
          this->erase(this->begin());
        }
      }

    void push_back(const value_type &x)
      {
        this->insert(this->end(), x);
      }

    void push_front(const value_type &x)
      {
        this->insert(this->begin(), x);
      }

    template <typename unary_predicate>
    void remove_if(unary_predicate pred)
      {
        iterator cur = this->begin();
        iterator end = this->end();
        while (cur != end)
        {
          if (pred(*cur)){
            cur = this->erase(cur);
          }
          else{
            ++cur;
          }
        }
      }

    void remove(const value_type& value)
      {
        this->remove_if(std::bind1st(std::equal_to<T>(), value));
      }


    void resize(size_type new_size,
                value_type x=value_type())
      {
        iterator cur = this->begin();
        iterator end = this->end();
        size_type len = 0;
        for (; cur != end && len < new_size; ++cur, ++len);
        if (len == new_size){
          this->erase(cur, end);
        }
        else{
          this->insert(end, new_size - len, x);
        }
      }

    void reverse()
      {
        active_list_.reverse();
      }

    void sort()
      {
        active_list_.sort(apply_binary_predicate<std::less<T> >(std::less<T>()));
      }
  
    template <typename binary_predicate>
    void sort(binary_predicate func)
      {
        active_list_.sort(apply_binary_predicate<binary_predicate>(func));
      }


    template <typename binary_predicate>
    void merge(pooled_list& x, binary_predicate func)
      {
        BOOST_ASSERT(this->p_pool_ == x.p_pool_);
        this->active_list_.merge(x.active_list_, apply_binary_predicate<binary_predicate>(func));
      }

    void merge(pooled_list& x)
      {
        BOOST_ASSERT(this->p_pool_ == x.p_pool_);
        this->active_list_.merge(x.active_list_, apply_binary_predicate<std::less<T> >(std::less<T>())); 
      }


    void splice(iterator position, pooled_list& x)
      {
        BOOST_ASSERT(this->p_pool_ == x.p_pool_);
        this->active_list_.splice(position.base(), x.active_list_);
      }

    void splice(iterator position, pooled_list& x, iterator i)
      {
        BOOST_ASSERT(this->p_pool_ == x.p_pool_);
        this->active_list_.splice(position.base(), x.active_list, i);
      }

    void splice(iterator position, pooled_list& x, iterator first, iterator last)
      {
        BOOST_ASSERT(this->p_pool_ == x.p_pool_);
        this->active_list_.splice(position.base(), x.active_list, first, last);
      }

    template<class binary_predicate>
    void unique(binary_predicate pred)
      {
        iterator cur = this->begin();
        iterator end = this->end();
        if(cur == end){
          return;
        }

        iterator next = cur;
        ++next;
        while(next != end)
        {
          if(pred(*cur, *next)){
            next = this->erase(next);
          }
          else{
            cur = next;
            ++next;
          }
        }
      }

    void unique( )
      {
        this->unique(std::equal_to<T>());
      }

  private:
  
    raw_data_ptr_list active_list_;
    pool* p_pool_;
  };

  template <typename T>
  bool operator==(pooled_list<T> l, pooled_list<T> r)
  {
    typename pooled_list<T>::const_iterator e1 = l.end();
    typename pooled_list<T>::const_iterator e2 = r.end();
    typename pooled_list<T>::const_iterator c1 = l.begin();
    typename pooled_list<T>::const_iterator c2 = r.begin();
    for(;c1 != e1 && c2 != e2 && *c1 == *c2; ++c2, ++c1);
    return c1 == e1 && c2 == e2;
  }

  template <typename T>
  bool operator!=(pooled_list<T> l, pooled_list<T> r)
  {
    return !(l==r);
  }

  template <typename T>
  bool operator<(pooled_list<T> l, pooled_list<T> r)
  {
    return std::lexicographical_compare(l.begin(), l.end(), r.begin(), r.end()); 
  }

  template <typename T>
  bool operator>(pooled_list<T> l, pooled_list<T> r)
  {
    return r<l; 
  }

  template <typename T>
  bool operator<=(pooled_list<T> l, pooled_list<T> r)
  {
    return !(l>r); 
  }
  template <typename T>
  bool operator>=(pooled_list<T> l, pooled_list<T> r)
  {
    return !(l<r);
  }



}
}

#endif 
