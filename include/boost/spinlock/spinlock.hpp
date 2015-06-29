/* spinlock.hpp
Provides yet another spinlock
(C) 2013-2014 Niall Douglas http://www.nedprod.com/
File Created: Sept 2013


Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef BOOST_SPINLOCK_HPP
#define BOOST_SPINLOCK_HPP

#include <assert.h>
#include <vector>
#include <memory>
#include <array>

#ifdef BOOST_SPINLOCK_ENABLE_VALGRIND
#include "valgrind/drd.h"
#define BOOST_SPINLOCK_ANNOTATE_RWLOCK_CREATE(p) ANNOTATE_RWLOCK_CREATE(p)
#define BOOST_SPINLOCK_ANNOTATE_RWLOCK_DESTROY(p) ANNOTATE_RWLOCK_DESTROY(p)
#define BOOST_SPINLOCK_ANNOTATE_RWLOCK_ACQUIRED(p, s) ANNOTATE_RWLOCK_ACQUIRED(p, s)
#define BOOST_SPINLOCK_ANNOTATE_RWLOCK_RELEASED(p, s) ANNOTATE_RWLOCK_RELEASED(p, s)
#define BOOST_SPINLOCK_ANNOTATE_IGNORE_READS_BEGIN() ANNOTATE_IGNORE_READS_BEGIN()
#define BOOST_SPINLOCK_ANNOTATE_IGNORE_READS_END() ANNOTATE_IGNORE_READS_END()
#define BOOST_SPINLOCK_ANNOTATE_IGNORE_WRITES_BEGIN() ANNOTATE_IGNORE_WRITES_BEGIN()
#define BOOST_SPINLOCK_ANNOTATE_IGNORE_WRITES_END() ANNOTATE_IGNORE_WRITES_END()
#define BOOST_SPINLOCK_DRD_IGNORE_VAR(x) DRD_IGNORE_VAR(x)
#define BOOST_SPINLOCK_DRD_STOP_IGNORING_VAR(x) DRD_STOP_IGNORING_VAR(x)
#define BOOST_SPINLOCK_RUNNING_ON_VALGRIND RUNNING_ON_VALGRIND
#else
#define BOOST_SPINLOCK_ANNOTATE_RWLOCK_CREATE(p)
#define BOOST_SPINLOCK_ANNOTATE_RWLOCK_DESTROY(p)
#define BOOST_SPINLOCK_ANNOTATE_RWLOCK_ACQUIRED(p, s)
#define BOOST_SPINLOCK_ANNOTATE_RWLOCK_RELEASED(p, s)
#define BOOST_SPINLOCK_ANNOTATE_IGNORE_READS_BEGIN()
#define BOOST_SPINLOCK_ANNOTATE_IGNORE_READS_END()
#define BOOST_SPINLOCK_ANNOTATE_IGNORE_WRITES_BEGIN()
#define BOOST_SPINLOCK_ANNOTATE_IGNORE_WRITES_END()
#define BOOST_SPINLOCK_DRD_IGNORE_VAR(x)
#define BOOST_SPINLOCK_DRD_STOP_IGNORING_VAR(x)
#define BOOST_SPINLOCK_RUNNING_ON_VALGRIND (0)
#endif

/*! \file spinlock.hpp
\brief Provides boost.spinlock
*/

/*! \mainpage
This is the proposed Boost.Spinlock library, a Boost C++ 11 library providing interesting spinlock related things.
*/

#if SPINLOCK_STANDALONE 
#include "bindlib/include/boost/config.hpp"
#else
#include "boost/config.hpp"
#endif

#include "bindlib/include/import.h"

#if ! defined BOOST_SPINLOCK_CONSTEXPR
# ifdef __cpp_constexpr
// clang 3.2 and earlier has buggy constexpr support
#  if !defined(__clang__) || (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__) >= 30300
#   define BOOST_SPINLOCK_CONSTEXPR constexpr
#  endif
# endif
#endif
#ifndef BOOST_SPINLOCK_CONSTEXPR
# define BOOST_SPINLOCK_CONSTEXPR
#endif

