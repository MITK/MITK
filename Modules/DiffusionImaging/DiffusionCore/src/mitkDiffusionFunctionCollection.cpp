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

#include "mitkDiffusionFunctionCollection.h"
#include "mitkNumericTypes.h"

#include <boost/math/special_functions/legendre.hpp>
#include <boost/math/special_functions/spherical_harmonic.hpp>
#include <boost/version.hpp>
#include <itkPointShell.h>
#include "itkVectorContainer.h"
#include "vnl/vnl_vector.h"
#include <vtkBox.h>
#include <vtkMath.h>
#include <itksys/SystemTools.hxx>
#include <boost/algorithm/string.hpp>
#include <itkShToOdfImageFilter.h>
#include <itkTensorImageToOdfImageFilter.h>

// Intersect a finite line (with end points p0 and p1) with all of the
// cells of a vtkImageData
std::vector< std::pair< itk::Index<3>, double > > mitk::imv::IntersectImage(const itk::Vector<double,3>& spacing, itk::Index<3>& si, itk::Index<3>& ei, itk::ContinuousIndex<float, 3>& sf, itk::ContinuousIndex<float, 3>& ef)
{
  std::vector< std::pair< itk::Index<3>, double > > out;
  if (si == ei)
  {
    double d[3];
    for (int i=0; i<3; ++i)
      d[i] = static_cast<double>(sf[i]-ef[i])*spacing[i];
    double len = std::sqrt( d[0]*d[0] + d[1]*d[1] + d[2]*d[2] );
    out.push_back(  std::pair< itk::Index<3>, double >(si, len) );
    return out;
  }

  double bounds[6];

  double entrancePoint[3];
  double exitPoint[3];

  double startPoint[3];
  double endPoint[3];

  double t0, t1;
  for (unsigned int i=0; i<3; ++i)
  {
    startPoint[i] = static_cast<double>(sf[i]);
    endPoint[i] = static_cast<double>(ef[i]);

    if (si[i]>ei[i])
    {
      auto t = si[i];
      si[i] = ei[i];
      ei[i] = t;
    }
  }

  for (auto x = si[0]; x<=ei[0]; ++x)
    for (auto y = si[1]; y<=ei[1]; ++y)
      for (auto z = si[2]; z<=ei[2]; ++z)
      {
        bounds[0] = static_cast<double>(x) - 0.5;
        bounds[1] = static_cast<double>(x) + 0.5;
        bounds[2] = static_cast<double>(y) - 0.5;
        bounds[3] = static_cast<double>(y) + 0.5;
        bounds[4] = static_cast<double>(z) - 0.5;
        bounds[5] = static_cast<double>(z) + 0.5;

        int entryPlane;
        int exitPlane;
        int hit = vtkBox::IntersectWithLine(bounds,
                                            startPoint,
                                            endPoint,
                                            t0,
                                            t1,
                                            entrancePoint,
                                            exitPoint,
                                            entryPlane,
                                            exitPlane);
        if (hit)
        {
          if (entryPlane>=0 && exitPlane>=0)
          {
            double d[3];
            for (int i=0; i<3; ++i)
              d[i] = (exitPoint[i] - entrancePoint[i])*spacing[i];
            double len = std::sqrt( d[0]*d[0] + d[1]*d[1] + d[2]*d[2] );

            itk::Index<3> idx; idx[0] = x; idx[1] = y; idx[2] = z;
            out.push_back(  std::pair< itk::Index<3>, double >(idx, len) );
          }
          else if (entryPlane>=0)
          {
            double d[3];
            for (int i=0; i<3; ++i)
              d[i] = (static_cast<double>(ef[i]) - entrancePoint[i])*spacing[i];
            double len = std::sqrt( d[0]*d[0] + d[1]*d[1] + d[2]*d[2] );

            itk::Index<3> idx; idx[0] = x; idx[1] = y; idx[2] = z;
            out.push_back(  std::pair< itk::Index<3>, double >(idx, len) );
          }
          else if (exitPlane>=0)
          {
            double d[3];
            for (int i=0; i<3; ++i)
              d[i] = (exitPoint[i]-static_cast<double>(sf[i]))*spacing[i];
            double len = std::sqrt( d[0]*d[0] + d[1]*d[1] + d[2]*d[2] );

            itk::Index<3> idx; idx[0] = x; idx[1] = y; idx[2] = z;
            out.push_back(  std::pair< itk::Index<3>, double >(idx, len) );
          }
        }
      }
  return out;
}

