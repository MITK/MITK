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
#include <boost/random/discrete_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/mersenne_twister.hpp>

namespace mitk
{

/**
* \brief  Abstract class for tracking handler. A tracking handler deals with determining the next progression direction of a streamline fiber. There are different handlers for tensor images, peak images, ... */

class MITKFIBERTRACKING_EXPORT TrackingDataHandler
{

public:

    enum MODE {
        DETERMINISTIC,
        PROBABILISTIC
    };

    TrackingDataHandler();
    virtual ~TrackingDataHandler(){}

    typedef itk::Statistics::MersenneTwisterRandomVariateGenerator ItkRngType;
    typedef boost::mt19937                BoostRngType;
    typedef itk::Image<unsigned char, 3>  ItkUcharImgType;
    typedef itk::Image<short, 3>          ItkShortImgType;
    typedef itk::Image<float, 3>          ItkFloatImgType;
    typedef itk::Image<double, 3>         ItkDoubleImgType;
    typedef vnl_vector_fixed< float, 3 >  TrackingDirectionType;

    virtual TrackingDirectionType ProposeDirection(const itk::Point<float, 3>& pos, std::deque< TrackingDirectionType >& olddirs, itk::Index<3>& oldIndex) = 0;  ///< predicts next progression direction at the given position

    virtual void InitForTracking() = 0;
    virtual itk::Vector<double, 3> GetSpacing() = 0;
    virtual itk::Point<float,3> GetOrigin() = 0;
    virtual itk::Matrix<double, 3, 3> GetDirection() = 0;
    virtual itk::ImageRegion<3> GetLargestPossibleRegion() = 0;
    virtual bool WorldToIndex(itk::Point<float, 3>& pos, itk::Index<3>& index) = 0;
    virtual void SetMode(MODE m) = 0;
    MODE GetMode(){ return m_Mode; }

    void SetInterpolate( bool interpolate ){ m_Interpolate = interpolate; }
    bool GetInterpolate() const { return m_Interpolate; }

    void SetAngularThreshold( float a ){ m_AngularThreshold = a; }
    void SetFlipX( bool f ){ m_FlipX = f; }
    void SetFlipY( bool f ){ m_FlipY = f; }
    void SetFlipZ( bool f ){ m_FlipZ = f; }
    void SetRandom( bool random )
    {
      m_Random = random;
      if (!random)
      {
        m_Rng.seed(0);
        std::srand(0);
        m_RngItk->SetSeed(0);
      }
      else
      {
        m_Rng.seed();
        m_RngItk->SetSeed();
        std::srand(std::time(0));
      }
    }

    double GetRandDouble(const double & a, const double & b)
    {
      return m_RngItk->GetUniformVariate(a, b);
    }

    static bool IsInsideMask(const itk::Point<float, 3> &pos, ItkUcharImgType::Pointer mask, bool interpolate)
    {
      if (mask.IsNull())
        return true;

      if (interpolate)
      {
        if ( mitk::TrackingDataHandler::GetImageValue<unsigned char, double>(pos, mask, true)<0.5)
          return false;
      }
      else
      {
        if ( mitk::TrackingDataHandler::GetImageValue<unsigned char>(pos, mask, false)==0)
          return false;
      }

      return true;
    }

    template< class TPixelType >
    static TPixelType GetImageValue(const itk::Point<float, 3>& itkP, itk::Image<TPixelType, 3>* image, vnl_vector_fixed<float, 8>& interpWeights){
      // transform physical point to index coordinates
      itk::Index<3> idx;
      itk::ContinuousIndex< float, 3> cIdx;
      image->TransformPhysicalPointToIndex(itkP, idx);
      image->TransformPhysicalPointToContinuousIndex(itkP, cIdx);

      TPixelType pix = 0.0;
      if ( image->GetLargestPossibleRegion().IsInside(idx) )
      {
        pix = image->GetPixel(idx);
      }
      else
        return pix;

      float frac_x = cIdx[0] - idx[0];
      float frac_y = cIdx[1] - idx[1];
      float frac_z = cIdx[2] - idx[2];
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
      {
        pix = 0;
        MITK_WARN << "nan values in image!";
      }

      return pix;
    }

