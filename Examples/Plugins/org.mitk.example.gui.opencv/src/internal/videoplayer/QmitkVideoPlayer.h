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

#ifndef QmitkVideoPlayer_h
#define QmitkVideoPlayer_h

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include <mitkOpenCVVideoSource.h>

class QmitkVideoBackground;
class QmitkOpenCVVideoControls;

/**
 * \brief Allows to play videos with OpenCV
 */
class QmitkVideoPlayer: public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:

  QmitkVideoPlayer();
  virtual ~QmitkVideoPlayer();

private:

  void CreateQtPartControl(QWidget* parent) override;

  void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  mitk::OpenCVVideoSource::Pointer m_VideoSource;
  QmitkVideoBackground* m_VideoBackground;
  QmitkOpenCVVideoControls* m_OpenCVVideoControls;
};

#endif // QmitkVideoPlayer_h
