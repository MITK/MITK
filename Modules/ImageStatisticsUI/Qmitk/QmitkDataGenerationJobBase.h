/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef __QMITK_DATA_GENERATION_JOB_BASE_H
#define __QMITK_DATA_GENERATION_JOB_BASE_H


//QT
#include <QRunnable>
#include <QObject>
#include <QMetaType>

//MITK
#include <mitkBaseData.h>
#include <mitkDataNode.h>
#include <mitkDataStorage.h>


#include <MitkImageStatisticsUIExports.h>

/*!
\brief QmitkDataGenerationJobBase
Base class for all jobs. Each job wraps an implementation of DataGenerationComputationInterface to compute a computation in a multi threaded environment with Qt
\details the signal ResultsAvailable is emitted when the job is finished
the signal Error is emitted in case of an error
The resulting data should be written to the provided outputDataNodes
\warning it's the responsibility of the job to store the data in the dataNode.
\example
QThreadPool* threadPool = QThreadPool::globalInstance();
auto voxelizationJob = new QmitkVoxelizationJob(doseImage, structContourModelSet, voxelizationNode);
connect(job, SIGNAL(ResultsAvailable(const mitk::DataStorage::SetOfObjects*, const QmitkDataGenerationJobBase*)), this, SLOT(OnFinalResultsAvailable(const mitk::DataStorage::SetOfObjects*, const QmitkDataGenerationJobBase*)));
threadPool->start(job);
*/
class MITKIMAGESTATISTICSUI_EXPORT QmitkDataGenerationJobBase : public QObject, public QRunnable
{
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:
  QmitkDataGenerationJobBase(const QmitkDataGenerationJobBase& other) = delete;
  QmitkDataGenerationJobBase& operator=(const QmitkDataGenerationJobBase& other) = delete;

  mitk::DataStorage::SetOfObjects::ConstPointer GetOutputDataNodes() const;

  void run() override; 

signals:
    void Error(QString err, const QmitkDataGenerationJobBase* job);
    /*! @brief Signal is emitted when results are available. 
    @param results a vector of DataNode objects produces by the job and ready tu use, put into storage.
    @param the job that produced the data
    */
    void ResultsAvailable(mitk::DataStorage::SetOfObjects::ConstPointer results, const QmitkDataGenerationJobBase* job);
    
protected:
  /*! @brief constructor with inputData and outputNodes
    @param inputBaseData the input BaseData that is required for computation
    @param outputDataNodes the nodes where the results should be stored
  */
  QmitkDataGenerationJobBase(const mitk::DataStorage::SetOfObjects* outputDataNodes, const std::vector<mitk::BaseData::ConstPointer>& inputBaseData);
  QmitkDataGenerationJobBase(mitk::DataNode* outputDataNode, const std::vector<mitk::BaseData::ConstPointer>& inputBaseData);

  /*! @brief constructor with outputNodes only
  @param outputDataNodes the nodes where the results should be stored
  */
  QmitkDataGenerationJobBase(const mitk::DataStorage::SetOfObjects* outputDataNodes);
  QmitkDataGenerationJobBase(mitk::DataNode* outputDataNode);

  virtual ~QmitkDataGenerationJobBase();

  void SetInputBaseData(const std::vector<mitk::BaseData::ConstPointer>& baseData);
  
  /**Does the real computation. Returns true if there where results produced.*/
  virtual bool RunComputation() = 0;

  mitk::DataStorage::SetOfObjects::ConstPointer m_OutputDataNodes;
  std::vector<mitk::BaseData::ConstPointer> m_InputBaseData;
};

#endif
