/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkSmartPointer.h"
#include "mitkCommon.h"
#include "mitkDataInteractor.h"
#include <mitkPointSet.h>

namespace mitk
{
  class ExampleInteractor : public DataInteractor
  {
  public:
    mitkClassMacro(ExampleInteractor, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      protected : ExampleInteractor();
    virtual ~ExampleInteractor();
    virtual void ConnectActionsAndFunctions();
    /**
     * Derived function.
     * Is executed when config object is set / changed.
     * Here it is used to read out the parameters set in the configuration file,
     * and set the member variables accordingly.
     */
    virtual void ConfigurationChanged();

  private:
    bool AddPoint(StateMachineAction *, InteractionEvent *); // function to add new points
    bool EnoughPoints(StateMachineAction *,
                      InteractionEvent *); // function changes color of pointset to indicate, it is full
    bool CheckPoint(const InteractionEvent *interactionEvent); // function checks if the clicked point is valid

    PointSet::Pointer m_PointSet;
    int m_NumberOfPoints;
    int m_MaximalNumberOfPoints;
  };
}
