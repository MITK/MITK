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
#include "itkSmartPointer.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include <MitkExports.h>
#include "mitkEventStateMachine.h"


namespace mitk
{
  class DataNode;
  class MITK_CORE_EXPORT EventInteractor : public EventStateMachine {

  public:
  typedef itk::SmartPointer<DataNode> NodeType;
    mitkClassMacro(EventInteractor, EventStateMachine);
    itkNewMacro(Self);

   bool operator<(const EventInteractor::Pointer eventInteractor);

   void SetDataNode(NodeType);

   NodeType GetDataNode();
   int GetLayer();

  protected:
    EventInteractor();
    virtual ~EventInteractor();


  private:
    NodeType m_DataNode;
  };

} /* namespace mitk */
#endif /* MITKEVENTINTERACTOR_H_ */
