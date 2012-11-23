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

#ifndef MITKEVENTINTERACTOR_H_
#define MITKEVENTINTERACTOR_H_

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include <MitkExports.h>
#include "mitkDataNode.h"
#include "mitkEventStateMachine.h"


namespace mitk
{

  class MITK_CORE_EXPORT EventInteractor : public EventStateMachine {

  public:
    mitkClassMacro(EventInteractor, EventStateMachine);
    itkNewMacro(Self);

   bool operator<(const EventInteractor::Pointer eventInteractor);

   void SetDataNode(DataNode::Pointer);

   DataNode::Pointer GetDataNode();
   int GetLayer();

  protected:
    EventInteractor();
    virtual ~EventInteractor();


  private:
    DataNode::Pointer m_DataNode;
  };

} /* namespace mitk */
#endif /* MITKEVENTINTERACTOR_H_ */
