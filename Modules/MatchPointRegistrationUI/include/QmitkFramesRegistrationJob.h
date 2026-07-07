/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFramesRegistrationJob_h
#define QmitkFramesRegistrationJob_h

// QT
#include <QObject>
#include <QRunnable>

// ITK
#include <itkCommand.h>

// MITK
#include <QmitkMappingJob.h>
#include <mitkDataNode.h>
#include <mitkImage.h>

// MatchPoint
#include <mapDeploymentDLLInfo.h>
#include <mapIterativeAlgorithmInterface.h>
#include <mapMultiResRegistrationAlgorithmInterface.h>
#include <mapRegistrationAlgorithmBase.h>
#include <mapRegistrationBase.h>

// Map4CTK
#include <mitkUIDHelper.h>
#include <mitkTimeFramesRegistrationHelper.h>

#include <MitkMatchPointRegistrationUIExports.h>

/**
 * \class QmitkFramesRegistrationJob
 * \brief QRunnable job that performs frame-by-frame registration of a 4D image in a background thread.
 *
 * This job registers individual time frames of a 4D image using a MatchPoint registration
 * algorithm and maps the registered frames back into a single result image. It is typically
 * used by GUI plugins to keep the UI responsive while the potentially long-running
 * registration and mapping operations proceed in a QThreadPool.
 *
 * The job inherits mapping settings from QmitkMappingJobSettings (interpolation, padding,
 * error handling) and observes algorithm events to relay progress and status information
 * via Qt signals.
 *
 * \sa QmitkRegistrationJob, QmitkMappingJob, QmitkMappingJobSettings
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkFramesRegistrationJob : public QObject,
                                                                       public QRunnable,
                                                                       public QmitkMappingJobSettings
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  /**
   * \brief Constructs the frames registration job with the given algorithm.
   *
   * Registers an ITK observer on the algorithm to receive algorithm events.
   *
   * \param[in] pAlgorithm Pointer to the MatchPoint registration algorithm to use.
   *   The job takes shared ownership via a smart pointer.
   * \pre pAlgorithm must not be \c nullptr.
   */
  QmitkFramesRegistrationJob(map::algorithm::RegistrationAlgorithmBase *pAlgorithm);

  /**
   * \brief Destructor. Removes the algorithm event observer.
   */
  ~QmitkFramesRegistrationJob() override;

  /**
   * \brief Executes the frame-by-frame registration and mapping.
   *
   * Uses mitk::TimeFramesRegistrationHelper to iterate over all time frames (except
   * those in the ignore list), register each frame, and map it according to the
   * configured mapping settings. Emits ResultIsAvailable on success, or Error on failure.
   * Always emits Finished when done.
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
   * \brief Emitted when the registered result image is available.
   * \param[in] spResult The resulting registered 4D image.
   * \param[in] pJob Pointer to this job instance for context.
   */
  void ResultIsAvailable(mitk::Image::Pointer spResult, const QmitkFramesRegistrationJob *pJob);

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

  /**
   * \brief Emitted during frame processing with overall progress.
   * \param[in] progress Progress value in the range [0, 1].
   */
  void FrameProcessed(double progress);

  /**
   * \brief Emitted when a single frame has been registered.
   * \param[in] progress Progress value in the range [0, 1].
   */
  void FrameRegistered(double progress);

  /**
   * \brief Emitted when a single frame has been mapped.
   * \param[in] progress Progress value in the range [0, 1].
   */
  void FrameMapped(double progress);

public:
  /** \brief The 4D target image data to be registered frame-by-frame. */
  mitk::BaseData::ConstPointer m_spTargetData;

  /** \brief Optional mask image applied to the target during registration. */
  mitk::Image::ConstPointer m_spTargetMask;

  /** \brief List of time frame indices to exclude from registration. */
  mitk::TimeFramesRegistrationHelper::IgnoreListType m_IgnoreList;

  /** \brief UID of the target data node for provenance tracking. */
  mitk::NodeUIDType m_TargetDataUID;

  /** \brief UID of the target mask data node for provenance tracking. */
  mitk::NodeUIDType m_TargetMaskDataUID;

  /**
   * \brief Returns the registration algorithm used by this job.
   * \return Const pointer to the loaded MatchPoint registration algorithm.
   */
  const map::algorithm::RegistrationAlgorithmBase *GetLoadedAlgorithm() const;

private:
  typedef map::algorithm::facet::IterativeAlgorithmInterface IIterativeAlgorithm;
  typedef map::algorithm::facet::MultiResRegistrationAlgorithmInterface IMultiResAlgorithm;

  mitk::Image::Pointer m_spMappedImageNode;

  ::itk::MemberCommand<QmitkFramesRegistrationJob>::Pointer m_spCommand;
  unsigned long m_ObserverID;
  map::algorithm::RegistrationAlgorithmBase::Pointer m_spLoadedAlgorithm;

  mitk::TimeFramesRegistrationHelper::Pointer m_helper;

  // Helper functions
  const mitk::Image *GetTargetDataAsImage() const;

  void OnMapAlgorithmEvent(::itk::Object *, const itk::EventObject &event);
};

#endif
