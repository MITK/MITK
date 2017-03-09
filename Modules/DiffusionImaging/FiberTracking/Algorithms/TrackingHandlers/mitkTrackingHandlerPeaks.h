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
* \brief  */

class MITKFIBERTRACKING_EXPORT TrackingHandlerPeaks : public TrackingDataHandler
{

public:

    TrackingHandlerPeaks();
    ~TrackingHandlerPeaks();

    typedef itk::Image< float, 4 >  PeakImgType; ///< MRtrix peak image type


    void InitForTracking();     ///< calls InputDataValidForTracking() and creates feature images
    vnl_vector_fixed<double,3> ProposeDirection(itk::Point<double, 3>& pos, int& candidates, std::deque< vnl_vector_fixed<double,3> >& olddirs, double& w, itk::Index<3>& oldIndex, ItkUcharImgType::Pointer mask=nullptr);  ///< predicts next progression direction at the given position

    void SetPeakThreshold(float thr){ m_PeakThreshold = thr; }
    void SetPeakImage( PeakImgType::Pointer image ){ m_PeakImage = image; }

    itk::Vector<double, 3> GetSpacing(){ return spacing3; }
    itk::Point<float,3> GetOrigin(){ return origin3; }
    itk::Matrix<double, 3, 3> GetDirection(){ return direction3; }
    itk::ImageRegion<3> GetLargestPossibleRegion(){ return imageRegion3; }

protected:

    vnl_vector_fixed<double,3> GetDirection(itk::Point<float, 3> itkP, bool interpolate, vnl_vector_fixed<double,3> oldDir);
    vnl_vector_fixed<double,3> GetMatchingDirection(itk::Index<3> idx3, vnl_vector_fixed<double,3>& oldDir);
    vnl_vector_fixed<double,3> GetDirection(itk::Index<3> idx3, int dirIdx);

    PeakImgType::Pointer m_PeakImage;
    float m_PeakThreshold;
    int m_NumDirs;

    itk::Vector<double, 3> spacing3;
    itk::Point<float, 3> origin3;
    itk::Matrix<double, 3, 3> direction3;
    itk::ImageRegion<3> imageRegion3;

    ItkUcharImgType::Pointer m_DummyImage;
};

}

#endif
