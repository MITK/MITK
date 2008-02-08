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


#ifndef OPERATION_H_HEADER_INCLUDED_C16E7D9E
#define OPERATION_H_HEADER_INCLUDED_C16E7D9E

#include "mitkCommon.h"
//##ModelId=3E9B07B10043

namespace mitk {
//##ModelId=3EA54FFA00B5
typedef int OperationType ;

//##ModelId=3E5F682C0289
//##Documentation
//## @brief Base class of all Operation-classes
//##
//## @ingroup Undo
class MITK_CORE_EXPORT Operation
{
public:
  //##ModelId=3E7830E70054
  //##Documentation
	//## Constructor
  Operation(OperationType operationType);

  //##ModelId=3F01770A0007
  virtual ~Operation();

  //##ModelId=3E7831B600CA
  OperationType GetOperationType();

  protected:
  //##ModelId=3E5F6A9001E9
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