    template< class TPixelType, class TOutPixelType=TPixelType >
    static TPixelType GetImageValue(const itk::Point<float, 3>& itkP, itk::Image<TPixelType, 3>* image, bool interpolate){
      // transform physical point to index coordinates
      itk::Index<3> idx;
      itk::ContinuousIndex< float, 3> cIdx;
      image->TransformPhysicalPointToIndex(itkP, idx);
      image->TransformPhysicalPointToContinuousIndex(itkP, cIdx);

      TOutPixelType pix = 0.0;
      if ( image->GetLargestPossibleRegion().IsInside(idx) )
      {
        pix = (TOutPixelType)image->GetPixel(idx);
        if (!interpolate)
          return pix;
      }
      else
        return pix;

      float frac_x = cIdx[0] - idx[0];
      float frac_y = cIdx[1] - idx[1];
      float frac_z = cIdx[2] - idx[2];
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
      if (idx[0] >= 0 && idx[0] < static_cast<itk::IndexValueType>(image->GetLargestPossibleRegion().GetSize(0) - 1) &&
          idx[1] >= 0 && idx[1] < static_cast<itk::IndexValueType>(image->GetLargestPossibleRegion().GetSize(1) - 1) &&
          idx[2] >= 0 && idx[2] < static_cast<itk::IndexValueType>(image->GetLargestPossibleRegion().GetSize(2) - 1))
      {
        // trilinear interpolation
        vnl_vector_fixed<float, 8> interpWeights;
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
        pix +=  (TOutPixelType)image->GetPixel(tmpIdx) * interpWeights[1];

        tmpIdx = idx; tmpIdx[1]++;
        pix +=  (TOutPixelType)image->GetPixel(tmpIdx) * interpWeights[2];

        tmpIdx = idx; tmpIdx[2]++;
        pix +=  (TOutPixelType)image->GetPixel(tmpIdx) * interpWeights[3];

        tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++;
        pix +=  (TOutPixelType)image->GetPixel(tmpIdx) * interpWeights[4];

        tmpIdx = idx; tmpIdx[1]++; tmpIdx[2]++;
        pix +=  (TOutPixelType)image->GetPixel(tmpIdx) * interpWeights[5];

        tmpIdx = idx; tmpIdx[2]++; tmpIdx[0]++;
        pix +=  (TOutPixelType)image->GetPixel(tmpIdx) * interpWeights[6];

        tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++; tmpIdx[2]++;
        pix +=  (TOutPixelType)image->GetPixel(tmpIdx) * interpWeights[7];
      }

      if (pix!=pix)
      {
        pix = 0;
        MITK_WARN << "nan values in image!";
      }

      return pix;
    }

    template< class TPixelType, int components >
    static itk::Vector< TPixelType, components > GetImageValue(const itk::Point<float, 3>& itkP, itk::Image<itk::Vector< TPixelType, components >, 3>* image, bool interpolate){
      // transform physical point to index coordinates
      itk::Index<3> idx;
      itk::ContinuousIndex< float, 3> cIdx;
      image->TransformPhysicalPointToIndex(itkP, idx);
      image->TransformPhysicalPointToContinuousIndex(itkP, cIdx);

      itk::Vector< TPixelType, components > pix; pix.Fill(0.0);
      if ( image->GetLargestPossibleRegion().IsInside(idx) )
      {
        pix = image->GetPixel(idx);
        if (!interpolate)
          return pix;
      }
      else
        return pix;

      float frac_x = cIdx[0] - idx[0];
      float frac_y = cIdx[1] - idx[1];
      float frac_z = cIdx[2] - idx[2];
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
      if (idx[0] >= 0 && idx[0] < static_cast<itk::IndexValueType>(image->GetLargestPossibleRegion().GetSize(0) - 1) &&
          idx[1] >= 0 && idx[1] < static_cast<itk::IndexValueType>(image->GetLargestPossibleRegion().GetSize(1) - 1) &&
          idx[2] >= 0 && idx[2] < static_cast<itk::IndexValueType>(image->GetLargestPossibleRegion().GetSize(2) - 1))
      {
        // trilinear interpolation
        vnl_vector_fixed<float, 8> interpWeights;
        interpWeights[0] = (  frac_x)*(  frac_y)*(  frac_z);
        interpWeights[1] = (1-frac_x)*(  frac_y)*(  frac_z);
        interpWeights[2] = (  frac_x)*(1-frac_y)*(  frac_z);
        interpWeights[3] = (  frac_x)*(  frac_y)*(1-frac_z);
        interpWeights[4] = (1-frac_x)*(1-frac_y)*(  frac_z);
        interpWeights[5] = (  frac_x)*(1-frac_y)*(1-frac_z);
        interpWeights[6] = (1-frac_x)*(  frac_y)*(1-frac_z);
        interpWeights[7] = (1-frac_x)*(1-frac_y)*(1-frac_z);

        pix = image->GetPixel(idx) * interpWeights[0];

        typename itk::Image<itk::Vector< TPixelType, components >, 3>::IndexType tmpIdx = idx; tmpIdx[0]++;
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
      {
        pix.Fill(0.0);
        mitkThrow() << "nan values in image!";
      }

      return pix;
    }

protected:

    float               m_AngularThreshold;
    bool                m_Interpolate;
    bool                m_FlipX;
    bool                m_FlipY;
    bool                m_FlipZ;
    MODE                m_Mode;
    BoostRngType        m_Rng;
    ItkRngType::Pointer m_RngItk;
    bool                m_NeedsDataInit;
    bool                m_Random;

    void DataModified()
    {
      m_NeedsDataInit = true;
    }

};

}

#endif
