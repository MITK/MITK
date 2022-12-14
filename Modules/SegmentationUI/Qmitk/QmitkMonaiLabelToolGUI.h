#ifndef QmitkMonaiLabelToolGUI_h_Included
#define QmitkMonaiLabelToolGUI_h_Included

#include "QmitkMultiLabelSegWithPreviewToolGUIBase.h"
#include "ui_QmitkMonaiLabelToolGUIControls.h"
#include <MitkSegmentationUIExports.h>

class MITKSEGMENTATIONUI_EXPORT QmitkMonaiLabelToolGUI : public QmitkMultiLabelSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkMonaiLabelToolGUI, QmitkMultiLabelSegWithPreviewToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected slots :

  void OnPreviewBtnClicked();
  void OnFetchBtnClicked();



protected:
  QmitkMonaiLabelToolGUI();
  ~QmitkMonaiLabelToolGUI() = default;

  void ConnectNewTool(mitk::SegWithPreviewTool* newTool) override;
  void InitializeUI(QBoxLayout* mainLayout) override;

  void EnableWidgets(bool enabled) override;

  Ui_QmitkMonaiLabelToolGUIControls m_Controls;

  bool m_FirstPreviewComputation = true;
  EnableConfirmSegBtnFunctionType m_SuperclassEnableConfirmSegBtnFnc;
};

#endif
