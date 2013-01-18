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

#ifndef mitkPointSetDataInteractor_h_
#define mitkPointSetDataInteractor_h_

#include "itkObject.h"
#include "itkSmartPointer.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include <MitkExports.h>
#include "mitkDataInteractor.h"
#include <mitkPointSet.h>

namespace mitk
{
  /**
   * Class PointSetDataInteractor
   * \brief Implementation of the PointSetInteractor using the new interaction concept
   *
   *TODO rest of expl
   */

  class MITK_CORE_EXPORT PointSetDataInteractor: public DataInteractor
  {

  public:
    mitkClassMacro(PointSetDataInteractor, DataInteractor);
    itkNewMacro(Self);

  protected:
    PointSetDataInteractor();
    virtual ~PointSetDataInteractor();
    virtual void ConnectActionsAndFunctions();

  private:
    bool AddPoint(StateMachineAction* , InteractionEvent*);
    bool RemovePoint(StateMachineAction* , InteractionEvent*);
    bool CloseContourPoint(StateMachineAction* , InteractionEvent*);
    bool MovePoint(StateMachineAction* , InteractionEvent*);
    bool MoveSet(StateMachineAction* , InteractionEvent*);
    bool SelectPoint(StateMachineAction* , InteractionEvent*);
    bool UnSelectPoint(StateMachineAction* , InteractionEvent*);
    bool Abort(StateMachineAction* , InteractionEvent*);

    PointSet::Pointer m_PointSet;
    int m_NumberOfPoints;
    int m_SelectedPointIndex;
  };

}
#endif
