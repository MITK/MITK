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


#ifndef MITKSURFACEDEFORMATIONINTERACTOR3D_H_HEADER_INCLUDED
#define MITKSURFACEDEFORMATIONINTERACTOR3D_H_HEADER_INCLUDED

#include "mitkInteractor.h"
#include "MitkExtExports.h"
#include "mitkCommon.h"

#include <vtkType.h>

class vtkPolyData;

namespace mitk
{

class DataNode;
class Surface;

/**
  * \brief Affine interaction with objects in 3D windows.
  *
  * NOTE: The interaction mechanism is similar to that of vtkPlaneWidget
  *
  * \ingroup Interaction
  */
class MitkExt_EXPORT SurfaceDeformationInteractor3D : public Interactor
{
public:
  mitkClassMacro(SurfaceDeformationInteractor3D, Interactor);
  mitkNewMacro3Param(Self, const char *, DataNode *, int);
  mitkNewMacro2Param(Self, const char *, DataNode *);

  /** \brief Sets the amount of precision */
  void SetPrecision( ScalarType precision );

  /**
    * \brief calculates how good the data, this statemachine handles, is hit
    * by the event.
    *
    * overwritten, cause we don't look at the boundingbox, we look at each point
    */
  virtual float CanHandleEvent(StateEvent const *stateEvent) const;


protected:
  /**
    * \brief Constructor with Param n for limited Set of Points
    *
    * if no n is set, then the number of points is unlimited*
    */
  SurfaceDeformationInteractor3D(const char *type,
    DataNode *dataNode, int n = -1);

  /**
    * \brief Default Destructor
    **/
  virtual ~SurfaceDeformationInteractor3D();

  virtual bool ExecuteAction( Action* action,
    mitk::StateEvent const* stateEvent );

  enum
  {
    COLORIZATION_GAUSS,
    COLORIZATION_CONSTANT
  };

  bool ColorizeSurface( vtkPolyData *polyData, const Point3D &pickedPoint,
    int mode, double scalar = 0.0 );


private:

  /** \brief to store the value of precision to pick a point */
  ScalarType m_Precision;

  Point3D m_InitialPickedPoint;
  Point2D m_InitialPickedDisplayPoint;
  double m_InitialPickedPointWorld[4];

  Point3D m_CurrentPickedPoint;
  Point2D m_CurrentPickedDisplayPoint;
  double m_CurrentPickedPointWorld[4];

  Point3D m_SurfaceColorizationCenter;

  Vector3D m_ObjectNormal;

  BaseGeometry::Pointer m_Geometry;

  Surface *m_Surface;
  vtkPolyData *m_PolyData;

  DataNode *m_PickedSurfaceNode;
  Surface *m_PickedSurface;
  vtkPolyData *m_PickedPolyData;

  vtkPolyData *m_OriginalPolyData;

  double m_GaussSigma;

};

}

#endif /* MITKSURFACEDEFORMATIONINTERACTOR3D_H_HEADER_INCLUDED */
