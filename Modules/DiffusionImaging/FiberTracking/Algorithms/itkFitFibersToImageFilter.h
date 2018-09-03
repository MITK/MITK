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
    VARIANCE,
    LASSO,
    VOXEL_VARIANCE,
    GROUP_LASSO,
    GROUP_VARIANCE,
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
  std::vector<unsigned int> group_sizes;

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

  void SetGroupSizes(std::vector<unsigned int> sizes)
  {
    unsigned int sum = 0;
    for (auto s : sizes)
      sum += s;
    if (sum!=m_A.cols())
    {
      MITK_INFO << "Group sizes do not match number of unknowns (" << sum << " vs. " << m_A.cols() << ")";
      return;
    }
    group_sizes = sizes;
  }

  VnlCostFunction(const int NumVars=0) : vnl_cost_function(NumVars)
  {
  }

  // Regularization: mean squared magnitude of weight vectors (small weights)
  void regu_MSM(vnl_vector<double> const &x, double& cost)
  {
    cost += 10000.0*m_Lambda*x.squared_magnitude()/dim;
  }

  // Regularization: mean squared deaviation of weights from mean weight (enforce uniform weights)
  void regu_Variance(vnl_vector<double> const &x, double& cost)
  {
    double mean = x.mean();
    vnl_vector<double> tx = x-mean;
    cost += 10000.0*m_Lambda*tx.squared_magnitude()/dim;
  }

  // Regularization: mean absolute magnitude of weight vectors (small weights) L1
  void regu_Lasso(vnl_vector<double> const &x, double& cost)
  {
    cost += m_Lambda*x.one_norm()/dim;
  }

  // Regularization: mean squared deaviation of weights from bundle mean weight (enforce uniform weights PER BUNDLE)
  void regu_GroupVariance(vnl_vector<double> const &x, double& cost)
  {
    vnl_vector<double> tx(x);
    unsigned int offset = 0;
    for (auto g : group_sizes)
    {
      double group_mean = 0;
      for (unsigned int i=0; i<g; ++i)
        group_mean += x[offset+i];
      group_mean /= g;

      for (unsigned int i=0; i<g; ++i)
        tx[offset+i] -= group_mean;

      offset += g;
    }

    cost += 10000.0*m_Lambda*tx.squared_magnitude()/dim;
  }

  // Regularization: voxel-weise mean squared deaviation of weights from voxel-wise mean weight (enforce locally uniform weights)
  void regu_VoxelVariance(vnl_vector<double> const &x, double& cost)
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

  // Regularization: group Lasso: sum_g(lambda_g * ||x_g||_2)
  void regu_GroupLasso(vnl_vector<double> const &x, double& cost)
  {
    unsigned int offset = 0;
    for (auto g : group_sizes)
    {
      double group_cost = 0;
      for (unsigned int i=0; i<g; ++i)
        group_cost += x[offset+i]*x[offset+i];
      cost += m_Lambda*std::sqrt(g)*std::sqrt(group_cost)/dim;
      offset += g;
    }
  }

  // gradients of regularization functions

  void grad_regu_MSM(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    dx += 10000.0*m_Lambda*2.0*x/dim;
  }

  void grad_regu_Variance(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    double mean = x.mean();
    vnl_vector<double> tx = x-mean; // difference to mean
    dx += 10000.0*tx*(2.0-2.0/dim)/dim;
  }

  void grad_regu_Lasso(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    for (int i=0; i<dim; ++i)
      if (x[i]>0)
        dx[i] += m_Lambda/dim;
  }

  void grad_regu_GroupVariance(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    vnl_vector<double> tx(x);
    unsigned int offset = 0;
    for (auto g : group_sizes)
    {
      double group_mean = 0;
      for (unsigned int i=0; i<g; ++i)
        group_mean += x[offset+i];
      group_mean /= g;

      for (unsigned int i=0; i<g; ++i)
        tx[offset+i] -= group_mean;

      offset += g;
    }

    dx += 10000.0*tx*(2.0-2.0/dim)/dim;
  }

  void grad_regu_VoxelVariance(vnl_vector<double> const &x, vnl_vector<double> &dx)
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

  void grad_regu_GroupLasso(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    unsigned int offset = 0;
    for (auto g : group_sizes)
    {
      double group_lambda = m_Lambda*std::sqrt(g)/dim;
      double group_l2 = 0;
      for (unsigned int i=0; i<g; ++i)
        group_l2 += x[offset+i]*x[offset+i];
      group_l2 = std::sqrt(group_l2);

      if (group_l2>0.0)
      {
        for (unsigned int i=0; i<g; ++i)
          dx[offset+i] += x[offset+i]*group_lambda/group_l2;
      }

      offset += g;
    }
  }

  void calc_regularization(vnl_vector<double> const &x, double& cost)
  {
    if (regularization==VOXEL_VARIANCE)
      regu_VoxelVariance(x, cost);
    else if (regularization==VARIANCE)
      regu_Variance(x, cost);
    else if (regularization==MSM)
      regu_MSM(x, cost);
    else if (regularization==LASSO)
      regu_Lasso(x, cost);
    else if (regularization==GROUP_LASSO)
      regu_GroupLasso(x, cost);
    else if (regularization==GROUP_VARIANCE)
      regu_GroupVariance(x, cost);
  }

  void calc_regularization_gradient(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    if (regularization==VOXEL_VARIANCE)
      grad_regu_VoxelVariance(x,dx);
    else if (regularization==VARIANCE)
      grad_regu_Variance(x,dx);
    else if (regularization==MSM)
      grad_regu_MSM(x,dx);
    else if (regularization==LASSO)
      grad_regu_Lasso(x,dx);
    else if (regularization==GROUP_LASSO)
      grad_regu_GroupLasso(x, dx);
    else if (regularization==GROUP_VARIANCE)
      grad_regu_GroupVariance(x, dx);
  }

  // cost function
  double f(vnl_vector<double> const &x)
  {
    // RMS error
    unsigned int N = m_b.size();
    vnl_vector<double> d; d.set_size(N);
    S->multiply(x,d);
    double cost = (d - m_b).squared_magnitude()/N;

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

    // (f(u(x)))' = f'(u(x)) * u'(x)
    // d/dx_j = 1/N * Sum_i A_i,j * 2*(A_i,j * x_j - b_i)
    S->transpose_multiply(d, dx);
    dx *= 2.0/N;

    calc_regularization_gradient(x,dx);
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
  typedef itk::Image<double, 3>                     DoubleImgType;

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  itkTypeMacro( FitFibersToImageFilter, ImageSource )

  itkSetMacro( ScalarImage, DoubleImgType::Pointer)
  itkGetMacro( ScalarImage, DoubleImgType::Pointer)
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
  itkSetMacro( FilterOutliers, bool)
  itkGetMacro( FilterOutliers, bool)
  itkSetMacro( Verbose, bool)
  itkGetMacro( Verbose, bool)

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

  itkGetMacro( FittedImageScalar, DoubleImgType::Pointer)
  itkGetMacro( ResidualImageScalar, DoubleImgType::Pointer)
  itkGetMacro( OverexplainedImageScalar, DoubleImgType::Pointer)
  itkGetMacro( UnderexplainedImageScalar, DoubleImgType::Pointer)

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

  void GetClosestPeak(itk::Index<4> idx, PeakImgType::Pointer m_PeakImage , vnl_vector_fixed<float,3> fiber_dir, int& id, double& w, double& peak_mag );

  void CreatePeakSystem();
  void CreateDiffSystem();
  void CreateScalarSystem();

  void GenerateOutputPeakImages();
  void GenerateOutputDiffImages();
  void GenerateOutputScalarImages();

  std::vector< mitk::FiberBundle::Pointer >   m_Tractograms;
  PeakImgType::Pointer                        m_PeakImage;
  VectorImgType::Pointer                      m_DiffImage;
  DoubleImgType::Pointer                      m_ScalarImage;
  UcharImgType::Pointer                       m_MaskImage;
  bool                                        m_FitIndividualFibers;
  double                                      m_GradientTolerance;
  double                                      m_Lambda;
  int                                         m_MaxIterations;
  double                                      m_Coverage;
  double                                      m_Overshoot;
  double                                      m_RMSE;
  bool                                        m_FilterOutliers;
  double                                      m_MeanWeight;
  double                                      m_MedianWeight;
  double                                      m_MinWeight;
  double                                      m_MaxWeight;
  bool                                        m_Verbose;
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

  DoubleImgType::Pointer                      m_UnderexplainedImageScalar;
  DoubleImgType::Pointer                      m_OverexplainedImageScalar;
  DoubleImgType::Pointer                      m_ResidualImageScalar;
  DoubleImgType::Pointer                      m_FittedImageScalar;

  mitk::DiffusionSignalModel<>*               m_SignalModel;

  vnl_sparse_matrix<double>                   A;
  vnl_vector<double>                          b;
  VnlCostFunction                             cost;
  unsigned int                                sz_x;
  unsigned int                                sz_y;
  unsigned int                                sz_z;
  int                                         dim_four_size;
  double                                      m_MeanTractDensity;
  double                                      m_MeanSignal;
  unsigned int                                fiber_count;

  VnlCostFunction::REGU                       m_Regularization;
  std::vector<unsigned int>                   m_GroupSizes;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFitFibersToImageFilter.cpp"
#endif

#endif // __itkFitFibersToImageFilter_h__
