/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-04-18 20:20:25 +0200 (Sa, 18 Apr 2009) $
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKAFFINEINTERACTOR3D_H_HEADER_INCLUDED
#define MITKAFFINEINTERACTOR3D_H_HEADER_INCLUDED

#include "mitkInteractor.h"
#include "MitkExtExports.h"
#include "mitkCommon.h"
#include "mitkSurface.h"

#include <vtkPolyData.h>
#include <vtkType.h>

namespace mitk
{

class DataNode;

/**
  * \brief Affine interaction with objects in 3D windows.
  *
  * NOTE: The interaction mechanism is similar to that of vtkPlaneWidget
  *
  * \ingroup Interaction
  */
class MitkExt_EXPORT AffineInteractor3D : public Interactor
{
public:
  enum { INTERACTION_MODE_TRANSLATION, INTERACTION_MODE_ROTATION };

  mitkClassMacro(AffineInteractor3D, Interactor);
  mitkNewMacro3Param(Self, const char *, DataNode *, int);
  mitkNewMacro2Param(Self, const char *, DataNode *);


  void SetInteractionMode( unsigned int interactionMode );
  void SetInteractionModeToTranslation();
  void SetInteractionModeToRotation();
  unsigned int GetInteractionMode() const;

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
  AffineInteractor3D(const char *type, 
    DataNode *dataTreeNode, int n = -1);

  /**
    * \brief Default Destructor
    **/
  virtual ~AffineInteractor3D();

  virtual bool ExecuteAction( Action* action, 
    mitk::StateEvent const* stateEvent );

  bool ColorizeSurface( vtkPolyData *polyData, const Point3D &pickedPoint,
    double scalar = 0.0 );


private:

  /** \brief to store the value of precision to pick a point */
  ScalarType m_Precision;

  bool m_InteractionMode;

  Point3D m_InitialPickedPoint;
  Point2D m_InitialPickedDisplayPoint;
  vtkFloatingPointType m_InitialPickedPointWorld[4];

  Point3D m_CurrentPickedPoint;
  Point2D m_CurrentPickedDisplayPoint;
  vtkFloatingPointType m_CurrentPickedPointWorld[4];

  Geometry3D::Pointer m_Geometry;

  Geometry3D::Pointer m_OriginalGeometry;

  Vector3D m_ObjectNormal;

};

}

#endif /* MITKAFFINEINTERACTOR3D_H_HEADER_INCLUDED */
