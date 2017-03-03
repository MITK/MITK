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

#ifndef _TrackingDataHandler
#define _TrackingDataHandler

#include <mitkBaseData.h>
#include <itkPoint.h>
#include <itkImage.h>
#include <deque>
#include <MitkFiberTrackingExports.h>

namespace mitk
{

/**
* \brief  . */

class MITKFIBERTRACKING_EXPORT TrackingDataHandler
{

public:

    TrackingDataHandler(){}
    ~TrackingDataHandler(){}

    typedef itk::Image<unsigned char, 3>  ItkUcharImgType;
    typedef itk::Image<short, 3>          ItkShortImgType;
    typedef itk::Image<float, 3>          ItkFloatImgType;
    typedef itk::Image<double, 3>         ItkDoubleImgType;

    virtual vnl_vector_fixed<double,3> ProposeDirection(itk::Point<double, 3>& pos, int& candidates, std::deque< vnl_vector_fixed<double,3> >& olddirs, double angularThreshold, double& w, itk::Index<3>& oldIndex, ItkUcharImgType::Pointer mask=nullptr) = 0;  ///< predicts next progression direction at the given position

    virtual void InitForTracking() = 0;
    virtual ItkUcharImgType::SpacingType GetSpacing() = 0;
    virtual itk::Point<float,3> GetOrigin() = 0;
    virtual ItkUcharImgType::DirectionType GetDirection() = 0;
    virtual ItkUcharImgType::RegionType GetLargestPossibleRegion() = 0;

protected:

    template< class TPixelType >
    TPixelType GetImageValue(itk::Point<float, 3> itkP, itk::Image<TPixelType, 3>* image, vnl_vector_fixed<double, 8>& interpWeights){
      // transform physical point to index coordinates
      itk::Index<3> idx;
      itk::ContinuousIndex< double, 3> cIdx;
      image->TransformPhysicalPointToIndex(itkP, idx);
      image->TransformPhysicalPointToContinuousIndex(itkP, cIdx);

      TPixelType pix = 0.0;
      if ( image->GetLargestPossibleRegion().IsInside(idx) )
      {
        pix = image->GetPixel(idx);
      }
      else
        return pix;

      double frac_x = cIdx[0] - idx[0];
      double frac_y = cIdx[1] - idx[1];
      double frac_z = cIdx[2] - idx[2];
      if (frac_x<0)
      {
        idx[0] -= 1;
        frac_x += 1;
      }
      if (frac_y<0)
      {
        idx[1] -= 1;
        frac_y += 1;
      }
      if (frac_z<0)
      {
        idx[2] -= 1;
        frac_z += 1;
      }
      frac_x = 1-frac_x;
      frac_y = 1-frac_y;
      frac_z = 1-frac_z;

      // int coordinates inside image?
      if (idx[0] >= 0 && idx[0] < image->GetLargestPossibleRegion().GetSize(0)-1 &&
          idx[1] >= 0 && idx[1] < image->GetLargestPossibleRegion().GetSize(1)-1 &&
          idx[2] >= 0 && idx[2] < image->GetLargestPossibleRegion().GetSize(2)-1)
      {
        // trilinear interpolation
        interpWeights[0] = (  frac_x)*(  frac_y)*(  frac_z);
        interpWeights[1] = (1-frac_x)*(  frac_y)*(  frac_z);
        interpWeights[2] = (  frac_x)*(1-frac_y)*(  frac_z);
        interpWeights[3] = (  frac_x)*(  frac_y)*(1-frac_z);
        interpWeights[4] = (1-frac_x)*(1-frac_y)*(  frac_z);
        interpWeights[5] = (  frac_x)*(1-frac_y)*(1-frac_z);
        interpWeights[6] = (1-frac_x)*(  frac_y)*(1-frac_z);
        interpWeights[7] = (1-frac_x)*(1-frac_y)*(1-frac_z);

        pix = image->GetPixel(idx) * interpWeights[0];

        typename itk::Image<TPixelType, 3>::IndexType tmpIdx = idx; tmpIdx[0]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[1];

        tmpIdx = idx; tmpIdx[1]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[2];

        tmpIdx = idx; tmpIdx[2]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[3];

        tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[4];

        tmpIdx = idx; tmpIdx[1]++; tmpIdx[2]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[5];

        tmpIdx = idx; tmpIdx[2]++; tmpIdx[0]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[6];

        tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++; tmpIdx[2]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[7];
      }

      if (pix!=pix)
        mitkThrow() << "nan values in image!";

      return pix;
    }

    template< class TPixelType >
    TPixelType GetImageValue(itk::Point<float, 3> itkP, itk::Image<TPixelType, 3>* image, bool interpolate){
      // transform physical point to index coordinates
      itk::Index<3> idx;
      itk::ContinuousIndex< double, 3> cIdx;
      image->TransformPhysicalPointToIndex(itkP, idx);
      image->TransformPhysicalPointToContinuousIndex(itkP, cIdx);

      TPixelType pix = 0.0;
      if ( image->GetLargestPossibleRegion().IsInside(idx) )
      {
        pix = image->GetPixel(idx);
        if (!interpolate)
          return pix;
      }
      else
        return pix;

      double frac_x = cIdx[0] - idx[0];
      double frac_y = cIdx[1] - idx[1];
      double frac_z = cIdx[2] - idx[2];
      if (frac_x<0)
      {
        idx[0] -= 1;
        frac_x += 1;
      }
      if (frac_y<0)
      {
        idx[1] -= 1;
        frac_y += 1;
      }
      if (frac_z<0)
      {
        idx[2] -= 1;
        frac_z += 1;
      }
      frac_x = 1-frac_x;
      frac_y = 1-frac_y;
      frac_z = 1-frac_z;

      // int coordinates inside image?
      if (idx[0] >= 0 && idx[0] < image->GetLargestPossibleRegion().GetSize(0)-1 &&
          idx[1] >= 0 && idx[1] < image->GetLargestPossibleRegion().GetSize(1)-1 &&
          idx[2] >= 0 && idx[2] < image->GetLargestPossibleRegion().GetSize(2)-1)
      {
        // trilinear interpolation
        vnl_vector_fixed<double, 8> interpWeights;
        interpWeights[0] = (  frac_x)*(  frac_y)*(  frac_z);
        interpWeights[1] = (1-frac_x)*(  frac_y)*(  frac_z);
        interpWeights[2] = (  frac_x)*(1-frac_y)*(  frac_z);
        interpWeights[3] = (  frac_x)*(  frac_y)*(1-frac_z);
        interpWeights[4] = (1-frac_x)*(1-frac_y)*(  frac_z);
        interpWeights[5] = (  frac_x)*(1-frac_y)*(1-frac_z);
        interpWeights[6] = (1-frac_x)*(  frac_y)*(1-frac_z);
        interpWeights[7] = (1-frac_x)*(1-frac_y)*(1-frac_z);

        pix = image->GetPixel(idx) * interpWeights[0];

        typename itk::Image<TPixelType, 3>::IndexType tmpIdx = idx; tmpIdx[0]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[1];

        tmpIdx = idx; tmpIdx[1]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[2];

        tmpIdx = idx; tmpIdx[2]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[3];

        tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[4];

        tmpIdx = idx; tmpIdx[1]++; tmpIdx[2]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[5];

        tmpIdx = idx; tmpIdx[2]++; tmpIdx[0]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[6];

        tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++; tmpIdx[2]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[7];
      }

      if (pix!=pix)
        mitkThrow() << "nan values in image!";

      return pix;
    }

};

}

#endif
