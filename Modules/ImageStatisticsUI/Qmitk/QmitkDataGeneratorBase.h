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

#include <MitkImageStatisticsUIExports.h>

class QmitkDataGenerationJobBase;

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

  mitk::DataStorage::Pointer GetDataStorage() const;
  bool GetAutoUpdate() const;

  bool IsGenerating() const;

  void Generate() const;

public slots:
  void SetDataStorage(mitk::DataStorage* storage);
  void SetAutoUpdate(bool autoUpdate);

protected slots:
  void OnJobError(QString error, const QmitkDataGenerationJobBase* failedJob) const;
  /*! @brief Wraps the resulting BaseData* into DataNode objects
  */
  void OnFinalResultsAvailable(mitk::DataStorage::SetOfObjects::ConstPointer results, const QmitkDataGenerationJobBase *job) const;

signals:
  /*! @brief Signal that is emitted if new final data is produced.
  */
  void NewDataAvailable(mitk::DataStorage::SetOfObjects::ConstPointer data) const;

  /*! @brief Signal that is emitted if the generator emits new jobs.
  */
  void GenerationStarted() const;

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
  QmitkDataGeneratorBase(mitk::DataStorage::Pointer storage);
  QmitkDataGeneratorBase();

  virtual bool NodeChangeIsRelevant(const mitk::DataNode* changedNode) const = 0;
  virtual std::vector<std::pair<mitk::DataNode::Pointer, mitk::DataNode::Pointer>> GetAllImageStructCombinations() const = 0;
  /** Indicates if there is already an valid and up-to-date result for the given node pair.*/
  virtual bool IsValidResultAvailable(const mitk::DataNode* imageNode, const mitk::DataNode* segNode) const = 0;
  /** Generate placeholder nodes for all (interim) results that will be produced to have a valid final result.
   and add them to the data storage. It is important that the data generation property is set correctly for the
   generated placeholder nodes.*/
  virtual void IndicateFutureResults(const mitk::DataNode* imageNode, const mitk::DataNode* segNode) const = 0;
  /*! @brief Returns the next job that needs to be done in order to complete the workflow.
   If no job instance is passed back, it either indicated that there is nothing to do (IsValidResultAvailable == true)
   or that currently a interim result is still missing and therefore the next job cannot be triggered.*/
  virtual QmitkDataGenerationJobBase* GetNextMissingGenerationJob(const mitk::DataNode* imageNode, const mitk::DataNode* segNode) const =0;
  /** Remove all obsolete data nodes for the given image and seg node from the data storage.
  Obsolete nodes are (interim) result nodes that are not the most recent any more.*/
  virtual void RemoveObsoleteDataNodes(const mitk::DataNode* imageNode, const mitk::DataNode* segNode) const = 0;

  void DoGenerate() const;

  mitk::WeakPointer<mitk::DataStorage> m_Storage;

  bool m_AutoUpdate = false;

  mutable std::shared_mutex m_DataMutex;
  mutable bool m_RunningGeneration = false;
  mutable bool m_RestartGeneration = false;

  /**Member is called when a node is added to the storage.*/
  void NodeAddedOrModified(const mitk::DataNode* node);

  unsigned long m_DataStorageDeletedTag;

};

#endif
