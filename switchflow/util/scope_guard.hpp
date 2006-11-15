//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SCOPEGUARD_H_
#define SCOPEGUARD_H_

template <class T>
class ref_holder
{
  T& ref_;
public:
  ref_holder(T& ref) : ref_(ref) {}
  operator T& () const 
  {
    return ref_;
  }
private:
    // Disable assignment - not implemented
    ref_holder& operator=(const ref_holder&);
};

template <class T>
inline ref_holder<T> by_ref(T& t)
{
  return ref_holder<T>(t);
}

class scope_guard_impl_base
{
  scope_guard_impl_base& operator =(const scope_guard_impl_base&);
protected:
  ~scope_guard_impl_base()
  {
  }
  scope_guard_impl_base(const scope_guard_impl_base& other) throw() 
    : dismissed_(other.dismissed_)
  {
    other.dismiss();
  }
  template <typename J>
  static void safe_execute(J& j) throw() 
  {
    if (!j.dismissed_)
      try
      {
        j.execute();
      }
      catch(...)
      {
      }
  }
  
  mutable bool dismissed_;
public:
  scope_guard_impl_base() throw() : dismissed_(false) 
  {
  }
  void dismiss() const throw() 
  {
    dismissed_ = true;
  }
};

typedef const scope_guard_impl_base& scope_guard;

template <typename F>
class scope_guard_impl0 : public scope_guard_impl_base
{
public:
  static scope_guard_impl0<F> make_guard(F fun)
  {
    return scope_guard_impl0<F>(fun);
  }
  ~scope_guard_impl0() throw() 
  {
    safe_execute(*this);
  }
  void execute() 
  {
    fun_();
  }
protected:
  scope_guard_impl0(F fun) : fun_(fun) 
  {
  }
  F fun_;
};

template <typename F> 
inline scope_guard_impl0<F> make_guard(F fun)
{
  return scope_guard_impl0<F>::make_guard(fun);
}

template <typename F, typename P1>
class scope_guard_impl1 : public scope_guard_impl_base
{
public:
  static scope_guard_impl1<F, P1> make_guard(F fun, P1 p1)
  {
    return scope_guard_impl1<F, P1>(fun, p1);
  }
  ~scope_guard_impl1() throw() 
  {
    safe_execute(*this);
  }
  void execute()
  {
    fun_(p1_);
  }
protected:
  scope_guard_impl1(F fun, P1 p1) : fun_(fun), p1_(p1) 
  {
  }
  F fun_;
  const P1 p1_;
};

template <typename F, typename P1> 
inline scope_guard_impl1<F, P1> make_guard(F fun, P1 p1)
{
  return scope_guard_impl1<F, P1>::make_guard(fun, p1);
}

template <typename F, typename P1, typename P2>
class scope_guard_impl2: public scope_guard_impl_base
{
public:
  static scope_guard_impl2<F, P1, P2> make_guard(F fun, P1 p1, P2 p2)
  {
    return scope_guard_impl2<F, P1, P2>(fun, p1, p2);
  }
  ~scope_guard_impl2() throw() 
  {
    safe_execute(*this);
  }
  void execute()
  {
    fun_(p1_, p2_);
  }
protected:
  scope_guard_impl2(F fun, P1 p1, P2 p2) : fun_(fun), p1_(p1), p2_(p2) 
  {
  }
  F fun_;
  const P1 p1_;
  const P2 p2_;
};

template <typename F, typename P1, typename P2>
inline scope_guard_impl2<F, P1, P2> make_guard(F fun, P1 p1, P2 p2)
{
  return scope_guard_impl2<F, P1, P2>::make_guard(fun, p1, p2);
}

template <typename F, typename P1, typename P2, typename P3>
class scope_guard_impl3 : public scope_guard_impl_base
{
public:
  static scope_guard_impl3<F, P1, P2, P3> make_guard(F fun, P1 p1, P2 p2, P3 p3)
  {
    return scope_guard_impl3<F, P1, P2, P3>(fun, p1, p2, p3);
  }
  ~scope_guard_impl3() throw() 
  {
    safe_execute(*this);
  }
  void execute()
  {
    fun_(p1_, p2_, p3_);
  }
protected:
  scope_guard_impl3(F fun, P1 p1, P2 p2, P3 p3) : fun_(fun), p1_(p1), p2_(p2), p3_(p3) 
  {
  }
  F fun_;
  const P1 p1_;
  const P2 p2_;
  const P3 p3_;
};

template <typename F, typename P1, typename P2, typename P3>
inline scope_guard_impl3<F, P1, P2, P3> make_guard(F fun, P1 p1, P2 p2, P3 p3)
{
  return scope_guard_impl3<F, P1, P2, P3>::make_guard(fun, p1, p2, p3);
}

