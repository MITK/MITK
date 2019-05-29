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
#include <mitkShImage.h>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkOdfImage.h>
#include <mitkTensorImage.h>
#include <mitkPeakImage.h>
#include <itkOrientationDistributionFunction.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>

namespace mitk{

class MITKDIFFUSIONCORE_EXPORT imv
{
public:

  static std::vector< std::pair< itk::Index<3>, double > > IntersectImage(const itk::Vector<double,3>& spacing, itk::Index<3>& si, itk::Index<3>& ei, itk::ContinuousIndex<float, 3>& sf, itk::ContinuousIndex<float, 3>& ef);

  static itk::Point<float, 3> GetItkPoint(double point[3])
  {
    itk::Point<float, 3> itkPoint;
    itkPoint[0] = static_cast<float>(point[0]);
    itkPoint[1] = static_cast<float>(point[1]);
    itkPoint[2] = static_cast<float>(point[2]);
    return itkPoint;
  }

  template< class TType >
  static itk::Point<float, 3> GetItkPoint(double point[3])
  {
    itk::Point<TType, 3> itkPoint;
    itkPoint[0] = static_cast<TType>(point[0]);
    itkPoint[1] = static_cast<TType>(point[1]);
    itkPoint[2] = static_cast<TType>(point[2]);
    return itkPoint;
  }

  template< class TType >
  static itk::Vector<float, 3> GetItkVector(double point[3])
  {
    itk::Vector<TType, 3> itkVector;
    itkVector[0] = static_cast<TType>(point[0]);
    itkVector[1] = static_cast<TType>(point[1]);
    itkVector[2] = static_cast<TType>(point[2]);
    return itkVector;
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
      float value = 0.0;
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

  template< class TType=float >
  static vnl_matrix_fixed< TType, 3, 3 > GetRotationMatrixVnl(TType rx, TType ry, TType rz)
  {
    rx = rx*itk::Math::pi/180;
    ry = ry*itk::Math::pi/180;
    rz = rz*itk::Math::pi/180;

    vnl_matrix_fixed< TType, 3, 3 > rotX; rotX.set_identity();
    rotX[1][1] = cos(rx);
    rotX[2][2] = rotX[1][1];
    rotX[1][2] = -sin(rx);
    rotX[2][1] = -rotX[1][2];

    vnl_matrix_fixed< TType, 3, 3 > rotY; rotY.set_identity();
    rotY[0][0] = cos(ry);
    rotY[2][2] = rotY[0][0];
    rotY[0][2] = sin(ry);
    rotY[2][0] = -rotY[0][2];

    vnl_matrix_fixed< TType, 3, 3 > rotZ; rotZ.set_identity();
    rotZ[0][0] = cos(rz);
    rotZ[1][1] = rotZ[0][0];
    rotZ[0][1] = -sin(rz);
    rotZ[1][0] = -rotZ[0][1];

    vnl_matrix_fixed< TType, 3, 3 > rot = rotZ*rotY*rotX;
    return rot;
  }

  template< class TType=float >
  static itk::Matrix< TType, 3, 3 > GetRotationMatrixItk(TType rx, TType ry, TType rz)
  {
    rx = rx*itk::Math::pi/180;
    ry = ry*itk::Math::pi/180;
    rz = rz*itk::Math::pi/180;

    itk::Matrix< TType, 3, 3 > rotX; rotX.SetIdentity();
    rotX[1][1] = cos(rx);
    rotX[2][2] = rotX[1][1];
    rotX[1][2] = -sin(rx);
    rotX[2][1] = -rotX[1][2];

    itk::Matrix< TType, 3, 3 > rotY; rotY.SetIdentity();
    rotY[0][0] = cos(ry);
    rotY[2][2] = rotY[0][0];
    rotY[0][2] = sin(ry);
    rotY[2][0] = -rotY[0][2];

    itk::Matrix< TType, 3, 3 > rotZ; rotZ.SetIdentity();
    rotZ[0][0] = cos(rz);
    rotZ[1][1] = rotZ[0][0];
    rotZ[0][1] = -sin(rz);
    rotZ[1][0] = -rotZ[0][1];

    itk::Matrix< TType, 3, 3 > rot = rotZ*rotY*rotX;
    return rot;
  }
};

class MITKDIFFUSIONCORE_EXPORT convert
{
public:
  static mitk::OdfImage::ItkOdfImageType::Pointer GetItkOdfFromTensorImage(mitk::Image::Pointer mitkImage);
  static mitk::OdfImage::Pointer GetOdfFromTensorImage(mitk::Image::Pointer mitkImage);
  static mitk::TensorImage::ItkTensorImageType::Pointer GetItkTensorFromTensorImage(mitk::Image::Pointer mitkImage);
  static mitk::PeakImage::ItkPeakImageType::Pointer GetItkPeakFromPeakImage(mitk::Image::Pointer mitkImage);

  template<unsigned int NUM_COEFFS>
  static typename itk::Image< itk::Vector<float, NUM_COEFFS>, 3>::Pointer GetItkShFromShImage(mitk::Image::Pointer mitkImage)
  {
    mitk::ShImage::Pointer mitkShImage = dynamic_cast<mitk::ShImage*>(mitkImage.GetPointer());
    if (mitkShImage.IsNull())
      mitkThrow() << "Input image is not a SH image!";

    typename itk::Image< itk::Vector<float, NUM_COEFFS>, 3>::Pointer itkvol;
    mitk::CastToItkImage(mitkImage, itkvol);

    return itkvol;
  }

  static mitk::OdfImage::ItkOdfImageType::Pointer GetItkOdfFromShImage(mitk::Image::Pointer mitkImage);
  static mitk::OdfImage::Pointer GetOdfFromShImage(mitk::Image::Pointer mitkImage);
  static mitk::OdfImage::ItkOdfImageType::Pointer GetItkOdfFromOdfImage(mitk::Image::Pointer mitkImage);
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
  static vnl_matrix<float> CalcShBasisForDirections(unsigned int sh_order, vnl_matrix<double> U, bool mrtrix=true);
  static float GetValue(const vnl_vector<float>& coefficients, const int& sh_order, const vnl_vector_fixed<double, 3>& dir, const bool mrtrix);
  static float GetValue(const vnl_vector<float> &coefficients, const int &sh_order, const double theta, const double phi, const bool mrtrix);
  static unsigned int ShOrder(int num_coeffs);

  template <unsigned int N, typename TComponent=float>
  static void SampleOdf(const vnl_vector<float>& coefficients, itk::OrientationDistributionFunction<TComponent, N>& odf)
  {
    auto dirs = odf.GetDirections();
    auto basis = CalcShBasisForDirections(ShOrder(coefficients.size()), *dirs);
    auto odf_vals = basis * coefficients;
    for (unsigned int i=0; i<odf_vals.size(); ++i)
      odf[i] = odf_vals[i];
  }
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

