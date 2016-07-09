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

#ifndef DIFFUSIONKURTOSISRECONSTRUCTIONIMAGEFILTER_H
#define DIFFUSIONKURTOSISRECONSTRUCTIONIMAGEFILTER_H

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"

#include "mitkDiffusionPropertyHelper.h"

// vnl includes
#include <vnl/algo/vnl_levenberg_marquardt.h>
#include <vnl/vnl_least_squares_function.h>

namespace itk
{

  // Fitting routines

  /** @struct Kurtosis_fit_lsq_function
      @brief A base least-squares function for the diffusion kurtosis fit (non-IVIM)

      The basic function fits the signal to S_0 = S * exp [ -b * D + -b^2 * D^2 * K^2 ]
      */
  struct kurtosis_fit_lsq_function :
      public vnl_least_squares_function
  {
  public:
    /** full lsq_function constructor */
    kurtosis_fit_lsq_function( unsigned int num_params, unsigned int num_measurements, UseGradient g=no_gradient)
      : vnl_least_squares_function( num_params, num_measurements, g),
        m_use_bounds(false)
    {}

    /** simplified constructor for the 2-parameters fit */
    kurtosis_fit_lsq_function( unsigned int number_measurements)
      : kurtosis_fit_lsq_function( 2, number_measurements, no_gradient )
    {}


    /** Initialize the function by setting measurements and the corresponding b-values */
    void initialize( vnl_vector< double > const& _meas, vnl_vector< double> const& _bvals )
    {
      meas = _meas;
      bvalues = _bvals;
    }


    /** use penalty terms on fitting to force the parameters stay within the default bounds */
    void use_bounds()
    {
      m_use_bounds = true;

      // initialize bounds
      double upper_bounds[2] = {4e-3, 4 };
      kurtosis_upper_bounds = vnl_vector<double>(2, 2, upper_bounds);
      kurtosis_lower_bounds = vnl_vector<double>(2, 0);
    }

    virtual void f(const vnl_vector<double> &x, vnl_vector<double> &fx)
    {
      for ( unsigned int s=0; s < fx.size(); s++ )
      {
        const double factor = ( meas[s] - M(x, s) );
        fx[s] = factor * factor + penalty_term(x);
      }
    }

  protected:

    /** Formula for diffusion term, use for internal computations */
    double Diff( double x1, double x2, double b)
    {
      const double quotient = -1. * b * x1 + b*b * x1 * x1 * x2 / 6;
      return exp(quotient);
    }

    /** The fitting measurement function, has to be reimplemented in the classes */
    virtual double M( vnl_vector<double> const& x, unsigned int idx )
    {
      const double bvalue = bvalues[idx];
      return meas[0] *  Diff( x[0], x[1], bvalue);
    }

    /** Penalty term on D and K during fitting, make sure the vector that is passed in contains (D, K) in this ordering */
    double penalty_term( vnl_vector<double> const& x)
    {
      double penalty = 0;

      // skip when turned off
      if( !m_use_bounds )
        return penalty;

      for( unsigned int i=0; i< x.size(); i++)
      {
        // 5% penalty boundary
        // use exponential function to scale the penalty (max when x[i] == bounds )
        double penalty_boundary = 0.05 * (kurtosis_upper_bounds[i] - kurtosis_lower_bounds[i]);

        if( x[i] < kurtosis_lower_bounds[i] + penalty_boundary )
        {
          penalty += 1e6 * exp( -1 * ( x[i] - kurtosis_lower_bounds[i]) / penalty_boundary );
        }
        else if ( x[i] > kurtosis_upper_bounds[i] - penalty_boundary )
        {
          penalty += 1e6 * exp( -1 * ( kurtosis_upper_bounds[i] - x[i]) / penalty_boundary );
        }
      }

      return penalty;
    }

    bool m_use_bounds;

    vnl_vector<double> kurtosis_upper_bounds;
    vnl_vector<double> kurtosis_lower_bounds;

    vnl_vector<double> meas;
    vnl_vector<double> bvalues;

  };