//************************************************************

template <class Obj, typename Mem_fun>
class obj_scope_guard_impl0 : public scope_guard_impl_base
{
public:
  static obj_scope_guard_impl0<Obj, Mem_fun> make_obj_guard(Obj& obj, Mem_fun mem_fun)
  {
    return obj_scope_guard_impl0<Obj, Mem_fun>(obj, mem_fun);
  }
  ~obj_scope_guard_impl0() throw() 
  {
    safe_execute(*this);
  }
  void execute() 
  {
    (obj_.*mem_fun_)();
  }
protected:
  obj_scope_guard_impl0(Obj& obj, Mem_fun mem_fun) 
    : obj_(obj), mem_fun_(mem_fun) {}
  Obj& obj_;
  Mem_fun mem_fun_;
};

template <class Obj, typename Mem_fun>
inline obj_scope_guard_impl0<Obj, Mem_fun> make_obj_guard(Obj& obj, Mem_fun mem_fun)
{
  return obj_scope_guard_impl0<Obj, Mem_fun>::make_obj_guard(obj, mem_fun);
}

template <class Obj, typename Mem_fun, typename P1>
class obj_scope_guard_impl1 : public scope_guard_impl_base
{
public:
  static obj_scope_guard_impl1<Obj, Mem_fun, P1> make_obj_guard(Obj& obj, Mem_fun mem_fun, P1 p1)
  {
    return obj_scope_guard_impl1<Obj, Mem_fun, P1>(obj, mem_fun, p1);
  }
  ~obj_scope_guard_impl1() throw() 
  {
    safe_execute(*this);
  }
  void execute() 
  {
    (obj_.*mem_fun_)(p1_);
  }
protected:
  obj_scope_guard_impl1(Obj& obj, Mem_fun mem_fun, P1 p1) 
    : obj_(obj), mem_fun_(mem_fun), p1_(p1) {}
  Obj& obj_;
  Mem_fun mem_fun_;
  const P1 p1_;
};

template <class Obj, typename Mem_fun, typename P1>
inline obj_scope_guard_impl1<Obj, Mem_fun, P1> make_obj_guard(Obj& obj, Mem_fun mem_fun, P1 p1)
{
  return obj_scope_guard_impl1<Obj, Mem_fun, P1>::make_obj_guard(obj, mem_fun, p1);
}

template <class Obj, typename Mem_fun, typename P1, typename P2>
class obj_scope_guard_impl2 : public scope_guard_impl_base
{
public:
  static obj_scope_guard_impl2<Obj, Mem_fun, P1, P2> make_obj_guard(Obj& obj, Mem_fun mem_fun, P1 p1, P2 p2)
  {
    return obj_scope_guard_impl2<Obj, Mem_fun, P1, P2>(obj, mem_fun, p1, p2);
  }
  ~obj_scope_guard_impl2() throw() 
  {
    safe_execute(*this);
  }
  void execute() 
  {
    (obj_.*mem_fun_)(p1_, p2_);
  }
protected:
  obj_scope_guard_impl2(Obj& obj, Mem_fun mem_fun, P1 p1, P2 p2) 
    : obj_(obj), mem_fun_(mem_fun), p1_(p1), p2_(p2) {}
  Obj& obj_;
  Mem_fun mem_fun_;
  const P1 p1_;
  const P2 p2_;
};

template <class Obj, typename Mem_fun, typename P1, typename P2>
inline obj_scope_guard_impl2<Obj, Mem_fun, P1, P2> make_obj_guard(Obj& obj, Mem_fun mem_fun, P1 p1, P2 p2)
{
  return obj_scope_guard_impl2<Obj, Mem_fun, P1, P2>::make_obj_guard(obj, mem_fun, p1, p2);
}

#define CONCATENATE_DIRECT(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_DIRECT(s1, s2)
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __LINE__)  

#ifdef __GNUC__
#define ON_BLOCK_EXIT scope_guard ANONYMOUS_VARIABLE(scope_guard) __attribute__ ((unused)) = make_guard
#define ON_BLOCK_EXIT_OBJ scope_guard ANONYMOUS_VARIABLE(scope_guard) __attribute__ ((unused)) = make_obj_guard
#else
#define ON_BLOCK_EXIT scope_guard ANONYMOUS_VARIABLE(scope_guard) = make_guard
#define ON_BLOCK_EXIT_OBJ scope_guard ANONYMOUS_VARIABLE(scope_guard) = make_obj_guard
#endif

#endif //SCOPEGUARD_H_
