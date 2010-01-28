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

#ifndef __SeedsInteractor_H
#define __SeedsInteractor_H

#include <mitkCommon.h>
#include "MitkExtExports.h"
#include <mitkInteractor.h>
#include <mitkVector.h>
#include <mitkImageAccessByItk.h>//needed by QmitkSimplexMeshes (SMDeformation.cpp and LungSegmentation.cpp)
#include <mitkDataTreeNode.h>
#include <mitkSeedsImage.h>


namespace mitk {

  //##Documentation
  //## @brief SeedsInteractor handles all actions on the seedsimage
  //## @ingroup Interaction
  class MitkExt_EXPORT SeedsInteractor : public Interactor
  {
  public:
    mitkClassMacro(SeedsInteractor, Interactor);
    mitkNewMacro2Param(Self, const char*, DataTreeNode*);

    /// sets the radius of the seeds.
    void SetRadius(int val){m_Radius=val;};
    void SetCurrentDrawState(int val){m_CurrentDrawState = val;};

    itkSetMacro(Config, unsigned int);
    itkGetMacro(Config, unsigned int);

  protected:
    /**
    * @brief Default Constructor
    **/
    SeedsInteractor(const char * type, DataTreeNode* dataTreeNode);
    /**
    * @brief Default Destructor
    **/
    virtual ~SeedsInteractor();

    virtual bool ExecuteAction(Action* action, StateEvent const* stateEvent);

  protected:
    SeedsImage::Pointer m_SeedsImage;
    SeedsImage::Pointer m_LastSeedsImage;
    Point3D event_point;
    Point3D last_point;

    int m_Radius;
    int m_DrawState;
    int m_CurrentDrawState;
    unsigned int m_Config;     // determine whether 254,255 or smoothed float values are used.
  };

}
#endif //__SeedsInteractor_H
