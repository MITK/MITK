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


#ifndef MITKFiberBundleINTERACTOR_H_HEADER_INCLUDED
#define MITKFiberBundleINTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include <mitkInteractor.h>
#include <mitkNumericTypes.h>
#include <MitkFiberTrackingExports.h>

#include <vtkType.h>

namespace mitk
{
  class DataNode;

  //##Documentation
  //## @brief Just select a point, that's the only way to interact with the point
  //##
  //## Interact with a point: Select the point without moving to get parameters that does not change
  //## All Set-operations would be done through the method "ExecuteAction", if there would be anyone.
  //## the identificationnumber of this point is set by this points and evalued from an empty place in the DataStructure
  //## @ingroup Interaction
  class MitkFiberTracking_EXPORT FiberBundleInteractor : public Interactor
  {
  public:
    mitkClassMacro(FiberBundleInteractor, Interactor);
    mitkNewMacro2Param(Self, const char*, DataNode*);

    //##Documentation
    //## @brief Sets the amount of precision
    void SetPrecision(unsigned int precision);

    //##Documentation
    //## @brief derived from mitk::Interactor; calculates Jurisdiction according to points
    //##
    //## standard method can not be used, since it doesn't calculate in points, only in BoundingBox of Points
    virtual float CanHandleEvent(StateEvent const* stateEvent) const;


  protected:
    //##Documentation
    //##@brief Constructor
    FiberBundleInteractor(const char * type, DataNode* dataNode);

    //##Documentation
    //##@brief Destructor
    virtual ~FiberBundleInteractor();

    //##Documentation
    //## @brief select the point on the given position
    virtual void SelectFiber(int position);

    //##Documentation
    //## @brief unselect all points that exist in mesh
    virtual void DeselectAllFibers();

    //##Documentation
    //## @brief Executes Actions
    virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent);


  private:

    //##Documentation
    //## @brief to calculate a direction vector from last point and actual point
    Point3D m_LastPoint;

    //##Documentation
    //## @brief to store a position
    unsigned int m_LastPosition;

//    Point3D m_InitialPickedPoint;
//    Point2D m_InitialPickedDisplayPoint;
//    double m_InitialPickedPointWorld[4];

    Point3D m_CurrentPickedPoint;
    Point2D m_CurrentPickedDisplayPoint;
    double m_CurrentPickedPointWorld[4];

  };
}
#endif /* MITKFiberBundleInteractor_H_HEADER_INCLUDED */