#if ! defined BOOST_SPINLOCK_RELAXED_CONSTEXPR
# ifdef __cpp_relaxed_constexpr
#  define BOOST_SPINLOCK_RELAXED_CONSTEXPR constexpr
# endif
#endif
#ifndef BOOST_SPINLOCK_RELAXED_CONSTEXPR
# define BOOST_SPINLOCK_RELAXED_CONSTEXPR
#endif

#if !defined BOOST_SPINLOCK_NOINLINE
# define BOOST_SPINLOCK_NOINLINE BOOST_NOINLINE
#endif

#if !defined BOOST_SPINLOCK_FORCEINLINE
# define BOOST_SPINLOCK_FORCEINLINE BOOST_FORCEINLINE
#endif

#ifndef BOOST_SPINLOCK_IN_THREAD_SANITIZER
# if defined(__has_feature)
#  if __has_feature(thread_sanitizer)
#   define BOOST_SPINLOCK_IN_THREAD_SANITIZER 1
#  endif
# elif defined(__SANITIZE_ADDRESS__)
#  define BOOST_SPINLOCK_IN_THREAD_SANITIZER 1
# endif
#endif
#ifndef BOOST_SPINLOCK_IN_THREAD_SANITIZER
# define BOOST_SPINLOCK_IN_THREAD_SANITIZER 0
#endif

#ifndef BOOST_SPINLOCK_V1_STL11_IMPL
#define BOOST_SPINLOCK_V1_STL11_IMPL std
#endif
#define BOOST_SPINLOCK_V1 (boost), (spinlock), (BOOST_BINDLIB_NAMESPACE_VERSION(v1, BOOST_SPINLOCK_V1_STL11_IMPL), inline)
#define BOOST_SPINLOCK_V1_NAMESPACE       BOOST_BINDLIB_NAMESPACE      (BOOST_SPINLOCK_V1)
#define BOOST_SPINLOCK_V1_NAMESPACE_BEGIN BOOST_BINDLIB_NAMESPACE_BEGIN(BOOST_SPINLOCK_V1)
#define BOOST_SPINLOCK_V1_NAMESPACE_END   BOOST_BINDLIB_NAMESPACE_END  (BOOST_SPINLOCK_V1)

#define BOOST_STL11_ATOMIC_MAP_NAMESPACE_BEGIN        BOOST_BINDLIB_NAMESPACE_BEGIN(BOOST_SPINLOCK_V1, (stl11, inline))
#define BOOST_STL11_ATOMIC_MAP_NAMESPACE_END          BOOST_BINDLIB_NAMESPACE_END  (BOOST_SPINLOCK_V1, (stl11, inline))
#define BOOST_STL11_ATOMIC_MAP_NO_ATOMIC_CHAR32_T // missing VS14
#define BOOST_STL11_ATOMIC_MAP_NO_ATOMIC_CHAR16_T // missing VS14
#define BOOST_STL11_CHRONO_MAP_NAMESPACE_BEGIN        BOOST_BINDLIB_NAMESPACE_BEGIN(BOOST_SPINLOCK_V1, (stl11, inline), (chrono))
#define BOOST_STL11_CHRONO_MAP_NAMESPACE_END          BOOST_BINDLIB_NAMESPACE_END  (BOOST_SPINLOCK_V1, (stl11, inline), (chrono))
#define BOOST_STL11_MUTEX_MAP_NAMESPACE_BEGIN         BOOST_BINDLIB_NAMESPACE_BEGIN(BOOST_SPINLOCK_V1, (stl11, inline))
#define BOOST_STL11_MUTEX_MAP_NAMESPACE_END           BOOST_BINDLIB_NAMESPACE_END  (BOOST_SPINLOCK_V1, (stl11, inline))
#define BOOST_STL11_THREAD_MAP_NAMESPACE_BEGIN        BOOST_BINDLIB_NAMESPACE_BEGIN(BOOST_SPINLOCK_V1, (stl11, inline))
#define BOOST_STL11_THREAD_MAP_NAMESPACE_END          BOOST_BINDLIB_NAMESPACE_END  (BOOST_SPINLOCK_V1, (stl11, inline))
#include BOOST_BINDLIB_INCLUDE_STL11(bindlib, BOOST_SPINLOCK_V1_STL11_IMPL, atomic)
#include BOOST_BINDLIB_INCLUDE_STL11(bindlib, BOOST_SPINLOCK_V1_STL11_IMPL, chrono)
#include BOOST_BINDLIB_INCLUDE_STL11(bindlib, BOOST_SPINLOCK_V1_STL11_IMPL, mutex)
#include BOOST_BINDLIB_INCLUDE_STL11(bindlib, BOOST_SPINLOCK_V1_STL11_IMPL, thread)

