#include "itkFitFibersToImageFilter.h"

#include <boost/progress.hpp>

namespace itk{

FitFibersToImageFilter::FitFibersToImageFilter()
  : m_FitIndividualFibers(true)
  , m_GradientTolerance(1e-5)
  , m_Lambda(0.1)
  , m_MaxIterations(20)
  , m_FiberSampling(10)
  , m_Coverage(0)
  , m_Overshoot(0)
  , m_FilterOutliers(true)
{
  this->SetNumberOfRequiredOutputs(3);
}

FitFibersToImageFilter::~FitFibersToImageFilter()
{

}

void FitFibersToImageFilter::GenerateData()
{
  int sz_x = m_PeakImage->GetLargestPossibleRegion().GetSize(0);
  int sz_y = m_PeakImage->GetLargestPossibleRegion().GetSize(1);
  int sz_z = m_PeakImage->GetLargestPossibleRegion().GetSize(2);
  int sz_peaks = m_PeakImage->GetLargestPossibleRegion().GetSize(3)/3 + 1; // +1 for zero - peak
  int num_voxels = sz_x*sz_y*sz_z;

  float minSpacing = 1;
  if(m_PeakImage->GetSpacing()[0]<m_PeakImage->GetSpacing()[1] && m_PeakImage->GetSpacing()[0]<m_PeakImage->GetSpacing()[2])
    minSpacing = m_PeakImage->GetSpacing()[0];
  else if (m_PeakImage->GetSpacing()[1] < m_PeakImage->GetSpacing()[2])
    minSpacing = m_PeakImage->GetSpacing()[1];
  else
    minSpacing = m_PeakImage->GetSpacing()[2];

  unsigned int num_unknowns = m_Tractograms.size();
  if (m_FitIndividualFibers)
  {
    num_unknowns = 0;
    for (unsigned int bundle=0; bundle<m_Tractograms.size(); bundle++)
    {
      m_Tractograms.at(bundle)->ResampleLinear(minSpacing/m_FiberSampling);
      num_unknowns += m_Tractograms.at(bundle)->GetNumFibers();
    }
  }

  unsigned int number_of_residuals = num_voxels * sz_peaks;

  // create linear system
  MITK_INFO << "Num. unknowns: " << num_unknowns;
  MITK_INFO << "Num. residuals: " << number_of_residuals;

  MITK_INFO << "Creating system ...";
  vnl_sparse_matrix<double> A;
  vnl_vector<double> b;
  A.set_size(number_of_residuals, num_unknowns);
  b.set_size(number_of_residuals); b.fill(0.0);

  double TD = 0;
  double FD = 0;
  unsigned int dir_count = 0;
  unsigned int fiber_count = 0;

  for (unsigned int bundle=0; bundle<m_Tractograms.size(); bundle++)
  {
    vtkSmartPointer<vtkPolyData> polydata = m_Tractograms.at(bundle)->GetFiberPolyData();

    for (int i=0; i<m_Tractograms.at(bundle)->GetNumFibers(); ++i)
    {
      vtkCell* cell = polydata->GetCell(i);
      int numPoints = cell->GetNumberOfPoints();
      vtkPoints* points = cell->GetPoints();

      if (numPoints<2)
        MITK_INFO << "FIBER WITH ONLY ONE POINT ENCOUNTERED!";

      for (int j=0; j<numPoints-1; ++j)
      {
        double* p1 = points->GetPoint(j);
        PointType4 p;
        p[0]=p1[0];
        p[1]=p1[1];
        p[2]=p1[2];
        p[3]=0;

        itk::Index<4> idx4;
        m_PeakImage->TransformPhysicalPointToIndex(p, idx4);
        if (!m_PeakImage->GetLargestPossibleRegion().IsInside(idx4))
          continue;

        double* p2 = points->GetPoint(j+1);
        vnl_vector_fixed<float,3> fiber_dir;
        fiber_dir[0] = p[0]-p2[0];
        fiber_dir[1] = p[1]-p2[1];
        fiber_dir[2] = p[2]-p2[2];
        fiber_dir.normalize();

        double w = 1;
        int peak_id = sz_peaks-1;
        vnl_vector_fixed<float,3> odf_peak = GetClosestPeak(idx4, m_PeakImage, fiber_dir, peak_id, w);
        float peak_mag = odf_peak.magnitude();

        int x = idx4[0];
        int y = idx4[1];
        int z = idx4[2];

        unsigned int linear_index = x + sz_x*y + sz_x*sz_y*z + sz_x*sz_y*sz_z*peak_id;

        if (b[linear_index] == 0 && peak_id<3)
        {
          dir_count++;
          FD += peak_mag;
        }
        TD += w;

        if (m_FitIndividualFibers)
        {
          b[linear_index] = peak_mag;
          A.put(linear_index, fiber_count, A.get(linear_index, fiber_count) + w);
        }
        else
        {
          b[linear_index] = peak_mag;
          A.put(linear_index, bundle, A.get(linear_index, bundle) + w);
        }
      }

      ++fiber_count;
    }
  }

  TD /= (dir_count*fiber_count);
  FD /= dir_count;
  A /= TD;
  b *= 100.0/FD;  // times 100 because we want to avoid too small values for computational reasons

  double init_lambda = 1e5;  // initialization for lambda estimation

  itk::TimeProbe clock;
  clock.Start();

  VnlCostFunction cost(num_unknowns);
  cost.SetProblem(A, b, init_lambda);
  m_Weights.set_size(num_unknowns); m_Weights.fill( TD/100.0 * FD/2.0 );
  vnl_lbfgsb minimizer(cost);
  vnl_vector<double> l; l.set_size(num_unknowns); l.fill(0);
  vnl_vector<long> bound_selection; bound_selection.set_size(num_unknowns); bound_selection.fill(1);
  minimizer.set_bound_selection(bound_selection);
  minimizer.set_lower_bound(l);
  minimizer.set_projected_gradient_tolerance(m_GradientTolerance);

  MITK_INFO << "Estimating regularization";
  minimizer.set_trace(false);
  minimizer.set_max_function_evals(1);
  minimizer.minimize(m_Weights);
  vnl_vector<double> dx; dx.set_size(num_unknowns); dx.fill(0.0);
  cost.grad_regu_localMSE(m_Weights, dx);
  double r = dx.magnitude()/m_Weights.magnitude();
  cost.m_Lambda *= m_Lambda*55.0/r;
  MITK_INFO << "Using regularization factor of " << cost.m_Lambda << " (λ: " << m_Lambda << ")";

  MITK_INFO << "Fitting fibers";
  minimizer.set_trace(true);
  minimizer.set_max_function_evals(m_MaxIterations);
  minimizer.minimize(m_Weights);

  std::vector< double > weights;
  if (m_FilterOutliers)
  {
    for (auto w : m_Weights)
      weights.push_back(w);
    std::sort(weights.begin(), weights.end());
    MITK_INFO << "Setting upper weight bound to " << weights.at(num_unknowns*0.95);
    vnl_vector<double> u; u.set_size(num_unknowns); u.fill(weights.at(num_unknowns*0.95));
    minimizer.set_upper_bound(u);
    bound_selection.fill(2);
    minimizer.set_bound_selection(bound_selection);
    minimizer.minimize(m_Weights);
    weights.clear();
  }

  for (auto w : m_Weights)
    weights.push_back(w);
  std::sort(weights.begin(), weights.end());

  MITK_INFO << "*************************";
  MITK_INFO << "Weight statistics";
  MITK_INFO << "Mean: " << m_Weights.mean();
  MITK_INFO << "Median: " << weights.at(num_unknowns*0.5);
  MITK_INFO << "75% quantile: " << weights.at(num_unknowns*0.75);
  MITK_INFO << "95% quantile: " << weights.at(num_unknowns*0.95);
  MITK_INFO << "99% quantile: " << weights.at(num_unknowns*0.99);
  MITK_INFO << "Min: " << weights.at(0);
  MITK_INFO << "Max: " << weights.at(num_unknowns-1);
  MITK_INFO << "*************************";
  MITK_INFO << "NumEvals: " << minimizer.get_num_evaluations();
  MITK_INFO << "NumIterations: " << minimizer.get_num_iterations();
  MITK_INFO << "Residual cost: " << minimizer.get_end_error();
  MITK_INFO << "Final RMS: " << cost.S->get_rms_error(m_Weights);

  clock.Stop();
  int h = clock.GetTotal()/3600;
  int m = ((int)clock.GetTotal()%3600)/60;
  int s = (int)clock.GetTotal()%60;
  MITK_INFO << "Optimization took " << h << "h, " << m << "m and " << s << "s";

  // transform back for peak image creation
  A *= FD/100.0;
  b *= FD/100.0;

  MITK_INFO << "Weighting fibers";
  if (m_FitIndividualFibers)
  {
    unsigned int fiber_count = 0;
    for (unsigned int bundle=0; bundle<m_Tractograms.size(); bundle++)
    {
      for (int i=0; i<m_Tractograms.at(bundle)->GetNumFibers(); i++)
      {
        m_Tractograms.at(bundle)->SetFiberWeight(i, m_Weights[fiber_count]);
        ++fiber_count;
      }
      m_Tractograms.at(bundle)->Compress(0.1);
      m_Tractograms.at(bundle)->ColorFibersByFiberWeights(false, true);
    }
  }
  else
  {
    for (unsigned int i=0; i<m_Tractograms.size(); ++i)
    {
      m_Tractograms.at(i)->SetFiberWeights(m_Weights[i]);
      m_Tractograms.at(i)->Compress(0.1);
      m_Tractograms.at(i)->ColorFibersByFiberWeights(false, true);
    }
  }

  MITK_INFO << "Generating output images ...";

  itk::ImageDuplicator< PeakImgType >::Pointer duplicator = itk::ImageDuplicator< PeakImgType >::New();
  duplicator->SetInputImage(m_PeakImage);
  duplicator->Update();
  m_UnderexplainedImage = duplicator->GetOutput();
  m_UnderexplainedImage->FillBuffer(0.0);

  duplicator->SetInputImage(m_UnderexplainedImage);
  duplicator->Update();
  m_OverexplainedImage = duplicator->GetOutput();
  m_OverexplainedImage->FillBuffer(0.0);

  duplicator->SetInputImage(m_OverexplainedImage);
  duplicator->Update();
  m_ResidualImage = duplicator->GetOutput();
  m_ResidualImage->FillBuffer(0.0);

  duplicator->SetInputImage(m_ResidualImage);
  duplicator->Update();
  m_FittedImage = duplicator->GetOutput();
  m_FittedImage->FillBuffer(0.0);

  vnl_vector<double> fitted_b; fitted_b.set_size(b.size());
  cost.S->multiply(m_Weights, fitted_b);

  for (unsigned int r=0; r<b.size(); r++)
  {
    itk::Index<4> idx;
    unsigned int linear_index = r;
    idx[0] = linear_index % sz_x; linear_index /= sz_x;
    idx[1] = linear_index % sz_y; linear_index /= sz_y;
    idx[2] = linear_index % sz_z; linear_index /= sz_z;
    int peak_id = linear_index % sz_peaks;

    if (peak_id<sz_peaks-1)
    {
      vnl_vector_fixed<float,3> peak_dir;

      idx[3] = peak_id*3;
      peak_dir[0] = m_PeakImage->GetPixel(idx);
      idx[3] += 1;
      peak_dir[1] = m_PeakImage->GetPixel(idx);
      idx[3] += 1;
      peak_dir[2] = m_PeakImage->GetPixel(idx);

      peak_dir.normalize();
      peak_dir *= fitted_b[r];

      idx[3] = peak_id*3;
      m_FittedImage->SetPixel(idx, peak_dir[0]);

      idx[3] += 1;
      m_FittedImage->SetPixel(idx, peak_dir[1]);

      idx[3] += 1;
      m_FittedImage->SetPixel(idx, peak_dir[2]);
    }
  }

  FD = 0;
  m_Coverage = 0;
  m_Overshoot = 0;

  itk::Index<4> idx;
  for (idx[0]=0; idx[0]<sz_x; ++idx[0])
    for (idx[1]=0; idx[1]<sz_y; ++idx[1])
      for (idx[2]=0; idx[2]<sz_z; ++idx[2])
      {
        vnl_vector_fixed<float,3> peak_dir;
        vnl_vector_fixed<float,3> fitted_dir;
        vnl_vector_fixed<float,3> overshoot_dir;
        for (idx[3]=0; idx[3]<(itk::IndexValueType)m_PeakImage->GetLargestPossibleRegion().GetSize(3); ++idx[3])
        {
          peak_dir[idx[3]%3] = m_PeakImage->GetPixel(idx);
          fitted_dir[idx[3]%3] = m_FittedImage->GetPixel(idx);
          m_ResidualImage->SetPixel(idx, m_PeakImage->GetPixel(idx) - m_FittedImage->GetPixel(idx));

          if (idx[3]%3==2)
          {
            FD += peak_dir.magnitude();

            itk::Index<4> tidx= idx;
            if (peak_dir.magnitude()>fitted_dir.magnitude())
            {
              m_Coverage += fitted_dir.magnitude();
              m_UnderexplainedImage->SetPixel(tidx, peak_dir[2]-fitted_dir[2]); tidx[3]--;
              m_UnderexplainedImage->SetPixel(tidx, peak_dir[1]-fitted_dir[1]); tidx[3]--;
              m_UnderexplainedImage->SetPixel(tidx, peak_dir[0]-fitted_dir[0]);
            }
            else
            {
              overshoot_dir[0] = fitted_dir[0]-peak_dir[0];
              overshoot_dir[1] = fitted_dir[1]-peak_dir[1];
              overshoot_dir[2] = fitted_dir[2]-peak_dir[2];
              m_Coverage += peak_dir.magnitude();
              m_Overshoot += overshoot_dir.magnitude();
              m_OverexplainedImage->SetPixel(tidx, overshoot_dir[2]); tidx[3]--;
              m_OverexplainedImage->SetPixel(tidx, overshoot_dir[1]); tidx[3]--;
              m_OverexplainedImage->SetPixel(tidx, overshoot_dir[0]);
            }
          }
        }
      }

  m_Coverage = 100.0*m_Coverage/FD;
  m_Overshoot = 100.0*m_Overshoot/FD;

  MITK_INFO << std::fixed << "Coverage: " << setprecision(1) << m_Coverage << "%";
  MITK_INFO << std::fixed << "Overshoot: " << setprecision(1) << m_Overshoot << "%";
}

vnl_vector_fixed<float,3> FitFibersToImageFilter::GetClosestPeak(itk::Index<4> idx, PeakImgType::Pointer peak_image , vnl_vector_fixed<float,3> fiber_dir, int& id, double& w )
{
  int m_NumDirs = peak_image->GetLargestPossibleRegion().GetSize()[3]/3;
  vnl_vector_fixed<float,3> out_dir; out_dir.fill(0);
  float angle = 0.8;

  for (int i=0; i<m_NumDirs; i++)
  {
    vnl_vector_fixed<float,3> dir;
    idx[3] = i*3;
    dir[0] = peak_image->GetPixel(idx);
    idx[3] += 1;
    dir[1] = peak_image->GetPixel(idx);
    idx[3] += 1;
    dir[2] = peak_image->GetPixel(idx);

    float mag = dir.magnitude();
    if (mag<mitk::eps)
      continue;

    dir.normalize();

    float a = dot_product(dir, fiber_dir);
    if (fabs(a)>angle)
    {
      angle = fabs(a);
      w = angle;
      if (a<0)
        out_dir = -dir;
      else
        out_dir = dir;
      out_dir *= mag;
      id = i;
    }
  }

  return out_dir;
}

std::vector<mitk::FiberBundle::Pointer> FitFibersToImageFilter::GetTractograms() const
{
  return m_Tractograms;
}

void FitFibersToImageFilter::SetTractograms(const std::vector<mitk::FiberBundle::Pointer> &tractograms)
{
  m_Tractograms = tractograms;
}

}



