/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 11316 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "QmitkStereoOptionsDialogBar.h"
#include "QmitkStdMultiWidget.h"

#include "QmitkSelectableGLWidget.h"
#include "QmitkStereoOptionsDialogBar.xpm"

#include <qaction.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qvbox.h>

QmitkStereoOptionsDialogBar
::QmitkStereoOptionsDialogBar( 
  QObject *parent, const char *name, QmitkStdMultiWidget *multiWidget, 
  mitk::DataTreeIteratorBase *dataIt )
: QmitkDialogBar( "Stereo options", parent, name, multiWidget, dataIt )
{
}

QmitkStereoOptionsDialogBar
::~QmitkStereoOptionsDialogBar()
{
}


QString 
QmitkStereoOptionsDialogBar
::GetFunctionalityName()
{
  return "StereoOptionsDialogBar";
}


QAction *
QmitkStereoOptionsDialogBar
::CreateAction( QObject *parent )
{
  QAction* action;
  action = new QAction( 
    tr( "Stereo options" ), 
    QPixmap((const char**)QmitkStereoOptionsDialogBar_xpm), 
    tr( "Stereo options" ), 
    0, 
    parent, 
    "QmitkStereoOptionsDialogBar" );

  return action;
}

QWidget *
QmitkStereoOptionsDialogBar
::CreateDialogBar( QWidget *parent )
{
  QVBox* layout = new QVBox( parent );

  new QLabel( "Stero visualization mode", layout );
  QComboBox* combobox = new QComboBox( layout );
  combobox->insertItem( "stereo off",      0 );
  combobox->insertItem( "red-blue stereo", 1 );
  combobox->insertItem( "D4D stereo",      2 );

  connect( combobox, SIGNAL(activated(int)), this, SLOT(stereoSelectionChanged(int)) );

  return layout;
}

void QmitkStereoOptionsDialogBar::stereoSelectionChanged( int id )
{
  /* From vtkRenderWindow.h tells us about stereo rendering:
  Set/Get what type of stereo rendering to use. CrystalEyes mode uses frame-sequential capabilities available in OpenGL to drive LCD shutter glasses and stereo projectors. RedBlue mode is a simple type of stereo for use with red-blue glasses. Anaglyph mode is a superset of RedBlue mode, but the color output channels can be configured using the AnaglyphColorMask and the color of the original image can be (somewhat maintained using AnaglyphColorSaturation; the default colors for Anaglyph mode is red-cyan. Interlaced stereo  mode produces a composite image where horizontal lines alternate between left and right views. StereoLeft and StereoRight modes choose one or the other stereo view. Dresden mode is yet another stereoscopic interleaving.
  */

  if (m_MultiWidget)
  {
 
    vtkRenderWindow * vtkrenderwindow = m_MultiWidget->mitkWidget4->GetRenderWindow();
    
    // note: foreground vtkRenderers (at least the department logo renderer) produce errors in stereoscopic visualization.
    // Therefore, we disable the logo visualization during stereo rendering.
    switch(id)
    {
    case 0:
      vtkrenderwindow->StereoRenderOff();
      break;
    case 1:
      vtkrenderwindow->SetStereoTypeToRedBlue();
      vtkrenderwindow->StereoRenderOn();
      m_MultiWidget->DisableDepartmentLogo();
      break;
    case 2:
      vtkrenderwindow->SetStereoTypeToDresden();
      vtkrenderwindow->StereoRenderOn();
      m_MultiWidget->DisableDepartmentLogo();
      break;
    }

    
    mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget4->GetRenderWindow())->SetMapperID(mitk::BaseRenderer::Standard3D);
    m_MultiWidget->RequestUpdate();

  }
}
