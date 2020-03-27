/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __QMITK_DATA_GENERATOR_BASE_H
#define __QMITK_DATA_GENERATOR_BASE_H

#include <shared_mutex>

//QT
#include <QObject>

//MITK
#include <mitkDataStorage.h>

#include "QmitkDataGenerationJobBase.h"

#include <MitkImageStatisticsUIExports.h>

/*!
\brief QmitkDataGeneratorBase
BaseClass that implements the organisation of data generation.
Use/see the class QmitkDataGenerator for more details.
*/
class MITKIMAGESTATISTICSUI_EXPORT QmitkDataGeneratorBase : public QObject
{
  Q_OBJECT
public:
  QmitkDataGeneratorBase(const QmitkDataGeneratorBase& other) = delete;
  QmitkDataGeneratorBase& operator=(const QmitkDataGeneratorBase& other) = delete;

  ~QmitkDataGeneratorBase();

  using JobResultMapType = QmitkDataGenerationJobBase::ResultMapType;

  mitk::DataStorage::Pointer GetDataStorage() const;
  bool GetAutoUpdate() const;

  /** Indicates if currently there is work in progress, so data generation jobs are running or pending.
   It is set to true when GenerationStarted is triggered and becomes false as soon as GenerationFinished is triggered.
  */
  bool IsGenerating() const;

  /** Checks data validity and triggers generation of data, if needed.
  The generation itselfs will be done in threads an orchastrated by this class. To learn if the threads are finished and
  everything is uptodate, listen to the signal GenerationFinished.
  @return indicates if everything is already valid (true) or if the generation of new data was triggerd (false).*/
  bool Generate() const;

  /** Indicates if for a given image and ROI a valid final result is available.*/
  virtual bool IsValidResultAvailable(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const = 0;

public slots:
  void SetDataStorage(mitk::DataStorage* storage);
  void SetAutoUpdate(bool autoUpdate);

protected slots:
  void OnJobError(QString error, const QmitkDataGenerationJobBase* failedJob) const;
  /*! @brief Wraps the resulting BaseData* into DataNode objects
  */
  void OnFinalResultsAvailable(JobResultMapType results, const QmitkDataGenerationJobBase *job) const;

signals:

  /*! @brief Signal that is emitted if a data generation job is started to generat outdated/inexistant data.
  */
  void DataGenerationStarted(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode, const QmitkDataGenerationJobBase* job) const;

  /*! @brief Signal that is emitted if new final data is produced.
  */
  void NewDataAvailable(mitk::DataStorage::SetOfObjects::ConstPointer data) const;

  /*! @brief Signal that is emitted if all jobs are finished.
  */
  void GenerationFinished() const;

  /*! @brief Signal that is emitted in case of job errors.
  */
  void JobError(QString error, const QmitkDataGenerationJobBase* failedJob) const;

protected:
  /*! @brief Constructor
  @param storage the data storage where all produced data should be stored
  */
  QmitkDataGeneratorBase(mitk::DataStorage::Pointer storage, QObject* parent = nullptr);
  QmitkDataGeneratorBase(QObject* parent = nullptr);

  using InputPairVectorType = std::vector<std::pair<mitk::DataNode::ConstPointer, mitk::DataNode::ConstPointer>>;

  virtual bool NodeChangeIsRelevant(const mitk::DataNode* changedNode) const = 0;
  virtual InputPairVectorType GetAllImageROICombinations() const = 0;
  /** Indicates if there is already an valid and up-to-date result for the given node pair.*/
  virtual void IndicateFutureResults(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const = 0;
  /*! @brief Is called to generate the next job instance that needs to be done and its associated dummy node
      in order to progress the data generation workflow.
   @remark The method assumes that the caller takes care of the job instance deletion.
   @return std::pair of job pointer and placeholder node associated with the job. Current combinations are possible:
   - 1) Both are null: nothing to do;
   - 2) Both are set: there is something to do for a pending dumme node;
   - 3) job null and node set: a job for this node is already work in progress.*/
  virtual std::pair<QmitkDataGenerationJobBase*,mitk::DataNode::Pointer> GetNextMissingGenerationJob(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const =0;
  /** Remove all obsolete data nodes for the given image and seg node from the data storage.
  Obsolete nodes are (interim) result nodes that are not the most recent any more.*/
  virtual void RemoveObsoleteDataNodes(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const = 0;
  /** Prepares result to be added to the storage in an appropriate way and returns the data node for that.*/
  virtual mitk::DataNode::Pointer PrepareResultForStorage(const std::string& label, mitk::BaseData* result, const QmitkDataGenerationJobBase* job) const = 0;

  /*! @brief Creates a data node for interim results
  @details It's the jobs responsibility to write the final results to the data of the DataNode.
  */
  static mitk::DataNode::Pointer CreateWIPDataNode(mitk::BaseData* dataDummy, const std::string& nodeName);

  InputPairVectorType FilterImageROICombinations(InputPairVectorType&& imageROICombinations) const;

  std::string GetPairDescription(const InputPairVectorType::value_type& imageAndSeg) const;

  bool DoGenerate() const;

  mitk::WeakPointer<mitk::DataStorage> m_Storage;

  bool m_AutoUpdate = false;

  mutable std::shared_mutex m_DataMutex;
  mutable bool m_InGenerate = false;
  mutable bool m_RestartGeneration = false;
  mutable bool m_WIP = false;
  mutable bool m_AddingToStorage = false;

  /**Member is called when a node is added to the storage.*/
  void NodeAddedOrModified(const mitk::DataNode* node);

  void EnsureRecheckingAndGeneration() const;

  unsigned long m_DataStorageDeletedTag;
};

#endif
