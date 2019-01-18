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


#ifndef QmitkImageStatisticsTableModel_h
#define QmitkImageStatisticsTableModel_h

#include "itkSimpleFastMutexLock.h"

#include "QmitkAbstractDataStorageModel.h"

//MITK
#include <MitkImageStatisticsUIExports.h>
#include "mitkImageStatisticsContainer.h"

/*!
\class QmitkImageStatisticsTableModel
Model that takes a mitk::ImageStatisticsContainer and represents it as model in context of the QT view-model-concept.
*/
class MITKIMAGESTATISTICSUI_EXPORT QmitkImageStatisticsTableModel : public QmitkAbstractDataStorageModel
{
    Q_OBJECT

public:
  enum class ViewMode {
    imageXStatistic,
    imageXMask
  };

  QmitkImageStatisticsTableModel(QObject *parent = nullptr);
  virtual ~QmitkImageStatisticsTableModel();

  void SetImageNodes(const std::vector<mitk::DataNode::ConstPointer>& nodes);
  void SetMaskNodes(const std::vector<mitk::DataNode::ConstPointer>& nodes);
  void SetViewMode(ViewMode m);
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
    /* the function returns the statistic container and the time point indicated by the index.
     If the index is not valid result.first will point to a nullptr.*/
    std::pair<mitk::ImageStatisticsContainer::ConstPointer, unsigned int> GetRelevantStatsticsByIndex(const QModelIndex &index) const;

    StatisticsContainerVector m_Statistics;

    /** Relevant images set by the user.*/
    std::vector<mitk::DataNode::ConstPointer> m_ImageNodes;
    /** Helper that is constructed when m_ImageNodes is set. It has the same order
    like m_ImageNodes, but each image is represented n times, while n is the number
    of time steps the respective image has. This structure makes the business logic
    to select the correct image given a QIndex much simpler and therfore easy to
    understand/maintaine. */
    std::vector<std::pair<mitk::DataNode::ConstPointer, unsigned int> > m_TimeStepResolvedImageNodes;
    /** relevant masks set by the user.*/
    std::vector<mitk::DataNode::ConstPointer> m_MaskNodes;
    /** @sa m_TimeStepResolvedImageNodes */
    std::vector<std::pair<mitk::DataNode::ConstPointer, unsigned int>> m_TimeStepResolvedMaskNodes;
    std::vector<std::string> m_StatisticNames;
    ViewMode m_ViewMode = ViewMode::imageXStatistic;

    itk::SimpleFastMutexLock m_Mutex;
};

#endif // mitkQmitkImageStatisticsTableModel_h
