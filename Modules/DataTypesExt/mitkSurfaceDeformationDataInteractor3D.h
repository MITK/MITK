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

#ifndef mitkSurfaceDeformationDataInteractor3D_h_
#define mitkSurfaceDeformationDataInteractor3D_h_

#include "mitkDataInteractor.h"
#include "MitkDataTypesExtExports.h"
#include "mitkSurface.h"

namespace mitk
{
/**
  * \brief SurfaceDeformation interaction with objects in 3D windows.
  *
  * \ingroup Interaction
  */

// Inherit from DataInteratcor, this provides functionality of a state machine and configurable inputs.
class MitkDataTypesExt_EXPORT SurfaceDeformationDataInteractor3D: public DataInteractor
{

public:
  mitkClassMacro(SurfaceDeformationDataInteractor3D, DataInteractor);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

protected:
  SurfaceDeformationDataInteractor3D();
  virtual ~SurfaceDeformationDataInteractor3D();
  /**
    * Here actions strings from the loaded state machine pattern are mapped to functions of
    * the DataInteractor. These functions are called when an action from the state machine pattern is executed.
    */
  virtual void ConnectActionsAndFunctions();
  /**
    * This function is called when a DataNode has been set/changed.
    */
  virtual void DataNodeChanged();

  /**
    * Initializes the movement, stores starting position.
    */
  virtual bool CheckOverObject (const InteractionEvent*);
  virtual bool SelectObject (StateMachineAction*, InteractionEvent*);
  virtual bool DeselectObject (StateMachineAction*, InteractionEvent*);
  virtual bool InitDeformation (StateMachineAction*, InteractionEvent*);
  virtual bool DeformObject (StateMachineAction*, InteractionEvent*);
  virtual bool ScaleRadius (StateMachineAction*, InteractionEvent*);

  enum
  {
    COLORIZATION_GAUSS,
    COLORIZATION_CONSTANT
  };

 private:

  bool ColorizeSurface(vtkPolyData* polyData, int timeStep, const Point3D& pickedPoint, int mode, double scalar = 0.0);

  double m_InitialPickedWorldPoint[4];
  Point3D m_InitialPickedPoint;
  Point3D m_SurfaceColorizationCenter;

  Surface* m_Surface;
  vtkPolyData* m_OriginalPolyData;

  double m_GaussSigma;
  Vector3D m_ObjectNormal;
};

}
#endif
