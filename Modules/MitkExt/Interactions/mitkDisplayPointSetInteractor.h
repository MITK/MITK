/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 10709 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKDISPLAYPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF
#define MITKDISPLAYPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkVector.h"
#include <mitkInteractor.h>
#include "mitkPointSetInteractor.h"
#include <mitkBaseRenderer.h>

namespace mitk
{
  class DataTreeNode;

  /**
   * \brief Interaction with a set of points.
   *
   * Points can be added, removed and moved.
   * \ingroup Interaction
   */
  class MitkExt_EXPORT DisplayPointSetInteractor : public PointSetInteractor
  {
  public:
    mitkClassMacro(DisplayPointSetInteractor, PointSetInteractor);
    mitkNewMacro3Param(Self, const char*, DataTreeNode*, int);
    mitkNewMacro2Param(Self, const char*, DataTreeNode*);

    Point2D GetLastDisplayCoordinates();

    BaseRenderer* GetLastRenderer();


  protected:
    /**
     * \brief Constructor with Param n for limited Set of Points
     *
     * if no n is set, then the number of points is unlimited*
     */
    DisplayPointSetInteractor(const char * type, 
      DataTreeNode* dataTreeNode, int n = -1);

    /**
     * \brief Default Destructor
     **/
    virtual ~DisplayPointSetInteractor();

    virtual bool ExecuteAction( Action* action, 
      mitk::StateEvent const* stateEvent );

    /** \brief last display-coordinates of the point 
     * 
     */
    Point2D m_LastDisplayCoordinates;


    mitk::BaseRenderer* m_LastRenderer;


  };
}
#endif /* MITKDisplayPointSetInteractor_H_HEADER_INCLUDED_C11202FF */
