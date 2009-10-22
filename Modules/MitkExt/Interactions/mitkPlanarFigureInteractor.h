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


#ifndef MITKPLANARFIGUREINTERACTOR_H_HEADER_INCLUDED
#define MITKPLANARFIGUREINTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkVector.h"
#include <mitkInteractor.h>

namespace mitk
{

class DataTreeNode;
class Geometry2D;
class DisplayGeometry;
class PlanarFigure;

/**
  * \brief Interaction with mitk::PlanarFigure objects via control-points
  *
  * \ingroup Interaction
  */
class MITKEXT_CORE_EXPORT PlanarFigureInteractor : public Interactor
{
public:
  mitkClassMacro(PlanarFigureInteractor, Interactor);
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
  PlanarFigureInteractor(const char *type, 
    DataTreeNode *dataTreeNode, int n = -1);

  /**
    * \brief Default Destructor
    **/
  virtual ~PlanarFigureInteractor();

  virtual bool ExecuteAction( Action *action, 
    mitk::StateEvent const *stateEvent );

  bool TransformPositionEventToIndex( const StateEvent *stateEvent,
    Point2D &indexPoint2D,
    const Geometry2D *planarFigureGeometry );

  int IsPositionInsideMarker(
    const StateEvent *StateEvent, const PlanarFigure *planarFigure,
    const Geometry2D *planarFigureGeometry,
    const Geometry2D *rendererGeometry,
    const DisplayGeometry *displayGeometry ) const;

private:

  /** \brief to store the value of precision to pick a point */
  ScalarType m_Precision;
};

}

#endif // MITKPLANARFIGUREINTERACTOR_H_HEADER_INCLUDED
