/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkImageStatisticsTreeModel_h
#define QmitkImageStatisticsTreeModel_h

#include <QmitkAbstractDataStorageModel.h>

//MITK
#include <MitkImageStatisticsUIExports.h>
#include <mitkImageStatisticsContainer.h>
#include <mitkITKEventObserverGuard.h>

#include <atomic>
#include <mutex>
#include <vector>

class QmitkImageStatisticsTreeItem;

/**
 * \brief Tree model that represents mitk::ImageStatisticsContainer data in the Qt model/view framework.
 *
 * This model queries the data storage for ImageStatisticsContainer objects matching
 * the configured image nodes, mask nodes, histogram bin count, and zero-voxel ignore flag.
 * The results are presented in a hierarchical tree structure:
 * - Level 1: Image
 * - Level 2: Mask (if present)
 * - Level 3: Label instances (if the mask has multiple labels)
 * - Level 4: Time steps (if more than one exists)
 *
 * \sa QmitkAbstractDataStorageModel
 * \sa mitk::ImageStatisticsContainer
 * \sa QmitkImageStatisticsWidget
 */
class MITKIMAGESTATISTICSUI_EXPORT QmitkImageStatisticsTreeModel : public QmitkAbstractDataStorageModel
{
    Q_OBJECT

public:

  /**
   * \brief Constructs the tree model.
   * \param[in] parent Optional parent QObject.
   */
  QmitkImageStatisticsTreeModel(QObject *parent = nullptr);

  /** \brief Destructor. Disconnects from the data storage. */
  ~QmitkImageStatisticsTreeModel() override;

  /**
   * \brief Sets the image nodes whose statistics should be displayed.
   *
   * Internally resolves each image's time steps for the hierarchical model.
   * Triggers a model reset and rebuild.
   *
   * \param[in] nodes Vector of image data nodes.
   */
  void SetImageNodes(const std::vector<mitk::DataNode::ConstPointer>& nodes);

  /**
   * \brief Sets the mask nodes whose statistics should be displayed.
   *
   * Handles the special case where a single-timestep mask is applied to a multi-timestep image.
   * Triggers a model reset and rebuild.
   *
   * \param[in] nodes Vector of mask data nodes.
   */
  void SetMaskNodes(const std::vector<mitk::DataNode::ConstPointer>& nodes);

  /**
   * \brief Clears all image nodes, mask nodes, statistics, and resets the model.
   */
  void Clear();

  /**
   * \brief Sets whether zero-valued voxels should be ignored when selecting statistics.
   *
   * Triggers a model reset and rebuild if the value changes.
   *
   * \param[in] _arg True to ignore zero-valued voxels; false to include them.
   */
  void SetIgnoreZeroValueVoxel(bool _arg);

  /**
   * \brief Returns whether zero-valued voxels are currently being ignored.
   * \return True if zero-valued voxels are ignored; false otherwise.
   */
  bool GetIgnoreZeroValueVoxel() const;

  /**
   * \brief Sets the number of histogram bins used to select matching statistics.
   *
   * Triggers a model reset and rebuild if the value changes.
   *
   * \param[in] nbins The number of histogram bins.
   */
  void SetHistogramNBins(unsigned int nbins);

  /**
   * \brief Returns the current number of histogram bins.
   * \return The number of histogram bins.
   */
  unsigned int GetHistogramNBins() const;

  /**
   * \brief Returns the item flags for the given model index.
   * \param[in] index The model index to query.
   * \return The Qt item flags for the index.
   */
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  /**
   * \brief Returns the data for the given index and role.
   *
   * Supports Qt::DisplayRole for statistics values and Qt::DecorationRole for label color icons
   * and WIP hourglass indicators in the first column.
   *
   * \param[in] index The model index to query.
   * \param[in] role The data role (e.g. Qt::DisplayRole, Qt::DecorationRole).
   * \return The data as QVariant, or an invalid QVariant if not applicable.
   */
  QVariant data(const QModelIndex &index, int role) const override;

  /**
   * \brief Returns the header data for the given section and orientation.
   *
   * The first column header is dynamically generated based on whether masks and
   * multiple time steps are present. Subsequent columns show statistic names.
   *
   * \param[in] section The column index.
   * \param[in] orientation The header orientation (only Qt::Horizontal is supported).
   * \param[in] role The data role (only Qt::DisplayRole is supported).
   * \return The header text as QVariant.
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  /**
   * \brief Returns the number of child rows under the given parent.
   * \param[in] parent The parent model index. Invalid index refers to the root.
   * \return The number of child rows.
   */
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Returns the number of columns (statistic names count + 1).
   * \param[in] parent The parent model index (unused).
   * \return The number of columns.
   */
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Returns the model index for the specified row, column, and parent.
   * \param[in] row The row number.
   * \param[in] column The column number.
   * \param[in] parent The parent model index.
   * \return The model index for the child, or an invalid index if it does not exist.
   */
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Returns the parent model index of the given child.
   * \param[in] child The child model index.
   * \return The parent model index, or an invalid index if the child is at the root level.
   */
  QModelIndex parent(const QModelIndex &child) const override;

signals:
  /**
   * \brief Emitted when new statistics data becomes available in the model.
   */
  void dataAvailable();

  /**
   * \brief Emitted when the model rebuild is finished (usually shortly after dataAvailable()).
   */
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
        --> 3. Level: Label instances [if Mask has more then one label]
           --> 4. Level: Timestep [if >1 exist] */
    void BuildHierarchicalModel();

    /** Registers the observers that keep the model in sync with display relevant changes
    of its inputs: renaming of image or mask nodes and renaming or recoloring of labels of
    multi label masks. Any previously registered observer is removed first.
    @remark Must not be called while one of the currently observed senders invokes an event. */
    void UpdateInputObservers();

    /** Requests a rebuild of the tree from the statistics the model already holds, e.g. after
    a display name or a label color has changed. The rebuild is deferred to the event loop, so
    that a batch of changes (e.g. one LabelModifiedEvent per label of a multi label mask)
    results in a single model update. */
    void RequestModelUpdate();

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

    /** Observers on the "name" property of every input node and on the label events of every
    multi label mask. Neither change reaches the model via DataStorage::ChangedNodeEvent.
    @sa UpdateInputObservers */
    std::vector<mitk::ITKEventObserverGuard> m_InputObservers;
    std::atomic<bool> m_ModelUpdatePending = false;

    std::mutex m_Mutex;
    std::unique_ptr<QmitkImageStatisticsTreeItem> m_RootItem;
    QVariant m_HeaderFirstColumn;
    itk::TimeStamp m_BuildTime;

    bool m_IgnoreZeroValueVoxel = false;
    unsigned int m_HistogramNBins = 100;
};

#endif
