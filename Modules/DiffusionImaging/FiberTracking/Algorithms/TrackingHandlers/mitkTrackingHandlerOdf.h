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

#ifndef _TrackingHandlerOdf
#define _TrackingHandlerOdf

#include "mitkTrackingDataHandler.h"
#include <mitkOdfImage.h>
#include <mitkTensorImage.h>
#include <itkOrientationDistributionFunction.h>
#include <MitkFiberTrackingExports.h>

namespace mitk
{

/**
* \brief Enables streamline tracking on sampled ODF images. */

class MITKFIBERTRACKING_EXPORT TrackingHandlerOdf : public TrackingDataHandler
{

public:

  TrackingHandlerOdf();
  ~TrackingHandlerOdf() override;

  typedef OdfImage::ItkOdfImageType ItkOdfImageType;
  typedef itk::Image< vnl_vector_fixed<float,3>, 3>  ItkPDImgType;


  void InitForTracking() override;     ///< calls InputDataValidForTracking() and creates feature images
  vnl_vector_fixed<float,3> ProposeDirection(const itk::Point<float, 3>& pos, std::deque< vnl_vector_fixed<float,3> >& olddirs, itk::Index<3>& oldIndex) override;  ///< predicts next progression direction at the given position
  bool WorldToIndex(itk::Point<float, 3>& pos, itk::Index<3>& index) override;

  void SetOdfImage( ItkOdfImageType::Pointer img ){ m_OdfImage = img; DataModified(); }
  void SetGfaImage( ItkFloatImgType::Pointer img ){ m_GfaImage = img; DataModified(); }

  ItkUcharImgType::SpacingType GetSpacing() override{ return m_OdfImage->GetSpacing(); }
  itk::Point<float,3> GetOrigin() override{ return m_OdfImage->GetOrigin(); }
  ItkUcharImgType::DirectionType GetDirection() override{ return m_OdfImage->GetDirection(); }
  ItkUcharImgType::RegionType GetLargestPossibleRegion() override{ return m_OdfImage->GetLargestPossibleRegion(); }

  int OdfPower() const;
  void SetNumProbSamples(int NumProbSamples);

  bool GetIsOdfFromTensor() const;
  void SetIsOdfFromTensor(bool OdfFromTensor);

protected:

  int SampleOdf(vnl_vector< float >& probs, vnl_vector< float >& angles);

  ItkFloatImgType::Pointer        m_GfaImage;     ///< GFA image used to determine streamline termination.
  ItkOdfImageType::Pointer        m_OdfImage;     ///< Input odf image.
  ItkOdfImageType::Pointer        m_WorkingOdfImage;     ///< Modified odf image.
  std::vector< int >              m_OdfHemisphereIndices;
  vnl_matrix< float >             m_OdfFloatDirs;
  int                             m_NumProbSamples;
  bool                            m_OdfFromTensor;

  itk::LinearInterpolateImageFunction< itk::Image< float, 3 >, float >::Pointer   m_GfaInterpolator;
  itk::LinearInterpolateImageFunction< itk::Image< ItkOdfImageType::PixelType, 3 >, float >::Pointer   m_OdfInterpolator;
};

}

#endif
