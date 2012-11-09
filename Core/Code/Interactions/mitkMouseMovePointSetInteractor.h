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


#ifndef MITKMOUSEMOVEPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF
#define MITKMOUSEMOVEPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF

#include <MitkExports.h>
#include "mitkVector.h"
#include <mitkPointSetInteractor.h>

namespace mitk
{
  class DataNode;

  /**
  * \brief Interaction with a single point by mouse movement.
  *
  * A new point is added by mouse movement, an existing point will be removed before adding a new one.
  * \ingroup Interaction
  */
  class MITK_CORE_EXPORT MouseMovePointSetInteractor : public PointSetInteractor
  {
  public:
    mitkClassMacro(MouseMovePointSetInteractor, Interactor);
    mitkNewMacro3Param(Self, const char*, DataNode*, int);
    mitkNewMacro2Param(Self, const char*, DataNode*);


    /**
    * \brief calculates how good the data, this statemachine handles, is hit
    * by the event.
    *
    * overwritten, cause we don't look at the boundingbox, we look at each point
    * and want to accept mouse movement for setting points
    */
    virtual float CanHandleEvent(StateEvent const* stateEvent) const;


  protected:
    /**
    * \brief Constructor with Param n for limited Set of Points
    *
    * if no n is set, then the number of points is unlimited*
    */
    MouseMovePointSetInteractor(const char * type, DataNode* dataNode, int n = -1);

    /**
    * \brief Default Destructor
    **/
    virtual ~MouseMovePointSetInteractor();

  private:
  };
}
#endif /* MITKMOUSEMOVEPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF */
