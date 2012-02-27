/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-05-03 17:20:26 +0200 (Mo, 03 Mai 2010) $
Version:   $Revision: 22675 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef _Qmitk_ToF_Image_Background_h_
#define _Qmitk_ToF_Image_Background_h_

#include "mitkTOFUIExports.h"

#include "qobject.h"

#include <vector>

class vtkRenderer;
class vtkRenderWindow;
class vtkImageActor;
class vtkImageImport;
class vtkActor2D;
class vtkVideoSizeCallback;


namespace mitk 
{

  struct ToFImageBackgroundVectorInfo
  {
    vtkRenderWindow*  renWin;
    vtkRenderer*      videoRenderer;
    vtkImageActor*    videoActor;
    vtkImageImport*   videoImport;
  };

}

/**
 * Displays a char image in the background directly
 *
 */
class QmitkToFImageBackground : public QObject
{
  Q_OBJECT

public:
  QmitkToFImageBackground();
   ~QmitkToFImageBackground();

  ////##Documentation
  ////## @brief sets the mitkRenderWindow in which the video is displayed.
  ////## must be initialized before enabling the background.
  void AddRenderWindow(vtkRenderWindow* renderWindow, int width, int height );
  void RemoveRenderWindow(vtkRenderWindow* renderWindow);
  bool IsRenderWindowIncluded(vtkRenderWindow* renderWindow);

  public slots:
    void UpdateBackground(unsigned char *src);

protected:
  void ResetBackground();

  void Modified();

  typedef std::vector<mitk::ToFImageBackgroundVectorInfo> RenderWindowVectorInfoType;
  RenderWindowVectorInfoType      m_renderWindowVectorInfo;
 
};

#endif

