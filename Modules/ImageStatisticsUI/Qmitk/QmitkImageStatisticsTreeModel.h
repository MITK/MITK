/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
The class is used to represent the information of mitk::ImageStatisticsContainer in the set datastorage in the context of the QT view-model-concept.
The represented ImageStatisticContainer are specified by setting the image and mask nodes that should be regarded.
In addition you may specified the statistic computation property HistorgramNBins and IgnoreZeroValueVoxel to select the correct
statistics.
*/
class MITKIMAGESTATISTICSUI_EXPORT QmitkImageStatisticsTreeModel : public QmitkAbstractDataStorageModel
{
    Q_OBJECT

public:

  QmitkImageStatisticsTreeModel(QObject *parent = nullptr);
  ~QmitkImageStatisticsTreeModel() override;

  void SetImageNodes(const std::vector<mitk::DataNode::ConstPointer>& nodes);
  void SetMaskNodes(const std::vector<mitk::DataNode::ConstPointer>& nodes);
  void Clear();

  /*! /brief Set flag to ignore zero valued voxels */
  void SetIgnoreZeroValueVoxel(bool _arg);
  /*! /brief Get status of zero value voxel ignoring. */
  bool GetIgnoreZeroValueVoxel() const;

  /*! /brief Set bin size for histogram resolution.*/
  void SetHistogramNBins(unsigned int nbins);
  /*! /brief Get bin size for histogram resolution.*/
  unsigned int GetHistogramNBins() const;

  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

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

    bool m_IgnoreZeroValueVoxel = false;
    unsigned int m_HistogramNBins = 100;
};

#endif // mitkQmitkImageStatisticsTreeModel_h
