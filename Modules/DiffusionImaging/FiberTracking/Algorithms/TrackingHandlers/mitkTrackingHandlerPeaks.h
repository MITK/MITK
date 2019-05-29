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

#ifndef _TrackingHandlerPeaks
#define _TrackingHandlerPeaks

#include "mitkTrackingDataHandler.h"
#include <itkDiffusionTensor3D.h>
#include <MitkFiberTrackingExports.h>

namespace mitk
{

/**
* \brief Enables deterministic streamline tracking on MRtrix style peak images (4D float images) */

class MITKFIBERTRACKING_EXPORT TrackingHandlerPeaks : public TrackingDataHandler
{

public:

  TrackingHandlerPeaks();
  ~TrackingHandlerPeaks() override;

  typedef itk::Image< float, 4 >  PeakImgType; ///< MRtrix peak image type


  void InitForTracking() override;     ///< calls InputDataValidForTracking() and creates feature images
  vnl_vector_fixed<float,3> ProposeDirection(const itk::Point<float, 3>& pos, std::deque< vnl_vector_fixed<float,3> >& olddirs, itk::Index<3>& oldIndex) override;  ///< predicts next progression direction at the given position
  bool WorldToIndex(itk::Point<float, 3>& pos, itk::Index<3>& index) override;

  void SetPeakImage( PeakImgType::Pointer image ){ m_PeakImage = image; DataModified(); }

  itk::Vector<double, 3> GetSpacing() override{ return spacing3; }
  itk::Point<float,3> GetOrigin() override{ return origin3; }
  itk::Matrix<double, 3, 3> GetDirection() override{ return direction3; }
  itk::ImageRegion<3> GetLargestPossibleRegion() override{ return imageRegion3; }

protected:

  vnl_vector_fixed<float,3> GetDirection(itk::Point<float, 3> itkP, bool interpolate, vnl_vector_fixed<float,3> oldDir);
  vnl_vector_fixed<float,3> GetMatchingDirection(itk::Index<3> idx3, vnl_vector_fixed<float,3>& oldDir);
  vnl_vector_fixed<float,3> GetDirection(itk::Index<3> idx3, int dirIdx);

  PeakImgType::ConstPointer m_PeakImage;
  int m_NumDirs;

  itk::Vector<double, 3> spacing3;
  itk::Point<float, 3> origin3;
  itk::Matrix<double, 3, 3> direction3;
  itk::ImageRegion<3> imageRegion3;

  ItkUcharImgType::Pointer m_DummyImage;
};

}

#endif
