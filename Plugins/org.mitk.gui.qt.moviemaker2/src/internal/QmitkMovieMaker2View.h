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

#ifndef QmitkMovieMaker2View_h
#define QmitkMovieMaker2View_h

#include <QmitkAbstractView.h>

class QmitkAnimationItem;
class QmitkAnimationWidget;
class QMenu;
class QStandardItemModel;

namespace Ui
{
  class QmitkMovieMaker2View;
}

class QmitkMovieMaker2View : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkMovieMaker2View();
  ~QmitkMovieMaker2View();

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
  void OnRecordButtonClicked();

private:
  void InitializeAnimationWidgets();
  void InitializeAnimationTreeViewWidgets();
  void InitializeAnimationModel();
  void InitializeAddAnimationMenu();
  void InitializePlaybackAndRecordWidgets();
  void InitializeRecordMenu();
  void ConnectAnimationTreeViewWidgets();
  void ConnectAnimationWidgets();
  void ConnectPlaybackAndRecordWidgets();
  void UpdateWidgets();
  void UpdateAnimationWidgets();
  void HideCurrentAnimationWidget();
  void ShowAnimationWidget(QmitkAnimationItem* animationItem);
  void RedrawTimeline();
  QmitkAnimationItem* GetSelectedAnimationItem() const;

  Ui::QmitkMovieMaker2View* m_Ui;
  QStandardItemModel* m_AnimationModel;
  QMap<QString, QmitkAnimationWidget*> m_AnimationWidgets;
  QMenu* m_AddAnimationMenu;
  QMenu* m_RecordMenu;
};

#endif
