/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataGeneratorBase_h
#define QmitkDataGeneratorBase_h

#include <mutex>

//QT
#include <QObject>

//MITK
#include <mitkDataStorage.h>

#include <QmitkDataGenerationJobBase.h>

#include <MitkImageStatisticsUIExports.h>

/**
 * \brief Base class that orchestrates asynchronous data generation for image/ROI pairs.
 *
 * This class manages the lifecycle of background data generation jobs for pairs of images and ROIs.
 * It ensures that for each image/ROI combination (defined by derived classes), a result instance
 * (e.g., mitk::ImageStatisticsContainer) is calculated when needed (missing or outdated) and stored
 * in the associated data storage. Derived classes define how to produce image/ROI pairs, how to detect
 * existing results, what constitutes the next generation step, and how to clean up obsolete data.
 * The base class observes the data storage for changes and orchestrates the checking and generation workflow.
 *
 * The data storage serves two roles: (1) it stores and retrieves final results, and (2) it tracks
 * the generation state via work-in-progress (WIP) placeholder nodes for pending or in-progress jobs.
 *
 * \sa QmitkDataGenerationJobBase
 * \sa QmitkImageAndRoiDataGeneratorBase
 * \sa QmitkImageStatisticsDataGenerator
 */
class MITKIMAGESTATISTICSUI_EXPORT QmitkDataGeneratorBase : public QObject
{
  Q_OBJECT
public:
  QmitkDataGeneratorBase(const QmitkDataGeneratorBase& other) = delete;
  QmitkDataGeneratorBase& operator=(const QmitkDataGeneratorBase& other) = delete;

  virtual ~QmitkDataGeneratorBase();

  /** \brief Alias for the result map type produced by generation jobs. */
  using JobResultMapType = QmitkDataGenerationJobBase::ResultMapType;

  /**
   * \brief Returns the data storage used by this generator.
   * \return The data storage, or nullptr if none is set.
   */
  mitk::DataStorage::Pointer GetDataStorage() const;

  /**
   * \brief Returns whether automatic updates are enabled.
   *
   * When enabled, the generator automatically triggers regeneration when:
   * - Input data nodes have been changed or modified
   * - Generation-relevant settings in derived classes have changed
   *
   * \return True if auto-update is enabled; false otherwise.
   */
  bool GetAutoUpdate() const;

  /**
   * \brief Returns whether data generation jobs are currently running or pending.
   *
   * Set to true when DataGenerationStarted is emitted and becomes false
   * when GenerationFinished is emitted.
   *
   * \return True if generation is in progress; false otherwise.
   */
  bool IsGenerating() const;

  /**
   * \brief Checks data validity and triggers generation of missing or outdated data.
   *
   * The generation is performed asynchronously using a thread pool. To detect when all
   * jobs have completed, connect to the GenerationFinished signal.
   *
   * \return True if all results are already valid; false if new generation was triggered.
   */
  bool Generate() const;

