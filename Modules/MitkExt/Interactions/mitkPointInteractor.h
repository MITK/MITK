/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKPOINTINTERACTOR_H_HEADER_INCLUDED
#define MITKPOINTINTERACTOR_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include <mitkInteractor.h>
#include <mitkVector.h>

namespace mitk
{
  class DataNode;

  //##Documentation
  //## @brief Interaction with a point
  //##
  //## Interact with a point: set point, select point, move point and remove point
  //## All Set-operations are done through the method "ExecuteAction".
  //## the identificationnumber of this point is set by this points and evalued from an empty place in the DataStructure
  //## @ingroup Interaction
  class MitkExt_EXPORT PointInteractor : public Interactor
  {
  public:
    mitkClassMacro(PointInteractor, Interactor);
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
    PointInteractor(const char * type, DataNode* dataTreeNode);

    //##Documentation
    //##@brief Destructor 
    virtual ~PointInteractor();

    //##Documentation
    //## @brief select the point on the given position
    virtual void SelectPoint(int position);

    //##Documentation
    //## @brief unselect all points that exist in mesh
    virtual void DeselectAllPoints();

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

  };
}
#endif /* MITKPOINTINTERACTOR_H_HEADER_INCLUDED */
