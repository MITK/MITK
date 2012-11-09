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

#if !defined(QmitkVideoPlayer_h)
#define QmitkVideoPlayer_h

#include <QmitkFunctionality.h>
#include <mitkOpenCVVideoSource.h>

class QmitkVideoBackground;
class QmitkOpenCVVideoControls;
/*!
\brief Allows to play videos with OpenCV
*/
class QmitkVideoPlayer: public QmitkFunctionality
{
  Q_OBJECT

  public:
    QmitkVideoPlayer();
    QmitkVideoPlayer(const QmitkVideoPlayer& other)
    {
      Q_UNUSED(other)
      throw std::runtime_error("Copy constructor not implemented");
    }

    virtual ~QmitkVideoPlayer();
    virtual bool IsExclusiveFunctionality() const;
    virtual void StdMultiWidgetAvailable(QmitkStdMultiWidget& stdMultiWidget);
  protected:
    // Pseudo Ctor
    void CreateQtPartControl(QWidget* parent);
protected:
  mitk::OpenCVVideoSource::Pointer m_VideoSource;
  QmitkVideoBackground* m_VideoBackground;
  QmitkOpenCVVideoControls* m_OpenCVVideoControls;
};

#endif // !defined(QmitkVideoPlayer_h)
