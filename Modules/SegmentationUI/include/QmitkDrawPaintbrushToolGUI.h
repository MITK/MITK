/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDrawPaintbrushToolGUI_h
#define QmitkDrawPaintbrushToolGUI_h

#include <QmitkPaintbrushToolGUI.h>
#include <MitkSegmentationUIExports.h>

/**
  \brief GUI for the drawing mode of mitk::PaintbrushTool.

  Specialization of QmitkPaintbrushToolGUI that provides the user interface for
  drawing (adding) segmentation pixels with the paintbrush tool. Shows a slider
  to change the brush size.

  \sa mitk::PaintbrushTool
  \sa QmitkPaintbrushToolGUI
  \sa QmitkErasePaintbrushToolGUI
*/

class MITKSEGMENTATIONUI_EXPORT QmitkDrawPaintbrushToolGUI : public QmitkPaintbrushToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkDrawPaintbrushToolGUI, QmitkPaintbrushToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

    ~QmitkDrawPaintbrushToolGUI() override;

signals:

public slots:

protected slots:

protected:
  QmitkDrawPaintbrushToolGUI();
};

#endif
