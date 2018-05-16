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

// Namespace ::SH
#include <boost/math/special_functions/legendre.hpp>
#include <boost/math/special_functions/spherical_harmonic.hpp>
#include <boost/version.hpp>
#include <itkPointShell.h>

// Namespace ::Gradients
#include "itkVectorContainer.h"
#include "vnl/vnl_vector.h"

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

double mitk::sh::Yj(int m, int l, float theta, float phi, bool mrtrix)
{
  if (!mrtrix)
  {
    if (m<0)
      return sqrt(2.0)*::boost::math::spherical_harmonic_r(l, -m, theta, phi);
    else if (m==0)
      return ::boost::math::spherical_harmonic_r(l, m, theta, phi);
    else
      return pow(-1.0,m)*sqrt(2.0)*::boost::math::spherical_harmonic_i(l, m, theta, phi);
  }
  else
  {
    double plm = ::boost::math::legendre_p<float>(l,abs(m),-cos(theta));
    double mag = sqrt((double)(2*l+1)/(4.0*itk::Math::pi)*::boost::math::factorial<float>(l-abs(m))/::boost::math::factorial<float>(l+abs(m)))*plm;
    if (m>0)
      return mag*cos(m*phi);
    else if (m==0)
      return mag;
    else
      return mag*sin(-m*phi);
  }

  return 0;
}

vnl_matrix<float> mitk::sh::CalcShBasisForDirections(int sh_order, vnl_matrix<double> U, bool mrtrix)
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
    for(int k=0; k<=sh_order; k+=2)
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

//------------------------- gradients-function ------------------------------------

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
  for(int i=0; i< (int)SHBasisOutput.rows(); i++)
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
