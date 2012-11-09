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

