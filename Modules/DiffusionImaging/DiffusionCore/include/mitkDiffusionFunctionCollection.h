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

#ifndef __mitkDiffusionFunctionCollection_h_
#define __mitkDiffusionFunctionCollection_h_


#include <MitkDiffusionCoreExports.h>
#include <vnl/vnl_vector.h>
#include <vnl/vnl_vector_fixed.h>
#include <itkVectorContainer.h>
#include <itkImage.h>
#include <itkLinearInterpolateImageFunction.h>
#include <mitkImage.h>
#include <mitkDiffusionPropertyHelper.h>

namespace mitk{

class MITKDIFFUSIONCORE_EXPORT imv
{
public:

  static std::vector< std::pair< itk::Index<3>, double > > IntersectImage(const itk::Vector<double,3>& spacing, itk::Index<3>& si, itk::Index<3>& ei, itk::ContinuousIndex<float, 3>& sf, itk::ContinuousIndex<float, 3>& ef);

  static itk::Point<float, 3> GetItkPoint(double point[3])
  {
    itk::Point<float, 3> itkPoint;
    itkPoint[0] = point[0];
    itkPoint[1] = point[1];
    itkPoint[2] = point[2];
    return itkPoint;
  }

  template< class TPixelType, class TOutPixelType=TPixelType >
  static TOutPixelType GetImageValue(const itk::Point<float, 3>& itkP, bool interpolate, typename itk::LinearInterpolateImageFunction< itk::Image< TPixelType, 3 >, float >::Pointer interpolator)
  {
    if (interpolator==nullptr)
      return 0.0;

    itk::ContinuousIndex< float, 3> cIdx;
    interpolator->ConvertPointToContinuousIndex(itkP, cIdx);

    if (interpolator->IsInsideBuffer(cIdx))
    {
      if (interpolate)
        return interpolator->EvaluateAtContinuousIndex(cIdx);
      else
      {
        itk::Index<3> idx;
        interpolator->ConvertContinuousIndexToNearestIndex(cIdx, idx);
        return interpolator->EvaluateAtIndex(idx);
      }
    }
    else
      return 0.0;
  }

  template< class TPixelType=unsigned char >
  static bool IsInsideMask(const itk::Point<float, 3>& itkP, bool interpolate, typename itk::LinearInterpolateImageFunction< itk::Image< TPixelType, 3 >, float >::Pointer interpolator, float threshold=0.5)
  {
    if (interpolator==nullptr)
      return false;

    itk::ContinuousIndex< float, 3> cIdx;
    interpolator->ConvertPointToContinuousIndex(itkP, cIdx);

    if (interpolator->IsInsideBuffer(cIdx))
    {
      double value = 0.0;
      if (interpolate)
        value = interpolator->EvaluateAtContinuousIndex(cIdx);
      else
      {
        itk::Index<3> idx;
        interpolator->ConvertContinuousIndexToNearestIndex(cIdx, idx);
        value = interpolator->EvaluateAtIndex(idx);
      }

      if (value>=threshold)
        return true;
    }
    return false;
  }

};

class MITKDIFFUSIONCORE_EXPORT sh
{
public:
  static double factorial(int number);
  static void Cart2Sph(double x, double y, double z, double* spherical);
  static vnl_vector_fixed<double, 3> Sph2Cart(const double& theta, const double& phi, const double& rad);
  static double legendre0(int l);
  static double spherical_harmonic(int m,int l,double theta,double phi, bool complexPart);
  static double Yj(int m, int k, float theta, float phi, bool mrtrix=true);
  static vnl_matrix<float> CalcShBasisForDirections(int sh_order, vnl_matrix<double> U, bool mrtrix=true);
  static float GetValue(const vnl_vector<float>& coefficients, const int& sh_order, const vnl_vector_fixed<double, 3>& dir, const bool mrtrix);
  static float GetValue(const vnl_vector<float> &coefficients, const int &sh_order, const double theta, const double phi, const bool mrtrix);
};

class MITKDIFFUSIONCORE_EXPORT gradients
{
private:
  typedef std::vector<unsigned int> IndiciesVector;
  typedef mitk::BValueMapProperty::BValueMap  BValueMap;

  typedef DiffusionPropertyHelper::GradientDirectionsContainerType GradientDirectionContainerType;
  typedef DiffusionPropertyHelper::GradientDirectionType GradientDirectionType;

public:

  static GradientDirectionContainerType::Pointer ReadBvalsBvecs(std::string bvals_file, std::string bvecs_file, double& reference_bval);
  static void WriteBvalsBvecs(std::string bvals_file, std::string bvecs_file, GradientDirectionContainerType::Pointer gradients, double reference_bval);
  static std::vector<unsigned int> GetAllUniqueDirections(const BValueMap &bValueMap, GradientDirectionContainerType *refGradientsContainer );

  static bool CheckForDifferingShellDirections(const BValueMap &bValueMap, GradientDirectionContainerType::ConstPointer refGradientsContainer);
  static vnl_matrix<double> ComputeSphericalHarmonicsBasis(const vnl_matrix<double> & QBallReference, const unsigned int & LOrder);
  static vnl_matrix<double> ComputeSphericalFromCartesian(const IndiciesVector  & refShell, const GradientDirectionContainerType * refGradientsContainer);
  static GradientDirectionContainerType::Pointer CreateNormalizedUniqueGradientDirectionContainer(const BValueMap &bValueMap, const GradientDirectionContainerType * origninalGradentcontainer);
};

}

#endif //__mitkDiffusionFunctionCollection_h_