  /**
   * \brief Checks whether a valid final result exists for the given image and ROI.
   * \param[in] imageNode The image data node to check.
   * \param[in] roiNode The ROI data node to check (may be nullptr if no ROI is used).
   * \return True if a valid, up-to-date result is available; false otherwise.
   */
  virtual bool IsValidResultAvailable(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const = 0;

public slots:
  /**
   * \brief Sets the data storage for monitoring, storing WIP placeholders, and final results.
   * \param[in] storage Pointer to the data storage to use. May be nullptr to disconnect.
   */
  void SetDataStorage(mitk::DataStorage* storage);

  /**
   * \brief Enables or disables automatic update on data changes.
   * \param[in] autoUpdate True to enable; false to disable.
   */
  void SetAutoUpdate(bool autoUpdate);

protected slots:
  /**
   * \brief Handles error signals from generation jobs and re-emits JobError.
   * \param[in] error The error message.
   * \param[in] failedJob Pointer to the job that failed.
   */
  void OnJobError(QString error, const QmitkDataGenerationJobBase* failedJob) const;
  /**
   * \brief Handles completed job results, adds them to the data storage, and emits NewDataAvailable.
   * \param[in] results The result map from the completed job.
   * \param[in] job Pointer to the job that produced the results.
   */
  void OnFinalResultsAvailable(JobResultMapType results, const QmitkDataGenerationJobBase *job) const;

signals:

  /**
   * \brief Emitted when a data generation job is started for outdated or missing data.
   * \param[in] imageNode The image data node being processed.
   * \param[in] roiNode The ROI data node being processed (may be nullptr).
   * \param[in] job Pointer to the started generation job.
   */
  void DataGenerationStarted(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode, const QmitkDataGenerationJobBase* job) const;

  /**
   * \brief Emitted when new final result data has been produced and added to the data storage.
   * \param[in] data The set of newly added result data nodes.
   */
  void NewDataAvailable(mitk::DataStorage::SetOfObjects::ConstPointer data) const;

  /**
   * \brief Emitted when all generation jobs have finished and all results are up to date.
   */
  void GenerationFinished() const;

  /**
   * \brief Emitted when a generation job encounters an error.
   * \param[in] error The error description.
   * \param[in] failedJob Pointer to the job that failed.
   */
  void JobError(QString error, const QmitkDataGenerationJobBase* failedJob) const;

protected:
  /**
   * \brief Constructs a generator with the given data storage.
   * \param[in] storage The data storage where all produced data should be stored.
   * \param[in] parent Optional parent QObject for Qt ownership.
   */
  QmitkDataGeneratorBase(mitk::DataStorage::Pointer storage, QObject* parent = nullptr);
  /**
   * \brief Constructs a generator without an initial data storage.
   * \param[in] parent Optional parent QObject for Qt ownership.
   */
  QmitkDataGeneratorBase(QObject* parent = nullptr);

  /** \brief Vector of image/ROI node pairs used as input for generation. */
  using InputPairVectorType = std::vector<std::pair<mitk::DataNode::ConstPointer, mitk::DataNode::ConstPointer>>;

  /**
   * \brief Determines whether a changed data node is relevant and should trigger a regeneration.
   * \param[in] changedNode The node that was changed in the data storage.
   * \return True if the change is relevant and an update should be triggered.
   */
  virtual bool ChangedNodeIsRelevant(const mitk::DataNode* changedNode) const = 0;
  /**
   * \brief Returns all image/ROI pairs for which data generation is needed.
   *
   * The ROI element of a pair may be nullptr if no ROI is required.
   *
   * \return A vector of image/ROI node pairs.
   */
  virtual InputPairVectorType GetAllImageROICombinations() const = 0;
  /**
   * \brief Adds placeholder nodes and WIP dummy data to the data storage for missing or outdated results.
   *
   * The placeholder nodes will be replaced by real results once generation completes.
   * Derived classes define the detection strategy and dummy data format.
   *
   * \param[in] imageNode The image data node.
   * \param[in] roiNode The ROI data node (may be nullptr).
   */
  virtual void IndicateFutureResults(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const = 0;
  /**
   * \brief Returns the next generation job and its associated placeholder node.
   *
   * \note The caller takes ownership of the returned job instance.
   *
   * \param[in] imageNode The image data node.
   * \param[in] roiNode The ROI data node (may be nullptr).
   * \return A pair of (job, placeholder node) with three possible outcomes:
   *         - Both null: nothing to do.
   *         - Both set: a pending placeholder needs computation; trigger the job.
   *         - Job null, node set: a job for this node is already in progress; wait for completion.
   */
  virtual std::pair<QmitkDataGenerationJobBase*,mitk::DataNode::Pointer> GetNextMissingGenerationJob(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const =0;
  /**
   * \brief Removes obsolete result nodes for the given image/ROI pair from the data storage.
   *
   * Obsolete nodes are interim result nodes that have been superseded by newer results.
   *
   * \param[in] imageNode The image data node.
   * \param[in] roiNode The ROI data node (may be nullptr).
   */
  virtual void RemoveObsoleteDataNodes(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const = 0;
  /**
   * \brief Wraps a generation result in a data node suitable for storage.
   * \param[in] label The result label from the job's result map.
   * \param[in] result The generated data to store.
   * \param[in] job The job that produced the result.
   * \return A data node ready to be added to the data storage.
   */
  virtual mitk::DataNode::Pointer PrepareResultForStorage(const std::string& label, mitk::BaseData* result, const QmitkDataGenerationJobBase* job) const = 0;

  /**
   * \brief Creates a WIP placeholder data node for future results.
   *
   * The created node is marked as a hidden helper object with a "pending" generation status.
   * Can be used by IndicateFutureResults() implementations.
   *
   * \param[in] dataDummy The dummy data object to attach to the node. Must not be nullptr.
   * \param[in] nodeName The name to assign to the node.
   * \return A newly created placeholder data node.
   * \throw mitk::Exception if dataDummy is nullptr.
   */
  static mitk::DataNode::Pointer CreateWIPDataNode(mitk::BaseData* dataDummy, const std::string& nodeName);

  /**
   * \brief Filters image/ROI pairs to only include those whose nodes exist in the data storage.
   * \param[in] imageROICombinations The pairs to filter (moved into the method).
   * \return A filtered vector containing only pairs with nodes present in the data storage.
   */
  InputPairVectorType FilterImageROICombinations(InputPairVectorType&& imageROICombinations) const;

  /**
   * \brief Returns a human-readable description of an image/ROI pair for debug logging.
   * \param[in] imageAndSeg The image/ROI pair to describe.
   * \return A descriptive string.
   */
  std::string GetPairDescription(const InputPairVectorType::value_type& imageAndSeg) const;

  /**
   * \brief Core generation loop that checks validity and launches jobs for all image/ROI pairs.
   * \return True if all results are valid; false if new jobs were triggered.
   */
  bool DoGenerate() const;

  /**
   * \brief Ensures generation is (re)started, either immediately or by flagging a restart if already running.
   */
  void EnsureRecheckingAndGeneration() const;

  mitk::WeakPointer<mitk::DataStorage> m_Storage;

  bool m_AutoUpdate = false;

  mutable std::mutex m_DataMutex;

  /** Time stamp for the last successful run through with the current image roi pairs.*/
  mutable itk::TimeStamp m_GenerationTime;

private: 
  /** Indicates if we are currently in the Generation() verification and generation of pending jobs triggering loop.
  Only needed for the internal logic.*/
  mutable bool m_InGenerate = false;
  /** Internal flag that is set if a generation was requested, while one generation loop was already ongoing.*/
  mutable bool m_RestartGeneration = false;
  /** Indicates if there are still jobs pending or computing (true) or if everything is valid (false).*/
  mutable bool m_WIP = false;
  /** Internal flag that indicates that generator is currently in the process of adding results to the storage*/
  mutable bool m_AddingToStorage = false;

  /**Member is called when a node is added to the storage.*/
  void NodeAddedOrModified(const mitk::DataNode* node);

  unsigned long m_DataStorageDeletedTag;
};

#endif
