/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 13561 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKSURFACEINTERACTOR_H_HEADER_INCLUDED
#define MITKSURFACEINTERACTOR_H_HEADER_INCLUDED

#include "mitkAffineInteractor.h"
#include "MitkExtExports.h"
#include "mitkPointLocator.h"

namespace mitk {

  //##Documentation
  //## @brief Interactor for Affine transformations on one or multiple surfaces
  //##
  //## A surface interacted by this class can translate, rotate and scale
  //## by modifying its geometry. The surface closest to the picking point is
  //## chosen for interaction automatically by CanHandleEvent
  //## @ingroup Interaction
  class MitkExt_EXPORT SurfaceInteractor : public AffineInteractor
  {
  public:
    mitkClassMacro(SurfaceInteractor,AffineInteractor);
    //  itkNewMacro(Self);
    mitkNewMacro2Param(Self, const char*, DataNode*);

  protected:

    //##Documentation
    //## @brief Constructor
    //##
    //## @param dataTreeNode is the node, this Interactor is connected to
    //## @param type is the type of StateMachine like declared in the XML-Configure-File
    SurfaceInteractor(const char * type, DataNode* dataTreeNode);

    //##Documentation
    //## @brief Destructor
    ~SurfaceInteractor(){};

    //##Documentation
    //## @brief calculates how good the data this state machine handles is hit by the event.
    //## 
    //## Returns a value between 0 and 1.
    //## (Used by GlobalInteraction to decide which DESELECTED state machine to send the event to.)
    virtual float CanHandleEvent(StateEvent const* stateEvent) const;

  private:

    typedef PointLocator      PLocType;
    typedef PLocType::Pointer PLPointer;

    PLPointer                  m_PtLoc;

  };

} // namespace mitk

#endif /* MITKSURFACEINTERACTOR_H_HEADER_INCLUDED */
