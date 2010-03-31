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

#ifndef FLMITKCOMFORTWIDGET_INCLUDED
#define FLMITKCOMFORTWIDGET_INCLUDED
#include "FLmitkRenderWindow/FLmitkRenderWindow.h"
#include "mitkSliceNavigationController.h"
#include "mitkDataNode.h"
class FLmitkComfortWidget : public FLmitkRenderWindow {
  public:
    FLmitkComfortWidget(int x,int y,int w,int h,const char* label=NULL);

    void SetNode(mitk::DataNode::Pointer node) {m_Node = node;}
    int handle(int event);
    mitk::SliceNavigationController::Pointer sliceCtrl;
  int m_LastX,m_LastY;
    mitk::DataNode::Pointer m_Node;     
};
#endif
