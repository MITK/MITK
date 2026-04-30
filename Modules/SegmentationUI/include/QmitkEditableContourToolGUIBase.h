/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkEditableContourToolGUIBase_h
#define QmitkEditableContourToolGUIBase_h

#include <QmitkToolGUI.h>
#include <MitkSegmentationUIExports.h>
#include <memory>

namespace mitk
{
  class EditableContourTool;
}

namespace Ui
{
  class QmitkEditableContourToolGUIControls;
}

/**
 * \ingroup org_mitk_gui_qt_interactivesegmentation_internal
 * \brief Base GUI class for editable contour-based segmentation tools.
 *
 * Provides common UI controls for contour tools including confirm/clear buttons,
 * auto-confirm toggle, add/subtract mode toggle, and an information panel. Used as the
 * base class for tool GUIs such as QmitkLassoToolGUI and QmitkLiveWireTool2DGUI.
 *
 * \sa mitk::EditableContourTool
 * \sa QmitkLassoToolGUI
 * \sa QmitkLiveWireTool2DGUI
 * \sa QmitkToolGUI
 */
class MITKSEGMENTATIONUI_EXPORT QmitkEditableContourToolGUIBase : public QmitkToolGUI
{
  Q_OBJECT

public:
  /**
   * \brief Mode of operation for the contour tool.
   */
  enum class Mode
  {
    Add,      ///< Add contour to the segmentation.
    Subtract  ///< Subtract contour from the segmentation.
  };

  mitkClassMacro(QmitkEditableContourToolGUIBase, QmitkToolGUI);
  itkFactorylessNewMacro(Self);

protected slots:

  void OnNewToolAssociated(mitk::Tool*);
  void OnConfirmSegmentation();
  void OnClearContour();
  void OnAutoConfirm(bool on);
  void OnModeToggled(Mode mode);
  void OnShowInformation(bool on);

protected:
  QmitkEditableContourToolGUIBase();
  ~QmitkEditableContourToolGUIBase() override;

  std::unique_ptr<Ui::QmitkEditableContourToolGUIControls> m_Controls;
  itk::SmartPointer<mitk::EditableContourTool> m_NewTool;
};

#endif
