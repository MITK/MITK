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

#include "itkObject.h"
#include "itkSmartPointer.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include <MitkCoreExports.h>
#include "mitkDataInteractor.h"
#include <mitkPointSet.h>


namespace mitk
{
  class MITK_CORE_EXPORT ExampleInteractor: public DataInteractor
  {
  public:
    mitkClassMacro(ExampleInteractor, DataInteractor);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

  protected:
    ExampleInteractor();
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
    bool AddPoint(StateMachineAction* , InteractionEvent*); // function to add new points
    bool EnoughPoints(StateMachineAction* , InteractionEvent*); // function changes color of pointset to indicate, it is full
    bool CheckPoint(const InteractionEvent* interactionEvent); // function checks if the clicked point is valid

    PointSet::Pointer m_PointSet;
    int m_NumberOfPoints;
    int m_MaximalNumberOfPoints;
  };
}
