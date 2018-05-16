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
        m_use_bounds(false),
        m_use_logscale(false),
        m_skip_fit(false)
    {}

    /** simplified constructor for the 2-parameters fit */
    kurtosis_fit_lsq_function( unsigned int number_measurements)
      : kurtosis_fit_lsq_function( 2, number_measurements, no_gradient )
    {}


    /** Initialize the function by setting measurements and the corresponding b-values */
    void initialize( vnl_vector< double > const& _meas, vnl_vector< double> const& _bvals )
    {
      meas = _meas;
      if( m_use_logscale )
      {
        for( unsigned int i=0; i< meas.size(); ++i)
        {
          // would produce NaN values, skip the fit
          // using the virtual function from the superclass (sets a boolean flag)
          if( meas[i] < vnl_math::eps )
          {
            m_skip_fit = true;
            throw_failure();

            continue;
          }

          meas[i] = log( meas[i] );

        }
      }

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

    void set_fit_logscale( bool flag )
    {
      this->m_use_logscale = flag;
    }

    void set_K_bounds( const vnl_vector_fixed<double, 2> k_bounds )
    {
      // init
      this->use_bounds();

      // override K bounds
      kurtosis_lower_bounds[1] = k_bounds[0];
      kurtosis_upper_bounds[1] = k_bounds[1];
    }

    virtual void f(const vnl_vector<double> &x, vnl_vector<double> &fx)
    {
      for ( unsigned int s=0; s < fx.size(); s++ )
      {
        const double factor = ( meas[s] - M(x, s) );
        fx[s] = factor * factor + penalty_term(x);
      }

      MITK_DEBUG("Fit.x_and_f") << x << " | " << fx;
    }

  protected:

    /** Formula for diffusion term, use for internal computations */
    double Diff( double x1, double x2, double b)
    {
      const double quotient = -1. * b * x1 + b*b * x1 * x1 * x2 / 6;

      if( m_use_logscale )
        return quotient;
      else
        return exp(quotient);
    }

    /** The fitting measurement function, has to be reimplemented in the classes */
    virtual double M( vnl_vector<double> const& x, unsigned int idx )
    {
      const double bvalue = bvalues[idx];

      double result = Diff( x[0], x[1], bvalue);

      if( m_use_logscale )
        return meas[0] + result;
      else
        return meas[0] * result ;
    }

    /** Penalty term on D and K during fitting, make sure the vector that is passed in contains (D, K) in this ordering */
    virtual double penalty_term( vnl_vector<double> const& x)
    {
      double penalty = 0;

      // skip when turned off
      if( !m_use_bounds )
        return penalty;

      // we have bounds for D and K only (the first two params )
      for( unsigned int i=0; i< 2; i++)
      {
        // 5% penalty boundary
        // use exponential function to scale the penalty (max when x[i] == bounds )
        double penalty_boundary = 0.02 * (kurtosis_upper_bounds[i] - kurtosis_lower_bounds[i]);

        if( x[i] < kurtosis_lower_bounds[i] + penalty_boundary )
        {
          penalty += 1e6 * exp( -1 * ( x[i] - kurtosis_lower_bounds[i]) / penalty_boundary );
        }
        else if ( x[i] > kurtosis_upper_bounds[i] - penalty_boundary )
        {
          penalty += 1e6 * exp( -1 * ( kurtosis_upper_bounds[i] - x[i]) / penalty_boundary );
        }
      }

      MITK_DEBUG("Fit.Orig.Penalty") << x << " || penalty: " << penalty;

      return penalty;
    }

    bool m_use_bounds;

    bool m_use_logscale;

    bool m_skip_fit;

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

      double result = Diff( x[0], x[1], bvalue);

      if( m_use_logscale )
        return log( x[2] ) + result;
      else
        return x[2] * result ;
    }
  };

  enum FitScale
  {
    STRAIGHT = 0,
    LOGARITHMIC
  };

  struct KurtosisFitConfiguration
  {
    KurtosisFitConfiguration()
      : omit_bzero(false)
      , use_K_limits(false)
      , exclude_high_b(false)
      , b_upper_threshold(10e9)
    {}

    bool omit_bzero;
    FitScale fit_scale;

    bool use_K_limits;
    vnl_vector_fixed<double, 2> K_limits;

    bool exclude_high_b;
    double b_upper_threshold;
  };

/**
  @class DiffusionKurtosisReconstructionImageFilter
  @brief This filter provides the fit of the kurtosis (non-IVIM) signal to the data

  It has two main modes of operation, either as an image filter to compute the D and K maps, i.e. fitting the values to each voxel or a computation on a single voxel
  or a voxel group (with mask) can be triggered by @sa GetSnapshot, GetCurrentSnapshot methods.
  */
