/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataSelectionWidget_h
#define QmitkDataSelectionWidget_h

#include <MitkSegmentationUIExports.h>

#include <ui_QmitkDataSelectionWidgetControls.h>
#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>

#include <vector>

namespace mitk
{
  class NodePredicateBase;
}

class QmitkSingleNodeSelectionWidget;

class MITKSEGMENTATIONUI_EXPORT QmitkDataSelectionWidget : public QWidget
{
  Q_OBJECT

public:
  enum Predicate
  {
    ImagePredicate,
    SegmentationPredicate,
    SurfacePredicate,
    ImageAndSegmentationPredicate,
    ContourModelPredicate,
    SegmentationOrSurfacePredicate
  };

  explicit QmitkDataSelectionWidget(QWidget* parent = nullptr);
  ~QmitkDataSelectionWidget() override;

  unsigned int AddDataSelection(Predicate predicate);
  unsigned int AddDataSelection(mitk::NodePredicateBase* predicate = nullptr);
  unsigned int AddDataSelection(const QString &labelText, const QString &info, const QString &popupTitel, const QString &popupHint, Predicate predicate);
  unsigned int AddDataSelection(const QString &labelText, const QString &info, const QString &popupTitel, const QString &popupHint, mitk::NodePredicateBase* predicate = nullptr);

  void SetDataStorage(mitk::DataStorage* dataStorage);
  mitk::DataStorage::Pointer GetDataStorage() const;
  mitk::DataNode::Pointer GetSelection(unsigned int index);
  void SetPredicate(unsigned int index, Predicate predicate);
  void SetPredicate(unsigned int index, const mitk::NodePredicateBase* predicate);
  const mitk::NodePredicateBase *GetPredicate(unsigned int index) const;
  void SetHelpText(const QString& text);

signals:
  void SelectionChanged(unsigned int index, const mitk::DataNode* selection);

private slots:
  void OnSelectionChanged(QList<mitk::DataNode::Pointer> selection);

private:
  Ui::QmitkDataSelectionWidgetControls m_Controls;
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  std::vector<QmitkSingleNodeSelectionWidget*> m_NodeSelectionWidgets;
};

#endif
