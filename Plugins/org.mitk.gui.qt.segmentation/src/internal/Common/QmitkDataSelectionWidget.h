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
  enum Predicate
  {
    ImagePredicate,
    SegmentationPredicate,
    SurfacePredicate
  };

  explicit QmitkDataSelectionWidget(QWidget* parent = NULL);
  ~QmitkDataSelectionWidget();

  unsigned int AddDataStorageComboBox(Predicate predicate);
  unsigned int AddDataStorageComboBox(mitk::NodePredicateBase* predicate = NULL);
  unsigned int AddDataStorageComboBox(const QString &labelText, Predicate predicate);
  unsigned int AddDataStorageComboBox(const QString &labelText, mitk::NodePredicateBase* predicate = NULL);

  mitk::DataStorage::Pointer GetDataStorage() const;
  mitk::DataNode::Pointer GetSelection(unsigned int index);
  void SetPredicate(unsigned int index, Predicate predicate);
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
