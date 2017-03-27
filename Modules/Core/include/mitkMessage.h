/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkMessageHIncluded
#define mitkMessageHIncluded

#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <memory>
#include <itkSimpleFastMutexLock.h>

/**
 * Adds a Message<> variable and methods to add/remove message delegates to/from
 * this variable.
*/
#define mitkNewMessageMacro(msgHandleObject)                                                                 \
  private: ::mitk::Message<> m_##msgHandleObject##Message;                                                   \
  public:                                                                                                \
  inline void Add##msgHandleObject##Listener(const ::mitk::MessageAbstractDelegate<>& delegate)              \
    { m_##msgHandleObject##Message += delegate; }                                                    \
  inline void Remove##msgHandleObject##Listener(const ::mitk::MessageAbstractDelegate<>& delegate)           \
    { m_##msgHandleObject##Message -= delegate; }                                                    \


#define mitkNewMessageWithReturnMacro(msgHandleObject, returnType)                                                 \
  private: ::mitk::Message<returnType> m_##msgHandleObject##Message;                                               \
  public:                                                                                                      \
  inline void Add##msgHandleObject##Listener(const ::mitk::MessageAbstractDelegate<returnType>& delegate)          \
    { m_##msgHandleObject##Message += delegate; }                                                          \
    inline void Remove##msgHandleObject##Listener(const ::mitk::MessageAbstractDelegate<returnType>& delegate)     \
    { m_##msgHandleObject##Message -= delegate; }                                                          \


#define mitkNewMessage1Macro(msgHandleObject, type1)                                                          \
  private: ::mitk::Message1< type1 > m_##msgHandleObject##Message;                                                \
  public:                                                                                                 \
  void Add##msgHandleObject##Listener(const ::mitk::MessageAbstractDelegate1< type1 >& delegate)              \
    { m_##msgHandleObject##Message += delegate; }                                                     \
    void Remove##msgHandleObject##Listener(const ::mitk::MessageAbstractDelegate1< type1 >& delegate)         \
    { m_##msgHandleObject##Message -= delegate; }

#define mitkNewMessage2Macro(msgHandleObject, type1, type2)                                                          \
  private: ::mitk::Message2< type1, type2 > m_##msgHandleObject##Message;                                                \
  public:                                                                                                 \
  void Add##msgHandleObject##Listener(const ::mitk::MessageAbstractDelegate2< type1, type2 >& delegate)              \
    { m_##msgHandleObject##Message += delegate; }                                                     \
    void Remove##msgHandleObject##Listener(const ::mitk::MessageAbstractDelegate2< type1, type2 >& delegate)         \
    { m_##msgHandleObject##Message -= delegate; }


namespace mitk {

template<typename A = void>
class MessageAbstractDelegate
{
public:

  virtual ~MessageAbstractDelegate()
  {
  }

  virtual A Execute() const = 0;
  virtual bool operator==(const MessageAbstractDelegate* cmd) const = 0;
  virtual MessageAbstractDelegate* Clone() const = 0;
};

template <typename T, typename A = void>
class MessageAbstractDelegate1
{
public:

  virtual ~MessageAbstractDelegate1()
  {
  }

  virtual A Execute(T t) const = 0;
  virtual bool operator==(const MessageAbstractDelegate1* cmd) const = 0;
  virtual MessageAbstractDelegate1* Clone() const = 0;
};

template <typename T, typename U, typename A = void>
class MessageAbstractDelegate2
{
public:

  virtual ~MessageAbstractDelegate2()
  {
  }

  virtual A Execute(T t, U u) const = 0;
  virtual bool operator==(const MessageAbstractDelegate2* cmd) const = 0;
  virtual MessageAbstractDelegate2* Clone() const = 0;
};

template <typename T, typename U, typename V, typename A = void>
class MessageAbstractDelegate3
{
public:

  virtual ~MessageAbstractDelegate3()
  {
  }

  virtual A Execute(T t, U u, V v) const = 0;
  virtual bool operator==(const MessageAbstractDelegate3* cmd) const = 0;
  virtual MessageAbstractDelegate3* Clone() const = 0;
};

template <typename T, typename U, typename V, typename W, typename A = void>
class MessageAbstractDelegate4
{
public:

  virtual ~MessageAbstractDelegate4()
  {
  }

  virtual A Execute(T t, U u, V v, W w) const = 0;
  virtual bool operator==(const MessageAbstractDelegate4* cmd) const = 0;
  virtual MessageAbstractDelegate4* Clone() const = 0;
};

/**
* This class essentially wraps a function pointer with signature
* A(R::*function)(). A is the return type of your callback function
* and R the type of the class implementing the function.
*
* Use this class to add a callback function to
* messages without parameters.
*/
template <class R, typename F>
class MessageDelegateBase
{
protected:
  MessageDelegateBase(R* object, F memberFunctionPointer)
    : m_Object(object)
    , m_MemberFunctionPointer(memberFunctionPointer)
  {
  }

  R& ObjectRef() const
  {
    return *m_Object;
  }

  R* ObjectPtr() const
  {
    return m_Object;
  }

  F MemberFunctionPointer() const
  {
    return m_MemberFunctionPointer;
  }

  bool Equal(const MessageDelegateBase* c) const
  {
    return m_Object == c->m_Object && m_MemberFunctionPointer == c->m_MemberFunctionPointer;
  }

private:
  R* m_Object;                // pointer to object
  F m_MemberFunctionPointer;  // pointer to member function
};

/**
* This class essentially wraps a function pointer with signature
* A(R::*function)(T). A is the return type of your callback function,
* R the type of the class implementing the function and T the type
* of the argument.
*
* Use this class to add a callback function to
* messages with one parameter.
*
* If you need more parameters, use MessageDelegate2 etc.
*/
template <class R, typename A = void>
class MessageDelegate
  : private MessageDelegateBase<R, A(R::*)()>
  , public MessageAbstractDelegate<A>
{
public:
  typedef MessageDelegateBase<R, A(R::*)()> MessageDelegateBaseType;

  MessageDelegate(R* object, A(R::*memberFunctionPointer)())
    : MessageDelegateBaseType(object, memberFunctionPointer)
  {
  }

  virtual A Execute() const
  {
    return (MessageDelegateBaseType::ObjectRef().*MessageDelegateBaseType::MemberFunctionPointer())();
  }

  virtual bool operator==(const MessageAbstractDelegate<A>* c) const
  {
    const MessageDelegate* cmd = dynamic_cast<const MessageDelegate* >(c);
    if (!cmd) return false;
    return MessageDelegateBaseType::Equal(cmd);
  }

  virtual MessageAbstractDelegate<A>* Clone() const
  {
    return new MessageDelegate(MessageDelegateBaseType::ObjectPtr(), MessageDelegateBaseType::MemberFunctionPointer());
  }
};

template <class R, typename T, typename A = void>
class MessageDelegate1
  : private MessageDelegateBase<R, A(R::*)(T)>
  , public MessageAbstractDelegate1<T, A>
{
public:
  typedef MessageDelegateBase<R, A(R::*)(T)> MessageDelegateBaseType;

  MessageDelegate1(R* object, A(R::*memberFunctionPointer)(T))
    : MessageDelegateBaseType(object, memberFunctionPointer)
  {
  }

  virtual A Execute(T t) const
  {
    return (MessageDelegateBaseType::ObjectRef().*MessageDelegateBaseType::MemberFunctionPointer())(t);
  }

  virtual bool operator==(const MessageAbstractDelegate1<T, A>* c) const
  {
    const MessageDelegate1* cmd = dynamic_cast<const MessageDelegate1* >(c);
    if (!cmd) return false;
    return MessageDelegateBaseType::Equal(cmd);
  }

  virtual MessageAbstractDelegate1<T, A>* Clone() const
  {
    return new MessageDelegate1(MessageDelegateBaseType::ObjectPtr(), MessageDelegateBaseType::MemberFunctionPointer());
  }
};

template <class R, typename T, typename U, typename A = void>
class MessageDelegate2
  : private MessageDelegateBase<R, A(R::*)(T, U)>
  , public MessageAbstractDelegate2<T, U, A>
{
public:
  typedef MessageDelegateBase<R, A(R::*)(T, U)> MessageDelegateBaseType;

  MessageDelegate2(R* object, A(R::*memberFunctionPointer)(T, U))
    : MessageDelegateBaseType(object, memberFunctionPointer)
  {
  }

  virtual A Execute(T t, U u) const
  {
    return (MessageDelegateBaseType::ObjectRef().*MessageDelegateBaseType::MemberFunctionPointer())(t, u);
  }

  virtual bool operator==(const MessageAbstractDelegate2<T, U, A>* c) const
  {
    const MessageDelegate2* cmd = dynamic_cast<const MessageDelegate2* >(c);
    if (!cmd) return false;
    return MessageDelegateBaseType::Equal(cmd);
  }

  virtual MessageAbstractDelegate2<T, U, A>* Clone() const
  {
    return new MessageDelegate2(MessageDelegateBaseType::ObjectPtr(), MessageDelegateBaseType::MemberFunctionPointer());
  }
};

template <class R, typename T, typename U, typename V, typename A = void>
class MessageDelegate3
  : private MessageDelegateBase<R, A(R::*)(T, U, V)>
  , public MessageAbstractDelegate3<T, U, V, A>
{
public:
  typedef MessageDelegateBase<R, A(R::*)(T, U, V)> MessageDelegateBaseType;

  MessageDelegate3(R* object, A(R::*memberFunctionPointer)(T, U, V))
    : MessageDelegateBaseType(object, memberFunctionPointer)
  {
  }

  virtual A Execute(T t, U u, V v) const
  {
    return (MessageDelegateBaseType::ObjectRef().*MessageDelegateBaseType::MemberFunctionPointer())(t, u, v);
  }

  virtual bool operator==(const MessageAbstractDelegate3<T, U, V, A>* c) const
  {
    const MessageDelegate3* cmd = dynamic_cast<const MessageDelegate3* >(c);
    if (!cmd) return false;
    return MessageDelegateBaseType::Equal(cmd);
  }

  virtual MessageAbstractDelegate3<T, U, V, A>* Clone() const
  {
    return new MessageDelegate3(MessageDelegateBaseType::ObjectPtr(), MessageDelegateBaseType::MemberFunctionPointer());
  }
};

template <class R, typename T, typename U, typename V, typename W, typename A = void>
class MessageDelegate4
  : private MessageDelegateBase<R, A(R::*)(T, U, V, W)>
  , public MessageAbstractDelegate4<T, U, V, W, A>
{
public:
  typedef MessageDelegateBase<R, A(R::*)(T, U, V, W)> MessageDelegateBaseType;

  MessageDelegate4(R* object, A(R::*memberFunctionPointer)(T, U, V, W))
    : MessageDelegateBaseType(object, memberFunctionPointer)
  {
  }

  virtual A Execute(T t, U u, V v, W w) const
  {
    return (MessageDelegateBaseType::ObjectRef().*MessageDelegateBaseType::MemberFunctionPointer())(t, u, v, w);
  }

  virtual bool operator==(const MessageAbstractDelegate4<T, U, V, W, A>* c) const
  {
    const MessageDelegate4<R, T, U, V, W, A>* cmd = dynamic_cast<const MessageDelegate4<R, T, U, V, W, A>* >(c);
    if (!cmd) return false;
    return MessageDelegateBaseType::Equal(cmd);
  }

  virtual MessageAbstractDelegate4<T, U, V, W, A>* Clone() const
  {
    return new MessageDelegate4(MessageDelegateBaseType::ObjectPtr(), MessageDelegateBaseType::MemberFunctionPointer());
  }
};

template<typename AbstractDelegate>
class MessageBase
{
public:

  virtual ~MessageBase()
  {
  }

  MessageBase()
  {
  }

  MessageBase(const MessageBase& o)
  {
    o.m_Mutex.Lock();
    for (auto item : o.m_Listeners) {
      if (o.m_ListenerToRemove.end() == o.m_ListenerToRemove.find(item.get())) {
        m_Listeners.push_back(AbstractDelegatePtr(item->Clone()));
      }
    }
    o.m_Mutex.Unlock();
  }

  MessageBase& operator=(const MessageBase& o)
  {
    MessageBase tmp(o);
    std::swap(tmp.m_Listeners, this->m_Listeners);
    return *this;
  }

  void AddListener(const AbstractDelegate& delegate) const
  {
    AbstractDelegatePtr msgCmd(delegate.Clone());

    m_Mutex.Lock();
    const auto iter = std::find_if(m_Listeners.begin(), m_Listeners.end(),
      [msgCmd](AbstractDelegatePtr item) { return *item == msgCmd.get(); });
    if (m_Listeners.end() == iter) {
      m_Listeners.push_back(msgCmd);
    } else {
      m_ListenerToRemove.erase(iter->get());
    }
    m_Mutex.Unlock();
  }

  void operator += (const AbstractDelegate& delegate) const
  {
    this->AddListener(delegate);
  }

  void RemoveListener(const AbstractDelegate& delegate) const
  {
    m_Mutex.Lock();
    const auto it = std::find_if(m_UsedListeners.begin(), m_UsedListeners.end(),
      [&delegate](typename decltype(m_UsedListeners)::const_reference item) { return delegate == item.first; });
    if (m_UsedListeners.end() != it) {
      m_ListenerToRemove.insert(it->first);
    } else {
      const auto iter = std::find_if(m_Listeners.begin(), m_Listeners.end(),
        [&delegate](const AbstractDelegatePtr& item) { return delegate == item.get(); });
      if (m_Listeners.end() != iter) {
        m_Listeners.erase(iter);
      }
    }
    m_Mutex.Unlock();
  }

  void operator -= (const AbstractDelegate& delegate) const
  {
    this->RemoveListener(delegate);
  }

  bool HasListeners() const
  {
    return !IsEmpty();
  }

  bool IsEmpty() const
  {
    m_Mutex.Lock();
    const auto isEmpty = m_Listeners.empty() || m_Listeners.size() == m_ListenerToRemove.size();
    m_Mutex.Unlock();
    return isEmpty;
  }

protected:

  AbstractDelegate* GetListener(AbstractDelegate* listener = nullptr) const
  {
    m_Mutex.Lock();
    if (m_Listeners.empty()) {
      m_Mutex.Unlock();
      return nullptr;
    }
    auto iter = listener ? std::find_if(m_Listeners.begin(), m_Listeners.end(),
      [listener](const AbstractDelegatePtr& item) { return *item == listener; }) : m_Listeners.begin();
    if (listener && !--m_UsedListeners[listener]) {
      m_UsedListeners.erase(listener);
      if (m_ListenerToRemove.end() != m_ListenerToRemove.find(listener)) {
        m_ListenerToRemove.erase(listener);
        iter = m_Listeners.erase(iter);
        --iter;
      }
    }
    if (listener) {
      ++iter;
      listener = m_Listeners.end() == iter ? nullptr : iter->get();
    } else {
      listener = iter->get();
    }
    if (listener) {
      ++m_UsedListeners[listener];
    }
    m_Mutex.Unlock();
    return listener;
  }

private:
  typedef std::shared_ptr<AbstractDelegate> AbstractDelegatePtr;
  typedef std::list<AbstractDelegatePtr> AbstractDelegateList;

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
  mutable std::set<AbstractDelegate*> m_ListenerToRemove;
  mutable std::map<AbstractDelegate*, size_t> m_UsedListeners;
  mutable AbstractDelegateList m_Listeners;
  mutable itk::SimpleFastMutexLock m_Mutex;

};

/**
* \brief Event/message/notification class.
*
* \sa mitk::BinaryThresholdTool
* \sa QmitkBinaryThresholdToolGUI
*
* This totally ITK, Qt, VTK, whatever toolkit independent class
* allows one class to send out messages and another class to
* receive these message. This class is templated over the
* return type (A) of the callback functions.
* There are variations of this class
* (Message1, Message2, etc.) for sending
* one, two or more parameters along with the messages.
*
* This is an implementation of the Observer pattern.
*
* \li There is no guarantee about the order of which observer is notified first. At the moment the observers which register first will be notified first.
* \li Notifications are <b>synchronous</b>, by direct method calls. There is no support for asynchronous messages.
*
* To conveniently add methods for registering/unregistering observers
* to Message variables of your class, you can use the mitkNewMessageMacro
* macros.
*
* Here is an example how to use the macros and templates:
*
* \code
*
* // An object to be send around
* class Law
* {
*   private:
*     std::string m_Description;
*
*   public:
*
*     Law(const std::string law) : m_Description(law)
*     { }
*
*     std::string GetDescription() const
*     {
*       return m_Description;
*     }
* };
*
* // The NewtonMachine will issue specific events
* class NewtonMachine
* {
*   mitkNewMessageMacro(AnalysisStarted);
*   mitkNewMessage1Macro(AnalysisStopped, bool);
*   mitkNewMessage1Macro(LawDiscovered, const Law&);
*
*   public:
*
*     void StartAnalysis()
*     {
*       // send the "started" signal
*       m_AnalysisStartedMessage();
*
*       // we found a new law of nature by creating one :-)
*       Law massLaw("F=ma");
*       m_LawDiscoveredMessage(massLaw);
*     }
*
*     void StopAnalysis()
*     {
*       // send the "stop" message with false, indicating
*       // that no error occured
*       m_AnalysisStoppedMessage(false);
*     }
* };
*
* class Observer
* {
*   private:
*
*     NewtonMachine* m_Machine;
*
*   public:
*
*     Observer(NewtonMachine* machine) : m_Machine(machine)
*     {
*       // Add "observers", i.e. function pointers to the machine
*       m_Machine->AddAnalysisStartedListener(
*         ::mitk::MessageDelegate<Observer>(this, &Observer::MachineStarted));
*       m_Machine->AddAnalysisStoppedListener(
*         ::mitk::MessageDelegate1<Observer, bool>(this, &Observer::MachineStopped));
*       m_Machine->AddLawDiscoveredListener(
*         ::mitk::MessageDelegate1<Observer, const Law&>(this, &Observer::LawDiscovered));
*      }
*
*     ~Observer()
*     {
*       // Always remove your observers when finished
*       m_Machine->RemoveAnalysisStartedListener(
*         ::mitk::MessagDelegate<Observer>(this, &Observer::MachineStarted));
*       m_Machine->RemoveAnalysisStoppedListener(
*         ::mitk::MessageDelegate1<Observer, bool>(this, &Observer::MachineStopped));
*       m_Machine->RemoveLawDiscoveredListener(
*         ::mitk::MessageDelegate1<Observer, const Law&>(this, &Observer::LawDiscovered));
*      }
*
*      void MachineStarted()
*      {
*        std::cout << "Observed machine has started" << std::endl;
*      }
*
*      void MachineStopped(bool error)
*      {
*        std::cout << "Observed machine stopped " << (error ? "with an error" : "") << std::endl;
*      }
*
*      void LawDiscovered(const Law& law)
*      {
*        std::cout << "New law of nature discovered: " << law.GetDescription() << std::endl;
*      }
*  };
*
*  NewtonMachine newtonMachine;
*  Observer observer(&newtonMachine);
*
*  // This will send two events to registered observers
*  newtonMachine.StartAnalysis();
*  // This will send one event to registered observers
*  newtonMachine.StopAnalysis();
*
* \endcode
*
* Another example of how to use these message classes can be
* found in the directory Testing, file mitkMessageTest.cpp
*
*/
template<typename A = void>
class Message : public MessageBase< MessageAbstractDelegate<A> >
{
public:
  typedef MessageBase<MessageAbstractDelegate<A>> MessageBaseType;

  void Send()
  {
    for (auto listener = MessageBaseType::GetListener(); listener; listener = MessageBaseType::GetListener(listener)) {
      listener->Execute();
    }
  }

  void operator()()
  {
    this->Send();
  }
};

// message with 1 parameter and return type
template <typename T, typename A = void>
class Message1 : public MessageBase< MessageAbstractDelegate1<T, A> >
{
public:
  typedef MessageBase<MessageAbstractDelegate1<T, A>> MessageBaseType;

  void Send(T t)
  {
    for (auto listener = MessageBaseType::GetListener(); listener; listener = MessageBaseType::GetListener(listener)) {
      listener->Execute(t);
    }
  }

  void operator()(T t)
  {
    this->Send(t);
  }
};

// message with 2 parameters and return type
template <typename T, typename U, typename A = void>
class Message2 : public MessageBase< MessageAbstractDelegate2<T, U, A> >
{
public:
  typedef MessageBase<MessageAbstractDelegate2<T, U, A>> MessageBaseType;

  void Send(T t, U u)
  {
    for (auto listener = MessageBaseType::GetListener(); listener; listener = MessageBaseType::GetListener(listener)) {
      listener->Execute(t, u);
    }
  }

  void operator()(T t, U u)
  {
    this->Send(t, u);
  }
};

// message with 3 parameters and return type
template <typename T, typename U, typename V, typename A = void>
class Message3 : public MessageBase< MessageAbstractDelegate3<T, U, V, A> >
{
public:
  typedef MessageBase<MessageAbstractDelegate3<T, U, V, A>> MessageBaseType;

  void Send(T t, U u, V v)
  {
    for (auto listener = MessageBaseType::GetListener(); listener; listener = MessageBaseType::GetListener(listener)) {
      listener->Execute(t, u, v);
    }
  }

  void operator()(T t, U u, V v)
  {
    this->Send(t, u, v);
  }
};

// message with 4 parameters and return type
template <typename T, typename U, typename V, typename W, typename A = void>
class Message4 : public MessageBase< MessageAbstractDelegate4<T, U, V, W> >
{
public:
  typedef MessageBase<MessageAbstractDelegate4<T, U, V, W, A>> MessageBaseType;

  void Send(T t, U u, V v, W w)
  {
    for (auto listener = MessageBaseType::GetListener(); listener; listener = MessageBaseType::GetListener(listener)) {
      listener->Execute(t, u, v, w);
    }
  }

  void operator()(T t, U u, V v, W w)
  {
    this->Send(t, u, v, w);
  }
};

} // namespace

#endif
