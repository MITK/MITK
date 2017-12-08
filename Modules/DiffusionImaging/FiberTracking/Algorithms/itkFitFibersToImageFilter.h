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
#include <mitkDiffusionPropertyHelper.h>
#include <mitkDiffusionSignalModel.h>

class VnlCostFunction : public vnl_cost_function
{
public:

  enum REGU
  {
    MSM,
    MSE,
    Local_MSE,
    NONE
  };

  vnl_sparse_matrix_linear_system< double >* S;
  vnl_sparse_matrix< double > m_A;
  vnl_sparse_matrix< double > m_A_Ones; // matrix indicating active weights with 1
  vnl_vector< double > m_b;
  double m_Lambda;  // regularization factor

  vnl_vector<double> row_sums;  // number of active weights per row
  vnl_vector<double> local_weight_means;  // mean weight of each row
  REGU regularization;

  void SetProblem(vnl_sparse_matrix< double >& A, vnl_vector<double>& b, double lambda, REGU regu)
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
    regularization = regu;
  }

  VnlCostFunction(const int NumVars=0) : vnl_cost_function(NumVars)
  {
  }

  // Regularization: mean squared deaviation of weights from mean weight (enforce uniform weights)
  void regu_MSE(vnl_vector<double> const &x, double& cost)
  {
    double mean = x.mean();
    vnl_vector<double> tx = x-mean;
    cost += 10000.0*m_Lambda*tx.squared_magnitude()/dim;
  }

  // Regularization: mean squared magnitude of weight vectors (small weights) L2
  void regu_MSM(vnl_vector<double> const &x, double& cost)
  {
    cost += 10000.0*m_Lambda*x.squared_magnitude()/dim;
  }

  // Regularization: voxel-weise mean squared deaviation of weights from voxel-wise mean weight (enforce locally uniform weights)
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
    cost += 10.0*m_Lambda*regu/dim;
  }

  // gradients of regularization functions

  void grad_regu_MSE(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    double mean = x.mean();
    vnl_vector<double> tx = x-mean; // difference to mean
    dx += 10000.0*tx*(2.0-2.0/dim)/dim;
  }

  void grad_regu_MSM(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    dx += 10000.0*m_Lambda*2.0*x/dim;
  }

  void grad_regu_L1(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    for (int i=0; i<dim; ++i)
      if (x[i]>0)
        dx[i] += 10000.0*m_Lambda/dim;
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
    dx += 10.0*tdx*2.0*m_Lambda/dim;
  }

  void calc_regularization(vnl_vector<double> const &x, double& cost)
  {
    if (regularization==Local_MSE)
      regu_localMSE(x, cost);
    else if (regularization==MSE)
      regu_MSE(x, cost);
    else if (regularization==MSM)
      regu_MSM(x, cost);
  }

  void calc_regularization_gradient(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    if (regularization==Local_MSE)
      grad_regu_localMSE(x,dx);
    else if (regularization==MSE)
      grad_regu_MSE(x,dx);
    else if (regularization==MSM)
      grad_regu_MSM(x,dx);
  }

  // cost function
  double f(vnl_vector<double> const &x)
  {
    // RMS error
    double cost = S->get_rms_error(x);
    cost *= cost;

    // regularize
    calc_regularization(x, cost);

    return cost;
  }

  // gradient of cost function
  void gradf(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    dx.fill(0.0);
    unsigned int N = m_b.size();

    // calculate output difference d
    vnl_vector<double> d; d.set_size(N);
    S->multiply(x,d);
    d -= m_b;

    S->transpose_multiply(d, dx);
    dx *= 2.0/N;

    if (regularization==Local_MSE)
      grad_regu_localMSE(x,dx);
    else if (regularization==MSE)
      grad_regu_MSE(x,dx);
    else if (regularization==MSM)
      grad_regu_MSM(x,dx);
  }
};

namespace itk{

/**
* \brief Fits the tractogram to the input image by assigning a weight to each fiber (similar to https://doi.org/10.1016/j.neuroimage.2015.06.092).  */


class FitFibersToImageFilter : public ImageSource< mitk::PeakImage::ItkPeakImageType >
{

public:

  typedef FitFibersToImageFilter Self;
  typedef ProcessObject Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  typedef itk::Point<float, 3> PointType3;
  typedef itk::Point<float, 4> PointType4;
  typedef mitk::DiffusionPropertyHelper::ImageType  VectorImgType;
  typedef mitk::PeakImage::ItkPeakImageType         PeakImgType;
  typedef itk::Image<unsigned char, 3>              UcharImgType;

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  itkTypeMacro( FitFibersToImageFilter, ImageSource )

  itkSetMacro( PeakImage, PeakImgType::Pointer)
  itkGetMacro( PeakImage, PeakImgType::Pointer)
  itkSetMacro( DiffImage, VectorImgType::Pointer)
  itkGetMacro( DiffImage, VectorImgType::Pointer)
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

  itkGetMacro( FittedImage, PeakImgType::Pointer)
  itkGetMacro( ResidualImage, PeakImgType::Pointer)
  itkGetMacro( OverexplainedImage, PeakImgType::Pointer)
  itkGetMacro( UnderexplainedImage, PeakImgType::Pointer)

  itkGetMacro( FittedImageDiff, VectorImgType::Pointer)
  itkGetMacro( ResidualImageDiff, VectorImgType::Pointer)
  itkGetMacro( OverexplainedImageDiff, VectorImgType::Pointer)
  itkGetMacro( UnderexplainedImageDiff, VectorImgType::Pointer)

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

  void SetSignalModel(mitk::DiffusionSignalModel<> *SignalModel);

  VnlCostFunction::REGU GetRegularization() const;
  void SetRegularization(const VnlCostFunction::REGU &GetRegularization);

protected:

  FitFibersToImageFilter();
  virtual ~FitFibersToImageFilter();

  vnl_vector_fixed<float,3> GetClosestPeak(itk::Index<4> idx, PeakImgType::Pointer m_PeakImage , vnl_vector_fixed<float,3> fiber_dir, int& id, double& w );

  void CreatePeakSystem();
  void CreateDiffSystem();

  void GenerateOutputPeakImages();
  void GenerateOutputDiffImages();

  std::vector< mitk::FiberBundle::Pointer >   m_Tractograms;
  PeakImgType::Pointer                        m_PeakImage;
  VectorImgType::Pointer                      m_DiffImage;
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

  PeakImgType::Pointer                        m_UnderexplainedImage;
  PeakImgType::Pointer                        m_OverexplainedImage;
  PeakImgType::Pointer                        m_ResidualImage;
  PeakImgType::Pointer                        m_FittedImage;

  VectorImgType::Pointer                      m_UnderexplainedImageDiff;
  VectorImgType::Pointer                      m_OverexplainedImageDiff;
  VectorImgType::Pointer                      m_ResidualImageDiff;
  VectorImgType::Pointer                      m_FittedImageDiff;

  mitk::DiffusionSignalModel<>*               m_SignalModel;

  vnl_sparse_matrix<double>                   A;
  vnl_vector<double>                          b;
  VnlCostFunction                             cost;
  int                                         sz_x;
  int                                         sz_y;
  int                                         sz_z;
  int                                         dim_four_size;
  double                                      m_MeanTractDensity;
  double                                      m_MeanSignal;
  unsigned int                                fiber_count;

  VnlCostFunction::REGU                       m_Regularization;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFitFibersToImageFilter.cpp"
#endif

#endif // __itkFitFibersToImageFilter_h__
