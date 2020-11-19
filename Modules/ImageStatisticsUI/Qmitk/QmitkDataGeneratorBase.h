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

#include "QmitkDataGenerationJobBase.h"

#include <MitkImageStatisticsUIExports.h>

/*!
\brief QmitkDataGeneratorBase
BaseClass that implements the organisation of (statistic) data generation for pairs of images and ROIs.
The key idea is that this class ensures that for vector of given image ROI pairs (defined by derived classes)
a result instance (e.g ImageStatisticsContainer) will be calculated, if needed (e.g. because it is missing or
not uptodate anymore), and stored in the data storage passed to a generator instance. While derived classes i.a.
specify how to generate the image ROI pairs, how to detect latest results, what the next generation step is and
how to remove obsolete data from the storage, the base class takes care of the observation of the data storage
and orchestrates the whole checking and generation workflow.
In all the generation/orchestration process the data storage, passed to the generator, 1) serves as place where the final
results are stored and searched and 2) it resembles the state of the genertion process with these final results and WIP
place holder nodes that indicate planed or currently processed generation steps.
*/
class MITKIMAGESTATISTICSUI_EXPORT QmitkDataGeneratorBase : public QObject
{
  Q_OBJECT
public:
  QmitkDataGeneratorBase(const QmitkDataGeneratorBase& other) = delete;
  QmitkDataGeneratorBase& operator=(const QmitkDataGeneratorBase& other) = delete;

  virtual ~QmitkDataGeneratorBase();

  using JobResultMapType = QmitkDataGenerationJobBase::ResultMapType;

  mitk::DataStorage::Pointer GetDataStorage() const;

  /** Indicates if the generator may trigger the update automatically (true). Reasons for an update are:
   - Input data has been changed or modified
   - Generation relevant settings in derived classes have been changed (must be implemented in derived classes)
   */
  bool GetAutoUpdate() const;

  /** Indicates if there is currently work in progress, thus data generation jobs are running or pending.
   It is set to true when GenerationStarted is triggered and becomes false as soon as GenerationFinished is triggered.
  */
  bool IsGenerating() const;

  /** Checks data validity and triggers generation of data, if needed.
  The generation itselfs will be done with a thread pool and is orchestrated by this class. To learn if the threads are finished and
  everything is uptodate, listen to the signal GenerationFinished.
  @return indicates if everything is already valid (true) or if the generation of new data was triggerd (false).*/
  bool Generate() const;

  /** Indicates if for a given image and ROI a valid final result is available.*/
  virtual bool IsValidResultAvailable(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const = 0;

public slots:
  /** Sets the data storage the generator should monitor and where WIP placeholder nodes and final result nodes should be stored.*/
  void SetDataStorage(mitk::DataStorage* storage);

  void SetAutoUpdate(bool autoUpdate);

protected slots:
  /** Used by QmitkDataGenerationJobBase to signal the generator that an error occured. */
  void OnJobError(QString error, const QmitkDataGenerationJobBase* failedJob) const;
  /** Used by QmitkDataGenerationJobBase to signal and communicate the results of there computation. */
  void OnFinalResultsAvailable(JobResultMapType results, const QmitkDataGenerationJobBase *job) const;

signals:

  /*! @brief Signal that is emitted if a data generation job is started to generat outdated/inexistant data.
  */
  void DataGenerationStarted(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode, const QmitkDataGenerationJobBase* job) const;

  /*! @brief Signal that is emitted if new final data is produced.
  */
  void NewDataAvailable(mitk::DataStorage::SetOfObjects::ConstPointer data) const;

  /*! @brief Signal that is emitted if all jobs are finished and everything is up to date.
  */
  void GenerationFinished() const;

  /*! @brief Signal that is emitted in case of job errors.
  */
  void JobError(QString error, const QmitkDataGenerationJobBase* failedJob) const;

protected:
  /*! @brief Constructor
  @param storage the data storage where all produced data should be stored
  @param parent
  */
  QmitkDataGeneratorBase(mitk::DataStorage::Pointer storage, QObject* parent = nullptr);
  QmitkDataGeneratorBase(QObject* parent = nullptr);

  using InputPairVectorType = std::vector<std::pair<mitk::DataNode::ConstPointer, mitk::DataNode::ConstPointer>>;

  /** This method must be implemented by derived to indicate if a changed node is relevant and therefore if an update must be triggered.*/
  virtual bool ChangedNodeIsRelevant(const mitk::DataNode* changedNode) const = 0;
  /** This method must be impemented by derived classes to return the pairs of images and ROIs
  (ROI may be null if no ROI is needed) for which data are needed.*/
  virtual InputPairVectorType GetAllImageROICombinations() const = 0;
  /** This method should indicate all missing and outdated (interim) results in the data storage, with new placeholder nodes and WIP dummy data
   added to the storage. The placeholder nodes will be replaced by the real results as soon as they are ready.
   The strategy how to detact which placeholder node is need and how the dummy data should look like must be implemented by derived classes.*/
  virtual void IndicateFutureResults(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const = 0;
  /*! @brief Is called to generate the next job instance that needs to be done and is associated dummy node
      in order to progress the data generation workflow.
   @remark The method can assume that the caller takes care of the job instance deletion.
   @return std::pair of job pointer and placeholder node associated with the job. Following combinations are possible:
   - Both are null: nothing to do;
   - Both are set: there is something to do for a pending dumme node -> trigger computation;
   - Job null and node set: a job for this node is already work in progress -> pass on till its finished.*/
  virtual std::pair<QmitkDataGenerationJobBase*,mitk::DataNode::Pointer> GetNextMissingGenerationJob(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const =0;
  /** Remove all obsolete data nodes for the given image and ROI node from the data storage.
  Obsolete nodes are (interim) result nodes that are not the most recent any more.*/
  virtual void RemoveObsoleteDataNodes(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const = 0;
  /** Prepares result to be added to the storage in an appropriate way and returns the data node for that.*/
  virtual mitk::DataNode::Pointer PrepareResultForStorage(const std::string& label, mitk::BaseData* result, const QmitkDataGenerationJobBase* job) const = 0;

  /*! Creates a data node for WIP place holder results. It can be used by IndicateFutureResults().*/
  static mitk::DataNode::Pointer CreateWIPDataNode(mitk::BaseData* dataDummy, const std::string& nodeName);

  /** Filters a passed pair vector. The returned pair vector only contains pair of nodes that exist in the data storage.*/
  InputPairVectorType FilterImageROICombinations(InputPairVectorType&& imageROICombinations) const;

  /** Return a descriptive label of a passed pair. Used e.g. for some debug log messages.*/
  std::string GetPairDescription(const InputPairVectorType::value_type& imageAndSeg) const;

  /** Internal part of the generation strategy. Here is where the heavy lifting is done.*/
  bool DoGenerate() const;

  /** Methods either directly calls generation or if its allready onging flags to restart the generation.*/
  void EnsureRecheckingAndGeneration() const;

  mitk::WeakPointer<mitk::DataStorage> m_Storage;

  bool m_AutoUpdate = false;

  mutable std::mutex m_DataMutex;

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