//------------------------- SH-function ------------------------------------

double mitk::sh::factorial(int number) {
  if(number <= 1) return 1;
  double result = 1.0;
  for(int i=1; i<=number; i++)
    result *= i;
  return result;
}

void mitk::sh::Cart2Sph(double x, double y, double z, double *spherical)
{
  double phi, th, rad;
  rad = sqrt(x*x+y*y+z*z);
  if( rad < mitk::eps )
  {
    th = itk::Math::pi/2;
    phi = itk::Math::pi/2;
  }
  else
  {
    th = acos(z/rad);
    phi = atan2(y, x);
  }
  spherical[0] = phi;
  spherical[1] = th;
  spherical[2] = rad;
}

vnl_vector_fixed<double, 3> mitk::sh::Sph2Cart(const double& theta, const double& phi, const double& rad)
{
  vnl_vector_fixed<double, 3> dir;
  dir[0] = rad * sin(theta) * cos(phi);
  dir[1] = rad * sin(theta) * sin(phi);
  dir[2] = rad * cos(theta);
  return dir;
}

double mitk::sh::legendre0(int l)
{
  if( l%2 != 0 )
  {
    return 0;
  }
  else
  {
    double prod1 = 1.0;
    for(int i=1;i<l;i+=2) prod1 *= i;
    double prod2 = 1.0;
    for(int i=2;i<=l;i+=2) prod2 *= i;
    return pow(-1.0,l/2.0)*(prod1/prod2);
  }
}

double mitk::sh::Yj(int m, int k, float theta, float phi, bool mrtrix)
{
  if (!mrtrix)
  {
    if (m<0)
      return sqrt(2.0)*static_cast<double>(::boost::math::spherical_harmonic_r(static_cast<unsigned int>(k), -m, theta, phi));
    else if (m==0)
      return static_cast<double>(::boost::math::spherical_harmonic_r(static_cast<unsigned int>(k), m, theta, phi));
    else
      return pow(-1.0,m)*sqrt(2.0)*static_cast<double>(::boost::math::spherical_harmonic_i(static_cast<unsigned int>(k), m, theta, phi));
  }
  else
  {
    double plm = static_cast<double>(::boost::math::legendre_p<float>(k,abs(m),-cos(theta)));
    double mag = sqrt((2.0*k+1.0)/(4.0*itk::Math::pi)*::boost::math::factorial<double>(k-abs(m))/::boost::math::factorial<double>(k+abs(m)))*plm;
    if (m>0)
      return mag*static_cast<double>(cos(m*phi));
    else if (m==0)
      return mag;
    else
      return mag*static_cast<double>(sin(-m*phi));
  }

  return 0;
}

