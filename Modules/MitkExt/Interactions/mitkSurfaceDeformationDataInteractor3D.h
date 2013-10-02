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

//#include "itkObject.h"
//#include "itkSmartPointer.h"
//#include "itkObjectFactory.h"
#include "mitkBaseRenderer.h"
#include "mitkCommon.h"
#include "mitkDataInteractor.h"
#include "MitkExtExports.h"
#include "mitkGeometry3D.h"
#include "mitkSurface.h"

namespace mitk
{
/**
  * \brief SurfaceDeformation interaction with objects in 3D windows.
  *
  * \ingroup Interaction
  */

// Inherit from DataInteratcor, this provides functionality of a state machine and configurable inputs.
class MitkExt_EXPORT SurfaceDeformationDataInteractor3D: public DataInteractor
{

public:
  mitkClassMacro(SurfaceDeformationDataInteractor3D, DataInteractor);
  itkNewMacro(Self);

  /** \brief Sets the amount of precision */
  void SetPrecision( ScalarType precision );

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
  virtual bool CheckOverObject (const InteractionEvent *);
  virtual bool SelectObject (StateMachineAction*, InteractionEvent*);
  virtual bool DeselectObject (StateMachineAction*, InteractionEvent*);
  virtual bool InitDeformation (StateMachineAction*, InteractionEvent*);
  virtual bool DeformObject (StateMachineAction*, InteractionEvent*);

  enum
  {
    COLORIZATION_GAUSS,
    COLORIZATION_CONSTANT
  };

  bool ColorizeSurface(BaseRenderer::Pointer renderer, vtkPolyData* polyData, const Point3D &pickedPoint,
                       int mode, double scalar = 0.0 );

private:

  /** \brief to store the value of precision to pick a point */
  ScalarType m_Precision;

  Point3D m_InitialPickedPoint;
  Point2D m_InitialPickedDisplayPoint;

  Point3D m_CurrentPickedPoint;
  Point2D m_CurrentPickedDisplayPoint;

  Point3D m_SurfaceColorizationCenter;

  Geometry3D::Pointer m_Geometry;

  Surface* m_Surface;
  vtkPolyData* m_PolyData;

  DataNode* m_PickedSurfaceNode;
  Surface * m_PickedSurface;

  vtkPolyData* m_PickedPolyData;
  vtkPolyData *m_OriginalPolyData;

  double m_GaussSigma;

  Vector3D m_ObjectNormal;
};

}
#endif
