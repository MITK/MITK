#ifndef __itkFitFibersToImageFilter_h__
#define __itkFitFibersToImageFilter_h__

// MITK
#include <mitkFiberBundle.h>
#include <itkImageSource.h>
#include <mitkPeakImage.h>
#include <vnl/algo/vnl_lbfgsb.h>
#include <vnl/vnl_sparse_matrix.h>
#include <vnl/vnl_sparse_matrix_linear_system.h>
#include <itkImageDuplicator.h>
#include <itkTimeProbe.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

namespace itk{

/**
* \brief Fits the tractogram to the input peak image by assigning a weight to each fiber (similar to https://doi.org/10.1016/j.neuroimage.2015.06.092).  */

class FitFibersToImageFilter : public ImageSource< mitk::PeakImage::ItkPeakImageType >
{

public:

  typedef FitFibersToImageFilter Self;
  typedef ProcessObject Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  typedef itk::Point<float, 4> PointType4;
  typedef mitk::PeakImage::ItkPeakImageType       PeakImgType;
  typedef itk::Image<unsigned char, 3>            UcharImgType;

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  itkTypeMacro( FitFibersToImageFilter, ImageSource )

  itkSetMacro( PeakImage, PeakImgType::Pointer)
  itkGetMacro( PeakImage, PeakImgType::Pointer)
  itkSetMacro( MaskImage, UcharImgType::Pointer)
  itkGetMacro( MaskImage, UcharImgType::Pointer)
  itkSetMacro( FitIndividualFibers, bool)
  itkGetMacro( FitIndividualFibers, bool)
  itkSetMacro( GradientTolerance, double)
  itkGetMacro( GradientTolerance, double)
  itkSetMacro( Lambda, double)
  itkGetMacro( Lambda, double)
  itkSetMacro( MaxIterations, int)
  itkGetMacro( MaxIterations, int)
  itkSetMacro( FiberSampling, float)
  itkGetMacro( FiberSampling, float)
  itkSetMacro( FilterOutliers, bool)
  itkGetMacro( FilterOutliers, bool)
  itkSetMacro( Verbose, bool)
  itkGetMacro( Verbose, bool)
  itkSetMacro( DeepCopy, bool)
  itkGetMacro( DeepCopy, bool)
  itkSetMacro( ResampleFibers, bool)
  itkGetMacro( ResampleFibers, bool)

  itkGetMacro( Weights, vnl_vector<double>)
  itkGetMacro( RmsDiffPerBundle, vnl_vector<double>)
  itkGetMacro( RmsDiffPerFiber, vnl_vector<double>)
  itkGetMacro( FittedImage, PeakImgType::Pointer)
  itkGetMacro( ResidualImage, PeakImgType::Pointer)
  itkGetMacro( OverexplainedImage, PeakImgType::Pointer)
  itkGetMacro( UnderexplainedImage, PeakImgType::Pointer)
  itkGetMacro( Coverage, double)
  itkGetMacro( Overshoot, double)
  itkGetMacro( RMSE, double)
  itkGetMacro( MeanWeight, double)
  itkGetMacro( MedianWeight, double)
  itkGetMacro( MinWeight, double)
  itkGetMacro( MaxWeight, double)
  itkGetMacro( NumUnknowns, unsigned int)
  itkGetMacro( NumResiduals, unsigned int)
  itkGetMacro( NumCoveredDirections, unsigned int)

  void SetTractograms(const std::vector<mitk::FiberBundle::Pointer> &tractograms);

  void GenerateData() override;

  std::vector<mitk::FiberBundle::Pointer> GetTractograms() const;

protected:

  FitFibersToImageFilter();
  virtual ~FitFibersToImageFilter();

  vnl_vector_fixed<float,3> GetClosestPeak(itk::Index<4> idx, PeakImgType::Pointer m_PeakImage , vnl_vector_fixed<float,3> fiber_dir, int& id, double& w );

  std::vector< mitk::FiberBundle::Pointer >   m_Tractograms;
  PeakImgType::Pointer                        m_PeakImage;
  UcharImgType::Pointer                       m_MaskImage;
  bool                                        m_FitIndividualFibers;
  double                                      m_GradientTolerance;
  double                                      m_Lambda;
  int                                         m_MaxIterations;
  float                                       m_FiberSampling;
  double                                      m_Coverage;
  double                                      m_Overshoot;
  double                                      m_RMSE;
  bool                                        m_FilterOutliers;
  double                                      m_MeanWeight;
  double                                      m_MedianWeight;
  double                                      m_MinWeight;
  double                                      m_MaxWeight;
  bool                                        m_Verbose;
  bool                                        m_DeepCopy;
  bool                                        m_ResampleFibers;
  unsigned int                                m_NumUnknowns;
  unsigned int                                m_NumResiduals;
  unsigned int                                m_NumCoveredDirections;

