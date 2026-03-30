/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMappingJob_h
#define QmitkMappingJob_h

// QT
#include <QObject>
#include <QRunnable>

// ITK
#include <itkCommand.h>

// MatchPoint
#include <mapRegistrationBase.h>

// MITK
#include <mitkDataNode.h>
#include <mitkImageMappingHelper.h>
#include <mitkPointSet.h>

#include <MitkMatchPointRegistrationUIExports.h>
#include <mitkUIDHelper.h>

/**
 * \struct QmitkMappingJobSettings
 * \brief Settings structure controlling the behavior of a mapping (image/point set transformation) job.
 *
 * Collects all parameters that influence how data is mapped using a registration:
 * interpolation type, handling of undefined and unregistered pixels, padding/error
 * values, and whether to perform geometry refinement instead of full mapping.
 *
 * \sa QmitkMappingJob, QmitkFramesRegistrationJob, QmitkMapperSettingsWidget
 */
struct MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkMappingJobSettings
{
public:
  /** \brief Display name for the mapped result. */
  std::string m_MappedName;

  /** \brief If true, the mapper refines the image geometry instead of performing a full pixel-wise mapping. */
  bool m_doGeometryRefinement;

  /** \brief If true, undefined pixels (outside the field of view) are allowed and filled with m_paddingValue.
   *  If false, the mapping will fail when undefined pixels are encountered. */
  bool m_allowUndefPixels;

  /** \brief Value assigned to undefined pixels. Only relevant if m_allowUndefPixels is true. */
  double m_paddingValue;

  /** \brief If true, pixels not covered by the registration are allowed and filled with m_errorValue.
   *  If false, the mapping will fail when unregistered pixels are encountered. */
  bool m_allowUnregPixels;

  /** \brief Value assigned to unregistered pixels. Only relevant if m_allowUnregPixels is true. */
  double m_errorValue;

  /** \brief Interpolation method used for image mapping. Only relevant for images when
   *  m_doGeometryRefinement is false. */
  mitk::ImageMappingInterpolator::Type m_InterpolatorType;

  /** \brief Human-readable display name of the selected interpolation method. */
  std::string m_InterpolatorLabel;

  /**
   * \brief Constructs default mapping job settings.
   *
   * Defaults: no geometry refinement, undefined and unregistered pixels allowed,
   * padding and error values set to 0, linear interpolation.
   */
  QmitkMappingJobSettings();
};

/**
 * \class QmitkMappingJob
 * \brief QRunnable job that maps (transforms) data using a MatchPoint registration in a background thread.
 *
 * This job applies a registration stored in a mitk::MAPRegistrationWrapper (wrapped in a
 * DataNode) to input data, which can be a mitk::Image, mitk::PointSet, or
 * mitk::MultiLabelSegmentation. The mapping behavior is controlled by the inherited
 * QmitkMappingJobSettings.
 *
 * When m_doGeometryRefinement is true, the job performs geometry refinement instead of
 * full pixel-wise mapping (only applicable to images).
 *
 * The job emits MapResultIsAvailable with the transformed data upon success, or Error
 * with a description string upon failure.
 *
 * \sa QmitkMappingJobSettings, QmitkRegistrationJob, QmitkFramesRegistrationJob
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkMappingJob : public QObject,
                                                            public QRunnable,
                                                            public QmitkMappingJobSettings
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  /** \brief Default constructor. Initializes with default mapping settings. */
  QmitkMappingJob();

  /** \brief Destructor. */
  ~QmitkMappingJob() override;

  /**
   * \brief Executes the mapping or geometry refinement operation.
   *
   * Depending on m_doGeometryRefinement, either refines the image geometry or maps the
   * input data (image, point set, or multi-label segmentation) using the stored registration.
   * Emits MapResultIsAvailable on success or Error on failure.
   */
  void run() override;

signals:
  /**
   * \brief Emitted when an error occurs during mapping.
   * \param[in] err Description of the error.
   */
  void Error(QString err);

  /**
   * \brief Emitted when the mapped result data is available.
   * \param[in] spMappedData The transformed/mapped data.
   * \param[in] job Pointer to this job instance for context.
   */
  void MapResultIsAvailable(mitk::BaseData::Pointer spMappedData, const QmitkMappingJob *job);

  /**
   * \brief Emitted for general MatchPoint algorithm event messages during mapping.
   * \param[in] info Information message string.
   */
  void AlgorithmInfo(QString info);

public:
  /** \brief Data node containing the mitk::MAPRegistrationWrapper to use for mapping. */
  mitk::DataNode::Pointer m_spRegNode;

  /** \brief The input data to be mapped (image, point set, or multi-label segmentation). */
  mitk::BaseData::ConstPointer m_spInputData;

  /** \brief UID of the input data node for provenance tracking. */
  mitk::NodeUIDType m_InputDataUID;

  /** \brief Optional reference geometry for image mapping. If null, the registration determines the output geometry. */
  mitk::BaseGeometry::Pointer m_spRefGeometry;

  /**
   * \brief Returns the MatchPoint registration extracted from the registration data node.
   * \return Const pointer to the underlying MatchPoint registration.
   * \pre m_spRegNode must contain a valid mitk::MAPRegistrationWrapper.
   */
  const map::core::RegistrationBase *GetRegistration() const;

protected:
  // mapped data.
  mitk::BaseData::Pointer m_spMappedData;

  ::itk::MemberCommand<QmitkMappingJob>::Pointer m_spCommand;
  unsigned long m_ObserverID;

  // Helper functions
  const mitk::Image *GetInputDataAsImage() const;
  const mitk::PointSet *GetInputDataAsPointSet() const;

  void OnMapAlgorithmEvent(::itk::Object *, const itk::EventObject &event);
};

#endif
