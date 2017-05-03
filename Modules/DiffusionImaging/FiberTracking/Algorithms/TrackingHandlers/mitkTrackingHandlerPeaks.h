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
    ~TrackingHandlerPeaks();

    typedef itk::Image< float, 4 >  PeakImgType; ///< MRtrix peak image type


    void InitForTracking();     ///< calls InputDataValidForTracking() and creates feature images
    vnl_vector_fixed<float,3> ProposeDirection(itk::Point<float, 3>& pos, std::deque< vnl_vector_fixed<float,3> >& olddirs, itk::Index<3>& oldIndex);  ///< predicts next progression direction at the given position

    void SetPeakThreshold(float thr){ m_PeakThreshold = thr; }
    void SetPeakImage( PeakImgType::Pointer image ){ m_PeakImage = image; }

    itk::Vector<double, 3> GetSpacing(){ return spacing3; }
    itk::Point<float,3> GetOrigin(){ return origin3; }
    itk::Matrix<double, 3, 3> GetDirection(){ return direction3; }
    itk::ImageRegion<3> GetLargestPossibleRegion(){ return imageRegion3; }
    void SetMode( MODE m )
    {
        if (m==MODE::DETERMINISTIC)
            m_Mode = m;
        else
            mitkThrow() << "Peak tracker is only implemented for deterministic mode.";
    }

protected:

    vnl_vector_fixed<float,3> GetDirection(itk::Point<float, 3> itkP, bool interpolate, vnl_vector_fixed<float,3> oldDir);
    vnl_vector_fixed<float,3> GetMatchingDirection(itk::Index<3> idx3, vnl_vector_fixed<float,3>& oldDir);
    vnl_vector_fixed<float,3> GetDirection(itk::Index<3> idx3, int dirIdx);

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
