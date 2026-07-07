/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRegistrationJob_h
#define QmitkRegistrationJob_h

// QT
#include <QObject>
#include <QRunnable>

// ITK
#include <itkCommand.h>

// MITK
#include <mitkUIDHelper.h>
#include <mitkDataNode.h>
#include <mitkImage.h>
#include <mitkMAPRegistrationWrapper.h>

// MatchPoint
#include <mapDeploymentDLLInfo.h>
#include <mapIterativeAlgorithmInterface.h>
#include <mapMultiResRegistrationAlgorithmInterface.h>
#include <mapRegistrationAlgorithmBase.h>
#include <mapRegistrationBase.h>

#include <MitkMatchPointRegistrationUIExports.h>

/**
 * \class QmitkRegistrationJob
 * \brief QRunnable job that performs image registration using a MatchPoint algorithm in a background thread.
 *
 * This job executes a MatchPoint registration algorithm on a pair of moving and target
 * data sets (typically images) in a QThreadPool, keeping the GUI thread responsive. It
 * observes algorithm events (iterations, level changes, status updates) and relays them
 * as Qt signals for progress reporting.
 *
 * Optional masks can be set for both moving and target data. On successful completion,
 * the job emits RegResultIsAvailable with a mitk::MAPRegistrationWrapper containing
 * the computed registration. The Finished signal is always emitted at the end.
 *
 * \sa QmitkFramesRegistrationJob, QmitkMappingJob, mitk::MAPAlgorithmHelper
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkRegistrationJob : public QObject, public QRunnable
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  /**
   * \brief Constructs the registration job with the given algorithm.
   *
   * Registers an ITK observer on the algorithm to receive algorithm events
   * and initializes default member values.
   *
   * \param[in] pAlgorithm Pointer to the MatchPoint registration algorithm to use.
   *   The job takes shared ownership via a smart pointer.
   * \pre pAlgorithm must not be \c nullptr.
   */
  QmitkRegistrationJob(::map::algorithm::RegistrationAlgorithmBase *pAlgorithm);

  /**
   * \brief Destructor. Removes the algorithm event observer.
   */
  ~QmitkRegistrationJob() override;

  /**
   * \brief Executes the registration algorithm.
   *
   * Sets moving and target data (and optional masks) on the algorithm via
   * mitk::MAPAlgorithmHelper, runs the registration, and wraps the result
   * in a mitk::MAPRegistrationWrapper. Emits RegResultIsAvailable on success
   * or Error on failure. Always emits Finished.
   */
  void run() override;

signals:
  /** \brief Emitted when the job has completed (regardless of success or failure). */
  void Finished();

  /**
   * \brief Emitted when an error occurs during registration.
   * \param[in] err Description of the error.
   */
  void Error(QString err);

  /**
   * \brief Emitted when the registration result is available.
   * \param[in] spResultRegistration The computed registration wrapped in a MAPRegistrationWrapper.
   * \param[in] pJob Pointer to this job instance for context.
   */
  void RegResultIsAvailable(mitk::MAPRegistrationWrapper::Pointer spResultRegistration,
                            const QmitkRegistrationJob *pJob);

  /**
   * \brief Emitted when the algorithm completes an iteration.
   * \param[in] info Descriptive text about the iteration.
   * \param[in] hasIterationCount True if the algorithm provides an iteration count.
   * \param[in] currentIteration The current iteration number (valid only if \p hasIterationCount is true).
   */
  void AlgorithmIterated(QString info, bool hasIterationCount, unsigned long currentIteration);

  /**
   * \brief Emitted when the algorithm changes its multi-resolution level.
   * \param[in] info Descriptive text about the level change.
   * \param[in] hasLevelCount True if the algorithm provides a level count.
   * \param[in] currentLevel The current level number (valid only if \p hasLevelCount is true).
   */
  void LevelChanged(QString info, bool hasLevelCount, unsigned long currentLevel);

  /**
   * \brief Emitted when the algorithm status changes (e.g., initializing, starting, stopping).
   * \param[in] info Status description string.
   */
  void AlgorithmStatusChanged(QString info);

  /**
   * \brief Emitted for general algorithm information messages.
   * \param[in] info Information message string.
   */
  void AlgorithmInfo(QString info);

public:
  /** \brief The target (fixed) data for the registration. */
  mitk::BaseData::ConstPointer m_spTargetData;

  /** \brief The moving data for the registration. */
  mitk::BaseData::ConstPointer m_spMovingData;

  /** \brief Optional mask image for the target data. */
  mitk::Image::ConstPointer m_spTargetMask;

  /** \brief Optional mask image for the moving data. */
  mitk::Image::ConstPointer m_spMovingMask;

  /** \brief If true, the job also maps the moving entity using the computed registration. */
  bool m_MapEntity;

  /** \brief If true, the registration result is stored. */
  bool m_StoreReg;

  /** \brief Flag indicating whether an error occurred during execution. */
  bool m_ErrorOccured;

  /** \brief Human-readable name for the registration job. */
  std::string m_JobName;

  /** \brief UID of the target data node for provenance tracking. */
  mitk::NodeUIDType m_TargetDataUID;

  /** \brief UID of the moving data node for provenance tracking. */
  mitk::NodeUIDType m_MovingDataUID;

  /** \brief UID of the target mask data node for provenance tracking. */
  mitk::NodeUIDType m_TargetMaskDataUID;

  /** \brief UID of the moving mask data node for provenance tracking. */
  mitk::NodeUIDType m_MovingMaskDataUID;

  /**
   * \brief Returns the registration algorithm used by this job.
   * \return Const pointer to the loaded MatchPoint registration algorithm.
   */
  const ::map::algorithm::RegistrationAlgorithmBase *GetLoadedAlgorithm() const;

protected:
  typedef ::map::algorithm::facet::IterativeAlgorithmInterface IIterativeAlgorithm;
  typedef ::map::algorithm::facet::MultiResRegistrationAlgorithmInterface IMultiResAlgorithm;

  // Result registration.
  ::map::core::RegistrationBase::Pointer m_spResultRegistration;
  mitk::DataNode::Pointer m_spRegNode;
  // mapped image. May be null if m_MapEntity is false.
  mitk::DataNode::Pointer m_spMappedImageNode;

  ::itk::MemberCommand<QmitkRegistrationJob>::Pointer m_spCommand;
  unsigned long m_ObserverID;
  ::map::algorithm::RegistrationAlgorithmBase::Pointer m_spLoadedAlgorithm;

  // Helper functions
  const mitk::Image *GetTargetDataAsImage() const;
  const mitk::Image *GetMovingDataAsImage() const;

  void OnMapAlgorithmEvent(::itk::Object *, const itk::EventObject &event);
};

#endif