// For dump
#include <ostream>


// Turn this on if you have a compiler which understands __transaction_relaxed
//#define BOOST_HAVE_TRANSACTIONAL_MEMORY_COMPILER

BOOST_SPINLOCK_V1_NAMESPACE_BEGIN

    BOOST_BINDLIB_DECLARE(BOOST_SPINLOCK_V1, "TODO FIXME") // TODO FIXME

    /*! \struct lockable_ptr
     * \brief Lets you use a pointer to memory as a spinlock :)
     */
    template<typename T> struct lockable_ptr : atomic<T *>
    {
      BOOST_SPINLOCK_CONSTEXPR lockable_ptr(T *v=nullptr) : atomic<T *>(v) { }
      //! Returns the memory pointer part of the atomic
      T *get() BOOST_NOEXCEPT_OR_NOTHROW
      {
        union
        {
          T *v;
          size_t n;
        } value;
        value.v=atomic<T *>::load(memory_order_relaxed);
        value.n&=~(size_t)1;
        return value.v;
      }
      //! Returns the memory pointer part of the atomic
      const T *get() const BOOST_NOEXCEPT_OR_NOTHROW
      {
        union
        {
          T *v;
          size_t n;
        } value;
        value.v=atomic<T *>::load(memory_order_relaxed);
        value.n&=~(size_t)1;
        return value.v;
      }
      T &operator*() BOOST_NOEXCEPT_OR_NOTHROW { return *get(); }
      const T &operator*() const BOOST_NOEXCEPT_OR_NOTHROW { return *get(); }
      T *operator->() BOOST_NOEXCEPT_OR_NOTHROW { return get(); }
      const T *operator->() const BOOST_NOEXCEPT_OR_NOTHROW { return get(); }
    };
    template<typename T> struct spinlockbase
    {
    protected:
      atomic<T> v;
    public:
      typedef T value_type;
      BOOST_SPINLOCK_RELAXED_CONSTEXPR spinlockbase() BOOST_NOEXCEPT_OR_NOTHROW : v(0)
      {
        BOOST_SPINLOCK_ANNOTATE_RWLOCK_CREATE(this);
        v.store(0, memory_order_release);
      }
      spinlockbase(const spinlockbase &) = delete;
      //! Atomically move constructs
      BOOST_SPINLOCK_RELAXED_CONSTEXPR spinlockbase(spinlockbase &&) BOOST_NOEXCEPT_OR_NOTHROW : v(0)
      {
        BOOST_SPINLOCK_ANNOTATE_RWLOCK_CREATE(this);
        //v.store(o.v.exchange(0, memory_order_acq_rel));
        v.store(0, memory_order_release);
      }
      ~spinlockbase()
      {
#ifdef BOOST_SPINLOCK_ENABLE_VALGRIND
        if(v.load(memory_order_acquire))
        {
          BOOST_SPINLOCK_ANNOTATE_RWLOCK_RELEASED(this, true);
        }
#endif
        BOOST_SPINLOCK_ANNOTATE_RWLOCK_DESTROY(this);
      }
      spinlockbase &operator=(const spinlockbase &) = delete;
      spinlockbase &operator=(spinlockbase &&) = delete;
      //! Returns the raw atomic
      BOOST_SPINLOCK_CONSTEXPR T load(memory_order o=memory_order_seq_cst) const BOOST_NOEXCEPT_OR_NOTHROW { return v.load(o); }
      //! Sets the raw atomic
      void store(T a, memory_order o=memory_order_seq_cst) BOOST_NOEXCEPT_OR_NOTHROW { v.store(a, o); }
      //! If atomic is zero, sets to 1 and returns true, else false.
      bool try_lock() BOOST_NOEXCEPT_OR_NOTHROW
      {
#if ! BOOST_SPINLOCK_IN_THREAD_SANITIZER  // no early outs for the sanitizer
#ifdef BOOST_SPINLOCK_USE_VOLATILE_READ_FOR_AVOIDING_CMPXCHG
        // MSVC's atomics always seq_cst, so use volatile read to create a true acquire
        volatile T *_v=(volatile T *) &v;
        if(*_v) // Avoid unnecessary cache line invalidation traffic
          return false;
#else
        if(v.load(memory_order_relaxed)) // Avoid unnecessary cache line invalidation traffic
          return false;
#endif
#endif
#if defined(__i386__) || defined(_M_IX86) || defined(__x86_64__) || defined(_M_X64)
        // Intel is a lot quicker if you use XCHG instead of CMPXCHG. ARM is definitely not!
        T ret=v.exchange(1, memory_order_acquire);
        if(!ret)
#else
        T expected=0;
        bool ret=v.compare_exchange_weak(expected, 1, memory_order_acquire, memory_order_relaxed);
        if(ret)
#endif
        {
          BOOST_SPINLOCK_ANNOTATE_RWLOCK_ACQUIRED(this, true);
          return true;
        }
        else return false;
      }
      BOOST_SPINLOCK_CONSTEXPR bool try_lock() const BOOST_NOEXCEPT_OR_NOTHROW
      {
        return v.load(memory_order_consume) ? false : true;  // Avoid unnecessary cache line invalidation traffic
      }
      //! If atomic equals expected, sets to 1 and returns true, else false with expected updated to actual value.
      bool try_lock(T &expected) BOOST_NOEXCEPT_OR_NOTHROW
      {
        T t(0);
#if ! BOOST_SPINLOCK_IN_THREAD_SANITIZER  // no early outs for the sanitizer
#ifdef BOOST_SPINLOCK_USE_VOLATILE_READ_FOR_AVOIDING_CMPXCHG
        // MSVC's atomics always seq_cst, so use volatile read to create a true acquire
        volatile T *_v = (volatile T *)&v;
        if((t=*_v)) // Avoid unnecessary cache line invalidation traffic
#else
        if((t=v.load(memory_order_relaxed))) // Avoid unnecessary cache line invalidation traffic
#endif
        {
          expected=t;
          return false;
        }
#endif
        bool ret=v.compare_exchange_weak(expected, 1, memory_order_acquire, memory_order_relaxed);
        if(ret)
        {
          BOOST_SPINLOCK_ANNOTATE_RWLOCK_ACQUIRED(this, true);
          return true;
        }
        else return false;
      }
      //! Sets the atomic to zero
      void unlock() BOOST_NOEXCEPT_OR_NOTHROW
      {
        BOOST_SPINLOCK_ANNOTATE_RWLOCK_RELEASED(this, true);
        v.store(0, memory_order_release);
      }
      BOOST_SPINLOCK_RELAXED_CONSTEXPR bool int_yield(size_t) BOOST_NOEXCEPT_OR_NOTHROW { return false; }
    };
    template<typename T> struct spinlockbase<lockable_ptr<T>>
    {
    private:
      lockable_ptr<T> v;
    public:
      typedef T *value_type;
      spinlockbase() BOOST_NOEXCEPT_OR_NOTHROW { }
      spinlockbase(const spinlockbase &) = delete;
      //! Atomically move constructs
      spinlockbase(spinlockbase &&o) BOOST_NOEXCEPT_OR_NOTHROW
      {
        v.store(o.v.exchange(nullptr, memory_order_acq_rel), memory_order_release);
      }
      spinlockbase &operator=(const spinlockbase &) = delete;
      spinlockbase &operator=(spinlockbase &&) = delete;
      //! Returns the memory pointer part of the atomic
      T *get() BOOST_NOEXCEPT_OR_NOTHROW { return v.get(); }
      T *operator->() BOOST_NOEXCEPT_OR_NOTHROW { return get(); }
      //! Returns the raw atomic
      T *load(memory_order o=memory_order_seq_cst) BOOST_NOEXCEPT_OR_NOTHROW { return v.load(o); }
#if 0 // Forces cmpxchng on everything else, so avoid if at all possible.
      //! Sets the memory pointer part of the atomic preserving lockedness
      void set(T *a) BOOST_NOEXCEPT_OR_NOTHROW
      {
        union
        {
          T *v;
          size_t n;
        } value;
        T *expected;
        do
        {
          value.v=v.load(memory_order_relaxed);
          expected=value.v;
          bool locked=value.n&1;
          value.v=a;
          if(locked) value.n|=1;
        } while(!v.compare_exchange_weak(expected, value.v, memory_order_acquire, memory_order_relaxed));
      }
#endif
      //! Sets the raw atomic
      void store(T *a, memory_order o=memory_order_seq_cst) BOOST_NOEXCEPT_OR_NOTHROW { v.store(a, o); }
      bool try_lock() BOOST_NOEXCEPT_OR_NOTHROW
      {
        union
        {
          T *v;
          size_t n;
        } value;
        value.v=v.load(memory_order_relaxed);
        if(value.n&1) // Avoid unnecessary cache line invalidation traffic
          return false;
        T *expected=value.v;
        value.n|=1;
        return v.compare_exchange_weak(expected, value.v, memory_order_acquire, memory_order_relaxed);
      }
      void unlock() BOOST_NOEXCEPT_OR_NOTHROW
      {
        union
        {
          T *v;
          size_t n;
        } value;
        value.v=v.load(memory_order_relaxed);
        assert(value.n&1);
        value.n&=~(size_t)1;
        v.store(value.v, memory_order_release);
      }
      BOOST_SPINLOCK_RELAXED_CONSTEXPR bool int_yield(size_t) BOOST_NOEXCEPT_OR_NOTHROW { return false; }
    };
    namespace detail
    {
      template<bool use_pause> inline void smt_pause() BOOST_NOEXCEPT
      {
      };
      template<> inline void smt_pause<true>() BOOST_NOEXCEPT
      {
#ifdef BOOST_SMT_PAUSE
        BOOST_SMT_PAUSE;
#endif
      };
    }
    //! \brief How many spins to loop, optionally calling the SMT pause instruction on Intel
    template<size_t spins, bool use_pause=true> struct spins_to_loop
    {
      template<class parenttype> struct policy : parenttype
      {
        static BOOST_CONSTEXPR_OR_CONST size_t spins_to_loop=spins;
        BOOST_SPINLOCK_CONSTEXPR policy() {}
        policy(const policy &) = delete;
        BOOST_SPINLOCK_CONSTEXPR policy(policy &&o) BOOST_NOEXCEPT : parenttype(std::move(o)) { }
        BOOST_SPINLOCK_RELAXED_CONSTEXPR inline bool int_yield(size_t n) BOOST_NOEXCEPT_OR_NOTHROW
        {
          if(parenttype::int_yield(n)) return true;
          if(n>=spins) return false;
          detail::smt_pause<use_pause>();
          return true;
        }
      };
    };
    //! \brief How many spins to yield the current thread's timeslice
    template<size_t spins> struct spins_to_yield
    {
      template<class parenttype> struct policy : parenttype
      {
        static BOOST_CONSTEXPR_OR_CONST size_t spins_to_yield=spins;
        BOOST_SPINLOCK_CONSTEXPR policy() {}
        policy(const policy &) = delete;
        BOOST_SPINLOCK_CONSTEXPR policy(policy &&o) BOOST_NOEXCEPT : parenttype(std::move(o)) { }
        BOOST_SPINLOCK_RELAXED_CONSTEXPR bool int_yield(size_t n) BOOST_NOEXCEPT_OR_NOTHROW
        {
          if(parenttype::int_yield(n)) return true;
          if(n>=spins) return false;
          this_thread::yield();
          return true;
        }
      };
    };
    //! \brief How many spins to sleep the current thread
    struct spins_to_sleep
    {
      template<class parenttype> struct policy : parenttype
      {
        BOOST_SPINLOCK_CONSTEXPR policy() {}
        policy(const policy &) = delete;
        BOOST_SPINLOCK_CONSTEXPR policy(policy &&o) BOOST_NOEXCEPT : parenttype(std::move(o)) { }
        BOOST_SPINLOCK_RELAXED_CONSTEXPR bool int_yield(size_t n) BOOST_NOEXCEPT_OR_NOTHROW
        {
          if(parenttype::int_yield(n)) return true;
          this_thread::sleep_for(chrono::milliseconds(1));
          return true;
        }
      };
    };
    //! \brief A spin policy which does nothing
    struct null_spin_policy
    {
      template<class parenttype> struct policy : parenttype
      {
      };
    };
    template<class T> inline bool is_lockable_locked(T &lockable) BOOST_NOEXCEPT_OR_NOTHROW;
    /*! \class spinlock
    \brief A policy configurable spin lock meeting BasicLockable and Lockable.
    
    Meets the requirements of BasicLockable and Lockable. Also provides a get() and set() for the
    type used for the spin lock.

    So what's wrong with boost/smart_ptr/detail/spinlock.hpp then, and why
    reinvent the wheel?

    1. Non-configurable spin. AFIO needs a bigger spin than smart_ptr provides.

    2. AFIO is C++ 11, and therefore can implement this in pure C++ 11 atomics.

    3. I don't much care for doing writes during the spin. It generates an
    unnecessary amount of cache line invalidation traffic. Better to spin-read
    and only write when the read suggests you might have a chance.
    
    4. This spin lock can use a pointer to memory as the spin lock. See locked_ptr<T>.
    */
    template<typename T, template<class> class spinpolicy2=spins_to_loop<125>::policy, template<class> class spinpolicy3=spins_to_yield<250>::policy, template<class> class spinpolicy4=spins_to_sleep::policy> class spinlock : public spinpolicy4<spinpolicy3<spinpolicy2<spinlockbase<T>>>>
    {
      typedef spinpolicy4<spinpolicy3<spinpolicy2<spinlockbase<T>>>> parenttype;
    public:
      BOOST_SPINLOCK_CONSTEXPR spinlock() { }
      spinlock(const spinlock &) = delete;
      BOOST_SPINLOCK_CONSTEXPR spinlock(spinlock &&o) BOOST_NOEXCEPT : parenttype(std::move(o)) { }
      void lock() BOOST_NOEXCEPT_OR_NOTHROW
      {
        for(size_t n=0;; n++)
        {
          if(parenttype::try_lock())
            return;
          parenttype::int_yield(n);
        }
      }
      //! Locks if the atomic is not the supplied value, else returning false
      bool lock(T only_if_not_this) BOOST_NOEXCEPT_OR_NOTHROW
      {
        for(size_t n=0;; n++)
        {
          T expected=0;
          if(parenttype::try_lock(expected))
            return true;
          if(expected==only_if_not_this)
            return false;
          parenttype::int_yield(n);
        }
      }
    };

    //! \brief Determines if a lockable is locked. Type specialise this for performance if your lockable allows examination.
    template<class T> inline bool is_lockable_locked(T &lockable) BOOST_NOEXCEPT_OR_NOTHROW
    {
      if(lockable.try_lock())
      {
        lockable.unlock();
        return true;
      }
      return false;
    }
    // For when used with a spinlock
    template<class T, template<class> class spinpolicy2, template<class> class spinpolicy3, template<class> class spinpolicy4> BOOST_SPINLOCK_CONSTEXPR inline T is_lockable_locked(spinlock<T, spinpolicy2, spinpolicy3, spinpolicy4> &lockable) BOOST_NOEXCEPT_OR_NOTHROW
    {
#ifdef BOOST_HAVE_TRANSACTIONAL_MEMORY_COMPILER
      // Annoyingly the atomic ops are marked as unsafe for atomic transactions, so ...
      return *((volatile T *) &lockable);
#else
      return lockable.load(memory_order_consume);
#endif
    }
    // For when used with a spinlock
    template<class T, template<class> class spinpolicy2, template<class> class spinpolicy3, template<class> class spinpolicy4> BOOST_SPINLOCK_CONSTEXPR inline T is_lockable_locked(const spinlock<T, spinpolicy2, spinpolicy3, spinpolicy4> &lockable) BOOST_NOEXCEPT_OR_NOTHROW
    {
#ifdef BOOST_HAVE_TRANSACTIONAL_MEMORY_COMPILER
      // Annoyingly the atomic ops are marked as unsafe for atomic transactions, so ...
      return *((volatile T *)&lockable);
#else
      return lockable.load(memory_order_consume);
#endif
    }
    // For when used with a locked_ptr
    template<class T, template<class> class spinpolicy2, template<class> class spinpolicy3, template<class> class spinpolicy4> BOOST_SPINLOCK_CONSTEXPR inline bool is_lockable_locked(spinlock<lockable_ptr<T>, spinpolicy2, spinpolicy3, spinpolicy4> &lockable) BOOST_NOEXCEPT_OR_NOTHROW
    {
      return ((size_t) lockable.load(memory_order_consume))&1;
    }

