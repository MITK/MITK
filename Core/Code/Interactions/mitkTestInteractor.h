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

#ifndef mitkTestInteractor_h_
#define mitkTestInteractor_h_

#include "itkObject.h"
#include "itkSmartPointer.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include <MitkExports.h>
#include "mitkDataInteractor.h"
#include <mitkPointSet.h>

namespace mitk
{
  /** Temporal Dummy Interactor that only servers during developement
   * to ensure the Interaction Framework does what it needs to do.
   */

  class DataNode;
  class MITK_CORE_EXPORT TestInteractor: public DataInteractor
  {

  public:
    mitkClassMacro(TestInteractor, DataInteractor);
    itkNewMacro(Self);

  protected:
    TestInteractor();
    virtual ~TestInteractor();
    virtual void ConnectActionsAndFunctions();
    virtual bool IsPointerOverData(InteractionEvent*);

  private:
    bool AddPoint(StateMachineAction* , InteractionEvent*);
    bool SelectPoint(StateMachineAction* , InteractionEvent*);
    bool DeSelectPoint(StateMachineAction* , InteractionEvent*);
    bool DeleteSelectedPoint(StateMachineAction* , InteractionEvent*);

    PointSet::Pointer m_PointSet;
    int m_NumberOfPoints;
    int m_SelectedPointIndex;
  };

} /* namespace mitk */
#endif /* mitkTestInteractor_h_ */
