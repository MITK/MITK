/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef OPERATION_H_HEADER_INCLUDED_C16E7D9E
#define OPERATION_H_HEADER_INCLUDED_C16E7D9E

#include <MitkCoreExports.h>
#include <itkEventObject.h>

#include <mitkCommon.h>

namespace mitk
{
  typedef int OperationType;

  //##Documentation
  //## @brief Base class of all Operation-classes
  //##
  //## @ingroup Undo
  class MITKCORE_EXPORT Operation
  {
  public:
    mitkClassMacroNoParent(Operation)

      //##Documentation
      //## Constructor
      Operation(OperationType operationType);

    virtual ~Operation();

    OperationType GetOperationType();

  protected:
    OperationType m_OperationType;
  };

  class MITKCORE_EXPORT OperationEndEvent : public itk::EndEvent
  {
  public:
    typedef OperationEndEvent Self;
    typedef itk::EndEvent Superclass;
    OperationEndEvent(Operation *operation = nullptr) : m_Operation(operation) {}
    ~OperationEndEvent() override {}
    const char *GetEventName() const override { return "OperationEndEvent"; }
    bool CheckEvent(const ::itk::EventObject *e) const override { return dynamic_cast<const Self *>(e); }
    ::itk::EventObject *MakeObject() const override { return new Self(m_Operation); }
    Operation *GetOperation() const { return m_Operation; }
  private:
    Operation *m_Operation;
    OperationEndEvent(const Self &);
    void operator=(const Self &);
  };

} // namespace mitk
#endif /* OPERATION_H_HEADER_INCLUDED_C16E7D9E */
