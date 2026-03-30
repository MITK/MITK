/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDescriptionParameterBackgroundJob_h
#define QmitkDescriptionParameterBackgroundJob_h


//QT
#include <QRunnable>
#include <QObject>

//MITK
#include <mitkDataNode.h>

#include <mitkDescriptionParameterImageGeneratorBase.h>
#include <mitkModelFitResultHelper.h>
#include <mitkModelFitInfo.h>

// ITK
#include <itkCommand.h>

#include <MitkPharmacokineticsUIExports.h>

/** \brief Background job for computing description parameter images asynchronously.
 *
 * Runs a DescriptionParameterImageGeneratorBase in a background thread (via QRunnable)
 * and emits Qt signals for progress updates, completion, errors, and result availability.
 * Results are packaged as DataNode objects suitable for insertion into a DataStorage.
 *
 * \sa mitk::DescriptionParameterImageGeneratorBase, mitk::modelFit::ModelFitResultHelper
 */
class MITKPHARMACOKINETICSUI_EXPORT DescriptionParameterBackgroundJob : public QObject, public QRunnable
{
    Q_OBJECT

public:
    /** \brief Construct a background job for the given generator.
     *
     * \param[in] generator  The description parameter image generator to execute.
     * \param[in] parentNode Optional parent data node for organizing result nodes
     *                       in the data storage. May be \c nullptr.
     */
    DescriptionParameterBackgroundJob(mitk::DescriptionParameterImageGeneratorBase* generator, mitk::DataNode* parentNode = nullptr);
    ~DescriptionParameterBackgroundJob() override;

    /** \brief Execute the parameter image generation in the background thread. */
    void run() override;

    /** \brief Get the parent node for the results of this job.
     *
     * \return The parent data node, or \c nullptr if none was specified.
     */
    mitk::DataNode* GetParentNode() const;

signals:
    /** \brief Emitted when the job has finished (successfully or not). */
    void Finished();

    /** \brief Emitted when an error occurs during computation.
     * \param[in] err The error message.
     */
    void Error(QString err);

    /** \brief Emitted when result data nodes are available.
     * \param[in] resultMap The vector of result data nodes.
     * \param[in] pJob      Pointer to this job instance.
     */
    void ResultsAreAvailable(mitk::modelFit::ModelFitResultNodeVectorType resultMap, const DescriptionParameterBackgroundJob* pJob);

    /** \brief Emitted to report computation progress.
     * \param[in] progress Progress value in the range [0.0, 1.0].
     */
    void JobProgress(double progress);

    /** \brief Emitted when the job status changes.
     * \param[in] info A human-readable status message.
     */
    void JobStatusChanged(QString info);

protected:
  static mitk::modelFit::ModelFitResultNodeVectorType CreateResultNodes(const mitk::DescriptionParameterImageGeneratorBase::ParameterImageMapType& paramimages);

	//Inputs
	mitk::DescriptionParameterImageGeneratorBase::Pointer m_Generator;
  mitk::DataNode::Pointer m_ParentNode;

  // Results
  mitk::modelFit::ModelFitResultNodeVectorType m_Results;

  ::itk::MemberCommand<DescriptionParameterBackgroundJob>::Pointer m_spCommand;
  unsigned long m_ObserverID;

  void OnComputeEvent(::itk::Object *, const itk::EventObject &event);
};

#endif