#ifndef BOOST_BEGIN_TRANSACT_LOCK
#ifdef BOOST_HAVE_TRANSACTIONAL_MEMORY_COMPILER
#undef BOOST_USING_INTEL_TSX
#define BOOST_BEGIN_TRANSACT_LOCK(lockable) __transaction_relaxed { (void) BOOST_SPINLOCK_V1_NAMESPACE::is_lockable_locked(lockable); {
#define BOOST_BEGIN_TRANSACT_LOCK_ONLY_IF_NOT(lockable, only_if_not_this) __transaction_relaxed { if((only_if_not_this)!=BOOST_SPINLOCK_V1_NAMESPACE::is_lockable_locked(lockable)) {
#define BOOST_END_TRANSACT_LOCK(lockable) } }
#define BOOST_BEGIN_NESTED_TRANSACT_LOCK(N) __transaction_relaxed
#define BOOST_END_NESTED_TRANSACT_LOCK(N)
#endif // BOOST_BEGIN_TRANSACT_LOCK
#endif

#ifndef BOOST_BEGIN_TRANSACT_LOCK
#define BOOST_BEGIN_TRANSACT_LOCK(lockable) { BOOST_SPINLOCK_V1_NAMESPACE::lock_guard<decltype(lockable)> __tsx_transaction(lockable);
#define BOOST_BEGIN_TRANSACT_LOCK_ONLY_IF_NOT(lockable, only_if_not_this) if(lockable.lock(only_if_not_this)) { BOOST_SPINLOCK_V1_NAMESPACE::lock_guard<decltype(lockable)> __tsx_transaction(lockable, BOOST_SPINLOCK_V1_NAMESPACE::adopt_lock_t());
#define BOOST_END_TRANSACT_LOCK(lockable) }
#define BOOST_BEGIN_NESTED_TRANSACT_LOCK(N)
#define BOOST_END_NESTED_TRANSACT_LOCK(N)
#endif // BOOST_BEGIN_TRANSACT_LOCK

BOOST_SPINLOCK_V1_NAMESPACE_END

#endif // BOOST_SPINLOCK_HPP
