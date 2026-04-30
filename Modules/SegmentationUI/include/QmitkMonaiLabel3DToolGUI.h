/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMonaiLabel3DToolGUI_h
#define QmitkMonaiLabel3DToolGUI_h

#include <MitkSegmentationUIExports.h>
#include <QmitkMonaiLabelToolGUI.h>

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::MonaiLabel3DTool providing 3D MONAI Label segmentation.
  \sa mitk::MonaiLabel3DTool
  \sa QmitkMonaiLabelToolGUI
*/
class MITKSEGMENTATIONUI_EXPORT QmitkMonaiLabel3DToolGUI : public QmitkMonaiLabelToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkMonaiLabel3DToolGUI, QmitkMonaiLabelToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected:
  QmitkMonaiLabel3DToolGUI();
  ~QmitkMonaiLabel3DToolGUI() = default;
};
#endif
