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


#ifndef MITKSURFACEDEFORMATIONINTERACTOR3D_H_HEADER_INCLUDED
#define MITKSURFACEDEFORMATIONINTERACTOR3D_H_HEADER_INCLUDED

#include "mitkInteractor.h"
#include "mitkCommon.h"

#include <vtkType.h>

class vtkPolyData;

namespace mitk
{

class DataTreeNode;
class Surface;

/**
  * \brief Affine interaction with objects in 3D windows.
  *
  * NOTE: The interaction mechanism is similar to that of vtkPlaneWidget
  *
  * \ingroup Interaction
  */
class MITKEXT_CORE_EXPORT SurfaceDeformationInteractor3D : public Interactor
{
public:
  mitkClassMacro(SurfaceDeformationInteractor3D, Interactor);
  mitkNewMacro3Param(Self, const char *, DataTreeNode *, int);
  mitkNewMacro2Param(Self, const char *, DataTreeNode *);

  /** \brief Sets the amount of precision */
  void SetPrecision( ScalarType precision );

  /**
    * \brief calculates how good the data, this statemachine handles, is hit
    * by the event.
    *
    * overwritten, cause we don't look at the boundingbox, we look at each point
    */
  virtual float CalculateJurisdiction(StateEvent const *stateEvent) const;


protected:
  /**
    * \brief Constructor with Param n for limited Set of Points
    *
    * if no n is set, then the number of points is unlimited*
    */
  SurfaceDeformationInteractor3D(const char *type, 
    DataTreeNode *dataTreeNode, int n = -1);

  /**
    * \brief Default Destructor
    **/
  virtual ~SurfaceDeformationInteractor3D();

  virtual bool ExecuteAction( Action* action, 
    mitk::StateEvent const* stateEvent );


private:

  /** \brief to store the value of precision to pick a point */
  ScalarType m_Precision;

  Point3D m_InitialInteractionPickedPoint;

  Point2D m_InitialInteractionPointDisplay;
  vtkFloatingPointType m_InitialInteractionPointWorld[4];

  Point2D m_CurrentInteractionPointDisplay;
  vtkFloatingPointType m_CurrentInteractionPointWorld[4];

  DataTreeNode *m_PickedSurfaceNode;
  Surface *m_PickedSurface;
  vtkPolyData *m_OriginalPolyData;

  double m_GaussSigma;

};

}

#endif /* MITKSURFACEDEFORMATIONINTERACTOR3D_H_HEADER_INCLUDED */
