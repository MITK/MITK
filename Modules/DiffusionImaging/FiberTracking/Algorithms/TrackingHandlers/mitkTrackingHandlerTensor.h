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
#include <mitkTensorImage.h>
#include <itkLinearInterpolateImageFunction.h>

namespace mitk
{

/**
* \brief Enables streamline tracking on tensor images. Supports multi tensor tracking by adding multiple tensor images. */

class MITKFIBERTRACKING_EXPORT TrackingHandlerTensor : public TrackingDataHandler
{

public:

  TrackingHandlerTensor();
  ~TrackingHandlerTensor() override;

  typedef TensorImage::PixelType    TensorType;
  typedef TensorImage::ItkTensorImageType ItkTensorImageType;
  typedef itk::Image< vnl_vector_fixed<float,3>, 3>  ItkPDImgType;


  void InitForTracking() override;     ///< calls InputDataValidForTracking() and creates feature images
  vnl_vector_fixed<float,3> ProposeDirection(const itk::Point<float, 3>& pos, std::deque< vnl_vector_fixed<float,3> >& olddirs, itk::Index<3>& oldIndex) override;  ///< predicts next progression direction at the given position
  bool WorldToIndex(itk::Point<float, 3>& pos, itk::Index<3>& index) override;

  void AddTensorImage( ItkTensorImageType::ConstPointer img ){ m_TensorImages.push_back(img); DataModified(); }
  void SetTensorImage( ItkTensorImageType::ConstPointer img ){ m_TensorImages.clear(); m_TensorImages.push_back(img); DataModified(); }
  void ClearTensorImages(){ m_TensorImages.clear(); DataModified(); }
  void SetFaImage( ItkFloatImgType::Pointer img ){ m_FaImage = img; DataModified(); }
  void SetInterpolateTensors( bool interpolateTensors ){ m_InterpolateTensors = interpolateTensors; }
  int GetNumTensorImages() const { return m_TensorImages.size(); }


  ItkUcharImgType::SpacingType GetSpacing() override{ return m_FaImage->GetSpacing(); }
  itk::Point<float,3> GetOrigin() override{ return m_FaImage->GetOrigin(); }
  ItkUcharImgType::DirectionType GetDirection() override{ return m_FaImage->GetDirection(); }
  ItkUcharImgType::RegionType GetLargestPossibleRegion() override{ return m_FaImage->GetLargestPossibleRegion(); }

protected:

  vnl_vector_fixed<float,3> GetMatchingDirection(itk::Index<3> idx, vnl_vector_fixed<float,3>& oldDir, int& image_num);
  vnl_vector_fixed<float,3> GetDirection(itk::Point<float, 3> itkP, vnl_vector_fixed<float,3> oldDir, TensorType& tensor);
  vnl_vector_fixed<float,3> GetLargestEigenvector(TensorType& tensor);

  std::vector< ItkDoubleImgType::Pointer >        m_EmaxImage;    ///< Stores largest eigenvalues per voxel (one for each tensor)
  ItkFloatImgType::Pointer                        m_FaImage;      ///< FA image used to determine streamline termination.
  std::vector< ItkPDImgType::Pointer >            m_PdImage;      ///< Stores principal direction of each tensor in each voxel.
  std::vector< ItkTensorImageType::ConstPointer > m_TensorImages;   ///< Input tensor images. For multi tensor tracking provide multiple tensor images.
  bool                                            m_InterpolateTensors;   ///< If false, then the peaks are interpolated. Otherwiese, The tensors are interpolated.
  int                                             m_NumberOfInputs;

  itk::LinearInterpolateImageFunction< itk::Image< float, 3 >, float >::Pointer   m_FaInterpolator;
};

}

#endif
