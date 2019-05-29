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


#ifndef QmitkImageStatisticsTreeModel_h
#define QmitkImageStatisticsTreeModel_h

#include "itkSimpleFastMutexLock.h"

#include "QmitkAbstractDataStorageModel.h"

//MITK
#include <MitkImageStatisticsUIExports.h>
#include "mitkImageStatisticsContainer.h"

class QmitkImageStatisticsTreeItem;

/*!
\class QmitkImageStatisticsTreeModel
Model that takes a mitk::ImageStatisticsContainer and represents it as model in context of the QT view-model-concept.
*/
class MITKIMAGESTATISTICSUI_EXPORT QmitkImageStatisticsTreeModel : public QmitkAbstractDataStorageModel
{
    Q_OBJECT

public:

  QmitkImageStatisticsTreeModel(QObject *parent = nullptr);
  virtual ~QmitkImageStatisticsTreeModel();

  void SetImageNodes(const std::vector<mitk::DataNode::ConstPointer>& nodes);
  void SetMaskNodes(const std::vector<mitk::DataNode::ConstPointer>& nodes);
  void Clear();

  virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
  virtual QVariant data(const QModelIndex &index, int role) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;

signals:
  void dataAvailable();
  /** Is emitted whenever the model changes are finished (usually a bit later than dataAvailable()).*/
  void modelChanged();

protected:
  /*
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void DataStorageChanged() override;
  /*
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void NodePredicateChanged() override;
  /*
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void NodeAdded(const mitk::DataNode *node) override;
  /*
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void NodeChanged(const mitk::DataNode *node) override;
  /*
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void NodeRemoved(const mitk::DataNode *node) override;

private:
    void UpdateByDataStorage();

    using StatisticsContainerVector = std::vector<mitk::ImageStatisticsContainer::ConstPointer>;
    /* builds a hierarchical tree model for the image statistics
    1. Level: Image
    --> 2. Level: Mask [if exist]
        --> 3. Level: Timestep [if >1 exist] */
    void BuildHierarchicalModel();

    StatisticsContainerVector m_Statistics;

    /** Relevant images set by the user.*/
    std::vector<mitk::DataNode::ConstPointer> m_ImageNodes;
    /** Helper that is constructed when m_ImageNodes is set. It has the same order
    like m_ImageNodes, but each image is represented n times, while n is the number
    of time steps the respective image has. This structure makes the business logic
    to select the correct image given a QIndex much simpler and therefore easy to
    understand/maintain. */
    std::vector<std::pair<mitk::DataNode::ConstPointer, unsigned int> > m_TimeStepResolvedImageNodes;
    /** relevant masks set by the user.*/
    std::vector<mitk::DataNode::ConstPointer> m_MaskNodes;
    /** @sa m_TimeStepResolvedImageNodes */
    std::vector<std::pair<mitk::DataNode::ConstPointer, unsigned int>> m_TimeStepResolvedMaskNodes;
    std::vector<std::string> m_StatisticNames;

    itk::SimpleFastMutexLock m_Mutex;
    QmitkImageStatisticsTreeItem *m_RootItem;
    QVariant m_HeaderFirstColumn;
};

#endif // mitkQmitkImageStatisticsTreeModel_h
