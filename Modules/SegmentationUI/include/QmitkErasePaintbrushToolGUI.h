/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkErasePaintbrushToolGUI_h
#define QmitkErasePaintbrushToolGUI_h

#include <QmitkPaintbrushToolGUI.h>
#include <MitkSegmentationUIExports.h>

/**
  \brief GUI for the erasing mode of mitk::PaintbrushTool.

  Specialization of QmitkPaintbrushToolGUI that provides the user interface for
  erasing (removing) segmentation pixels with the paintbrush tool. Shows a slider
  to change the brush size.

  \sa mitk::PaintbrushTool
  \sa QmitkPaintbrushToolGUI
  \sa QmitkDrawPaintbrushToolGUI
*/

class MITKSEGMENTATIONUI_EXPORT QmitkErasePaintbrushToolGUI : public QmitkPaintbrushToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkErasePaintbrushToolGUI, QmitkPaintbrushToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

    signals :

    public slots :

    protected slots :

    protected :

    QmitkErasePaintbrushToolGUI();
  ~QmitkErasePaintbrushToolGUI() override;
};

#endif
