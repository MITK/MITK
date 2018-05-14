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
#include <mitkDiffusionFunctionCollection.h>
#include <itkLinearInterpolateImageFunction.h>

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
  MODE GetMode() const { return m_Mode; }

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
      std::srand(std::time(nullptr));
    }
  }

  double GetRandDouble(const double & a, const double & b)
  {
    return m_RngItk->GetUniformVariate(a, b);
  }
  bool GetFlipX() const { return m_FlipX; }
  bool GetFlipY() const { return m_FlipY; }
  bool GetFlipZ() const { return m_FlipZ; }

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