mitk::OdfImage::ItkOdfImageType::Pointer mitk::convert::GetItkOdfFromShImage(mitk::Image::Pointer mitkImage)
{
  mitk::ShImage::Pointer mitkShImage = dynamic_cast<mitk::ShImage*>(mitkImage.GetPointer());
  if (mitkShImage.IsNull())
    mitkThrow() << "Input image is not a SH image!";
  mitk::OdfImage::ItkOdfImageType::Pointer output;
  switch (mitkShImage->ShOrder())
  {
  case 2:
  {
    typedef itk::ShToOdfImageFilter< float, 2 > ShConverterType;
    typename ShConverterType::InputImageType::Pointer itkvol = ShConverterType::InputImageType::New();
    mitk::CastToItkImage(mitkImage, itkvol);
    typename ShConverterType::Pointer converter = ShConverterType::New();
    converter->SetInput(itkvol);
    converter->Update();
    output = converter->GetOutput();
    break;
  }
  case 4:
  {
    typedef itk::ShToOdfImageFilter< float, 4 > ShConverterType;
    typename ShConverterType::InputImageType::Pointer itkvol = ShConverterType::InputImageType::New();
    mitk::CastToItkImage(mitkImage, itkvol);
    typename ShConverterType::Pointer converter = ShConverterType::New();
    converter->SetInput(itkvol);
    converter->Update();
    output = converter->GetOutput();
    break;
  }
  case 6:
  {
    typedef itk::ShToOdfImageFilter< float, 6 > ShConverterType;
    typename ShConverterType::InputImageType::Pointer itkvol = ShConverterType::InputImageType::New();
    mitk::CastToItkImage(mitkImage, itkvol);
    typename ShConverterType::Pointer converter = ShConverterType::New();
    converter->SetInput(itkvol);
    converter->Update();
    output = converter->GetOutput();
    break;
  }
  case 8:
  {
    typedef itk::ShToOdfImageFilter< float, 8 > ShConverterType;
    typename ShConverterType::InputImageType::Pointer itkvol = ShConverterType::InputImageType::New();
    mitk::CastToItkImage(mitkImage, itkvol);
    typename ShConverterType::Pointer converter = ShConverterType::New();
    converter->SetInput(itkvol);
    converter->Update();
    output = converter->GetOutput();
    break;
  }
  case 10:
  {
    typedef itk::ShToOdfImageFilter< float, 10 > ShConverterType;
    typename ShConverterType::InputImageType::Pointer itkvol = ShConverterType::InputImageType::New();
    mitk::CastToItkImage(mitkImage, itkvol);
    typename ShConverterType::Pointer converter = ShConverterType::New();
    converter->SetInput(itkvol);
    converter->Update();
    output = converter->GetOutput();
    break;
  }
  case 12:
  {
    typedef itk::ShToOdfImageFilter< float, 12 > ShConverterType;
    typename ShConverterType::InputImageType::Pointer itkvol = ShConverterType::InputImageType::New();
    mitk::CastToItkImage(mitkImage, itkvol);
    typename ShConverterType::Pointer converter = ShConverterType::New();
    converter->SetInput(itkvol);
    converter->Update();
    output = converter->GetOutput();
    break;
  }
  default:
    mitkThrow() << "SH orders higher than 12 are not supported!";
  }

  return output;
}

mitk::OdfImage::Pointer mitk::convert::GetOdfFromShImage(mitk::Image::Pointer mitkImage)
{
  mitk::OdfImage::Pointer image = mitk::OdfImage::New();
  auto img = GetItkOdfFromShImage(mitkImage);
  image->InitializeByItk( img.GetPointer() );
  image->SetVolume( img->GetBufferPointer() );
  return image;
}

mitk::OdfImage::ItkOdfImageType::Pointer mitk::convert::GetItkOdfFromTensorImage(mitk::Image::Pointer mitkImage)
{
  typedef itk::TensorImageToOdfImageFilter< float, float > FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( GetItkTensorFromTensorImage(mitkImage) );
  filter->Update();
  return filter->GetOutput();
}

mitk::TensorImage::ItkTensorImageType::Pointer mitk::convert::GetItkTensorFromTensorImage(mitk::Image::Pointer mitkImage)
{
  typedef mitk::ImageToItk< mitk::TensorImage::ItkTensorImageType > CasterType;
  CasterType::Pointer caster = CasterType::New();
  caster->SetInput(mitkImage);
  caster->Update();
  return caster->GetOutput();
}

mitk::PeakImage::ItkPeakImageType::Pointer mitk::convert::GetItkPeakFromPeakImage(mitk::Image::Pointer mitkImage)
{
  typedef mitk::ImageToItk< mitk::PeakImage::ItkPeakImageType > CasterType;
  CasterType::Pointer caster = CasterType::New();
  caster->SetInput(mitkImage);
  caster->SetCopyMemFlag(true);
  caster->Update();
  return caster->GetOutput();
}

mitk::OdfImage::Pointer mitk::convert::GetOdfFromTensorImage(mitk::Image::Pointer mitkImage)
{
  mitk::OdfImage::Pointer image = mitk::OdfImage::New();
  auto img = GetItkOdfFromTensorImage(mitkImage);
  image->InitializeByItk( img.GetPointer() );
  image->SetVolume( img->GetBufferPointer() );
  return image;
}

