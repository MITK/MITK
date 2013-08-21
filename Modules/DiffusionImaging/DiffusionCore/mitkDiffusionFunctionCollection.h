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


#include "DiffusionCoreExports.h"
#include "vnl/vnl_vector.h"
#include "vnl/vnl_vector_fixed.h"
#include "itkVectorContainer.h"

namespace mitk{

class DiffusionCore_EXPORT sh
{
public:
  static double factorial(int number);
  static void Cart2Sph(double x, double y, double z, double* cart);
  static double legendre0(int l);
  static double spherical_harmonic(int m,int l,double theta,double phi, bool complexPart);
  static double Yj(int m, int k, double theta, double phi);
};

class DiffusionCore_EXPORT gradients
{
private:
  typedef std::vector<unsigned int> IndiciesVector;
  typedef std::map<double, IndiciesVector > BValueMap;
  typedef vnl_vector_fixed< double, 3 > GradientDirectionType;
  typedef itk::VectorContainer< unsigned int, GradientDirectionType > GradientDirectionContainerType;

public:
  static std::vector<unsigned int> GetAllUniqueDirections(const std::map<double , std::vector<unsigned int> > & refBValueMap, const GradientDirectionContainerType *refGradientsContainer );
  static bool CheckForDifferingShellDirections(const std::map<double , std::vector<unsigned int> > & refBValueMap, const GradientDirectionContainerType * refGradientsContainer);
  static vnl_matrix<double> ComputeSphericalHarmonicsBasis(const vnl_matrix<double> & QBallReference, const unsigned int & LOrder);
  static vnl_matrix<double> ComputeSphericalFromCartesian(const IndiciesVector  & refShell, const GradientDirectionContainerType * refGradientsContainer);
  static mitk::gradients::GradientDirectionContainerType::Pointer CreateNormalizedUniqueGradientDirectionContainer(const BValueMap &bValueMap, const GradientDirectionContainerType * origninalGradentcontainer);


  template<typename type>
  static double dot (vnl_vector_fixed< type ,3> const& v1, vnl_vector_fixed< type ,3 > const& v2 );

};

}

#endif //__mitkDiffusionFunctionCollection_h_