  /** @struct kurtosis_fit_omit_unweighted
      @brief A fitting function handling the unweighted signal b_0 as a fitted parameter */
  struct kurtosis_fit_omit_unweighted
      : public kurtosis_fit_lsq_function
  {
  public:
    /** simplified constructor for the 3-parameters fit */
    kurtosis_fit_omit_unweighted( unsigned int number_measurements)
      : kurtosis_fit_lsq_function( 3, number_measurements, no_gradient )
    {}

  protected:
    virtual double M(const vnl_vector<double> &x, unsigned int idx) override
    {
      const double bvalue = bvalues[idx];
      return x[2] *  Diff( x[0], x[1], bvalue);
    }

    virtual double penalty_term(const vnl_vector<double> &x)
    {
      // grab last two elements (D, K) from the 3-param fit and get penalty from superclass
      vnl_vector<double> xx(x.data_block()[0], 2) ;
      return kurtosis_fit_lsq_function::penalty_term(xx);
    }
  };


template< class TInputPixelType, class TOutputPixelType >
class DiffusionKurtosisReconstructionImageFilter :
    public ImageToImageFilter< VectorImage< TInputPixelType, 3>, Image<TOutputPixelType, 3> >
{
public:

  struct KurtosisSnapshot
  {
    // input data structures
    //vnl_vector<double> filtered_measurements;
    vnl_vector<double> bvalues;
    vnl_vector<double> measurements;
    vnl_vector<unsigned int> weighted_image_indices;

    // variables holding the fitted values
    double m_f;
    double m_D;
    double m_K;
  };

  //-- class typedefs
  typedef DiffusionKurtosisReconstructionImageFilter Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef ImageToImageFilter< VectorImage< TInputPixelType, 3>,
                              Image< TOutputPixelType,3 > >   Superclass;

  /** Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** Runtime information support. */
  itkTypeMacro(DiffusionKurtosisReconstructionImageFilter, ImageToImageFilter)

  typedef TOutputPixelType                         OutputPixelType;
  typedef TInputPixelType                          InputPixelType;

  typedef typename Superclass::InputImageType      InputImageType;
  typedef Image< OutputPixelType, 3 >              OutputImageType;

  typedef typename Superclass::OutputImageRegionType   OutputImageRegionType;


  /** Holds each magnetic field gradient used to acquire one DWImage */
  typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType
                                                   GradientDirectionContainerType;

  // vector image typedefs for regularized fit
  typedef itk::VectorImage<float,3> VectorImageType;
  typedef itk::Image<itk::Vector<double, 3>, 3> InitialFitImageType;

  //-- input (Set) methods
  /** Set the initial solution for fitting, make sure the length and the values correspond to the parameters
    *   x0 = ( S_0, ADC_0, AKC_0 ) when also the S_0 is estimated
    *   x0 = ( ADC_0, AKC_0 ) when the S_0 is used in fitting
    */
  void SetInitialSolution(const vnl_vector<double>& x0 );

  /** Set whether the S_0 value is fitted or used in fitting */
  void SetOmitUnweightedValue( bool flag)
  { this->m_OmitBZero = flag; }

  /** Set the smoothing parameter (gaussian) for estimating the maps */
  void SetSmoothingSigma( const double sigma );

  KurtosisSnapshot GetSnapshot();

  /** Set the reference bvalue of the input DW image */
  void SetReferenceBValue( double bvalue )
  { this->m_ReferenceBValue = bvalue; }

  /** Set the gradient directions */
  void SetGradientDirections( GradientDirectionContainerType::Pointer gradients );

protected:
  DiffusionKurtosisReconstructionImageFilter();
  virtual ~DiffusionKurtosisReconstructionImageFilter() {}

  void BeforeThreadedGenerateData() override;

  void ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId) override;

  void AfterThreadedGenerateData() {}

  double m_ReferenceBValue;

  vnl_vector<double> m_BValues;

  vnl_vector<double> m_InitialPosition;

  bool m_OmitBZero;

private:


};

} //end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionKurtosisReconstructionImageFilter.cxx"
#endif


#endif // DIFFUSIONKURTOSISRECONSTRUCTIONIMAGEFILTER_H
