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
#include <mitkQBallImage.h>
#include <itkOrientationDistributionFunction.h>
#include <MitkFiberTrackingExports.h>

namespace mitk
{

/**
* \brief Enables streamline tracking on tensor images. Supports multi tensor tracking by adding multiple tensor images. */

class MITKFIBERTRACKING_EXPORT TrackingHandlerOdf : public TrackingDataHandler
{

public:

    TrackingHandlerOdf();
    ~TrackingHandlerOdf();

    typedef itk::DiffusionTensor3D<float>    TensorType;
    typedef itk::Image< itk::Vector< float, QBALL_ODFSIZE >, 3 > ItkOdfImageType;
    typedef itk::Image< vnl_vector_fixed<float,3>, 3>  ItkPDImgType;


    void InitForTracking();     ///< calls InputDataValidForTracking() and creates feature images
    vnl_vector_fixed<float,3> ProposeDirection(itk::Point<float, 3>& pos, std::deque< vnl_vector_fixed<float,3> >& olddirs, itk::Index<3>& oldIndex);  ///< predicts next progression direction at the given position


    void SetGfaThreshold(float gfaThreshold){ m_GfaThreshold = gfaThreshold; }
    void SetOdfImage( ItkOdfImageType::Pointer img ){ m_OdfImage = img; }
    void SetGfaImage( ItkFloatImgType::Pointer img ){ m_GfaImage = img; }
    void SetMode( MODE m ){ m_Mode = m; }


    ItkUcharImgType::SpacingType GetSpacing(){ return m_OdfImage->GetSpacing(); }
    itk::Point<float,3> GetOrigin(){ return m_OdfImage->GetOrigin(); }
    ItkUcharImgType::DirectionType GetDirection(){ return m_OdfImage->GetDirection(); }
    ItkUcharImgType::RegionType GetLargestPossibleRegion(){ return m_OdfImage->GetLargestPossibleRegion(); }

    int OdfPower() const;
    void SetOdfPower(int OdfPower);

    void SetSecondOrder(bool SecondOrder);

    bool MinMaxNormalize() const;
    void setMinMaxNormalize(bool MinMaxNormalize);

protected:

    vnl_vector< float > GetSecondOrderProbabilities(itk::Point<float, 3>& itkP, vnl_vector< float >& angles, vnl_vector< float >& probs);

    float   m_GfaThreshold;
    ItkFloatImgType::Pointer        m_GfaImage;     ///< GFA image used to determine streamline termination.
    ItkOdfImageType::Pointer        m_OdfImage;     ///< Input odf image.
    ItkOdfImageType::Pointer        m_WorkingOdfImage;     ///< Modified odf image.
    std::vector< int >              m_OdfHemisphereIndices;
    vnl_matrix< float >             m_OdfFloatDirs;
    BoostRngType                    m_Rng;
    int                             m_OdfPower;
    bool                            m_SecondOrder;
    bool                            m_MinMaxNormalize;

    std::vector< int >              m_OdfReducedIndices;
};

}

#endif