  // output
  vnl_vector<double>                          m_RmsDiffPerBundle;
  vnl_vector<double>                          m_Weights;
  vnl_vector<double>                          m_RmsDiffPerFiber;
  PeakImgType::Pointer                        m_UnderexplainedImage;
  PeakImgType::Pointer                        m_OverexplainedImage;
  PeakImgType::Pointer                        m_ResidualImage;
  PeakImgType::Pointer                        m_FittedImage;
};

}


class VnlCostFunction : public vnl_cost_function
{
public:

  vnl_sparse_matrix_linear_system< double >* S;
  vnl_sparse_matrix< double > m_A;
  vnl_sparse_matrix< double > m_A_Ones; // matrix indicating active weights with 1
  vnl_vector< double > m_b;
  double m_Lambda;  // regularization factor

  vnl_vector<double> row_sums;  // number of active weights per row
  vnl_vector<double> local_weight_means;  // mean weight of each row

  void SetProblem(vnl_sparse_matrix< double >& A, vnl_vector<double>& b, double lambda)
  {
    S = new vnl_sparse_matrix_linear_system<double>(A, b);
    m_A = A;
    m_b = b;
    m_Lambda = lambda;

    m_A_Ones.set_size(m_A.rows(), m_A.cols());
    m_A.reset();
    while (m_A.next())
      m_A_Ones.put(m_A.getrow(), m_A.getcolumn(), 1);

    unsigned int N = m_b.size();
    vnl_vector<double> ones; ones.set_size(dim); ones.fill(1.0);
    row_sums.set_size(N);
    m_A_Ones.mult(ones, row_sums);
    local_weight_means.set_size(N);
  }

  VnlCostFunction(const int NumVars) : vnl_cost_function(NumVars)
  {
  }

  void regu_MSE(vnl_vector<double> const &x, double& cost)
  {
    double mean = x.mean();
    vnl_vector<double> tx = x-mean;
    cost += m_Lambda*1e8*tx.squared_magnitude()/x.size();
  }

  void regu_MSM(vnl_vector<double> const &x, double& cost)
  {
    cost += m_Lambda*1e8*x.squared_magnitude()/x.size();
  }

  void regu_localMSE(vnl_vector<double> const &x, double& cost)
  {
    m_A_Ones.mult(x, local_weight_means);
    local_weight_means = element_quotient(local_weight_means, row_sums);

    m_A_Ones.reset();
    double regu = 0;
    while (m_A_Ones.next())
    {
      double d = 0;
      if (x[m_A_Ones.getcolumn()]>local_weight_means[m_A_Ones.getrow()])
        d = std::exp(x[m_A_Ones.getcolumn()]) - std::exp(local_weight_means[m_A_Ones.getrow()]);
      else
        d = x[m_A_Ones.getcolumn()] - local_weight_means[m_A_Ones.getrow()];
      regu += d*d;
    }
    cost += m_Lambda*regu/dim;
  }

  void grad_regu_MSE(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    double mean = x.mean();
    vnl_vector<double> tx = x-mean;

    vnl_vector<double> tx2(dim, 0.0);
    vnl_vector<double> h(dim, 1.0);
    for (int c=0; c<dim; c++)
    {
      h[c] = dim-1;
      tx2[c] += dot_product(h,tx);
      h[c] = 1;
    }
    dx += tx2*m_Lambda*1e8*2.0/(dim*dim);

  }

  void grad_regu_MSM(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    dx += m_Lambda*1e8*2.0*x/dim;
  }

  void grad_regu_localMSE(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    m_A_Ones.mult(x, local_weight_means);
    local_weight_means = element_quotient(local_weight_means, row_sums);

    vnl_vector<double> exp_x = x.apply(std::exp);
    vnl_vector<double> exp_means = local_weight_means.apply(std::exp);

    vnl_vector<double> tdx(dim, 0);
    m_A_Ones.reset();
    while (m_A_Ones.next())
    {
      int c = m_A_Ones.getcolumn();
      int r = m_A_Ones.getrow();

      if (x[c]>local_weight_means[r])
        tdx[c] += exp_x[c] * ( exp_x[c] - exp_means[r] );
      else
        tdx[c] += x[c] - local_weight_means[r];
    }
    dx += tdx*2.0*m_Lambda/dim;
  }


  double f(vnl_vector<double> const &x)
  {
    double cost = S->get_rms_error(x);
    cost *= cost;

    regu_localMSE(x, cost);

    return cost;
  }

  void gradf(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    dx.fill(0.0);
    unsigned int N = m_b.size();

    vnl_vector<double> d; d.set_size(N);
    S->multiply(x,d);
    d -= m_b;

    S->transpose_multiply(d, dx);
    dx *= 2.0/N;

    grad_regu_localMSE(x,dx);
  }
};

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFitFibersToImageFilter.cpp"
#endif

#endif // __itkFitFibersToImageFilter_h__
