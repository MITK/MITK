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

#ifndef _TrackingHandlerTensor
#define _TrackingHandlerTensor

#include "mitkTrackingDataHandler.h"
#include <itkDiffusionTensor3D.h>
#include <MitkFiberTrackingExports.h>

namespace mitk
{

/**
* \brief Enables streamline tracking on tensor images. Supports multi tensor tracking by adding multiple tensor images. */

class MITKFIBERTRACKING_EXPORT TrackingHandlerTensor : public TrackingDataHandler
{

public:

    TrackingHandlerTensor();
    ~TrackingHandlerTensor();

    typedef itk::DiffusionTensor3D<float>    TensorType;
    typedef itk::Image< TensorType, 3 > ItkTensorImageType;
    typedef itk::Image< vnl_vector_fixed<float,3>, 3>  ItkPDImgType;


    void InitForTracking();     ///< calls InputDataValidForTracking() and creates feature images
    vnl_vector_fixed<float,3> ProposeDirection(itk::Point<float, 3>& pos, std::deque< vnl_vector_fixed<float,3> >& olddirs, itk::Index<3>& oldIndex);  ///< predicts next progression direction at the given position

    void SetF(float f){ m_F = f; }
    void SetG(float g){ m_G = g; }
    void SetFaThreshold(float FaThreshold){ m_FaThreshold = FaThreshold; }
    void AddTensorImage( ItkTensorImageType::Pointer img ){ m_TensorImages.push_back(img); }
    void ClearTensorImages(){ m_TensorImages.clear(); }
    void SetFaImage( ItkFloatImgType::Pointer img ){ m_FaImage = img; }
    void SetInterpolateTensors( bool interpolateTensors ){ m_InterpolateTensors = interpolateTensors; }
    void SetMode( MODE m )
    {
        if (m==MODE::DETERMINISTIC)
            m_Mode = m;
        else
            mitkThrow() << "Peak tracker is only implemented for deterministic mode.";
    }


    ItkUcharImgType::SpacingType GetSpacing(){ return m_FaImage->GetSpacing(); }
    itk::Point<float,3> GetOrigin(){ return m_FaImage->GetOrigin(); }
    ItkUcharImgType::DirectionType GetDirection(){ return m_FaImage->GetDirection(); }
    ItkUcharImgType::RegionType GetLargestPossibleRegion(){ return m_FaImage->GetLargestPossibleRegion(); }

protected:

    vnl_vector_fixed<float,3> GetMatchingDirection(itk::Index<3> idx, vnl_vector_fixed<float,3>& oldDir, int& image_num);
    vnl_vector_fixed<float,3> GetDirection(itk::Point<float, 3> itkP, vnl_vector_fixed<float,3> oldDir, TensorType& tensor);
    vnl_vector_fixed<float,3> GetLargestEigenvector(TensorType& tensor);

    float   m_FaThreshold;
    float   m_F;
    float   m_G;

    std::vector< ItkDoubleImgType::Pointer >        m_EmaxImage;    ///< Stores largest eigenvalues per voxel (one for each tensor)
    ItkFloatImgType::Pointer                        m_FaImage;      ///< FA image used to determine streamline termination.
    std::vector< ItkPDImgType::Pointer >            m_PdImage;      ///< Stores principal direction of each tensor in each voxel.
    std::vector< ItkTensorImageType::Pointer >      m_TensorImages;   ///< Input tensor images. For multi tensor tracking provide multiple tensor images.
    bool                                            m_InterpolateTensors;   ///< If false, then the peaks are interpolated. Otherwiese, The tensors are interpolated.
    int                                             m_NumberOfInputs;
};

}

#endif
