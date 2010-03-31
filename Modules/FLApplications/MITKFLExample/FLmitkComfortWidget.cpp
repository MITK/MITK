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

#include <Fl/Fl.h>
#include "FLmitkComfortWidget.h"
//#include "mitkLevelWindow.h"
// 
FLmitkComfortWidget::FLmitkComfortWidget(int x,int y,int w,int h,const char* label) : FLmitkRenderWindow(x,y,w,h,label) { 

}
int FLmitkComfortWidget::handle(int event) {
  switch(event) {
    case FL_MOUSEWHEEL:
      if (Fl::event_dy() > 0) {
	sliceCtrl->GetSlice()->Next();
      } else  {
	sliceCtrl->GetSlice()->Previous();
      }
      GetRenderer()->Render();
      draw();
      return 1;
      break;
    case FL_PUSH:
      m_LastX = Fl::event_x();
      m_LastY = Fl::event_y();
      return 1;
      break;
    case FL_DRAG:
      int dx = Fl::event_x() - m_LastX;	
      int dy = Fl::event_y() - m_LastY;	
      m_LastX = Fl::event_x();
      m_LastY = Fl::event_y();
      mitk::LevelWindow lvlWin;
      if (m_Node.IsNotNull() && m_Node->GetLevelWindow(lvlWin,GetRenderer())) {
	lvlWin.SetLevelWindow(lvlWin.GetLevel() + dy, lvlWin.GetWindow() + dx);
	m_Node->SetLevelWindow(lvlWin,GetRenderer());
	m_Node->SetLevelWindow(lvlWin,NULL);
	GetRenderer()->Render();
	draw();
      }
      return 1;
  }
  return FLmitkRenderWindow::handle(event);
}

