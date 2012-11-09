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

#ifndef FLMITKCOMFORTWIDGET_INCLUDED
#define FLMITKCOMFORTWIDGET_INCLUDED
#include "FLmitkRenderWindow/FLmitkRenderWindow.h"
#include "mitkSliceNavigationController.h"
class FLmitkComfortWidget : public FLmitkRenderWindow {
  public:
    FLmitkComfortWidget(int x,int y,int w,int h,const char* label=NULL) : FLmitkRenderWindow(x,y,w,h,label) {

  }

    mitk::SliceNavigationController::Pointer sliceCtrl;
}
#endif
