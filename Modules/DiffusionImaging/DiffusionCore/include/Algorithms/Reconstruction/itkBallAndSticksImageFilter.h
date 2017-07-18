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
#ifndef __itkBallAndSticksImageFilter_h_
#define __itkBallAndSticksImageFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include <mitkDiffusionPropertyHelper.h>
#include <vnl/algo/vnl_levenberg_marquardt.h>
#include <vnl/vnl_least_squares_function.h>
#include <vnl/algo/vnl_lbfgsb.h>
#include <mitkQBallImage.h>

namespace itk{
/** \class BallAndSticksImageFilter
 */

template< class TInPixelType, class TOutPixelType >
class BallAndSticksImageFilter :
        public ImageToImageFilter< VectorImage< TInPixelType, 3 >, Image< TOutPixelType, 3 > >
{

public:

    typedef BallAndSticksImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< VectorImage< TInPixelType, 3 >, Image< TOutPixelType, 3 > >  Superclass;
    typedef mitk::DiffusionPropertyHelper::GradientDirectionType GradientDirectionType;
    typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer GradientContainerType;
    typedef itk::Image< unsigned char, 3> ItkUcharImageType;
    typedef itk::Image< float, 4 >        PeakImageType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(BallAndSticksImageFilter, ImageToImageFilter)

    typedef typename Superclass::InputImageType InputImageType;
    typedef typename Superclass::OutputImageType OutputImageType;
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

    itkSetMacro( MaskImage, ItkUcharImageType::Pointer )
    itkSetMacro( B_value, double )
    itkSetMacro( GradientDirections, GradientContainerType )

    itkGetMacro( PeakImage, PeakImageType::Pointer )

    protected:
        BallAndSticksImageFilter();
    ~BallAndSticksImageFilter() {}
    void PrintSelf(std::ostream& os, Indent indent) const;

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType);

    double                      m_B_value;
    vnl_vector<double>          m_B_values;
    vnl_vector<double>          m_Nonzero_B_values;
    GradientContainerType       m_GradientDirections;
    ItkUcharImageType::Pointer  m_MaskImage;
    PeakImageType::Pointer      m_PeakImage;

    vnl_vector<double> FitSingleVoxel( const typename InputImageType::PixelType &input);

    /**
     * \brief The lestSquaresFunction struct for Non-Linear-Least-Squres fit of monoexponential model Si = S0*exp(-b*ADC)
     */
    struct ballStickLeastSquaresFunction: public vnl_least_squares_function
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

      void set_gradient_directions(const GradientContainerType& directions)
      {
        gradientDirections = directions;
      }

      GradientContainerType gradientDirections;
      vnl_vector<double> measurements;
      vnl_vector<double> bValues;
      double S0;
      double upper_bounds[3];
      double lower_bounds[3];

      ballStickLeastSquaresFunction(unsigned int number_of_parameters=4, unsigned int number_of_measurements=1) :
          vnl_least_squares_function(number_of_parameters, number_of_measurements, no_gradient)
      {
        lower_bounds[0] = 0;
        upper_bounds[0] = 1;

        lower_bounds[1] = 0.0001;
        upper_bounds[1] = 0.003;

        lower_bounds[2] = 0.0001;
        upper_bounds[2] = 0.003;

      }

      void Sph2Cart(vnl_vector_fixed<double,3>& dir, const double &phi, const double &theta)
      {
          dir[0] = std::sin(phi)*std::cos(theta);
          dir[1] = std::sin(phi)*std::sin(theta);
          dir[2] = std::cos(phi);
      }

      double penalty_term( vnl_vector<double> const& x)
      {
        double penalty = 0;

        for (int i=0; i<3; i++)
        {
          if (x[i]>upper_bounds[i])
          {
            penalty += 1e6*exp( (x[i]-upper_bounds[i])*10 );
          }
          else if (x[i]<lower_bounds[i])
          {
            penalty += 1e6*exp( (lower_bounds[i]-x[i])*10 );
          }
        }

        return penalty;
      }

      void f(const vnl_vector<double>& x, vnl_vector<double>& fx) override
      {
        const double & f = x[0];
        const double & dISO = x[1];
        const double & dANISO = x[2];
        int offset = 3;

        const double & phi = x[offset];
        const double & theta = x[offset+1];
        vnl_vector_fixed<double,3> dir;
        Sph2Cart(dir, phi, theta);

        for(unsigned int s=0; s<measurements.size(); s++)
        {
          double s_iso = S0 * std::exp(-bValues[s] * dISO);
          GradientDirectionType g = gradientDirections->GetElement(s);
          g.normalize();
          double dot = dot_product(g, dir);
          double s_aniso = S0 * std::exp(-bValues[s] * dANISO * dot*dot );

          double approx = f*s_iso + (1-f)*s_aniso;

          const double factor = measurements[s] - approx;

          fx[s] = factor*factor + penalty_term(x);
        }
      }

    };

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkBallAndSticksImageFilter.txx"
#endif

#endif //__itkBallAndSticksImageFilter_h_

