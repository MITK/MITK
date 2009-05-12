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


#ifndef MITKAFFINEINTERACTOR_H_HEADER_INCLUDED_C188C29F
#define MITKAFFINEINTERACTOR_H_HEADER_INCLUDED_C188C29F

#include "mitkCommon.h"
#include "mitkInteractor.h"
#include "mitkVector.h"

namespace mitk {
class DisplayPositionEvent;

//##Documentation
//## @brief Interactor for Affine transformations translate, rotate and scale
//##
//## An object of this class can translate, rotate and scale the data objects
//## by modifying its geometry.
//## @ingroup Interaction
class MITK_CORE_EXPORT AffineInteractor : public Interactor
{
public:
  mitkClassMacro(AffineInteractor,Interactor);
//  itkNewMacro(Self);
  mitkNewMacro2Param(Self, const char*, DataTreeNode*);

protected:
//  AffineInteractor(); //obsolete

  //##Documentation
  //## @brief Constructor
  //##
  //## @param dataTreeNode is the node, this Interactor is connected to
  //## @param type is the type of StateMachine like declared in the XML-Configure-File
  AffineInteractor(const char * type, DataTreeNode* dataTreeNode);

  //##Documentation
  //## @brief Destructor
  ~AffineInteractor(){};

  virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent);

  bool CheckSelected(const mitk::Point3D& worldPoint, int timestep);
  bool ConvertDisplayEventToWorldPosition(mitk::DisplayPositionEvent const* displayEvent, mitk::Point3D& worldPoint);

  mitk::Point3D m_LastMousePosition;
};

} // namespace mitk

#endif /* MITKAFFINEINTERACTOR_H_HEADER_INCLUDED_C188C29F */
