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

#include <MitkFiberTrackingExports.h>
#include <boost/random/discrete_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <deque>
#include <itkImage.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>
#include <itkPoint.h>
#include <mitkBaseData.h>
#include <mitkDiffusionFunctionCollection.h>
#include <mitkStreamlineTractographyParameters.h>

namespace mitk
{
  /**
   * \brief  Abstract class for tracking handler. A tracking handler deals with determining the next progression
   * direction of a streamline fiber. There are different handlers for tensor images, peak images, ... */

  class MITKFIBERTRACKING_EXPORT TrackingDataHandler
  {
  public:
    TrackingDataHandler();
    virtual ~TrackingDataHandler() {}

    typedef itk::Statistics::MersenneTwisterRandomVariateGenerator ItkRngType;
    typedef boost::mt19937 BoostRngType;
    typedef itk::Image<unsigned char, 3> ItkUcharImgType;
    typedef itk::Image<short, 3> ItkShortImgType;
    typedef itk::Image<float, 3> ItkFloatImgType;
    typedef itk::Image<double, 3> ItkDoubleImgType;
    typedef vnl_vector_fixed<float, 3> TrackingDirectionType;
    typedef mitk::StreamlineTractographyParameters::MODE MODE;

    virtual TrackingDirectionType ProposeDirection(
      const itk::Point<float, 3> &pos,
      std::deque<TrackingDirectionType> &olddirs,
      itk::Index<3> &oldIndex) = 0; ///< predicts next progression direction at the given position

    virtual void InitForTracking() = 0;
    virtual itk::Vector<double, 3> GetSpacing() = 0;
    virtual itk::Point<float, 3> GetOrigin() = 0;
    virtual itk::Matrix<double, 3, 3> GetDirection() = 0;
    virtual itk::ImageRegion<3> GetLargestPossibleRegion() = 0;
    virtual bool WorldToIndex(itk::Point<float, 3> &pos, itk::Index<3> &index) = 0;

    void SetParameters(std::shared_ptr<mitk::StreamlineTractographyParameters> parameters)
    {
      m_Parameters = parameters;

      if (m_Parameters->m_FixRandomSeed)
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

    double GetRandDouble(const double &a, const double &b) { return m_RngItk->GetUniformVariate(a, b); }

  protected:
    void CalculateMinVoxelSize()
    {
      itk::Vector<double, 3> imageSpacing = this->GetSpacing();
      float minVoxelSize = 0;
      if (imageSpacing[0] < imageSpacing[1] && imageSpacing[0] < imageSpacing[2])
        minVoxelSize = static_cast<float>(imageSpacing[0]);
      else if (imageSpacing[1] < imageSpacing[2])
        minVoxelSize = static_cast<float>(imageSpacing[1]);
      else
        minVoxelSize = static_cast<float>(imageSpacing[2]);

      if (m_Parameters == nullptr)
        mitkThrow() << "No tractography parameter opbect set!";

      m_Parameters->SetMinVoxelSizeMm(minVoxelSize);
    }

    BoostRngType m_Rng;
    ItkRngType::Pointer m_RngItk;
    bool m_NeedsDataInit;
    std::shared_ptr<mitk::StreamlineTractographyParameters> m_Parameters;

    void DataModified() { m_NeedsDataInit = true; }

    vnl_matrix_fixed<float, 3, 3> m_FloatImageRotation;
  };

} // namespace mitk

#endif
