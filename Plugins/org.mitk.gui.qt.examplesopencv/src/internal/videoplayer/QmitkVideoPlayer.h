/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-03-21 14:46:24 +0100 (Sat, 21 Mar 2009) $
Version:   $Revision: 15412 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
