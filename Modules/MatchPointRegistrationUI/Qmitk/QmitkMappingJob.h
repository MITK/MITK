/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef __QMITK_MAPPING_JOB_H
#define __QMITK_MAPPING_JOB_H

// QT
#include <QObject>
#include <QRunnable>

// ITK
#include "itkCommand.h"

// MatchPoint
#include <mapRegistrationBase.h>

// MITK
#include <mitkDataNode.h>
#include <mitkImageMappingHelper.h>
#include <mitkPointSet.h>

#include <MitkMatchPointRegistrationUIExports.h>
#include <mitkUIDHelper.h>

struct MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkMappingJobSettings
{
public:
  /**Job name*/
  std::string m_MappedName;
  /**Indicates of mapper should try to refine geometry (true) or map the data (false)*/
  bool m_doGeometryRefinement;
  /**Indicates if the mapper should allow undefined pixels (true) or mapping should fail (false)*/
  bool m_allowUndefPixels;
  /** Value of undefined pixels. Only relevant if m_allowUndefPixels is true. */
  double m_paddingValue;
  /**Indicates if the mapper should allow pixels that are not covered by the registration (true) or mapping should fail
   * (false)*/
  bool m_allowUnregPixels;
  /** Value of unreged pixels. Only relevant if m_allowUnregPixels is true. */
  double m_errorValue;
  /** Type of interpolator. Only relevant for images and if m_doGeometryRefinement is false. */
  mitk::ImageMappingInterpolator::Type m_InterpolatorType;
  /** Display name of the interpolator*/
  std::string m_InterpolatorLabel;

  QmitkMappingJobSettings();
};

class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkMappingJob : public QObject,
                                                            public QRunnable,
                                                            public QmitkMappingJobSettings
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  QmitkMappingJob();
  ~QmitkMappingJob() override;

  void run() override;

signals:
  void Error(QString err);
  /**Signal is emitted to return the mapped data itself. Use it if you are only interested in the mapped data*/
  void MapResultIsAvailable(mitk::BaseData::Pointer spMappedData, const QmitkMappingJob *job);
  void AlgorithmInfo(QString info);

public:
  // Inputs
  mitk::DataNode::Pointer m_spRegNode;
  mitk::BaseData::ConstPointer m_spInputData;
  mitk::NodeUIDType m_InputDataUID;
  mitk::BaseGeometry::Pointer m_spRefGeometry;

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
