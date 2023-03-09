/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
class QmitkVideoPlayer : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:
  QmitkVideoPlayer();
  ~QmitkVideoPlayer() override;

private:
  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart *renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart *renderWindowPart) override;

  mitk::OpenCVVideoSource::Pointer m_VideoSource;
  QmitkVideoBackground *m_VideoBackground;
  QmitkOpenCVVideoControls *m_OpenCVVideoControls;
};

#endif