mitk::OdfImage::ItkOdfImageType::Pointer mitk::convert::GetItkOdfFromOdfImage(mitk::Image::Pointer mitkImage)
{
  typedef mitk::ImageToItk< mitk::OdfImage::ItkOdfImageType > CasterType;
  CasterType::Pointer caster = CasterType::New();
  caster->SetInput(mitkImage);
  caster->Update();
  return caster->GetOutput();
}

vnl_matrix<float> mitk::sh::CalcShBasisForDirections(unsigned int sh_order, vnl_matrix<double> U, bool mrtrix)
{
  vnl_matrix<float> sh_basis  = vnl_matrix<float>(U.cols(), (sh_order*sh_order + sh_order + 2)/2 + sh_order );
  for(unsigned int i=0; i<U.cols(); i++)
  {
    double x = U(0,i);
    double y = U(1,i);
    double z = U(2,i);
    double spherical[3];
    mitk::sh::Cart2Sph(x,y,z,spherical);
    U(0,i) = spherical[0];
    U(1,i) = spherical[1];
    U(2,i) = spherical[2];
  }

  for(unsigned int i=0; i<U.cols(); i++)
  {
    for(int k=0; k<=static_cast<int>(sh_order); k+=2)
    {
      for(int m=-k; m<=k; m++)
      {
        int j = (k*k + k + 2)/2 + m - 1;
        double phi = U(0,i);
        double th = U(1,i);
        sh_basis(i,j) = mitk::sh::Yj(m,k,th,phi, mrtrix);
      }
    }
  }

  return sh_basis;
}

unsigned int mitk::sh::ShOrder(int num_coeffs)
{
  int c=3, d=2-2*num_coeffs;
  int D = c*c-4*d;
  if (D>0)
  {
    int s = (-c+static_cast<int>(sqrt(D)))/2;
    if (s<0)
      s = (-c-static_cast<int>(sqrt(D)))/2;
    return static_cast<unsigned int>(s);
  }
  else if (D==0)
    return static_cast<unsigned int>(-c/2);
  return 0;
}

float mitk::sh::GetValue(const vnl_vector<float> &coefficients, const int &sh_order, const double theta, const double phi, const bool mrtrix)
{
  float val = 0;
  for(int k=0; k<=sh_order; k+=2)
  {
    for(int m=-k; m<=k; m++)
    {
      unsigned int j = static_cast<unsigned int>((k*k + k + 2)/2 + m - 1);
      val += coefficients[j] * mitk::sh::Yj(m, k, theta, phi, mrtrix);
    }
  }

  return val;
}

float mitk::sh::GetValue(const vnl_vector<float> &coefficients, const int &sh_order, const vnl_vector_fixed<double, 3> &dir, const bool mrtrix)
{
  double spherical[3];
  mitk::sh::Cart2Sph(dir[0], dir[1], dir[2], spherical);

  float val = 0;
  for(int k=0; k<=sh_order; k+=2)
  {
    for(int m=-k; m<=k; m++)
    {
      int j = (k*k + k + 2)/2 + m - 1;
      val += coefficients[j] * mitk::sh::Yj(m, k, spherical[1], spherical[0], mrtrix);
    }
  }

  return val;
}

//------------------------- gradients-function ------------------------------------


