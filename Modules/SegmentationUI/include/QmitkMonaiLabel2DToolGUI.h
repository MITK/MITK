/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMonaiLabel2DToolGUI_h
#define QmitkMonaiLabel2DToolGUI_h

#include <MitkSegmentationUIExports.h>
#include <QmitkMonaiLabelToolGUI.h>

/**
  \brief GUI for mitk::MonaiLabel2DTool providing 2D MONAI Label segmentation.
  \sa mitk::MonaiLabel2DTool
  \sa QmitkMonaiLabelToolGUI
*/
class MITKSEGMENTATIONUI_EXPORT QmitkMonaiLabel2DToolGUI : public QmitkMonaiLabelToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkMonaiLabel2DToolGUI, QmitkMonaiLabelToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected:
  QmitkMonaiLabel2DToolGUI();
  ~QmitkMonaiLabel2DToolGUI() = default;
};

#endif
