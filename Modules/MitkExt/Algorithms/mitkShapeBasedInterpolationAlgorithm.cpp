/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkShapeBasedInterpolationAlgorithm.h"
#include "mitkImageCast.h"

#include "ipSegmentation.h"

mitk::Image::Pointer 
mitk::ShapeBasedInterpolationAlgorithm::Interpolate(
                               Image::ConstPointer lowerSlice, unsigned int lowerSliceIndex,
                               Image::ConstPointer upperSlice, unsigned int upperSliceIndex,
                               unsigned int requestedIndex,
                               unsigned int /*sliceDimension*/, // commented variables are not used
                               Image::Pointer resultImage,
                               unsigned int /*timeStep*/,
                               Image::ConstPointer /*referenceImage*/)
{
  // convert these slices to the ipSegmentation data type (into an ITK image)
  itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer correctPixelTypeLowerITKSlice;
  CastToItkImage( lowerSlice, correctPixelTypeLowerITKSlice );
  assert ( correctPixelTypeLowerITKSlice.IsNotNull() );

  itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer correctPixelTypeUpperITKSlice;
  CastToItkImage( upperSlice, correctPixelTypeUpperITKSlice );
  assert ( correctPixelTypeUpperITKSlice.IsNotNull() );

  // correct direction info
  itk::Image< ipMITKSegmentationTYPE, 2 >::DirectionType imageDirection;
  imageDirection.SetIdentity();
  correctPixelTypeLowerITKSlice->SetDirection(imageDirection);
  correctPixelTypeUpperITKSlice->SetDirection(imageDirection);

  // back-convert to MITK images to access a mitkIpPicDescriptor
  Image::Pointer correctPixelTypeLowerMITKSlice = Image::New();
  CastToMitkImage( correctPixelTypeLowerITKSlice, correctPixelTypeLowerMITKSlice );
  mitkIpPicDescriptor* lowerPICSlice = correctPixelTypeLowerMITKSlice->GetSliceData()->GetPicDescriptor();
  
  Image::Pointer correctPixelTypeUpperMITKSlice = Image::New();
  CastToMitkImage( correctPixelTypeUpperITKSlice, correctPixelTypeUpperMITKSlice );
  mitkIpPicDescriptor* upperPICSlice = correctPixelTypeUpperMITKSlice->GetSliceData()->GetPicDescriptor();

  // calculate where the current slice is in comparison to the lower and upper neighboring slices
  float ratio = (float)(requestedIndex - lowerSliceIndex) / (float)(upperSliceIndex - lowerSliceIndex);

  mitkIpPicDescriptor* ipPicResult = ipMITKSegmentationInterpolate( lowerPICSlice, upperPICSlice, ratio ); // magic
  if (!ipPicResult) return NULL;

  Geometry3D::Pointer originalGeometry = resultImage->GetGeometry();
  resultImage->Initialize( ipPicResult );
  resultImage->SetPicSlice( ipPicResult );
  resultImage->SetGeometry( originalGeometry );

  mitkIpPicFree( ipPicResult );

  return resultImage;
}

