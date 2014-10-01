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

#ifndef mitkSinglePointDataInteractor_h_
#define mitkSinglePointDataInteractor_h_

#include "itkObject.h"
#include "itkSmartPointer.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include <MitkCoreExports.h>
#include "mitkPointSetDataInteractor.h"
#include <mitkPointSet.h>

namespace mitk
{
  /**
   * Class SinglePointDataInteractor
   * \brief Implementation of the single point interaction
   *
   * Interactor operates on a single point set, when a data node is set, its containing point set is clear for initialization.
   */

  // Inherit from DataInteratcor, this provides functionality of a state machine and configurable inputs.
  class MITK_CORE_EXPORT SinglePointDataInteractor: public PointSetDataInteractor
  {

  public:
    mitkClassMacro(SinglePointDataInteractor, PointSetDataInteractor)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

  protected:
    SinglePointDataInteractor();
    virtual ~SinglePointDataInteractor();

    /** Adds a point at the given coordinates.
     *  This function overwrites the behavior of PointSetDataInteractor such that instead of adding new points
     *  the first points position is updated. All other interaction (move,delete) is still handled by PointSetDataInteractor.
     */
    virtual bool AddPoint(StateMachineAction*, InteractionEvent* event);

    /**
     * @brief SetMaxPoints Sets the maximal number of points for the pointset
     * Overwritten, per design this class will always have a maximal number of one.
     * @param maxNumber
     */
    virtual void SetMaxPoints(unsigned int /*maxNumber*/ = 0)
    {}

    virtual void DataNodeChanged();
  };

}
#endif
