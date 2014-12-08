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

#ifndef QmitkMovieMakerView_h
#define QmitkMovieMakerView_h

#include <QmitkAbstractView.h>

class QmitkAnimationItem;
class QmitkAnimationWidget;
class QmitkFFmpegWriter;
class QMenu;
class QStandardItemModel;
class QTimer;

namespace Ui
{
  class QmitkMovieMakerView;
}

class QmitkMovieMakerView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkMovieMakerView();
  ~QmitkMovieMakerView();

  void CreateQtPartControl(QWidget* parent);
  void SetFocus();

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
  void OnFPSSpinBoxValueChanged(int value);
  void OnTimerTimeout();

private:
  void InitializeAnimationWidgets();
  void InitializeAnimationTreeViewWidgets();
  void InitializeAnimationModel();
  void InitializeAddAnimationMenu();
  void InitializePlaybackAndRecordWidgets();
  void InitializeRecordMenu();
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
  QVector<QPair<QmitkAnimationItem*, double> > GetActiveAnimations(double t) const;

  QmitkFFmpegWriter* m_FFmpegWriter;
  Ui::QmitkMovieMakerView* m_Ui;
  QStandardItemModel* m_AnimationModel;
  QMap<QString, QmitkAnimationWidget*> m_AnimationWidgets;
  QMenu* m_AddAnimationMenu;
  QMenu* m_RecordMenu;
  QTimer* m_Timer;
  double m_TotalDuration;
  int m_NumFrames;
  int m_CurrentFrame;
};

#endif
