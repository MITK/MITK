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


#ifndef OPERATION_H_HEADER_INCLUDED_C16E7D9E
#define OPERATION_H_HEADER_INCLUDED_C16E7D9E

#include <MitkExports.h>
#include <itkEventObject.h>

namespace mitk {
typedef int OperationType ;

//##Documentation
//## @brief Base class of all Operation-classes
//##
//## @ingroup Undo
class MITK_CORE_EXPORT Operation
{
public:
  //##Documentation
  //## Constructor
  Operation(OperationType operationType);

  virtual ~Operation();

  OperationType GetOperationType();

  protected:
  OperationType m_OperationType;
};

class MITK_CORE_EXPORT OperationEndEvent : public itk::EndEvent
{
public:
  typedef OperationEndEvent Self;
  typedef itk::EndEvent Superclass;
  OperationEndEvent(Operation* operation = NULL) :
    m_Operation(operation) {}
  virtual ~OperationEndEvent() {}
  virtual const char * GetEventName() const { return "OperationEndEvent"; }
  virtual bool CheckEvent(const ::itk::EventObject* e) const
    { return dynamic_cast<const Self*>(e); }
  virtual ::itk::EventObject* MakeObject() const
    { return new Self(m_Operation); }
  Operation* GetOperation() const { return m_Operation; }
private:
  Operation* m_Operation;
  OperationEndEvent(const Self&);
  void operator=(const Self&);
};

}//namespace mitk
#endif /* OPERATION_H_HEADER_INCLUDED_C16E7D9E */
