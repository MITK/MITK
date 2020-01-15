/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataSelectionWidget_h
#define QmitkDataSelectionWidget_h

#include <ui_QmitkDataSelectionWidgetControls.h>
#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <vector>

namespace mitk
{
  class NodePredicateBase;
}

class QmitkDataStorageComboBox;

class QmitkDataSelectionWidget : public QWidget
{
  Q_OBJECT

public:
  enum PredicateType
  {
    ImagePredicate,
    MaskPredicate,
    SegmentationPredicate,
    SurfacePredicate
  };

  explicit QmitkDataSelectionWidget(QWidget* parent = nullptr);
  ~QmitkDataSelectionWidget() override;

  unsigned int AddDataStorageComboBox(PredicateType predicate);
  unsigned int AddDataStorageComboBox(mitk::NodePredicateBase* predicate = nullptr);
  unsigned int AddDataStorageComboBox(const QString &labelText, PredicateType predicate);
  unsigned int AddDataStorageComboBox(const QString &labelText, mitk::NodePredicateBase* predicate = nullptr);

  mitk::DataStorage::Pointer GetDataStorage() const;
  mitk::DataNode::Pointer GetSelection(unsigned int index);
  void SetPredicate(unsigned int index, PredicateType predicate);
  void SetPredicate(unsigned int index, mitk::NodePredicateBase* predicate);
  void SetHelpText(const QString& text);

signals:
  void SelectionChanged(unsigned int index, const mitk::DataNode* selection);

private slots:
  void OnSelectionChanged(const mitk::DataNode* selection);

private:
  Ui::QmitkDataSelectionWidgetControls m_Controls;
  std::vector<QmitkDataStorageComboBox*> m_DataStorageComboBoxes;
};

#endif