template< class TInputPixelType, class TOutputPixelType >
class DiffusionKurtosisReconstructionImageFilter :
    public ImageToImageFilter< VectorImage< TInputPixelType, 3>, Image<TOutputPixelType, 3> >
{
public:

  /**
    @struct KurtosisSnapshot

    @brief Struct describing a result (and the data) of a Kurtosis model fit
    */
  struct KurtosisSnapshot
  {
    KurtosisSnapshot()
      : m_f(1), m_BzeroFit(1), m_D(0.001), m_K(0) {}

    // input data structures
    //vnl_vector<double> filtered_measurements;
    vnl_vector<double> bvalues;
    vnl_vector<double> measurements;

    vnl_vector<double> fit_bvalues;
    vnl_vector<double> fit_measurements;
    vnl_vector<unsigned int> weighted_image_indices;

    bool m_fittedBZero;

    // variables holding the fitted values
    double m_f;
    double m_BzeroFit;
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

  typedef itk::Image< short , 3>            MaskImageType;

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

  /**
    Trigger a single computation of the Kurtosis values from the given input vector and the bvalues and returns the result as a KurtosisSnapshot object */
  KurtosisSnapshot GetSnapshot( const itk::VariableLengthVector< TInputPixelType > &input, vnl_vector<double> bvalues, KurtosisFitConfiguration kf_conf);

  /**
    Trigger a single computation of the kurtosis values, first the bvalues vector is computed internally but then also stored into the returend snapshot */
  KurtosisSnapshot GetSnapshot( const itk::VariableLengthVector< TInputPixelType > &input, GradientDirectionContainerType::Pointer, float bvalue, KurtosisFitConfiguration kf_conf);

  /**
    * Returns the value of the current data presented to the filter.

      If a mask is set, the voxels are first averaged before passed to the fitting procedure
    */
  KurtosisSnapshot GetCurrentSnapshot(bool omit_bzero);

  /** Set the reference bvalue of the input DW image */
  void SetReferenceBValue( double bvalue )
  { this->m_ReferenceBValue = bvalue; }

  /** Set the gradient directions */
  void SetGradientDirections( GradientDirectionContainerType::Pointer gradients );

  /** Restrict map generation to an image region */
  void SetMapOutputRegion( OutputImageRegionType region )
  {
    m_MapOutputRegion = region;
    this->m_ApplyPriorSmoothing = true;
  }

  void SetImageMask( MaskImageType::Pointer mask );

  /** Set smoothing sigma (default = 1.5 ), automatically enables smoothing prior to fitting */
  void SetSmoothingSigma( double sigma )
  {
    this->m_SmoothingSigma = sigma;
    this->m_ApplyPriorSmoothing = true;
  }

  /** Activate/Deactivate the gaussian smoothing applied to the input prior to fitting ( default = off ) */
  void SetUseSmoothingPriorToFitting( bool flag)
  {
    this->m_ApplyPriorSmoothing = flag;
  }

  /** Set boundaries enforced by penalty terms in the fitting procedure */
  void SetBoundariesForKurtosis( double lower, double upper )
  {
    m_UseKBounds = true;

    m_KurtosisBounds[0] = lower; m_KurtosisBounds[1] = upper;
  }

  /** Exclude measurements associated with b-values higher than max_bvalue from fitting */
  void SetMaximalBValueUsedForFitting( double max_bvalue )
  {
    m_MaxFitBValue = max_bvalue;
  }

  /** Select the method used in fitting of the data

    STRAIHT - fit the exponential signal equation S / S_0 = exp [ ... ]
    LOGARITHMIC - fit the logarithmic signal equation ln( S / S_0 ) = []
    */
  void SetFittingScale( FitScale scale )
  {
    m_ScaleForFitting = scale;
  }

protected:
  DiffusionKurtosisReconstructionImageFilter();
  virtual ~DiffusionKurtosisReconstructionImageFilter() {}

  void GenerateOutputInformation() override;

  void AfterThreadedGenerateData() override;

  void BeforeThreadedGenerateData() override;

  void ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId) override;

  double m_ReferenceBValue;

  vnl_vector<double> m_BValues;

  vnl_vector<double> m_InitialPosition;

  bool m_OmitBZero;

  OutputImageRegionType m_MapOutputRegion;

  MaskImageType::Pointer m_MaskImage;

  typename InputImageType::Pointer m_ProcessedInputImage;

  bool m_ApplyPriorSmoothing;
  double m_SmoothingSigma;

  bool m_UseKBounds;
  vnl_vector_fixed<double, 2> m_KurtosisBounds;
  double m_MaxFitBValue;

  FitScale m_ScaleForFitting;

private:


};

} //end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionKurtosisReconstructionImageFilter.cxx"
#endif


#endif // DIFFUSIONKURTOSISRECONSTRUCTIONIMAGEFILTER_H
