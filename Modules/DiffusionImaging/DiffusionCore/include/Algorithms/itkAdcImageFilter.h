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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef __itkAdcImageFilter_h_
#define __itkAdcImageFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include <mitkDiffusionPropertyHelper.h>
#include <vnl/algo/vnl_levenberg_marquardt.h>
#include <vnl/vnl_least_squares_function.h>

namespace itk{
/** \class AdcImageFilter
 */

template< class TInPixelType, class TOutPixelType >
class AdcImageFilter :
    public ImageToImageFilter< VectorImage< TInPixelType, 3 >, Image< TOutPixelType, 3 > >
{

public:

  typedef AdcImageFilter Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef ImageToImageFilter< VectorImage< TInPixelType, 3 >, Image< TOutPixelType, 3 > >  Superclass;
  typedef mitk::DiffusionPropertyHelper::GradientDirectionType GradientDirectionType;
  typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer GradientContainerType;
  typedef itk::Image< unsigned char, 3> ItkUcharImageType;

  /** Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** Runtime information support. */
  itkTypeMacro(AdcImageFilter, ImageToImageFilter)

  typedef typename Superclass::InputImageType InputImageType;
  typedef typename Superclass::OutputImageType OutputImageType;
  typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

  itkSetMacro( MaskImage, ItkUcharImageType::Pointer )
  itkSetMacro( FitSignal, bool )
  itkSetMacro( B_value, double )
  itkSetMacro( GradientDirections, GradientContainerType )

  /**
   * \brief The lestSquaresFunction struct for Non-Linear-Least-Squres fit of monoexponential model Si = S0*exp(-b*ADC)
   */
  struct adcLeastSquaresFunction: public vnl_least_squares_function
  {

    void set_S0(double val)
    {
      S0 = val;
    }

    void set_measurements(const vnl_vector<double>& m)
    {
      measurements.set_size(m.size());
      measurements.copy_in(m.data_block());
    }

    void set_bvalues(const vnl_vector<double>& x)
    {
      bValues.set_size(x.size());
      bValues.copy_in(x.data_block());
    }

    vnl_vector<double> measurements;
    vnl_vector<double> bValues;
    double S0;

    adcLeastSquaresFunction(unsigned int number_of_measurements=1) :
        vnl_least_squares_function(1, number_of_measurements, no_gradient)
    {
    }

    void f(const vnl_vector<double>& x, vnl_vector<double>& fx) override {

      const double & ADC = x[0];

      for(unsigned int s=0; s<measurements.size(); s++)
      {
        double approx = S0 * std::exp(-bValues[s] * ADC);
        fx[s] = vnl_math_abs( measurements[s] - approx );
      }
    }
  };

  protected:
    AdcImageFilter();
  ~AdcImageFilter() override {}
  void PrintSelf(std::ostream& os, Indent indent) const override;

  void BeforeThreadedGenerateData() override;
  void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType) override;

  bool      m_FitSignal;
  double    m_B_value;
  vnl_vector<double> m_B_values;
  vnl_vector<double> m_Nonzero_B_values;
  GradientContainerType m_GradientDirections;
  ItkUcharImageType::Pointer m_MaskImage;

  double FitSingleVoxel( const typename InputImageType::PixelType &input);

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAdcImageFilter.txx"
#endif

#endif //__itkAdcImageFilter_h_

