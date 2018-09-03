#include "itkFitFibersToImageFilter.h"

#include <boost/progress.hpp>
#include <mitkDiffusionFunctionCollection.h>

namespace itk{

FitFibersToImageFilter::FitFibersToImageFilter()
  : m_PeakImage(nullptr)
  , m_DiffImage(nullptr)
  , m_ScalarImage(nullptr)
  , m_MaskImage(nullptr)
  , m_FitIndividualFibers(true)
  , m_GradientTolerance(1e-5)
  , m_Lambda(0.1)
  , m_MaxIterations(20)
  , m_Coverage(0)
  , m_Overshoot(0)
  , m_RMSE(0.0)
  , m_FilterOutliers(false)
  , m_MeanWeight(1.0)
  , m_MedianWeight(1.0)
  , m_MinWeight(1.0)
  , m_MaxWeight(1.0)
  , m_Verbose(true)
  , m_NumUnknowns(0)
  , m_NumResiduals(0)
  , m_NumCoveredDirections(0)
  , m_SignalModel(nullptr)
  , sz_x(0)
  , sz_y(0)
  , sz_z(0)
  , m_MeanTractDensity(0)
  , m_MeanSignal(0)
  , fiber_count(0)
  , m_Regularization(VnlCostFunction::REGU::VOXEL_VARIANCE)
{
  this->SetNumberOfRequiredOutputs(3);
}

FitFibersToImageFilter::~FitFibersToImageFilter()
{

}

void FitFibersToImageFilter::CreateDiffSystem()
{
  sz_x = m_DiffImage->GetLargestPossibleRegion().GetSize(0);
  sz_y = m_DiffImage->GetLargestPossibleRegion().GetSize(1);
  sz_z = m_DiffImage->GetLargestPossibleRegion().GetSize(2);
  dim_four_size = m_DiffImage->GetVectorLength();
  int num_voxels = sz_x*sz_y*sz_z;

  if (m_MaskImage.IsNotNull() &&
      (m_MaskImage->GetLargestPossibleRegion().GetSize()[0]!=sz_x ||
       m_MaskImage->GetLargestPossibleRegion().GetSize()[1]!=sz_y ||
       m_MaskImage->GetLargestPossibleRegion().GetSize()[2]!=sz_z))
    mitkThrow() << "Mask and peak image must have same size!";

  auto spacing = m_DiffImage->GetSpacing();

  m_NumResiduals = num_voxels * dim_four_size;

  MITK_INFO << "Num. unknowns: " << m_NumUnknowns;
  MITK_INFO << "Num. residuals: " << m_NumResiduals;
  MITK_INFO << "Creating system ...";

  A.set_size(m_NumResiduals, m_NumUnknowns);
  b.set_size(m_NumResiduals); b.fill(0.0);

  m_MeanTractDensity = 0;
  m_MeanSignal = 0;
  m_NumCoveredDirections = 0;
  fiber_count = 0;
  vnl_vector<int> voxel_indicator; voxel_indicator.set_size(sz_x*sz_y*sz_z); voxel_indicator.fill(0);

  unsigned int numFibers = 0;
  for (unsigned int bundle=0; bundle<m_Tractograms.size(); bundle++)
    numFibers += m_Tractograms.at(bundle)->GetNumFibers();
  boost::progress_display disp(numFibers);
  m_GroupSizes.clear();
  for (unsigned int bundle=0; bundle<m_Tractograms.size(); bundle++)
  {
    vtkSmartPointer<vtkPolyData> polydata = m_Tractograms.at(bundle)->GetFiberPolyData();
    m_GroupSizes.push_back(m_Tractograms.at(bundle)->GetNumFibers());
    for (unsigned int i=0; i<m_Tractograms.at(bundle)->GetNumFibers(); ++i)
    {
      ++disp;
      vtkCell* cell = polydata->GetCell(i);
      int numPoints = cell->GetNumberOfPoints();
      vtkPoints* points = cell->GetPoints();

      if (numPoints<2)
        MITK_INFO << "FIBER WITH ONLY ONE POINT ENCOUNTERED!";

      for (int j=0; j<numPoints-1; ++j)
      {
        PointType3 startVertex = mitk::imv::GetItkPoint(points->GetPoint(j));
        itk::Index<3> startIndex;
        itk::ContinuousIndex<float, 3> startIndexCont;
        m_DiffImage->TransformPhysicalPointToIndex(startVertex, startIndex);
        m_DiffImage->TransformPhysicalPointToContinuousIndex(startVertex, startIndexCont);

        PointType3 endVertex = mitk::imv::GetItkPoint(points->GetPoint(j+1));
        itk::Index<3> endIndex;
        itk::ContinuousIndex<float, 3> endIndexCont;
        m_DiffImage->TransformPhysicalPointToIndex(endVertex, endIndex);
        m_DiffImage->TransformPhysicalPointToContinuousIndex(endVertex, endIndexCont);

        mitk::DiffusionSignalModel<>::GradientType fiber_dir;
        fiber_dir[0] = endVertex[0]-startVertex[0];
        fiber_dir[1] = endVertex[1]-startVertex[1];
        fiber_dir[2] = endVertex[2]-startVertex[2];
        fiber_dir.Normalize();

        std::vector< std::pair< itk::Index<3>, double > > segments = mitk::imv::IntersectImage(spacing, startIndex, endIndex, startIndexCont, endIndexCont);
        for (std::pair< itk::Index<3>, double > seg : segments)
        {
          if (!m_DiffImage->GetLargestPossibleRegion().IsInside(seg.first) || (m_MaskImage.IsNotNull() && m_MaskImage->GetPixel(seg.first)==0))
            continue;

          int x = seg.first[0];
          int y = seg.first[1];
          int z = seg.first[2];

          mitk::DiffusionSignalModel<>::PixelType simulated_pixel = m_SignalModel->SimulateMeasurement(fiber_dir)*seg.second;
          VectorImgType::PixelType measured_pixel = m_DiffImage->GetPixel(seg.first);

          double simulated_mean = 0;
          double measured_mean = 0;
          int num_nonzero_g = 0;
          for (int g=0; g<dim_four_size; ++g)
          {
            if( m_SignalModel->GetGradientDirection(g).GetNorm()<mitk::eps )
              continue;
            simulated_mean += simulated_pixel[g];
            measured_mean += (double)measured_pixel[g];
            ++num_nonzero_g;
          }
          simulated_mean /= num_nonzero_g;
          measured_mean /= num_nonzero_g;
          simulated_pixel -= simulated_mean;

          if (voxel_indicator[x + sz_x*y + sz_x*sz_y*z]==0)
            m_MeanSignal += measured_mean;
          m_MeanTractDensity += simulated_mean;
          voxel_indicator[x + sz_x*y + sz_x*sz_y*z] = 1;

          for (int g=0; g<dim_four_size; ++g)
          {
            unsigned int linear_index = x + sz_x*y + sz_x*sz_y*z + sz_x*sz_y*sz_z*g;

            if (m_FitIndividualFibers)
            {
              b[linear_index] = (double)measured_pixel[g] - measured_mean;
              A.put(linear_index, fiber_count, A.get(linear_index, fiber_count) + simulated_pixel[g]);
            }
            else
            {
              b[linear_index] = (double)measured_pixel[g] - measured_mean;
              A.put(linear_index, bundle, A.get(linear_index, bundle) + simulated_pixel[g]);
            }
          }
        }

      }

      ++fiber_count;
    }
  }

  m_NumCoveredDirections = voxel_indicator.sum();

  m_MeanTractDensity /= (m_NumCoveredDirections*fiber_count);
  m_MeanSignal /= m_NumCoveredDirections;
  A /= m_MeanTractDensity;
  b *= 100.0/m_MeanSignal;  // times 100 because we want to avoid too small values for computational reasons

  // NEW FIT
  //  m_MeanTractDensity /= m_NumCoveredDirections;
  //  m_MeanSignal /= m_NumCoveredDirections;
  //  b /= m_MeanSignal;
  //  b *= m_MeanTractDensity;
}

void FitFibersToImageFilter::CreatePeakSystem()
{
  sz_x = m_PeakImage->GetLargestPossibleRegion().GetSize(0);
  sz_y = m_PeakImage->GetLargestPossibleRegion().GetSize(1);
  sz_z = m_PeakImage->GetLargestPossibleRegion().GetSize(2);
  dim_four_size = m_PeakImage->GetLargestPossibleRegion().GetSize(3)/3 + 1; // +1 for zero - peak
  int num_voxels = sz_x*sz_y*sz_z;

  itk::Vector< double, 3 > spacing;
  spacing[0] = m_PeakImage->GetSpacing()[0];
  spacing[1] = m_PeakImage->GetSpacing()[1];
  spacing[2] = m_PeakImage->GetSpacing()[2];

  PointType3 origin;
  origin[0] = m_PeakImage->GetOrigin()[0];
  origin[1] = m_PeakImage->GetOrigin()[1];
  origin[2] = m_PeakImage->GetOrigin()[2];

  UcharImgType::RegionType::SizeType size;
  size[0] = m_PeakImage->GetLargestPossibleRegion().GetSize()[0];
  size[1] = m_PeakImage->GetLargestPossibleRegion().GetSize()[1];
  size[2] = m_PeakImage->GetLargestPossibleRegion().GetSize()[2];
  UcharImgType::RegionType imageRegion; imageRegion.SetSize(size);

  itk::Matrix<double, 3, 3> direction;
  for (int r=0; r<3; r++)
    for (int c=0; c<3; c++)
      direction[r][c] = m_PeakImage->GetDirection()[r][c];

  if (m_MaskImage.IsNull())
  {
    m_MaskImage = UcharImgType::New();
    m_MaskImage->SetSpacing( spacing );
    m_MaskImage->SetOrigin( origin );
    m_MaskImage->SetDirection( direction );
    m_MaskImage->SetRegions( imageRegion );
    m_MaskImage->Allocate();
    m_MaskImage->FillBuffer(1);
  }
  else if (m_MaskImage->GetLargestPossibleRegion().GetSize()[0]!=sz_x ||
           m_MaskImage->GetLargestPossibleRegion().GetSize()[1]!=sz_y ||
           m_MaskImage->GetLargestPossibleRegion().GetSize()[2]!=sz_z)
    mitkThrow() << "Mask and peak image must have same size!";


  m_NumResiduals = num_voxels * dim_four_size;

  MITK_INFO << "Num. unknowns: " << m_NumUnknowns;
  MITK_INFO << "Num. residuals: " << m_NumResiduals;
  MITK_INFO << "Creating system ...";

  A.set_size(m_NumResiduals, m_NumUnknowns);
  b.set_size(m_NumResiduals); b.fill(0.0);

  m_MeanTractDensity = 0;
  m_MeanSignal = 0;
  m_NumCoveredDirections = 0;
  fiber_count = 0;

  m_GroupSizes.clear();

  int numFibers = 0;
  for (unsigned int bundle=0; bundle<m_Tractograms.size(); bundle++)
    numFibers += m_Tractograms.at(bundle)->GetNumFibers();
  boost::progress_display disp(numFibers);
  for (unsigned int bundle=0; bundle<m_Tractograms.size(); bundle++)
  {
    vtkSmartPointer<vtkPolyData> polydata = m_Tractograms.at(bundle)->GetFiberPolyData();

    m_GroupSizes.push_back(m_Tractograms.at(bundle)->GetNumFibers());
    for (unsigned int i=0; i<m_Tractograms.at(bundle)->GetNumFibers(); ++i)
    {
      ++disp;
      vtkCell* cell = polydata->GetCell(i);
      int numPoints = cell->GetNumberOfPoints();
      vtkPoints* points = cell->GetPoints();

      if (numPoints<2)
        MITK_INFO << "FIBER WITH ONLY ONE POINT ENCOUNTERED!";

      for (int j=0; j<numPoints-1; ++j)
      {
        PointType3 startVertex = mitk::imv::GetItkPoint(points->GetPoint(j));
        itk::Index<3> startIndex;
        itk::ContinuousIndex<float, 3> startIndexCont;
        m_MaskImage->TransformPhysicalPointToIndex(startVertex, startIndex);
        m_MaskImage->TransformPhysicalPointToContinuousIndex(startVertex, startIndexCont);

        PointType3 endVertex = mitk::imv::GetItkPoint(points->GetPoint(j+1));
        itk::Index<3> endIndex;
        itk::ContinuousIndex<float, 3> endIndexCont;
        m_MaskImage->TransformPhysicalPointToIndex(endVertex, endIndex);
        m_MaskImage->TransformPhysicalPointToContinuousIndex(endVertex, endIndexCont);

        vnl_vector_fixed<float,3> fiber_dir;
        fiber_dir[0] = endVertex[0]-startVertex[0];
        fiber_dir[1] = endVertex[1]-startVertex[1];
        fiber_dir[2] = endVertex[2]-startVertex[2];
        fiber_dir.normalize();

        std::vector< std::pair< itk::Index<3>, double > > segments = mitk::imv::IntersectImage(spacing, startIndex, endIndex, startIndexCont, endIndexCont);
        for (std::pair< itk::Index<3>, double > seg : segments)
        {
          if (!m_MaskImage->GetLargestPossibleRegion().IsInside(seg.first) || m_MaskImage->GetPixel(seg.first)==0)
            continue;

          itk::Index<4> idx4;
          idx4[0]=seg.first[0];
          idx4[1]=seg.first[1];
          idx4[2]=seg.first[2];
          idx4[3]=0;

          double w = 1;
          int peak_id = dim_four_size-1;

          double peak_mag = 0;
          GetClosestPeak(idx4, m_PeakImage, fiber_dir, peak_id, w, peak_mag);
          w *= seg.second;

          int x = idx4[0];
          int y = idx4[1];
          int z = idx4[2];

          unsigned int linear_index = x + sz_x*y + sz_x*sz_y*z + sz_x*sz_y*sz_z*peak_id;

          if (b[linear_index] == 0 && peak_id<dim_four_size-1)
          {
            m_NumCoveredDirections++;
            m_MeanSignal += peak_mag;
          }
          m_MeanTractDensity += w;

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
      }

      ++fiber_count;
    }
  }
  m_MeanTractDensity /= (m_NumCoveredDirections*fiber_count);
  m_MeanSignal /= m_NumCoveredDirections;
  A /= m_MeanTractDensity;
  b *= 100.0/m_MeanSignal;  // times 100 because we want to avoid too small values for computational reasons

  // NEW FIT
  //  m_MeanTractDensity /= m_NumCoveredDirections;
  //  m_MeanSignal /= m_NumCoveredDirections;
  //  b /= m_MeanSignal;
  //  b *= m_MeanTractDensity;
}

void FitFibersToImageFilter::CreateScalarSystem()
{
  sz_x = m_ScalarImage->GetLargestPossibleRegion().GetSize(0);
  sz_y = m_ScalarImage->GetLargestPossibleRegion().GetSize(1);
  sz_z = m_ScalarImage->GetLargestPossibleRegion().GetSize(2);
  int num_voxels = sz_x*sz_y*sz_z;

  if (m_MaskImage.IsNotNull() &&
      (m_MaskImage->GetLargestPossibleRegion().GetSize()[0]!=sz_x ||
       m_MaskImage->GetLargestPossibleRegion().GetSize()[1]!=sz_y ||
       m_MaskImage->GetLargestPossibleRegion().GetSize()[2]!=sz_z))
    mitkThrow() << "Mask and peak image must have same size!";

  auto spacing = m_ScalarImage->GetSpacing();
  m_NumResiduals = num_voxels;

  MITK_INFO << "Num. unknowns: " << m_NumUnknowns;
  MITK_INFO << "Num. residuals: " << m_NumResiduals;
  MITK_INFO << "Creating system ...";

  A.set_size(m_NumResiduals, m_NumUnknowns);
  b.set_size(m_NumResiduals); b.fill(0.0);

  m_MeanTractDensity = 0;
  m_MeanSignal = 0;
  int numCoveredVoxels = 0;
  fiber_count = 0;

  int numFibers = 0;
  for (unsigned int bundle=0; bundle<m_Tractograms.size(); bundle++)
    numFibers += m_Tractograms.at(bundle)->GetNumFibers();
  boost::progress_display disp(numFibers);
  m_GroupSizes.clear();
  for (unsigned int bundle=0; bundle<m_Tractograms.size(); bundle++)
  {
    vtkSmartPointer<vtkPolyData> polydata = m_Tractograms.at(bundle)->GetFiberPolyData();

    m_GroupSizes.push_back(m_Tractograms.at(bundle)->GetNumFibers());
    for (unsigned int i=0; i<m_Tractograms.at(bundle)->GetNumFibers(); ++i)
    {
      ++disp;
      vtkCell* cell = polydata->GetCell(i);
      int numPoints = cell->GetNumberOfPoints();
      vtkPoints* points = cell->GetPoints();

      for (int j=0; j<numPoints-1; ++j)
      {
        PointType3 startVertex = mitk::imv::GetItkPoint(points->GetPoint(j));
        itk::Index<3> startIndex;
        itk::ContinuousIndex<float, 3> startIndexCont;
        m_ScalarImage->TransformPhysicalPointToIndex(startVertex, startIndex);
        m_ScalarImage->TransformPhysicalPointToContinuousIndex(startVertex, startIndexCont);

        PointType3 endVertex = mitk::imv::GetItkPoint(points->GetPoint(j+1));
        itk::Index<3> endIndex;
        itk::ContinuousIndex<float, 3> endIndexCont;
        m_ScalarImage->TransformPhysicalPointToIndex(endVertex, endIndex);
        m_ScalarImage->TransformPhysicalPointToContinuousIndex(endVertex, endIndexCont);

        std::vector< std::pair< itk::Index<3>, double > > segments = mitk::imv::IntersectImage(spacing, startIndex, endIndex, startIndexCont, endIndexCont);
        for (std::pair< itk::Index<3>, double > seg : segments)
        {
          if (!m_ScalarImage->GetLargestPossibleRegion().IsInside(seg.first) || (m_MaskImage.IsNotNull() && m_MaskImage->GetPixel(seg.first)==0))
            continue;

          float image_value = m_ScalarImage->GetPixel(seg.first);
          int x = seg.first[0];
          int y = seg.first[1];
          int z = seg.first[2];

          unsigned int linear_index = x + sz_x*y + sz_x*sz_y*z;

          if (b[linear_index] == 0)
          {
            numCoveredVoxels++;
            m_MeanSignal += image_value;
          }
          m_MeanTractDensity += seg.second;

          if (m_FitIndividualFibers)
          {
            b[linear_index] = image_value;
            A.put(linear_index, fiber_count, A.get(linear_index, fiber_count) + seg.second);
          }
          else
          {
            b[linear_index] = image_value;
            A.put(linear_index, bundle, A.get(linear_index, bundle) + seg.second);
          }
        }
      }

      ++fiber_count;
    }
  }
  m_MeanTractDensity /= (numCoveredVoxels*fiber_count);
  m_MeanSignal /= numCoveredVoxels;
  A /= m_MeanTractDensity;
  b *= 100.0/m_MeanSignal;  // times 100 because we want to avoid too small values for computational reasons

  // NEW FIT
  //  m_MeanTractDensity /= numCoveredVoxels;
  //  m_MeanSignal /= numCoveredVoxels;
  //  b /= m_MeanSignal;
  //  b *= m_MeanTractDensity;
}

void FitFibersToImageFilter::GenerateData()
{
  m_NumUnknowns = m_Tractograms.size();
  if (m_FitIndividualFibers)
  {
    m_NumUnknowns = 0;
    for (unsigned int bundle=0; bundle<m_Tractograms.size(); bundle++)
      m_NumUnknowns += m_Tractograms.at(bundle)->GetNumFibers();
  }
  else
    m_FilterOutliers = false;
  if (m_NumUnknowns<1)
  {
    MITK_INFO << "No fibers in tractogram.";
    return;
  }

  fiber_count = 0;
  sz_x = 0;
  sz_y = 0;
  sz_z = 0;
  m_MeanTractDensity = 0;
  m_MeanSignal = 0;
  if (m_PeakImage.IsNotNull())
    CreatePeakSystem();
  else if (m_DiffImage.IsNotNull())
    CreateDiffSystem();
  else if (m_ScalarImage.IsNotNull())
    CreateScalarSystem();
  else
    mitkThrow() << "No input image set!";

  MITK_INFO << "Initializing optimizer";
  double init_lambda = fiber_count;  // initialization for lambda estimation

  itk::TimeProbe clock;
  clock.Start();

  cost = VnlCostFunction(m_NumUnknowns);
  cost.SetProblem(A, b, init_lambda, m_Regularization);
  cost.SetGroupSizes(m_GroupSizes);
  m_Weights.set_size(m_NumUnknowns);
  m_Weights.fill( 1.0/m_NumUnknowns );
  vnl_lbfgsb minimizer(cost);
  vnl_vector<double> l; l.set_size(m_NumUnknowns); l.fill(0);
  vnl_vector<long> bound_selection; bound_selection.set_size(m_NumUnknowns); bound_selection.fill(1);
  minimizer.set_bound_selection(bound_selection);
  minimizer.set_lower_bound(l);
  minimizer.set_projected_gradient_tolerance(m_GradientTolerance);

  if (m_Regularization==VnlCostFunction::REGU::MSM)
    MITK_INFO << "Regularization type: MSM";
  else if (m_Regularization==VnlCostFunction::REGU::VARIANCE)
    MITK_INFO << "Regularization type: VARIANCE";
  else if (m_Regularization==VnlCostFunction::REGU::LASSO)
    MITK_INFO << "Regularization type: LASSO";
  else if (m_Regularization==VnlCostFunction::REGU::VOXEL_VARIANCE)
    MITK_INFO << "Regularization type: VOXEL_VARIANCE";
  else if (m_Regularization==VnlCostFunction::REGU::GROUP_LASSO)
    MITK_INFO << "Regularization type: GROUP_LASSO";
  else if (m_Regularization==VnlCostFunction::REGU::GROUP_VARIANCE)
    MITK_INFO << "Regularization type: GROUP_VARIANCE";
  else if (m_Regularization==VnlCostFunction::REGU::NONE)
    MITK_INFO << "Regularization type: NONE";

  if (m_Regularization!=VnlCostFunction::REGU::NONE)  // REMOVE FOR NEW FIT AND SET cost.m_Lambda = m_Lambda
  {
    MITK_INFO << "Estimating regularization";
    minimizer.set_trace(false);
    minimizer.set_max_function_evals(2);
    minimizer.minimize(m_Weights);
    vnl_vector<double> dx; dx.set_size(m_NumUnknowns); dx.fill(0.0);
    cost.calc_regularization_gradient(m_Weights, dx);

    if (m_Weights.magnitude()==0)
    {
      MITK_INFO << "Regularization estimation failed. Using default value.";
      cost.m_Lambda = fiber_count*m_Lambda;
    }
    else
    {
      double r = dx.magnitude()/m_Weights.magnitude();  // wtf???
      cost.m_Lambda *= m_Lambda*55.0/r;
      MITK_INFO << r << " - " << m_Lambda*55.0/r;
      if (cost.m_Lambda>10e7)
      {
        MITK_INFO << "Regularization estimation failed. Using default value.";
        cost.m_Lambda = fiber_count*m_Lambda;
      }
    }
  }
  else
    cost.m_Lambda = 0;
  MITK_INFO << "Using regularization factor of " << cost.m_Lambda << " (λ: " << m_Lambda << ")";

  MITK_INFO << "Fitting fibers";
  minimizer.set_trace(m_Verbose);

  minimizer.set_max_function_evals(m_MaxIterations);
  minimizer.minimize(m_Weights);

  std::vector< double > weights;
  if (m_FilterOutliers)
  {
    for (auto w : m_Weights)
      weights.push_back(w);
    std::sort(weights.begin(), weights.end());
    MITK_INFO << "Setting upper weight bound to " << weights.at(m_NumUnknowns*0.99);
    vnl_vector<double> u; u.set_size(m_NumUnknowns); u.fill(weights.at(m_NumUnknowns*0.99));
    minimizer.set_upper_bound(u);
    bound_selection.fill(2);
    minimizer.set_bound_selection(bound_selection);
    minimizer.minimize(m_Weights);
    weights.clear();
  }

  for (auto w : m_Weights)
    weights.push_back(w);
  std::sort(weights.begin(), weights.end());

  m_MeanWeight = m_Weights.mean();
  m_MedianWeight = weights.at(m_NumUnknowns*0.5);
  m_MinWeight = weights.at(0);
  m_MaxWeight = weights.at(m_NumUnknowns-1);

  MITK_INFO << "*************************";
  MITK_INFO << "Weight statistics";
  MITK_INFO << "Sum: " << m_Weights.sum();
  MITK_INFO << "Mean: " << m_MeanWeight;
  MITK_INFO << "1% quantile: " << weights.at(m_NumUnknowns*0.01);
  MITK_INFO << "5% quantile: " << weights.at(m_NumUnknowns*0.05);
  MITK_INFO << "25% quantile: " << weights.at(m_NumUnknowns*0.25);
  MITK_INFO << "Median: " << m_MedianWeight;
  MITK_INFO << "75% quantile: " << weights.at(m_NumUnknowns*0.75);
  MITK_INFO << "95% quantile: " << weights.at(m_NumUnknowns*0.95);
  MITK_INFO << "99% quantile: " << weights.at(m_NumUnknowns*0.99);
  MITK_INFO << "Min: " << m_MinWeight;
  MITK_INFO << "Max: " << m_MaxWeight;
  MITK_INFO << "*************************";
  MITK_INFO << "NumEvals: " << minimizer.get_num_evaluations();
  MITK_INFO << "NumIterations: " << minimizer.get_num_iterations();
  MITK_INFO << "Residual cost: " << minimizer.get_end_error();
  m_RMSE = cost.S->get_rms_error(m_Weights);
  MITK_INFO << "Final RMSE: " << m_RMSE;

  clock.Stop();
  int h = clock.GetTotal()/3600;
  int m = ((int)clock.GetTotal()%3600)/60;
  int s = (int)clock.GetTotal()%60;
  MITK_INFO << "Optimization took " << h << "h, " << m << "m and " << s << "s";

  MITK_INFO << "Weighting fibers";
  m_RmsDiffPerBundle.set_size(m_Tractograms.size());
  std::streambuf *old = cout.rdbuf(); // <-- save
  std::stringstream ss;
  std::cout.rdbuf (ss.rdbuf());
  if (m_FitIndividualFibers)
  {
    unsigned int fiber_count = 0;
    for (unsigned int bundle=0; bundle<m_Tractograms.size(); bundle++)
    {
      vnl_vector< double > temp_weights;
      temp_weights.set_size(m_Weights.size());
      temp_weights.copy_in(m_Weights.data_block());

      for (unsigned int i=0; i<m_Tractograms.at(bundle)->GetNumFibers(); i++)
      {
        m_Tractograms.at(bundle)->SetFiberWeight(i, m_Weights[fiber_count]);
        temp_weights[fiber_count] = 0;

        ++fiber_count;
      }
      double d_rms = cost.S->get_rms_error(temp_weights) - m_RMSE;
      m_RmsDiffPerBundle[bundle] = d_rms;
    }
  }
  else
  {
    for (unsigned int i=0; i<m_Tractograms.size(); ++i)
    {
      vnl_vector< double > temp_weights;
      temp_weights.set_size(m_Weights.size());
      temp_weights.copy_in(m_Weights.data_block());
      temp_weights[i] = 0;
      double d_rms = cost.S->get_rms_error(temp_weights) - m_RMSE;
      m_RmsDiffPerBundle[i] = d_rms;

      m_Tractograms.at(i)->SetFiberWeights(m_Weights[i]);
    }
  }
  std::cout.rdbuf (old);

  // transform back
  A *= m_MeanSignal/100.0;
  b *= m_MeanSignal/100.0;

  MITK_INFO << "Generating output images ...";
  if (m_PeakImage.IsNotNull())
    GenerateOutputPeakImages();
  else if (m_DiffImage.IsNotNull())
    GenerateOutputDiffImages();
  else if (m_ScalarImage.IsNotNull())
    GenerateOutputScalarImages();

  m_Coverage = m_Coverage/m_MeanSignal;
  m_Overshoot = m_Overshoot/m_MeanSignal;

  MITK_INFO << std::fixed << "Coverage: " << setprecision(2) << 100.0*m_Coverage << "%";
  MITK_INFO << std::fixed << "Overshoot: " << setprecision(2) << 100.0*m_Overshoot << "%";
}

void FitFibersToImageFilter::GenerateOutputDiffImages()
{
  VectorImgType::PixelType pix; pix.SetSize(m_DiffImage->GetVectorLength()); pix.Fill(0);
  itk::ImageDuplicator< VectorImgType >::Pointer duplicator = itk::ImageDuplicator< VectorImgType >::New();
  duplicator->SetInputImage(m_DiffImage);
  duplicator->Update();
  m_UnderexplainedImageDiff = duplicator->GetOutput();
  m_UnderexplainedImageDiff->FillBuffer(pix);

  duplicator->SetInputImage(m_UnderexplainedImageDiff);
  duplicator->Update();
  m_OverexplainedImageDiff = duplicator->GetOutput();
  m_OverexplainedImageDiff->FillBuffer(pix);

  duplicator->SetInputImage(m_OverexplainedImageDiff);
  duplicator->Update();
  m_ResidualImageDiff = duplicator->GetOutput();
  m_ResidualImageDiff->FillBuffer(pix);

  duplicator->SetInputImage(m_ResidualImageDiff);
  duplicator->Update();
  m_FittedImageDiff = duplicator->GetOutput();
  m_FittedImageDiff->FillBuffer(pix);

  vnl_vector<double> fitted_b; fitted_b.set_size(b.size());
  cost.S->multiply(m_Weights, fitted_b);

  itk::ImageRegionIterator<VectorImgType> it1 = itk::ImageRegionIterator<VectorImgType>(m_DiffImage, m_DiffImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<VectorImgType> it2 = itk::ImageRegionIterator<VectorImgType>(m_FittedImageDiff, m_FittedImageDiff->GetLargestPossibleRegion());
  itk::ImageRegionIterator<VectorImgType> it3 = itk::ImageRegionIterator<VectorImgType>(m_ResidualImageDiff, m_ResidualImageDiff->GetLargestPossibleRegion());
  itk::ImageRegionIterator<VectorImgType> it4 = itk::ImageRegionIterator<VectorImgType>(m_UnderexplainedImageDiff, m_UnderexplainedImageDiff->GetLargestPossibleRegion());
  itk::ImageRegionIterator<VectorImgType> it5 = itk::ImageRegionIterator<VectorImgType>(m_OverexplainedImageDiff, m_OverexplainedImageDiff->GetLargestPossibleRegion());

  m_MeanSignal = 0;
  m_Coverage = 0;
  m_Overshoot = 0;

  while( !it2.IsAtEnd() )
  {
    itk::Index<3> idx3 = it2.GetIndex();
    VectorImgType::PixelType original_pix =it1.Get();
    VectorImgType::PixelType fitted_pix =it2.Get();
    VectorImgType::PixelType residual_pix =it3.Get();
    VectorImgType::PixelType underexplained_pix =it4.Get();
    VectorImgType::PixelType overexplained_pix =it5.Get();

    int num_nonzero_g = 0;
    double original_mean = 0;
    for (int g=0; g<dim_four_size; ++g)
    {
      if( m_SignalModel->GetGradientDirection(g).GetNorm()>=mitk::eps )
      {
        original_mean += original_pix[g];
        ++num_nonzero_g;
      }
    }
    original_mean /= num_nonzero_g;

    for (int g=0; g<dim_four_size; ++g)
    {
      unsigned int linear_index = idx3[0] + sz_x*idx3[1] + sz_x*sz_y*idx3[2] + sz_x*sz_y*sz_z*g;

      fitted_pix[g] = fitted_b[linear_index] + original_mean;
      residual_pix[g] = original_pix[g] - fitted_b[linear_index] - original_mean;

      if (residual_pix[g]<0)
      {
        overexplained_pix[g] = residual_pix[g];
        m_Coverage += b[linear_index] + original_mean;
        m_Overshoot -= residual_pix[g];
      }
      else if (residual_pix[g]>=0)
      {
        underexplained_pix[g] = residual_pix[g];
        m_Coverage += fitted_b[linear_index] + original_mean;
      }
      m_MeanSignal += b[linear_index] + original_mean;
    }

    it2.Set(fitted_pix);
    it3.Set(residual_pix);
    it4.Set(underexplained_pix);
    it5.Set(overexplained_pix);

    ++it1;
    ++it2;
    ++it3;
    ++it4;
    ++it5;
  }
}

void FitFibersToImageFilter::GenerateOutputScalarImages()
{
  itk::ImageDuplicator< DoubleImgType >::Pointer duplicator = itk::ImageDuplicator< DoubleImgType >::New();
  duplicator->SetInputImage(m_ScalarImage);
  duplicator->Update();
  m_UnderexplainedImageScalar = duplicator->GetOutput();
  m_UnderexplainedImageScalar->FillBuffer(0);

  duplicator->SetInputImage(m_UnderexplainedImageScalar);
  duplicator->Update();
  m_OverexplainedImageScalar = duplicator->GetOutput();
  m_OverexplainedImageScalar->FillBuffer(0);

  duplicator->SetInputImage(m_OverexplainedImageScalar);
  duplicator->Update();
  m_ResidualImageScalar = duplicator->GetOutput();
  m_ResidualImageScalar->FillBuffer(0);

  duplicator->SetInputImage(m_ResidualImageScalar);
  duplicator->Update();
  m_FittedImageScalar = duplicator->GetOutput();
  m_FittedImageScalar->FillBuffer(0);

  vnl_vector<double> fitted_b; fitted_b.set_size(b.size());
  cost.S->multiply(m_Weights, fitted_b);

  itk::ImageRegionIterator<DoubleImgType> it1 = itk::ImageRegionIterator<DoubleImgType>(m_ScalarImage, m_ScalarImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<DoubleImgType> it2 = itk::ImageRegionIterator<DoubleImgType>(m_FittedImageScalar, m_FittedImageScalar->GetLargestPossibleRegion());
  itk::ImageRegionIterator<DoubleImgType> it3 = itk::ImageRegionIterator<DoubleImgType>(m_ResidualImageScalar, m_ResidualImageScalar->GetLargestPossibleRegion());
  itk::ImageRegionIterator<DoubleImgType> it4 = itk::ImageRegionIterator<DoubleImgType>(m_UnderexplainedImageScalar, m_UnderexplainedImageScalar->GetLargestPossibleRegion());
  itk::ImageRegionIterator<DoubleImgType> it5 = itk::ImageRegionIterator<DoubleImgType>(m_OverexplainedImageScalar, m_OverexplainedImageScalar->GetLargestPossibleRegion());

  m_MeanSignal = 0;
  m_Coverage = 0;
  m_Overshoot = 0;

  while( !it2.IsAtEnd() )
  {
    itk::Index<3> idx3 = it2.GetIndex();
    DoubleImgType::PixelType original_pix =it1.Get();
    DoubleImgType::PixelType fitted_pix =it2.Get();
    DoubleImgType::PixelType residual_pix =it3.Get();
    DoubleImgType::PixelType underexplained_pix =it4.Get();
    DoubleImgType::PixelType overexplained_pix =it5.Get();

    unsigned int linear_index = idx3[0] + sz_x*idx3[1] + sz_x*sz_y*idx3[2];

    fitted_pix = fitted_b[linear_index];
    residual_pix = original_pix - fitted_pix;

    if (residual_pix<0)
    {
      overexplained_pix = residual_pix;
      m_Coverage += b[linear_index];
      m_Overshoot -= residual_pix;
    }
    else if (residual_pix>=0)
    {
      underexplained_pix = residual_pix;
      m_Coverage += fitted_b[linear_index];
    }
    m_MeanSignal += b[linear_index];

    it2.Set(fitted_pix);
    it3.Set(residual_pix);
    it4.Set(underexplained_pix);
    it5.Set(overexplained_pix);

    ++it1;
    ++it2;
    ++it3;
    ++it4;
    ++it5;
  }
}

VnlCostFunction::REGU FitFibersToImageFilter::GetRegularization() const
{
  return m_Regularization;
}

void FitFibersToImageFilter::SetRegularization(const VnlCostFunction::REGU &Regularization)
{
  m_Regularization = Regularization;
}

void FitFibersToImageFilter::GenerateOutputPeakImages()
{
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
    itk::Index<4> idx4;
    unsigned int linear_index = r;
    idx4[0] = linear_index % sz_x; linear_index /= sz_x;
    idx4[1] = linear_index % sz_y; linear_index /= sz_y;
    idx4[2] = linear_index % sz_z; linear_index /= sz_z;
    int peak_id = linear_index % dim_four_size;

    if (peak_id<dim_four_size-1)
    {
      vnl_vector_fixed<float,3> peak_dir;

      idx4[3] = peak_id*3;
      peak_dir[0] = m_PeakImage->GetPixel(idx4);
      idx4[3] += 1;
      peak_dir[1] = m_PeakImage->GetPixel(idx4);
      idx4[3] += 1;
      peak_dir[2] = m_PeakImage->GetPixel(idx4);

      peak_dir.normalize();
      peak_dir *= fitted_b[r];

      idx4[3] = peak_id*3;
      m_FittedImage->SetPixel(idx4, peak_dir[0]);

      idx4[3] += 1;
      m_FittedImage->SetPixel(idx4, peak_dir[1]);

      idx4[3] += 1;
      m_FittedImage->SetPixel(idx4, peak_dir[2]);
    }
  }

  m_MeanSignal = 0;
  m_Coverage = 0;
  m_Overshoot = 0;

  itk::Index<4> idx4;
  for (idx4[0]=0; idx4[0]<sz_x; ++idx4[0])
    for (idx4[1]=0; idx4[1]<sz_y; ++idx4[1])
      for (idx4[2]=0; idx4[2]<sz_z; ++idx4[2])
      {
        itk::Index<3> idx3; idx3[0] = idx4[0]; idx3[1] = idx4[1]; idx3[2] = idx4[2];
        if (m_MaskImage.IsNotNull() && m_MaskImage->GetPixel(idx3)==0)
          continue;

        vnl_vector_fixed<float,3> peak_dir;
        vnl_vector_fixed<float,3> fitted_dir;
        vnl_vector_fixed<float,3> overshoot_dir;
        for (idx4[3]=0; idx4[3]<(itk::IndexValueType)m_PeakImage->GetLargestPossibleRegion().GetSize(3); ++idx4[3])
        {
          peak_dir[idx4[3]%3] = m_PeakImage->GetPixel(idx4);
          fitted_dir[idx4[3]%3] = m_FittedImage->GetPixel(idx4);
          m_ResidualImage->SetPixel(idx4, m_PeakImage->GetPixel(idx4) - m_FittedImage->GetPixel(idx4));

          if (idx4[3]%3==2)
          {
            m_MeanSignal += peak_dir.magnitude();

            itk::Index<4> tidx= idx4;
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
}

void FitFibersToImageFilter::GetClosestPeak(itk::Index<4> idx, PeakImgType::Pointer peak_image , vnl_vector_fixed<float,3> fiber_dir, int& id, double& w, double& peak_mag )
{
  int m_NumDirs = peak_image->GetLargestPossibleRegion().GetSize()[3]/3;
  vnl_vector_fixed<float,3> out_dir; out_dir.fill(0);
  float angle = 0.9;

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

    float a = fabs(dot_product(dir, fiber_dir))/mag;
    if (a>angle)
    {
      angle = a;
      w = angle;
      peak_mag = mag;
      id = i;
    }
  }
}

std::vector<mitk::FiberBundle::Pointer> FitFibersToImageFilter::GetTractograms() const
{
  return m_Tractograms;
}

void FitFibersToImageFilter::SetTractograms(const std::vector<mitk::FiberBundle::Pointer> &tractograms)
{
  m_Tractograms = tractograms;
}

void FitFibersToImageFilter::SetSignalModel(mitk::DiffusionSignalModel<> *SignalModel)
{
  m_SignalModel = SignalModel;
}

}



