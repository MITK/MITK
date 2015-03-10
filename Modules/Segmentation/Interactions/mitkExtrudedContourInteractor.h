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

#ifndef __ExtrudedContourInteractor_H
#define __ExtrudedContourInteractor_H

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include <mitkInteractor.h>
#include <mitkNumericTypes.h>
#include <mitkContour.h>
#include <mitkDataNode.h>

namespace mitk {

  //##Documentation
  //## @brief Interactor for the creation of an mitk::Contour
  //## @ingroup Interaction
  class MITKSEGMENTATION_EXPORT ExtrudedContourInteractor : public mitk::Interactor
  {
  public:
    mitkClassMacro(ExtrudedContourInteractor, Interactor);
    mitkNewMacro2Param(Self, const char*, DataNode*);

    itkGetObjectMacro(Contour, mitk::Contour);
    itkGetObjectMacro(ContourNode, mitk::DataNode);

  protected:
    ExtrudedContourInteractor(const char * type, DataNode* dataNode);
    virtual ~ExtrudedContourInteractor();

    virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent);

    /**
    * entry method for any interaction. Method is called if user
    * presses the left mouse button down.
    */
    virtual void Press (mitk::Point3D& op);

    /**
    * this method is finally called after user release the left mouse button
    */
    virtual void Release (mitk::Point3D& op);

    /**
    * method is called when the user moves the mouse with left mouse button down
    */
    virtual void Move (mitk::Point3D& op);

  protected:
    bool m_Positive;
    bool m_Started;

    mitk::Contour::Pointer m_Contour;
    mitk::DataNode::Pointer m_ContourNode;
  };

}
#endif //__ExtrudedContourInteractor_H
