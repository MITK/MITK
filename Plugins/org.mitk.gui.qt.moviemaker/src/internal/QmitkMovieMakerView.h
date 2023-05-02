/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMovieMakerView_h
#define QmitkMovieMakerView_h

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include <memory>
#include <utility>
#include <vector>

class QmitkAnimationItem;
class QmitkAnimationWidget;
class QMenu;
class QStandardItemModel;
class QTimer;

namespace mitk
{
  class VideoRecorder;
}

namespace Ui
{
  class QmitkMovieMakerView;
}

class QmitkMovieMakerView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkMovieMakerView();
  ~QmitkMovieMakerView() override;

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderWindowPart) override;

private slots:
  void OnMoveAnimationUpButtonClicked();
  void OnMoveAnimationDownButtonClicked();
  void OnAddAnimationButtonClicked();
  void OnRemoveAnimationButtonClicked();
  void OnAnimationTreeViewRowsInserted(const QModelIndex& parent, int start, int end);
  void OnAnimationTreeViewRowsRemoved(const QModelIndex& parent, int start, int end);
  void OnAnimationTreeViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void OnStartComboBoxCurrentIndexChanged(int index);
  void OnDurationSpinBoxValueChanged(double value);
  void OnDelaySpinBoxValueChanged(double value);
  void OnPlayButtonToggled(bool checked);
  void OnStopButtonClicked();
  void OnRecordButtonClicked();
  void OnEncodingFinished();
  void OnFPSSpinBoxValueChanged(int value);
  void OnTimerTimeout();

private:
  void InitializeAnimationWidgets();
  void InitializeAnimationTreeViewWidgets();
  void InitializeAnimationModel();
  void InitializeAddAnimationMenu();
  void InitializePlaybackAndRecordWidgets();
  void InitializeRecordMenu();
  void InitializeRecordingProgress();
  void InitializeTimer(QWidget* parent);
  void ConnectAnimationTreeViewWidgets();
  void ConnectAnimationWidgets();
  void ConnectPlaybackAndRecordWidgets();
  void ConnectTimer();
  void RenderCurrentFrame();
  void UpdateWidgets();
  void UpdateAnimationWidgets();
  void HideCurrentAnimationWidget();
  void ShowAnimationWidget(QmitkAnimationItem* animationItem);
  void RedrawTimeline();
  void CalculateTotalDuration();
  QmitkAnimationItem* GetSelectedAnimationItem() const;
  std::vector<std::pair<QmitkAnimationItem*, double>> GetActiveAnimations(double t) const;

  QWidget* m_Parent;
  Ui::QmitkMovieMakerView* m_Ui;
  QStandardItemModel* m_AnimationModel;
  std::map<QString, QmitkAnimationWidget*> m_AnimationWidgets;
  QMenu* m_AddAnimationMenu;
  QMenu* m_RecordMenu;
  QTimer* m_Timer;
  double m_TotalDuration;
  int m_NumFrames;
  int m_CurrentFrame;
  std::unique_ptr<mitk::VideoRecorder> m_VideoRecorder;
};

#endif
