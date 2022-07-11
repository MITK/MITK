/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKMASKUTIL_TPP
#define MITKMASKUTIL_TPP

#include <mitkMaskUtilities.h>
#include <mitkImageAccessByItk.h>
#include <itkExtractImageFilter.h>
#include <itkChangeInformationImageFilter.h>
#include <mitkITKImageImport.h>

namespace mitk
{
    template <class TPixel, unsigned int VImageDimension>
    void MaskUtilities<TPixel, VImageDimension>::SetImage(const ImageType* image)
    {
        if (image != m_Image)
        {
            m_Image = image;
        }
    }

    template <class TPixel, unsigned int VImageDimension>
    void MaskUtilities<TPixel, VImageDimension>::SetMask(const MaskType* mask)
    {
        if (mask != m_Mask)
        {
            m_Mask = mask;
        }
    }

    template <class TPixel, unsigned int VImageDimension>
    bool MaskUtilities<TPixel, VImageDimension>::CheckMaskSanity()
        {
            if (m_Mask==nullptr || m_Image==nullptr)
            {
                MITK_ERROR << "Set an image and a mask first";
            }

            typedef itk::Image< TPixel, VImageDimension > ImageType;
            typedef typename ImageType::PointType PointType;
            typedef typename ImageType::DirectionType DirectionType;

            bool maskSanity = true;


            if (m_Mask==nullptr)
            {
                MITK_ERROR << "Something went wrong when casting the mitk mask image to an itk mask image. Do the mask and the input image have the same dimension?";
                // note to self: We could try to convert say a 2d mask to a 3d mask if the image is 3d. (mask and image dimension have to match.)
            }
            // check direction
            DirectionType imageDirection = m_Image->GetDirection();
            DirectionType maskDirection = m_Mask->GetDirection();
            for(unsigned int i = 0; i < imageDirection.ColumnDimensions; ++i )
            {
              for(unsigned int j = 0; j < imageDirection.ColumnDimensions; ++j )
              {
                double differenceDirection = imageDirection[i][j] - maskDirection[i][j];
                if (fabs(differenceDirection) > MASK_SUITABILITY_TOLERANCE_DIRECTION)
                {
                  maskSanity = false;
                  MITK_INFO << "Mask needs to have same direction as image! (Image direction: " << imageDirection << "; Mask direction: " << maskDirection << ")";
                }
              }
            }

            // check spacing
            PointType imageSpacing = m_Image->GetSpacing();
            PointType maskSpacing = m_Mask->GetSpacing();
            for (unsigned int i = 0; i < VImageDimension; i++)
            {
                if ( fabs( maskSpacing[i] - imageSpacing[i] ) > MASK_SUITABILITY_TOLERANCE_COORDINATE )
                {
                    maskSanity = false;
                    MITK_INFO << "Spacing of mask and image is not equal. Mask: " << maskSpacing << " image: " << imageSpacing;
                }
            }

            // check alignment
            // Make sure that the voxels of mask and image are correctly "aligned", i.e., voxel boundaries are the same in both images
            PointType imageOrigin = m_Image->GetOrigin();
            PointType maskOrigin = m_Mask->GetOrigin();

            typedef itk::ContinuousIndex<double, VImageDimension> ContinousIndexType;
            ContinousIndexType maskOriginContinousIndex, imageOriginContinousIndex;

            m_Image->TransformPhysicalPointToContinuousIndex(maskOrigin, maskOriginContinousIndex);
            m_Image->TransformPhysicalPointToContinuousIndex(imageOrigin, imageOriginContinousIndex);

            for ( unsigned int i = 0; i < ImageType::ImageDimension; ++i )
            {
              double misalignment = maskOriginContinousIndex[i] - floor( maskOriginContinousIndex[i] + 0.5 );
              // misalignment must be a multiple (int) of spacing in that direction
              if (  fmod(misalignment,imageSpacing[i])  > MASK_SUITABILITY_TOLERANCE_COORDINATE)
              {
                  maskSanity = false;
                  MITK_INFO << "Pixels/voxels of mask and image are not sufficiently aligned! (Misalignment: " << fmod(misalignment,imageSpacing[i]) << ")";
              }
            }

            // mask must be completely inside image region
            // Make sure that mask region is contained within image region
            if ( m_Mask!=nullptr &&
              !m_Image->GetLargestPossibleRegion().IsInside( m_Mask->GetLargestPossibleRegion() ) )
            {
              maskSanity = false;
              MITK_INFO << "Mask region needs to be inside of image region! (Image region: "
                << m_Image->GetLargestPossibleRegion() << "; Mask region: " << m_Mask->GetLargestPossibleRegion() << ")";
            }
            return maskSanity;
        }

    template <class TPixel, unsigned int VImageDimension>
    typename MaskUtilities<TPixel, VImageDimension >::ImageType::ConstPointer MaskUtilities<TPixel, VImageDimension>::ExtractMaskImageRegion()
    {
        if (m_Mask==nullptr || m_Image==nullptr)
        {
            MITK_ERROR << "Set an image and a mask first";
        }

        bool maskSanity = CheckMaskSanity();

        if (!maskSanity)
        {
            MITK_ERROR << "Mask and image are not compatible";
        }

        typedef itk::ExtractImageFilter< ImageType, ImageType > ExtractImageFilterType;

        typename ImageType::SizeType imageSize = m_Image->GetBufferedRegion().GetSize();
        typename ImageType::SizeType maskSize = m_Mask->GetBufferedRegion().GetSize();

        typename itk::Image<TPixel, VImageDimension>::ConstPointer resultImg;

        bool maskSmallerImage = false;
        for ( unsigned int i = 0; i < ImageType::ImageDimension; ++i )
        {
          if ( maskSize[i] < imageSize[i] )
          {
            maskSmallerImage = true;
          }
        }

        if ( maskSmallerImage )
        {
          typename ExtractImageFilterType::Pointer extractImageFilter = ExtractImageFilterType::New();
          typename MaskType::PointType maskOrigin = m_Mask->GetOrigin();
          typename ImageType::PointType imageOrigin = m_Image->GetOrigin();
          typename MaskType::SpacingType maskSpacing = m_Mask->GetSpacing();
          typename ImageType::RegionType extractionRegion;
          typename ImageType::IndexType extractionRegionIndex;


          for (unsigned int i=0; i < maskOrigin.GetPointDimension(); i++)
          {
              extractionRegionIndex[i] = (maskOrigin[i] - imageOrigin[i]) / maskSpacing[i];
          }

          extractionRegion.SetIndex(extractionRegionIndex);
          extractionRegion.SetSize(m_Mask->GetLargestPossibleRegion().GetSize());

          extractImageFilter->SetInput( m_Image );
          extractImageFilter->SetExtractionRegion( extractionRegion );
          extractImageFilter->SetCoordinateTolerance(MASK_SUITABILITY_TOLERANCE_COORDINATE);
          extractImageFilter->SetDirectionTolerance(MASK_SUITABILITY_TOLERANCE_DIRECTION);
          extractImageFilter->Update();
          auto extractedImg = extractImageFilter->GetOutput();
          extractedImg->SetOrigin(m_Mask->GetOrigin());
          extractedImg->SetLargestPossibleRegion(m_Mask->GetLargestPossibleRegion());
          extractedImg->SetBufferedRegion(m_Mask->GetBufferedRegion());
          resultImg = extractedImg;
        }
        else
        {
          resultImg = m_Image;
        }

        return resultImg;
    }

}

#endif