mitk::gradients::GradientDirectionContainerType::Pointer mitk::gradients::ReadBvalsBvecs(std::string bvals_file, std::string bvecs_file, double& reference_bval)
{
  mitk::gradients::GradientDirectionContainerType::Pointer directioncontainer = mitk::gradients::GradientDirectionContainerType::New();

  std::vector<float> bvec_entries;
  if (!itksys::SystemTools::FileExists(bvecs_file))
    mitkThrow() << "bvecs file not existing: " << bvecs_file;
  else
  {
    std::string line;
    std::ifstream myfile (bvecs_file.c_str());
    if (myfile.is_open())
    {
      while (std::getline(myfile, line))
      {
        std::vector<std::string> strs;
        boost::split(strs,line,boost::is_any_of("\t \n"));
        for (auto token : strs)
        {
          if (!token.empty())
          {
            try
            {
              bvec_entries.push_back(boost::lexical_cast<float>(token));
            }
            catch(...)
            {
              mitkThrow() << "Encountered invalid bvecs file entry >" << token << "<";
            }
          }
        }
      }
      myfile.close();
    }
    else
    {
      mitkThrow() << "bvecs file could not be opened: " << bvals_file;
    }
  }

  reference_bval = -1;
  std::vector<float> bval_entries;
  if (!itksys::SystemTools::FileExists(bvals_file))
    mitkThrow() << "bvals file not existing: " << bvals_file;
  else
  {
    std::string line;
    std::ifstream myfile (bvals_file.c_str());
    if (myfile.is_open())
    {
      while (std::getline(myfile, line))
      {
        std::vector<std::string> strs;
        boost::split(strs,line,boost::is_any_of("\t \n"));
        for (auto token : strs)
        {
          if (!token.empty())
          {
            try {
              bval_entries.push_back(boost::lexical_cast<float>(token));
              if (bval_entries.back()>reference_bval)
                reference_bval = bval_entries.back();
            }
            catch(...)
            {
              mitkThrow() << "Encountered invalid bvals file entry >" << token << "<";
            }
          }
        }
      }
      myfile.close();
    }
    else
    {
      mitkThrow() << "bvals file could not be opened: " << bvals_file;
    }
  }

  for(unsigned int i=0; i<bval_entries.size(); i++)
  {
    double b_val = bval_entries.at(i);

    mitk::gradients::GradientDirectionType vec;
    vec[0] = bvec_entries.at(i);
    vec[1] = bvec_entries.at(i+bval_entries.size());
    vec[2] = bvec_entries.at(i+2*bval_entries.size());

    // Adjust the vector length to encode gradient strength
    if (reference_bval>0)
    {
      double factor = b_val/reference_bval;
      if(vec.magnitude() > 0)
      {
        vec.normalize();
        vec[0] = sqrt(factor)*vec[0];
        vec[1] = sqrt(factor)*vec[1];
        vec[2] = sqrt(factor)*vec[2];
      }
    }

    directioncontainer->InsertElement(i,vec);
  }

  return directioncontainer;
}

void mitk::gradients::WriteBvalsBvecs(std::string bvals_file, std::string bvecs_file, GradientDirectionContainerType::Pointer gradients, double reference_bval)
{
  std::ofstream myfile;
  myfile.open (bvals_file.c_str());
  for(unsigned int i=0; i<gradients->Size(); i++)
  {
    double twonorm = gradients->ElementAt(i).two_norm();
    myfile << std::round(reference_bval*twonorm*twonorm) << " ";
  }
  myfile.close();

  std::ofstream myfile2;
  myfile2.open (bvecs_file.c_str());
  for(int j=0; j<3; j++)
  {
    for(unsigned int i=0; i<gradients->Size(); i++)
    {
      GradientDirectionType direction = gradients->ElementAt(i);
      direction.normalize();
      myfile2 << direction.get(j) << " ";
    }
    myfile2 << std::endl;
  }
}

std::vector<unsigned int> mitk::gradients::GetAllUniqueDirections(const BValueMap & refBValueMap, GradientDirectionContainerType *refGradientsContainer )
{

  IndiciesVector directioncontainer;
  auto mapIterator = refBValueMap.begin();

  if(refBValueMap.find(0) != refBValueMap.end() && refBValueMap.size() > 1)
    mapIterator++; //skip bzero Values

  for( ; mapIterator != refBValueMap.end(); mapIterator++){

    IndiciesVector currentShell = mapIterator->second;

    while(currentShell.size()>0)
    {
      unsigned int wntIndex = currentShell.back();
      currentShell.pop_back();

      auto containerIt = directioncontainer.begin();
      bool directionExist = false;
      while(containerIt != directioncontainer.end())
      {
        if (fabs(dot_product(refGradientsContainer->ElementAt(*containerIt), refGradientsContainer->ElementAt(wntIndex)))  > 0.9998)
        {
          directionExist = true;
          break;
        }
        containerIt++;
      }
      if(!directionExist)
      {
        directioncontainer.push_back(wntIndex);
      }
    }
  }

  return directioncontainer;
}


