/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYMESSAGE_H_
#define BERRYMESSAGE_H_

#include <vector>
#include <Poco/Mutex.h>

/**
 * Adds a Message<> variable and methods to add/remove message delegates to/from
 * this variable.
 */
#define berryNewMessageMacro(msgHandleObject)                                                            \
  private: Message<> m_ ## msgHandleObject ## Message;                                                   \
  public:                                                                                                \
  inline void Add ## msgHandleObject ## Listener(const MessageAbstractDelegate<>& delegate)              \
    { m_ ## msgHandleObject ## Message += delegate; }                                                    \
  inline void Remove ## msgHandleObject ## Listener(const MessageAbstractDelegate<>& delegate)           \
    { m_ ## msgHandleObject ## Message -= delegate; }                                                    \


#define berryNewMessageWithReturnMacro(msgHandleObject, returnType)                                            \
  private: Message<returnType> m_ ## msgHandleObject ## Message;                                               \
  public:                                                                                                      \
  inline void Add ## msgHandleObject ## Listener(const MessageAbstractDelegate<returnType>& delegate)          \
    { m_ ## msgHandleObject ## Message += delegate; }                                                          \
    inline void Remove ## msgHandleObject ## Listener(const MessageAbstractDelegate<returnType>& delegate)     \
    { m_ ## msgHandleObject ## Message -= delegate; }                                                          \


#define berryNewMessage1Macro(msgHandleObject, type1)                                                     \
  private: Message1< type1 > m_msgHandleObject ## Message;                                                \
  public:                                                                                                 \
  void Add ## msgHandleObject ## Listener(const MessageAbstractDelegate1< type1 >& delegate)              \
    { m_ ## msgHandleObject ## Message += delegate; }                                                     \
    void Remove ## msgHandleObject ## Listener(const MessageAbstractDelegate1< type1 >& delegate)         \
    { m_ ## msgHandleObject ## Message -= delegate; }                                                     \


namespace berry
{

template<typename A = void>
class MessageAbstractDelegate
{
public:

  virtual ~MessageAbstractDelegate()
  {
  }

  virtual A Execute() = 0;
  virtual bool operator==(const MessageAbstractDelegate* cmd) = 0;
  virtual MessageAbstractDelegate* Clone() const = 0;
};

template<typename T, typename A = void>
class MessageAbstractDelegate1
{
public:

  virtual ~MessageAbstractDelegate1()
  {
  }

  virtual A Execute(T t) = 0;
  virtual bool operator==(const MessageAbstractDelegate1* cmd) = 0;
  virtual MessageAbstractDelegate1* Clone() const = 0;
};

template<typename T, typename U, typename A = void>
class MessageAbstractDelegate2
{
public:

  virtual ~MessageAbstractDelegate2()
  {
  }

  virtual A Execute(T t, U u) const = 0;
  virtual bool operator==(const MessageAbstractDelegate2* cmd) = 0;
  virtual MessageAbstractDelegate2* Clone() const = 0;
};

template<typename T, typename U, typename V, typename A = void>
class MessageAbstractDelegate3
{
public:

  virtual ~MessageAbstractDelegate3()
  {
  }

  virtual A Execute(T t, U u, V v) const = 0;
  virtual bool operator==(const MessageAbstractDelegate3* cmd) = 0;
  virtual MessageAbstractDelegate3* Clone() const = 0;
};

template<typename T, typename U, typename V, typename W, typename A = void>
class MessageAbstractDelegate4
{
public:

  virtual ~MessageAbstractDelegate4()
  {
  }

  virtual A Execute(T t, U u, V v, W w) const = 0;
  virtual bool operator==(const MessageAbstractDelegate4* cmd) = 0;
  virtual MessageAbstractDelegate4* Clone() const = 0;
};

template<class R, typename A = void>
class MessageDelegate: public MessageAbstractDelegate<A>
{
public:

  // constructor - takes pointer to an object and pointer to a member and stores
  // them in two private variables
  MessageDelegate(R* object, A(R::*memberFunctionPointer)()) :
    m_Object(object), m_MemberFunctionPointer(memberFunctionPointer)
  {
  }

  virtual ~MessageDelegate()
  {
  }

  // override function "Call"
  virtual A Execute()
  {
    return (m_Object->*m_MemberFunctionPointer)(); // execute member function
  }

  bool operator==(const MessageAbstractDelegate<A>* c)
  {
    const MessageDelegate<R, A>* cmd =
        dynamic_cast<const MessageDelegate<R, A>*> (c);
    if (!cmd)
      return false;

    if ((void*) this->m_Object != (void*) cmd->m_Object)
      return false;
    if (this->m_MemberFunctionPointer != cmd->m_MemberFunctionPointer)
      return false;
    return true;
  }

  MessageAbstractDelegate<A>* Clone() const
  {
    return new MessageDelegate(m_Object, m_MemberFunctionPointer);
  }

private:
  R* m_Object; // pointer to object
  A (R::*m_MemberFunctionPointer)(); // pointer to member function
};

template<class R, typename T, typename A = void>
class MessageDelegate1: public MessageAbstractDelegate1<T, A>
{
public:

  // constructor - takes pointer to an object and pointer to a member and stores
  // them in two private variables
  MessageDelegate1(R* object, A(R::*memberFunctionPointer)(T)) :
    m_Object(object), m_MemberFunctionPointer(memberFunctionPointer)
  {
  }

  virtual ~MessageDelegate1()
  {
  }

  // override function "Call"
  virtual A Execute(T t)
  {
    return (m_Object->*m_MemberFunctionPointer)(t); // execute member function
  }

  bool operator==(const MessageAbstractDelegate1<T, A>* c)
  {
    const MessageDelegate1<R, T, A>* cmd = dynamic_cast<const MessageDelegate1<
        R, T, A>*> (c);
    if (!cmd)
      return false;

    if ((void*) this->m_Object != (void*) cmd->m_Object)
      return false;
    if (this->m_MemberFunctionPointer != cmd->m_MemberFunctionPointer)
      return false;
    return true;
  }

  MessageAbstractDelegate1<T, A>* Clone() const
  {
    return new MessageDelegate1(m_Object, m_MemberFunctionPointer);
  }

private:
  R* m_Object; // pointer to object
  A (R::*m_MemberFunctionPointer)(T); // pointer to member function
};

template<class R, typename T, typename U, typename A = void>
class MessageDelegate2: public MessageAbstractDelegate2<T, U, A>
{
public:

  // constructor - takes pointer to an object and pointer to a member and stores
  // them in two private variables
  MessageDelegate2(R* object, A(R::*memberFunctionPointer)(T, U)) :
    m_Object(object), m_MemberFunctionPointer(memberFunctionPointer)
  {
  }

  virtual ~MessageDelegate2()
  {
  }

  // override function "Call"
  virtual A Execute(T t, U u) const
  {
    return (m_Object->*m_MemberFunctionPointer)(t, u); // execute member function
  }

  bool operator==(const MessageAbstractDelegate2<T, U, A>* c)
  {
    const MessageDelegate2<R, T, U, A>* cmd =
        dynamic_cast<const MessageDelegate2<R, T, U, A>*> (c);
    if (!cmd)
      return false;

    if ((void*) this->m_Object != (void*) cmd->m_Object)
      return false;
    if (this->m_MemberFunctionPointer != cmd->m_MemberFunctionPointer)
      return false;
    return true;
  }

  MessageAbstractDelegate2<T, U, A>* Clone() const
  {
    return new MessageDelegate2(m_Object, m_MemberFunctionPointer);
  }

private:
  R* m_Object; // pointer to object
  A (R::*m_MemberFunctionPointer)(T, U); // pointer to member function
};

template<class R, typename T, typename U, typename V, typename A = void>
class MessageDelegate3: public MessageAbstractDelegate3<T, U, V, A>
{
public:

  // constructor - takes pointer to an object and pointer to a member and stores
  // them in two private variables
  MessageDelegate3(R* object, A(R::*memberFunctionPointer)(T, U, V)) :
    m_Object(object), m_MemberFunctionPointer(memberFunctionPointer)
  {
  }

  virtual ~MessageDelegate3()
  {
  }

  // override function "Call"
  virtual A Execute(T t, U u, V v) const
  {
    return (m_Object->*m_MemberFunctionPointer)(t, u, v); // execute member function
  }

  bool operator==(const MessageAbstractDelegate3<T, U, V, A>* c)
  {
    const MessageDelegate3<R, T, U, V, A>* cmd =
        dynamic_cast<const MessageDelegate3<R, T, U, V, A>*> (c);
    if (!cmd)
      return false;

    if ((void*) this->m_Object != (void*) cmd->m_Object)
      return false;
    if (this->m_MemberFunctionPointer != cmd->m_MemberFunctionPointer)
      return false;
    return true;
  }

  MessageAbstractDelegate3<T, U, V, A>* Clone() const
  {
    return new MessageDelegate3(m_Object, m_MemberFunctionPointer);
  }

private:
  R* m_Object; // pointer to object
  A (R::*m_MemberFunctionPointer)(T, U, V); // pointer to member function
};

template<class R, typename T, typename U, typename V, typename W,
    typename A = void>
class MessageDelegate4: public MessageAbstractDelegate4<T, U, V, W, A>
{
public:

  // constructor - takes pointer to an object and pointer to a member and stores
  // them in two private variables
  MessageDelegate4(R* object, A(R::*memberFunctionPointer)(T, U, V, W)) :
    m_Object(object), m_MemberFunctionPointer(memberFunctionPointer)
  {
  }

  virtual ~MessageDelegate4()
  {
  }

  // override function "Call"
  virtual A Execute(T t, U u, V v, W w) const
  {
    return (m_Object->*m_MemberFunctionPointer)(t, u, v, w); // execute member function
  }

  bool operator==(const MessageAbstractDelegate4<T, U, V, W, A>* c)
  {
    const MessageDelegate4<R, T, U, V, W, A>* cmd =
        dynamic_cast<const MessageDelegate4<R, T, U, V, W, A>*> (c);
    if (!cmd)
      return false;

    if ((void*) this->m_Object != (void*) cmd->m_Object)
      return false;
    if (this->m_MemberFunctionPointer != cmd->m_MemberFunctionPointer)
      return false;
    return true;
  }

  MessageAbstractDelegate4<T, U, V, W, A>* Clone() const
  {
    return new MessageDelegate4(m_Object, m_MemberFunctionPointer);
  }

private:
  R* m_Object; // pointer to object
  A (R::*m_MemberFunctionPointer)(T, U, V, W); // pointer to member function
};

struct AbstractExceptionHandler
{
  virtual void HandleException(const std::exception& exc) = 0;
  virtual AbstractExceptionHandler* Clone() const = 0;

  virtual ~AbstractExceptionHandler()
  {
  }
};

template<typename R>
struct MessageExceptionHandler: public AbstractExceptionHandler
{
  typedef void (R::*HandleExceptionCallback)(const std::exception&);

  MessageExceptionHandler(R* r, HandleExceptionCallback c) :
    m_Object(r), m_ExceptionCallback(c)
  {
  }

  void HandleException(const std::exception& exc)
  {
    (m_Object->*m_ExceptionCallback)(exc);
  }

  AbstractExceptionHandler* Clone() const
  {
    if (!m_Object) return 0;
    return new MessageExceptionHandler(m_Object, m_ExceptionCallback);
  }

private:

  R* m_Object;
  HandleExceptionCallback m_ExceptionCallback;
};

template<typename D>
class DelegateList
{

public:

  typedef D Delegate;
  typedef std::vector<Delegate*> ListType;

  DelegateList() :
    m_ExcHandler(0)
  {

  }

  ~DelegateList()
  {
    for (typename ListType::iterator iter = m_Delegates.begin(); iter
        != m_Delegates.end(); ++iter)
    {
      delete *iter;
    }

    delete m_ExcHandler;
  }

  void Add(const Delegate& d) const
  {
    Delegate* copy = d.Clone();

    Poco::FastMutex::ScopedLock lock(m_Mutex);
    for (typename ListType::iterator iter = m_Delegates.begin(); iter
        != m_Delegates.end(); ++iter)
    {
      if ((*iter)->operator==(copy))
      {
        delete copy;
        return;
      }
    }
    m_Delegates.push_back(copy);
  }

  void Remove(const Delegate& d) const
  {
    Poco::FastMutex::ScopedLock lock(m_Mutex);
    for (typename ListType::iterator iter = m_Delegates.begin(); iter
        != m_Delegates.end(); ++iter)
    {
      if ((*iter)->operator==(&d))
      {
        delete *iter;
        m_Delegates.erase(iter);
        return;
      }
    }
  }

  bool IsEmpty() const
  {
    return m_Delegates.empty();
  }

  template<typename Dummy>
  void Send0(Dummy) const
  {
    ListType delegates;

    {
      Poco::FastMutex::ScopedLock lock(m_Mutex);
      delegates.assign(m_Delegates.begin(), m_Delegates.end());
    }

    for (typename ListType::iterator iter = delegates.begin(); iter
        != delegates.end(); ++iter)
    {
      try
      {
        // notify each listener
        (*iter)->Execute();
      }
      catch (const std::exception& e)
      {
        if (m_ExcHandler)
        {
          m_ExcHandler->HandleException(e);
        }
        else
        {
          throw e;
        }
      }
      catch (...)
      {
        if (m_ExcHandler)
        {
          m_ExcHandler->HandleException(std::exception());
        }
        else
          throw ;
        }
      }
    }

    template<typename T>
    void Send1(T t) const
    {
      ListType delegates;

      {
        Poco::FastMutex::ScopedLock lock(m_Mutex);
        delegates.assign(m_Delegates.begin(), m_Delegates.end());
      }

      for (typename ListType::iterator iter = delegates.begin(); iter
          != delegates.end(); ++iter)
      {
        try
        {
          // notify each listener
          (*iter)->Execute(t);
        }
        catch (const std::exception& e)
        {
          if (m_ExcHandler)
          {
            m_ExcHandler->HandleException(e);
          }
          else
          {
            throw e;
          }
        }
        catch (...)
        {
          if (m_ExcHandler)
          {
            m_ExcHandler->HandleException(std::exception());
          }
          else throw;
        }
      }
    }

    template<typename T, typename U>
    void Send2(T t, U u) const
    {
      ListType delegates;

      {
        Poco::FastMutex::ScopedLock lock(m_Mutex);
        delegates.assign(m_Delegates.begin(), m_Delegates.end());
      }

      for (typename ListType::iterator iter = delegates.begin(); iter
          != delegates.end(); ++iter)
      {
        try
        {
          // notify each listener
          (*iter)->Execute(t, u);
        }
        catch (const std::exception& e)
        {
          if (m_ExcHandler)
          {
            m_ExcHandler->HandleException(e);
          }
          else
          {
            throw e;
          }
        }
        catch (...)
        {
          if (m_ExcHandler)
          {
            m_ExcHandler->HandleException(std::exception());
          }
          else throw;
        }
      }
    }

    template<typename T, typename U, typename V>
    void Send3(T t, U u, V v) const
    {
      ListType delegates;

      {
        Poco::FastMutex::ScopedLock lock(m_Mutex);
        delegates.assign(m_Delegates.begin(), m_Delegates.end());
      }

      for (typename ListType::iterator iter = delegates.begin(); iter
          != delegates.end(); ++iter)
      {
        try
        {
          // notify each listener
          (*iter)->Execute(t, u, v);
        }
        catch (const std::exception& e)
        {
          if (m_ExcHandler)
          {
            m_ExcHandler->HandleException(e);
          }
          else
          {
            throw e;
          }
        }
        catch (...)
        {
          if (m_ExcHandler)
          {
            m_ExcHandler->HandleException(std::exception());
          }
          else throw;
        }
      }
    }

    template<typename T, typename U, typename V, typename W>
    void Send4(T t, U u, V v, W w) const
    {
      ListType delegates;

      {
        Poco::FastMutex::ScopedLock lock(m_Mutex);
        delegates.assign(m_Delegates.begin(), m_Delegates.end());
      }

      for (typename ListType::iterator iter = delegates.begin(); iter
          != delegates.end(); ++iter)
      {
        try
        {
          // notify each listener
          (*iter)->Execute(t, u, v, w);
        }
        catch (const std::exception& e)
        {
          if (m_ExcHandler)
          {
            m_ExcHandler->HandleException(e);
          }
          else
          {
            throw e;
          }
        }
        catch (...)
        {
          if (m_ExcHandler)
          {
            m_ExcHandler->HandleException(std::exception());
          }
          else throw;
        }
      }
    }

    void SetExceptionHandler(const AbstractExceptionHandler& handler)
    {
      Poco::FastMutex::ScopedLock lock(m_Mutex);
      delete m_ExcHandler;
      m_ExcHandler = handler.Clone();
    }

    AbstractExceptionHandler* GetExceptionHandler() const
    {
      Poco::FastMutex::ScopedLock lock(m_Mutex);
      if (m_ExcHandler) return m_ExcHandler->Clone();
      return 0;
    }

    ListType GetDelegates() const
    {
      Poco::FastMutex::ScopedLock lock(m_Mutex);
      return m_Delegates;
    }

  private:

    /**
     * \brief List of listeners.
     *
     * This is declared mutable for a reason: Imagine an object that sends out notifications, e.g.
     *
     * \code
     class Database {
     public:
     Message Modified;
     };
     * \endcode
     *
     * Now imaginge someone gets a <tt>const Database</tt> object, because he/she should not write to the
     * database. He/she should anyway be able to register for notifications about changes in the database
     * -- this is why AddListener and RemoveListener are declared <tt>const</tt>. m_Listeners must be
     *  mutable so that AddListener and RemoveListener can modify it regardless of the object's constness.
     */
    mutable std::vector<Delegate*> m_Delegates;

    mutable AbstractExceptionHandler* m_ExcHandler;
    mutable Poco::FastMutex m_Mutex;
  };

  /**
   * \brief Event/message/notification class.
   *
   * This class
   * allows one class to send out messages and another class to
   * receive these message. There are subclasses for sending
   * parameters along with the messages.
   *
   * This is an implementation of the Observer pattern.
   *
   * \li There is no guarantee about the order of which observer is notified first. At the moment the observers which register first will be notified first.
   * \li Notifications are <b>synchronous</b>, by direct method calls. There is no support for asynchronous messages.
   *
   */
  // message without parameters (pure signals)
  template<typename A = void>
  class Message
  {
  public:

    typedef Message Self;
    typedef MessageAbstractDelegate<A> AbstractDelegate;
    typedef typename DelegateList<AbstractDelegate>::ListType ListenerList;

    void AddListener(const AbstractDelegate& delegate) const
    {
      delegates.Add(delegate);
    }

    void operator +=(const AbstractDelegate& delegate) const
    {
      this->AddListener(delegate);
    }

    void RemoveListener(const AbstractDelegate& delegate) const
    {
      delegates.Remove(delegate);
    }

    void operator -=(const AbstractDelegate& delegate) const
    {
      this->RemoveListener(delegate);
    }

    void Send() const
    {
      delegates.Send0(0);
    }

    void operator()() const
    {
      this->Send();
    }

    void SetExceptionHandler(const AbstractExceptionHandler& handler)
    {
      delegates.SetExceptionHandler(handler);
    }

    AbstractExceptionHandler* GetExceptionHandler() const
    {
      return delegates.GetExceptionHandler();
    }

    ListenerList GetListeners() const
    {
      return delegates.GetDelegates();
    }

    bool HasListeners() const
    {
      return !delegates.IsEmpty();
    }

    bool IsEmpty() const
    {
      return delegates.IsEmpty();
    }

  protected:

    DelegateList<AbstractDelegate> delegates;

  };

  // message with 1 parameter and return type
  template<typename T, typename A = void>
  class Message1
  {
  public:

    typedef Message1 Self;
    typedef MessageAbstractDelegate1<T, A> AbstractDelegate;
    typedef typename DelegateList<AbstractDelegate>::ListType ListenerList;

    void AddListener(const AbstractDelegate& delegate) const
    {
      delegates.Add(delegate);
    }

    void operator +=(const AbstractDelegate& delegate) const
    {
      this->AddListener(delegate);
    }

    void RemoveListener(const AbstractDelegate& delegate) const
    {
      delegates.Remove(delegate);
    }

    void operator -=(const AbstractDelegate& delegate) const
    {
      this->RemoveListener(delegate);
    }

    void Send(T t) const
    {
      delegates.Send1(t);
    }

    void operator()(T t) const
    {
      this->Send(t);
    }

    void SetExceptionHandler(const AbstractExceptionHandler& handler)
    {
      delegates.SetExceptionHandler(handler);
    }

    AbstractExceptionHandler* GetExceptionHandler() const
    {
      return delegates.GetExceptionHandler();
    }

    ListenerList GetListeners() const
    {
      return delegates.GetDelegates();
    }

    bool HasListeners() const
    {
      return !delegates.IsEmpty();
    }

    bool IsEmpty() const
    {
      return delegates.IsEmpty();
    }

  protected:

    DelegateList<AbstractDelegate> delegates;

  };

  // message with 2 parameters and return type
  template<typename T, typename U, typename A = void>
  class Message2
  {
  public:

    typedef Message2 Self;
    typedef MessageAbstractDelegate2<T, U, A> AbstractDelegate;
    typedef typename DelegateList<AbstractDelegate>::ListType ListenerList;

    void AddListener(const AbstractDelegate& delegate) const
    {
      delegates.Add(delegate);
    }

    void operator +=(const AbstractDelegate& delegate) const
    {
      this->AddListener(delegate);
    }

    void RemoveListener(const AbstractDelegate& delegate) const
    {
      delegates.Remove(delegate);
    }

    void operator -=(const AbstractDelegate& delegate) const
    {
      this->RemoveListener(delegate);
    }

    void Send(T t, U u) const
    {
      delegates.Send2(t, u);
    }

    void operator()(T t, U u) const
    {
      this->Send(t, u);
    }

    void SetExceptionHandler(const AbstractExceptionHandler& handler)
    {
      delegates.SetExceptionHandler(handler);
    }

    AbstractExceptionHandler* GetExceptionHandler() const
    {
      return delegates.GetExceptionHandler();
    }

    ListenerList GetListeners() const
    {
      return delegates.GetDelegates();
    }

    bool HasListeners() const
    {
      return !delegates.IsEmpty();
    }

    bool IsEmpty() const
    {
      return delegates.IsEmpty();
    }

  protected:

    DelegateList<AbstractDelegate> delegates;

  };

  // message with 3 parameters and return type
  template<typename T, typename U, typename V, typename A = void>
  class Message3
  {
  public:

    typedef Message3 Self;
    typedef MessageAbstractDelegate3<T, U, V, A> AbstractDelegate;
    typedef typename DelegateList<AbstractDelegate>::ListType ListenerList;

    void AddListener(const AbstractDelegate& delegate) const
    {
      delegates.Add(delegate);
    }

    void operator +=(const AbstractDelegate& delegate) const
    {
      this->AddListener(delegate);
    }

    void RemoveListener(const AbstractDelegate& delegate) const
    {
      delegates.Remove(delegate);
    }

    void operator -=(const AbstractDelegate& delegate) const
    {
      this->RemoveListener(delegate);
    }

    void Send(T t, U u, V v) const
    {
      delegates.Send3(t, u, v);
    }

    void operator()(T t, U u, V v) const
    {
      this->Send(t, u, v);
    }

    void SetExceptionHandler(const AbstractExceptionHandler& handler)
    {
      delegates.SetExceptionHandler(handler);
    }

    AbstractExceptionHandler* GetExceptionHandler() const
    {
      return delegates.GetExceptionHandler();
    }

    ListenerList GetListeners() const
    {
      return delegates.GetDelegates();
    }

    bool HasListeners() const
    {
      return !delegates.IsEmpty();
    }

    bool IsEmpty() const
    {
      return delegates.IsEmpty();
    }

  protected:

    DelegateList<AbstractDelegate> delegates;

  };

  // message with 4 parameters and return type
  template<typename T, typename U, typename V, typename W, typename A = void>
  class Message4
  {
  public:

    typedef Message4 Self;
    typedef MessageAbstractDelegate4<T, U, V, W, A> AbstractDelegate;
    typedef typename DelegateList<AbstractDelegate>::ListType ListenerList;

    void AddListener(const AbstractDelegate& delegate) const
    {
      delegates.Add(delegate);
    }

    void operator +=(const AbstractDelegate& delegate) const
    {
      this->AddListener(delegate);
    }

    void RemoveListener(const AbstractDelegate& delegate) const
    {
      delegates.Remove(delegate);
    }

    void operator -=(const AbstractDelegate& delegate) const
    {
      this->RemoveListener(delegate);
    }

    void Send(T t, U u, V v, W w) const
    {
      delegates.Send4(t, u, v, w);
    }

    void operator()(T t, U u, V v, W w) const
    {
      this->Send(t, u , v, w);
    }

    void SetExceptionHandler(const AbstractExceptionHandler& handler)
    {
      delegates.SetExceptionHandler(handler);
    }

    AbstractExceptionHandler* GetExceptionHandler() const
    {
      return delegates.GetExceptionHandler();
    }

    ListenerList GetListeners() const
    {
      return delegates.GetDelegates();
    }

    bool HasListeners() const
    {
      return !delegates.IsEmpty();
    }

    bool IsEmpty() const
    {
      return delegates.IsEmpty();
    }

  protected:

    DelegateList<AbstractDelegate> delegates;

  };

} // namespace berry

#endif /*BERRYMESSAGE_H_*/
