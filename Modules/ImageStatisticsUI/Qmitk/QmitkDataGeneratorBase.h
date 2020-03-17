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

class QmitkRTJobBase;

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

  mitk::DataStorage* GetDataStorage() const;
  bool GetAutoUpdate() const;

  void Generate() const;

public slots:
  void SetDataStorage(mitk::DataStorage* storage);
  void SetAutoUpdate(bool autoUpdate);

protected slots:
  void OnJobError(QString error, const QmitkRTJobBase* failedJob);
  /*! @brief Wraps the resulting BaseData* into DataNode objects
  */
  void OnFinalResultsAvailable(const mitk::DataStorage::SetOfObjects* results, const QmitkRTJobBase *job);

signals:
  /*! @brief Signal that is emitted if new final data is produced.
  */
  void NewDataAvailable(const mitk::DataStorage::SetOfObjects* data);
  /*! @brief Signal that is emitted if all jobs are finished.
  */
  void AllJobsGenerated();
  /*! @brief Signal that is emitted in case of job errors.
  */
  void JobError(QString error, const QmitkRTJobBase* failedJob);
protected:
  /*! @brief Constructor
  @param storage the data storage where all produced data should be stored
  */
  QmitkDataGeneratorBase(mitk::DataStorage::Pointer storage);
  QmitkDataGeneratorBase();

  virtual bool NodeChangeIsRelevant(const mitk::DataNode* changedNode) const = 0;
  virtual std::vector<std::pair<mitk::DataNode::Pointer, mitk::DataNode::Pointer>> GetAllImageStructCombinations() const = 0;
  virtual void DoGenerate() const = 0;

  mitk::WeakPointer<mitk::DataStorage> m_Storage;

  bool m_AutoUpdate = false;

  mutable std::shared_mutex m_DataMutex;
  mutable bool m_RunningGeneration = false;
  mutable bool m_RestartGeneration = false;

  /**Member is called when a node is added to the storage.*/
  void NodeAddedToStorage(const mitk::DataNode* node);

  /**Member is called when a node modified.*/
  void NodeModified(const mitk::DataNode* node);

  unsigned long m_DataStorageDeletedTag;

};

#endif
