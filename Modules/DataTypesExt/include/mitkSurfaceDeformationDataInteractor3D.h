/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceDeformationDataInteractor3D_h
#define mitkSurfaceDeformationDataInteractor3D_h

#include "MitkDataTypesExtExports.h"
#include "mitkDataInteractor.h"
#include "mitkSurface.h"

namespace mitk
{
  /**
    * \brief SurfaceDeformation interaction with objects in 3D windows.
    *
    * \ingroup Interaction
    */

  // Inherit from DataInteratcor, this provides functionality of a state machine and configurable inputs.
  class MITKDATATYPESEXT_EXPORT SurfaceDeformationDataInteractor3D : public DataInteractor
  {
  public:
    mitkClassMacro(SurfaceDeformationDataInteractor3D, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      protected : SurfaceDeformationDataInteractor3D();
    ~SurfaceDeformationDataInteractor3D() override;
    /**
      * Here actions strings from the loaded state machine pattern are mapped to functions of
      * the DataInteractor. These functions are called when an action from the state machine pattern is executed.
      */
    void ConnectActionsAndFunctions() override;
    /**
      * This function is called when a DataNode has been set/changed.
      */
    void DataNodeChanged() override;

    /**
      * Initializes the movement, stores starting position.
      */
    virtual bool CheckOverObject(const InteractionEvent *);
    virtual void SelectObject(StateMachineAction *, InteractionEvent *);
    virtual void DeselectObject(StateMachineAction *, InteractionEvent *);
    virtual void InitDeformation(StateMachineAction *, InteractionEvent *);
    virtual void DeformObject(StateMachineAction *, InteractionEvent *);
    virtual void ScaleRadius(StateMachineAction *, InteractionEvent *);

    enum
    {
      COLORIZATION_GAUSS,
      COLORIZATION_CONSTANT
    };

  private:
    void ColorizeSurface(
      vtkPolyData *polyData, int timeStep, const Point3D &pickedPoint, int mode, double scalar = 0.0);

    double m_InitialPickedWorldPoint[4];
    Point3D m_InitialPickedPoint;
    Point3D m_SurfaceColorizationCenter;

    Surface *m_Surface;
    vtkPolyData *m_OriginalPolyData;

    double m_GaussSigma;
    Vector3D m_ObjectNormal;
  };
}
#endif
