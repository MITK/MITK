/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSinglePointDataInteractor_h
#define mitkSinglePointDataInteractor_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkSmartPointer.h>
#include <mitkCommon.h>
#include <mitkPointSetDataInteractor.h>
#include <MitkCoreExports.h>
#include <mitkPointSet.h>

namespace mitk
{
  /**
   * \brief Implementation of the single point interaction.
   *
   * Interactor operates on a single point set. When a data node is set, its
   * containing point set is cleared for initialization. Instead of adding new
   * points, the first point's position is updated. All other interaction
   * (move, delete) is handled by the base class PointSetDataInteractor.
   *
   * \sa PointSetDataInteractor
   */
  class MITKCORE_EXPORT SinglePointDataInteractor : public PointSetDataInteractor
  {
  public:
    mitkClassMacro(SinglePointDataInteractor, PointSetDataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      protected : SinglePointDataInteractor();
    ~SinglePointDataInteractor() override;

    /** \brief Add or move the single point at the given coordinates.
     *
     * Overwrites PointSetDataInteractor::AddPoint() so that instead of adding
     * new points, the first point's position is updated. All other interaction
     * (move, delete) is still handled by PointSetDataInteractor.
     */
    void AddPoint(StateMachineAction *, InteractionEvent *event) override;

    /** \brief Overridden to enforce a maximum of one point.
     *
     * This method is a no-op by design, since this class always operates
     * on exactly one point.
     */
    virtual void SetMaxPoints(unsigned int maxNumber = 0);

    /** \brief Called when the data node changes.
     *
     * Ensures the data node contains a point set. If one exists, it is cleared;
     * otherwise a new empty PointSet is created and assigned.
     */
    void DataNodeChanged() override;
  };
}
#endif
