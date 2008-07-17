/*=========================================================================

Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef CHERRYMESSAGE_H_
#define CHERRYMESSAGE_H_

#include <vector>

namespace cherry {

class MessageCommand
{
  public:

    virtual ~MessageCommand()
    {
    }

    virtual void Execute() = 0;
    virtual bool operator==(const MessageCommand* cmd) = 0;
};


template <typename T>
class MessageCommand1
{
  public:

    virtual ~MessageCommand1()
    {
    }

    virtual void Execute(T t) = 0;
    virtual bool operator==(const MessageCommand1* cmd) = 0;
};

//template <typename T, typename U>
//class MessageCommand2
//{
//  public:
//
//    virtual ~MessageCommand2()
//    {
//    }
//
//    virtual void Execute(T t, U u) = 0;
//    virtual bool operator==(const MessageCommand2* cmd) = 0;
//};

template <typename A, typename T, typename U>
class MessageCommand2
{
  public:

    virtual ~MessageCommand2()
    {
    }

    virtual A Execute(T t, U u) const = 0;
    virtual bool operator==(const MessageCommand2* cmd) = 0;
};

template <class R>
class TMessageCommand : public MessageCommand
{
  public:

    // constructor - takes pointer to an object and pointer to a member and stores
    // them in two private variables
    TMessageCommand(R* object, void(R::*memberFunctionPointer)())
    :m_Object(object),
    m_MemberFunctionPointer(memberFunctionPointer)
    {
    }

    virtual ~TMessageCommand()
    {
    }

    // override function "Call"
    virtual void Execute()
    {
      return (m_Object->*m_MemberFunctionPointer)();    // execute member function
    }

    bool operator==(const MessageCommand* c)
    {
      const TMessageCommand<R>* cmd = dynamic_cast<const TMessageCommand<R>* >(c);
      if (!cmd) return false;

      if ((void*)this->m_Object != (void*)cmd->m_Object) return false;
      if (this->m_MemberFunctionPointer != cmd->m_MemberFunctionPointer) return false;
      return true;
    }

  private:
    R* m_Object;                            // pointer to object
    void (R::*m_MemberFunctionPointer)();   // pointer to member function
};


template <class R, typename T>
class TMessageCommand1 : public MessageCommand1<T>
{
  public:

    // constructor - takes pointer to an object and pointer to a member and stores
    // them in two private variables
    TMessageCommand1(R* object, void(R::*memberFunctionPointer)(T))
    :m_Object(object),
    m_MemberFunctionPointer(memberFunctionPointer)
    {
    }

    virtual ~TMessageCommand1()
    {
    }

    // override function "Call"
    virtual void Execute(T t)
    {
      return (m_Object->*m_MemberFunctionPointer)(t);    // execute member function
    }

    bool operator==(const MessageCommand1<T>* c)
    {
      const TMessageCommand1<R,T>* cmd = dynamic_cast<const TMessageCommand1<R,T>* >(c);
      if (!cmd) return false;

      if ((void*)this->m_Object != (void*)cmd->m_Object) return false;
      if (this->m_MemberFunctionPointer != cmd->m_MemberFunctionPointer) return false;
      return true;
    }

  private:
    R* m_Object;                            // pointer to object
    void (R::*m_MemberFunctionPointer)(T);   // pointer to member function
};

//template <class R, typename T, typename U>
//class TMessageCommand2 : public MessageCommand2<T,U>
//{
//  public:
//
//    // constructor - takes pointer to an object and pointer to a member and stores
//    // them in two private variables
//    TMessageCommand2(R* object, void(R::*memberFunctionPointer)(T, U))
//    :m_Object(object),
//    m_MemberFunctionPointer(memberFunctionPointer)
//    {
//    }
//
//    virtual ~TMessageCommand2()
//    {
//    }
//
//    // override function "Call"
//    virtual void Execute(T t, U u)
//    {
//      return (m_Object->*m_MemberFunctionPointer)(t,u);             // execute member function
//    }
//
//    bool operator==(const MessageCommand2<T,U>* c)
//    {
//      const TMessageCommand2<R,T,U>* cmd = dynamic_cast<const TMessageCommand2<R,T,U>* >(c);
//      if (!cmd) return false;
//
//      if ((void*)this->m_Object != (void*)cmd->m_Object) return false;
//      if (this->m_MemberFunctionPointer != cmd->m_MemberFunctionPointer) return false;
//      return true;
//    }
//
//  private:
//    R* m_Object;                            // pointer to object
//    void (R::*m_MemberFunctionPointer)(T, U);   // pointer to member function
//};

template <typename A, class R, typename T, typename U>
class TMessageCommand2 : public MessageCommand2<A,T,U>
{
  public:

    // constructor - takes pointer to an object and pointer to a member and stores
    // them in two private variables
    TMessageCommand2(R* object, A(R::*memberFunctionPointer)(T, U))
    :m_Object(object),
    m_MemberFunctionPointer(memberFunctionPointer)
    {
    }

    virtual ~TMessageCommand2()
    {
    }

    // override function "Call"
    virtual A Execute(T t, U u) const
    {
      return (m_Object->*m_MemberFunctionPointer)(t,u);             // execute member function
    }

    bool operator==(const MessageCommand2<A,T,U>* c)
    {
      const TMessageCommand2<A,R,T,U>* cmd = dynamic_cast<const TMessageCommand2<A,R,T,U>* >(c);
      if (!cmd) return false;

      if ((void*)this->m_Object != (void*)cmd->m_Object) return false;
      if (this->m_MemberFunctionPointer != cmd->m_MemberFunctionPointer) return false;
      return true;
    }

  private:
    R* m_Object;                            // pointer to object
    A (R::*m_MemberFunctionPointer)(T, U);   // pointer to member function
};

// message without parameters (pure signals)
class Message
{
  public:

    typedef Message Self;

    ~Message() {
      for (ListenerList::iterator iter = m_Listeners.begin();
           iter != m_Listeners.end(); ++iter )
      {
        delete *iter;
      }
    }

    template <class R>
    void AddListener( R* const receiver, void(R::*fnptr)() ) const
    {
      TMessageCommand<R>* msgCmd = new TMessageCommand<R>(receiver, fnptr);
      for (typename ListenerList::iterator iter = m_Listeners.begin();
           iter != m_Listeners.end();
           ++iter )
      {
        if ((*iter)->operator==(msgCmd)) {
          delete msgCmd;
          return;
        }
      }
      m_Listeners.push_back(msgCmd);
    }

    template <class R>
    void RemoveListener( R* receiver, void(R::*fnptr)() ) const
    {
      // remove handler
      TMessageCommand<R> msgCmd(receiver, fnptr);
      for (typename ListenerList::iterator iter = m_Listeners.begin();
           iter != m_Listeners.end();
           ++iter )
      {
        if ((*iter)->operator==(&msgCmd))
        {
          delete *iter;
          m_Listeners.erase( iter );
          return;
        }
      }
    }

    void Send()
    {
      for ( ListenerList::iterator iter = m_Listeners.begin();
            iter != m_Listeners.end();
            ++iter )
      {
        // notify each listener
        (*iter)->Execute();
      }
    }

    void operator()()
    {
      this->Send();
    }

  protected:

    typedef std::vector<MessageCommand* > ListenerList;

    mutable ListenerList m_Listeners;

};


// message with 1 parameter
template <typename T>
class Message1
{
  public:

    typedef Message1 Self;

    ~Message1() {
      for (typename ListenerList::iterator iter = m_Listeners.begin();
           iter != m_Listeners.end(); ++iter )
      {
        delete *iter;
      }
    }

    template <class R>
    void AddListener( R* const receiver, void(R::*fnptr)(T) ) const
    {
      TMessageCommand1<R,T>* msgCmd = new TMessageCommand1<R, T>(receiver, fnptr);
      for (typename ListenerList::iterator iter = m_Listeners.begin();
           iter != m_Listeners.end();
           ++iter )
      {
        if ((*iter)->operator==(msgCmd)) {
          delete msgCmd;
          return;
        }
      }
      m_Listeners.push_back(msgCmd);
    }

    template <class R>
    void RemoveListener( R* receiver, void(R::*fnptr)(T) ) const
    {
      // remove handler
      TMessageCommand1<R,T> msgCmd(receiver, fnptr);
      for (typename ListenerList::iterator iter = m_Listeners.begin();
           iter != m_Listeners.end();
           ++iter )
      {
        if ((*iter)->operator==(&msgCmd))
        {
          delete *iter;
          m_Listeners.erase( iter );
          return;
        }
      }
    }

    void Send(T t)
    {
      for ( typename ListenerList::iterator iter = m_Listeners.begin();
            iter != m_Listeners.end();
            ++iter )
      {
        // notify each listener
        (*iter)->Execute(t);
      }
    }

    void operator()(T t)
    {
      this->Send(t);
    }

  protected:

    typedef std::vector<MessageCommand1<T>* > ListenerList;

    mutable ListenerList m_Listeners;

};


//// message with 2 parameters
//template <typename T, typename U>
//class Message2
//{
//  public:
//
//    typedef Message2 Self;
//
//    ~Message2() {
//      for (typename ListenerList::iterator iter = m_Listeners.begin();
//           iter != m_Listeners.end(); ++iter )
//      {
//        delete *iter;
//      }
//    }
//
//    template <class R>
//    void AddListener( R* receiver, void(R::*fnptr)(T,U) ) const
//    {
//      TMessageCommand2<R,T,U>* msgCmd = new TMessageCommand2<R, T, U>(receiver, fnptr);
//      for (typename ListenerList::iterator iter = m_Listeners.begin();
//           iter != m_Listeners.end();
//           ++iter )
//      {
//        if ((*iter)->operator==(msgCmd)) {
//          delete msgCmd;
//          return;
//        }
//      }
//      m_Listeners.push_back(msgCmd);
//    }
//
//    template <class R>
//    void RemoveListener( R* receiver, void(R::*fnptr)(T,U) ) const
//    {
//      // remove handler
//      TMessageCommand2<R,T,U> msgCmd(receiver, fnptr);
//      for (typename ListenerList::iterator iter = m_Listeners.begin();
//           iter != m_Listeners.end();
//           ++iter )
//      {
//        if ((*iter)->operator==(&msgCmd))
//        {
//          delete *iter;
//          m_Listeners.erase( iter );
//          return;
//        }
//      }
//    }
//
//    void Send(T t, U u)
//    {
//      for ( typename ListenerList::iterator iter = m_Listeners.begin();
//            iter != m_Listeners.end();
//            ++iter )
//      {
//        // notify each listener
//        (*iter)->Execute(t,u);
//      }
//    }
//
//    void operator()(T t, U u)
//    {
//      this->Send(t, u);
//    }
//
//
//  protected:
//
//    typedef std::vector<MessageCommand2<T,U>* > ListenerList;
//
//    mutable ListenerList m_Listeners;
//
//};

// message with 2 parameters and return type
template <typename T, typename U, typename A = void>
class Message2
{
  public:

    typedef Message2 Self;
    typedef std::vector<MessageCommand2<A,T,U>* > ListenerList;

    ~Message2() {
      for (typename ListenerList::iterator iter = m_Listeners.begin();
           iter != m_Listeners.end(); ++iter )
      {
        delete *iter;
      }
    }

    template <class R>
    void AddListener( R* receiver, A(R::*fnptr)(T,U) ) const
    {
      TMessageCommand2<A,R,T,U>* msgCmd = new TMessageCommand2<A, R, T, U>(receiver, fnptr);
      for (typename ListenerList::iterator iter = m_Listeners.begin();
           iter != m_Listeners.end();
           ++iter )
      {
        if ((*iter)->operator==(msgCmd)) {
          delete msgCmd;
          return;
        }
      }
      m_Listeners.push_back(msgCmd);
    }

    template <class R>
    void RemoveListener( R* receiver, A(R::*fnptr)(T,U) ) const
    {
      // remove handler
      TMessageCommand2<A,R,T,U> msgCmd(receiver, fnptr);
      for (typename ListenerList::iterator iter = m_Listeners.begin();
           iter != m_Listeners.end();
           ++iter )
      {
        if ((*iter)->operator==(&msgCmd))
        {
          delete *iter;
          m_Listeners.erase( iter );
          return;
        }
      }
    }

    void Send(T t, U u)
    {
      for ( typename ListenerList::iterator iter = m_Listeners.begin();
            iter != m_Listeners.end();
            ++iter )
      {
        // notify each listener
        (*iter)->Execute(t,u);
      }
    }

    void operator()(T t, U u)
    {
      this->Send(t, u);
    }

    const ListenerList& GetListeners()
    {
      return m_Listeners;
    }


  protected:

    mutable ListenerList m_Listeners;

};

} // namespace cherry

#endif /*CHERRYMESSAGE_H_*/
