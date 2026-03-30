/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkParameterFitBackgroundJob_h
#define QmitkParameterFitBackgroundJob_h


//QT
#include <QRunnable>
#include <QObject>

//MITK
#include <mitkDataNode.h>

#include <mitkParameterFitImageGeneratorBase.h>
#include <mitkModelFitResultHelper.h>
#include <mitkModelFitInfo.h>

// ITK
#include <itkCommand.h>

#include <MitkModelFitUIExports.h>

/**
 * \class ParameterFitBackgroundJob
 * \brief QRunnable-based background job for executing a parameter fit image generation.
 *
 * This class wraps a mitk::ParameterFitImageGeneratorBase and runs the fitting process
 * asynchronously in a thread pool. It observes ITK progress, start, and end events from
 * the generator and relays them as Qt signals. Upon completion, the generated result
 * nodes (parameter images, derived parameter images, criterion images, and evaluation
 * parameter images) are made available via the ResultsAreAvailable signal.
 *
 * \sa mitk::ParameterFitImageGeneratorBase
 * \sa mitk::modelFit::ModelFitInfo
 * \sa mitk::modelFit::CreateResultNodeMap
 */
class MITKMODELFITUI_EXPORT ParameterFitBackgroundJob : public QObject, public QRunnable
{
    Q_OBJECT

public:
  /**
   * \brief Constructs a ParameterFitBackgroundJob.
   *
   * \param[in] generator Pointer to the fit image generator to execute. Must not be nullptr.
   * \param[in] fitInfo Pointer to the model fit information. Must not be nullptr.
   * \param[in] parentNode Optional parent data node for the result nodes. May be nullptr.
   *
   * \throw mitk::Exception if generator or fitInfo is nullptr.
   */
  ParameterFitBackgroundJob(mitk::ParameterFitImageGeneratorBase* generator, const mitk::modelFit::ModelFitInfo* fitInfo, mitk::DataNode* parentNode = nullptr);

  /**
   * \brief Constructs a ParameterFitBackgroundJob with additional relevant nodes.
   *
   * \param[in] generator Pointer to the fit image generator to execute. Must not be nullptr.
   * \param[in] fitInfo Pointer to the model fit information. Must not be nullptr.
   * \param[in] parentNode Optional parent data node for the result nodes. May be nullptr.
   * \param[in] additionalRelevantNodes Additional data nodes that are relevant for the fit
   *            session (e.g., input data nodes).
   *
   * \throw mitk::Exception if generator or fitInfo is nullptr.
   */
  ParameterFitBackgroundJob(mitk::ParameterFitImageGeneratorBase* generator, const mitk::modelFit::ModelFitInfo* fitInfo, mitk::DataNode* parentNode, mitk::modelFit::ModelFitResultNodeVectorType additionalRelevantNodes);

  /** \brief Destructor. Removes the ITK observer from the generator. */
  ~ParameterFitBackgroundJob() override;

  /**
   * \brief Executes the fitting process.
   *
   * Calls Generate() on the fit image generator, then creates result nodes from
   * the generated images. Emits ResultsAreAvailable on success, or Error on failure.
   * Always emits Finished at the end.
   */
  void run() override;

  /**
   * \brief Returns the parent data node for result storage.
   * \return Pointer to the parent node, or nullptr if none was specified.
   */
  mitk::DataNode* GetParentNode() const;

  /**
   * \brief Returns the additional relevant data nodes.
   * \return Vector of additional data nodes associated with this job.
   */
  mitk::modelFit::ModelFitResultNodeVectorType GetAdditionalRelevantNodes() const;

signals:
    /** \brief Emitted when the job has completed (regardless of success or failure). */
    void Finished();

    /** \brief Emitted when an error occurs during fitting.
     *  \param[out] err Description of the error. */
    void Error(QString err);

    /**
     * \brief Emitted when the fit results are available.
     * \param[out] resultMap Vector of data nodes containing the generated result images.
     * \param[out] pJob Pointer to this job instance for identification.
     */
    void ResultsAreAvailable(mitk::modelFit::ModelFitResultNodeVectorType resultMap, const ParameterFitBackgroundJob* pJob);

    /**
     * \brief Emitted to report the progress of the fitting process.
     * \param[out] progress The current progress value (0.0 to 1.0).
     */
    void JobProgress(double progress);

    /**
     * \brief Emitted to report status changes during the fitting process.
     * \param[out] info A human-readable status message.
     */
    void JobStatusChanged(QString info);

protected:
	//Inputs
	mitk::ParameterFitImageGeneratorBase::Pointer m_Generator;
  mitk::modelFit::ModelFitInfo::ConstPointer m_ModelFitInfo;
  mitk::DataNode::Pointer m_ParentNode;
  mitk::modelFit::ModelFitResultNodeVectorType m_AdditionalRelevantNodes;

  // Results
  mitk::modelFit::ModelFitResultNodeVectorType m_Results;

  ::itk::MemberCommand<ParameterFitBackgroundJob>::Pointer m_spCommand;
  unsigned long m_ObserverID;

  void OnFitEvent(::itk::Object *, const itk::EventObject &event);
};

#endif
