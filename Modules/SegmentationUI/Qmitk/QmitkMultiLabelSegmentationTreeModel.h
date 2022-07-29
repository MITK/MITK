/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkMultiLabelSegmentationTreeModel_h
#define QmitkMultiLabelSegmentationTreeModel_h

#include "mitkLabelSetImage.h"

// qt
#include <QAbstractItemModel>

#include "MitkSegmentationUIExports.h"


class QmitkMultiLabelSegTreeItem;

/*!
\class QmitkMultiLabelSegmentationTreeModel
The class is used to represent the information of an MITK MultiLabel segmentation instance (labels, spacial groups...).
*/
class MITKSEGMENTATIONUI_EXPORT QmitkMultiLabelSegmentationTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:

  QmitkMultiLabelSegmentationTreeModel(QObject *parent = nullptr);
  ~QmitkMultiLabelSegmentationTreeModel() override;

  void SetSegmentation(mitk::LabelSetImage* segmentation);

  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;

  enum TableColumns
  {
    NAME_COL = 0,
    LOCKED_COL,
    COLOR_COL,
    VISIBLE_COL
  };

signals:
  void dataAvailable();
  /** Is emitted whenever the model changes are finished (usually a bit later than dataAvailable()).*/
  void modelChanged();

protected:
  void OnModified(const itk::Object* caller, const itk::EventObject&);


private:
  void AddObserver();
  void RemoveObserver();

  void UpdateBySegmentation();

  /* builds a hierarchical tree model for the image statistics
  1. Level: Image
  --> 2. Level: Mask [if exist]
      --> 3. Level: Timestep [if >1 exist] */
  void BuildHierarchicalModel();

  mitk::LabelSetImage::Pointer m_Segmentation;

  std::mutex m_Mutex;

  QmitkMultiLabelSegTreeItem* m_RootItem;

  unsigned long m_ModifiedObserverTag;
  bool m_Observed;
};

#endif // mitkQmitkMultiLabelSegmentationTreeModel_h
