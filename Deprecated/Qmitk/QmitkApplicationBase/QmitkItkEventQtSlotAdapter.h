/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#ifndef QMITKITKEVENTQTSLOTADAPTER_H_INCLUDED
#define QMITKITKEVENTQTSLOTADAPTER_H_INCLUDED

#include <itkCommand.h>

/**
  \brief Connects an itk-Event to a member function.

  This class allows you to connect an itk-Event to one of your "void memberfunction()".
  Originally this was intended for use with Qt slots, but it should work with any object, not
  only QObjects. To create a connection you just create a new object of this class:
@code
    new QItkEventQtSlotAdapter<YourListenerClass>(m_ItkObjectPointer, itk::SomeEvent(), this, &YourListenerClass::aMemberFunction)
@endcode
  When you don't need to listen to the itk-Event, then just delete your object.
*/
template <class T>
class QItkEventQtSlotAdapter
{
  public:
    
  QItkEventQtSlotAdapter(itk::Object* itkObject, const itk::EventObject& itkEvent, 
                         T* qtObject, void (T::*qtSlot)())
    : m_itkObject(itkObject),
      m_itkEvent(itkEvent),
      m_qtObject(qtObject),
      m_qtSlot(qtSlot)
  {
    m_CommandConst = itk::MemberCommand<QItkEventQtSlotAdapter>::New();
    m_CommandConst->SetCallbackFunction(this, &QItkEventQtSlotAdapter::CallbackConst);
    m_ConstObserverTag = m_itkObject->AddObserver( m_itkEvent, m_CommandConst );
    
    m_CommandNonConst = itk::MemberCommand<QItkEventQtSlotAdapter>::New();
    m_CommandNonConst->SetCallbackFunction(this, &QItkEventQtSlotAdapter::CallbackNonConst);
    m_NonConstObserverTag = m_itkObject->AddObserver( m_itkEvent, m_CommandNonConst );
  }

  ~QItkEventQtSlotAdapter()
  {
    m_itkObject->RemoveObserver(m_ConstObserverTag);
    m_itkObject->RemoveObserver(m_NonConstObserverTag);
  }

  void CallbackNonConst(itk::Object*, const itk::EventObject&)
  {
    ((*m_qtObject).*(m_qtSlot))(); // call qt slot
  }

  void CallbackConst(const itk::Object*, const itk::EventObject&)
  {
    ((*m_qtObject).*(m_qtSlot))(); // call qt slot
  }

  private:
    itk::Object* m_itkObject;
    const itk::EventObject& m_itkEvent;
    T* m_qtObject;
    void (T::*m_qtSlot)();
    typename itk::MemberCommand<QItkEventQtSlotAdapter>::Pointer m_CommandConst;
    typename itk::MemberCommand<QItkEventQtSlotAdapter>::Pointer m_CommandNonConst;
    unsigned long m_ConstObserverTag;
    unsigned long m_NonConstObserverTag;

    QItkEventQtSlotAdapter(QItkEventQtSlotAdapter&); // hide copy constructor
    void operator=(const QItkEventQtSlotAdapter&);   // hide operator=
};

#endif