bool mitk::gradients::CheckForDifferingShellDirections(const BValueMap & refBValueMap, GradientDirectionContainerType::ConstPointer refGradientsContainer)
{
  auto mapIterator = refBValueMap.begin();

  if(refBValueMap.find(0) != refBValueMap.end() && refBValueMap.size() > 1)
    mapIterator++; //skip bzero Values

  for( ; mapIterator != refBValueMap.end(); mapIterator++){

    auto mapIterator_2 = refBValueMap.begin();
    if(refBValueMap.find(0) != refBValueMap.end() && refBValueMap.size() > 1)
      mapIterator_2++; //skip bzero Values

    for( ; mapIterator_2 != refBValueMap.end(); mapIterator_2++){

      if(mapIterator_2 == mapIterator) continue;

      IndiciesVector currentShell = mapIterator->second;
      IndiciesVector testShell = mapIterator_2->second;
      for (unsigned int i = 0; i< currentShell.size(); i++)
        if (fabs(dot_product(refGradientsContainer->ElementAt(currentShell[i]), refGradientsContainer->ElementAt(testShell[i])))  <= 0.9998) { return true; }

    }
  }
  return false;
}

vnl_matrix<double> mitk::gradients::ComputeSphericalFromCartesian(const IndiciesVector & refShell, const GradientDirectionContainerType * refGradientsContainer)
{

  vnl_matrix<double> Q(3, refShell.size());
  Q.fill(0.0);

  for(unsigned int i = 0; i < refShell.size(); i++)
  {
    GradientDirectionType dir = refGradientsContainer->ElementAt(refShell[i]);
    double x = dir.normalize().get(0);
    double y = dir.normalize().get(1);
    double z = dir.normalize().get(2);
    double cart[3];
    mitk::sh::Cart2Sph(x,y,z,cart);
    Q(0,i) = cart[0];
    Q(1,i) = cart[1];
    Q(2,i) = cart[2];
  }
  return Q;
}

vnl_matrix<double> mitk::gradients::ComputeSphericalHarmonicsBasis(const vnl_matrix<double> & QBallReference, const unsigned int & LOrder)
{
  vnl_matrix<double> SHBasisOutput(QBallReference.cols(), (LOrder+1)*(LOrder+2)*0.5);
  SHBasisOutput.fill(0.0);
  for(unsigned int i=0; i<SHBasisOutput.rows(); i++)
    for(int k = 0; k <= (int)LOrder; k += 2)
      for(int m =- k; m <= k; m++)
      {
        int j = ( k * k + k + 2 ) / 2.0 + m - 1;
        double phi = QBallReference(0,i);
        double th = QBallReference(1,i);
        double val = mitk::sh::Yj(m,k,th,phi);
        SHBasisOutput(i,j) = val;
      }
  return SHBasisOutput;
}

mitk::gradients::GradientDirectionContainerType::Pointer mitk::gradients::CreateNormalizedUniqueGradientDirectionContainer(const mitk::gradients::BValueMap & bValueMap,
                                                                                                                           const GradientDirectionContainerType *origninalGradentcontainer)
{
  mitk::gradients::GradientDirectionContainerType::Pointer directioncontainer = mitk::gradients::GradientDirectionContainerType::New();
  auto mapIterator = bValueMap.begin();

  if(bValueMap.find(0) != bValueMap.end() && bValueMap.size() > 1){
    mapIterator++; //skip bzero Values
    vnl_vector_fixed<double, 3> vec;
    vec.fill(0.0);
    directioncontainer->push_back(vec);
  }

  for( ; mapIterator != bValueMap.end(); mapIterator++){

    IndiciesVector currentShell = mapIterator->second;

    while(currentShell.size()>0)
    {
      unsigned int wntIndex = currentShell.back();
      currentShell.pop_back();

      mitk::gradients::GradientDirectionContainerType::Iterator containerIt = directioncontainer->Begin();
      bool directionExist = false;
      while(containerIt != directioncontainer->End())
      {
        if (fabs(dot_product(containerIt.Value(), origninalGradentcontainer->ElementAt(wntIndex)))  > 0.9998)
        {
          directionExist = true;
          break;
        }
        containerIt++;
      }
      if(!directionExist)
      {
        GradientDirectionType dir(origninalGradentcontainer->ElementAt(wntIndex));
        directioncontainer->push_back(dir.normalize());
      }
    }
  }

  return directioncontainer;
}
