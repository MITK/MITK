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

#ifndef __ContourInteractor_H
#define __ContourInteractor_H

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include <mitkInteractor.h>
#include <mitkVector.h>

namespace mitk {

  //##Documentation
  //## @brief Interactor for the creation of an mitk::Contour
  //## @ingroup Interaction
  class MitkExt_EXPORT ContourInteractor : public mitk::Interactor
  {
  public:
    mitkClassMacro(ContourInteractor, Interactor);
    mitkNewMacro2Param(Self, const char*, DataNode*);


  protected:
    ContourInteractor(const char * type, DataNode* dataNode);
    virtual ~ContourInteractor();

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
  };

}
#endif //__ContourInteractor_H
