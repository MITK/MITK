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

#include <mitkBaseData.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <metaCommand.h>
#include <mitkCommandLineParser.h>
#include <usAny.h>
#include <mitkIOUtil.h>
#include <boost/lexical_cast.hpp>
#include <itksys/SystemTools.hxx>
#include <itkDirectory.h>
#include <mitkFiberBundle.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkDiffusionPropertyHelper.h>
#include <vnl/vnl_linear_system.h>
#include <Eigen/Dense>
#include <mitkStickModel.h>
#include <mitkBallModel.h>
#include <vigra/regression.hxx>
#include <itkImageFileWriter.h>
#include <itkImageDuplicator.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>
#include <mitkPeakImage.h>
#include <vnl/algo/vnl_lbfgsb.h>
#include <vnl/vnl_sparse_matrix.h>
#include <vnl/vnl_sparse_matrix_linear_system.h>
#include <vnl/algo/vnl_lsqr.h>
#include <itkImageDuplicator.h>
#include <itkTimeProbe.h>
#include <random>
#include <itkParticleSwarmOptimizer.h>
#include <itkOnePlusOneEvolutionaryOptimizer.h>
#include <itkGradientDescentOptimizer.h>
#include <itkSPSAOptimizer.h>

using namespace std;
typedef itksys::SystemTools ist;
typedef itk::Point<float, 4> PointType4;
typedef itk::Image< float, 4 >  PeakImgType;

vnl_vector_fixed<float,3> GetClosestPeak(itk::Index<4> idx, PeakImgType::Pointer peak_image , vnl_vector_fixed<float,3> fiber_dir, int& id, double& w )
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

vnl_vector<double> FitFibers( std::string , std::vector< mitk::FiberBundle::Pointer > input_tracts, mitk::Image::Pointer inputImage, vnl_sparse_matrix< double >& A, vnl_vector<double>& b, bool single_fiber_fit, int max_iter, float g_tol, float lambda )
{
  typedef mitk::ImageToItk< PeakImgType > CasterType;
  CasterType::Pointer caster = CasterType::New();
  caster->SetInput(inputImage);
  caster->Update();
  PeakImgType::Pointer itkImage = caster->GetOutput();

  unsigned int* image_size = inputImage->GetDimensions();
  int sz_x = image_size[0];
  int sz_y = image_size[1];
  int sz_z = image_size[2];
  int sz_peaks = image_size[3]/3 + 1; // +1 for zero - peak
  int num_voxels = sz_x*sz_y*sz_z;

  unsigned int num_unknowns = input_tracts.size();
  if (single_fiber_fit)
  {
    num_unknowns = 0;
    for (unsigned int bundle=0; bundle<input_tracts.size(); bundle++)
      num_unknowns += input_tracts.at(bundle)->GetNumFibers();
  }

  unsigned int number_of_residuals = num_voxels * sz_peaks;

  // create linear system
  MITK_INFO << "Num. unknowns: " << num_unknowns;
  MITK_INFO << "Num. residuals: " << number_of_residuals;

  MITK_INFO << "Creating system ...";
  A.set_size(number_of_residuals, num_unknowns);
  b.set_size(number_of_residuals); b.fill(0.0);

  double TD = 0;
  double FD = 0;
  unsigned int dir_count = 0;
  unsigned int fiber_count = 0;

  for (unsigned int bundle=0; bundle<input_tracts.size(); bundle++)
  {
    vtkSmartPointer<vtkPolyData> polydata = input_tracts.at(bundle)->GetFiberPolyData();

    for (int i=0; i<input_tracts.at(bundle)->GetNumFibers(); ++i)
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
        itkImage->TransformPhysicalPointToIndex(p, idx4);
        if (!itkImage->GetLargestPossibleRegion().IsInside(idx4))
          continue;

        double* p2 = points->GetPoint(j+1);
        vnl_vector_fixed<float,3> fiber_dir;
        fiber_dir[0] = p[0]-p2[0];
        fiber_dir[1] = p[1]-p2[1];
        fiber_dir[2] = p[2]-p2[2];
        fiber_dir.normalize();

        double w = 1;
        int peak_id = sz_peaks-1;
        vnl_vector_fixed<float,3> odf_peak = GetClosestPeak(idx4, itkImage, fiber_dir, peak_id, w);
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

        if (single_fiber_fit)
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
  vnl_vector<double> x; x.set_size(num_unknowns); x.fill( TD/100.0 * FD/2.0 );
  vnl_lbfgsb minimizer(cost);
  vnl_vector<double> l; l.set_size(num_unknowns); l.fill(0);
  vnl_vector<long> bound_selection; bound_selection.set_size(num_unknowns); bound_selection.fill(1);
  minimizer.set_bound_selection(bound_selection);
  minimizer.set_lower_bound(l);
  minimizer.set_projected_gradient_tolerance(g_tol);

  MITK_INFO << "Estimating regularization";
  minimizer.set_trace(false);
  minimizer.set_max_function_evals(1);
  minimizer.minimize(x);
  vnl_vector<double> dx; dx.set_size(num_unknowns); dx.fill(0.0);
  cost.grad_regu_localMSE(x, dx);
  double r = dx.magnitude()/x.magnitude();
  cost.m_Lambda *= lambda*55.0/r;
  MITK_INFO << "Using regularization factor of " << cost.m_Lambda << " (λ: " << lambda << ")";

  MITK_INFO << "Fitting fibers";
  minimizer.set_trace(true);
  minimizer.set_max_function_evals(max_iter);
  minimizer.minimize(x);

  // SECOND RUN
  std::vector< double > weights;
  for (auto w : x)
    weights.push_back(w);
  sort(weights.begin(), weights.end());
  MITK_INFO << "Setting upper weight bound to " << weights.at(num_unknowns*0.95);
  vnl_vector<double> u; u.set_size(num_unknowns); u.fill(weights.at(num_unknowns*0.95));
  minimizer.set_upper_bound(u);
  bound_selection.fill(2);
  minimizer.set_bound_selection(bound_selection);
  minimizer.minimize(x);

  weights.clear();
  for (auto w : x)
    weights.push_back(w);
  sort(weights.begin(), weights.end());

  MITK_INFO << "*************************";
  MITK_INFO << "Weight statistics";
  MITK_INFO << "Mean: " << x.mean();
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
  MITK_INFO << "Final RMS: " << cost.S->get_rms_error(x);

  clock.Stop();
  int h = clock.GetTotal()/3600;
  int m = ((int)clock.GetTotal()%3600)/60;
  int s = (int)clock.GetTotal()%60;
  MITK_INFO << "Optimization took " << h << "h, " << m << "m and " << s << "s";

  // transform back for peak image creation
  A *= FD/100.0;
  b *= FD/100.0;

  return x;
}

std::vector< string > get_file_list(const std::string& path)
{
  std::vector< string > file_list;
  itk::Directory::Pointer dir = itk::Directory::New();

  if (dir->Load(path.c_str()))
  {
    int n = dir->GetNumberOfFiles();
    for (int r = 0; r < n; r++)
    {
      const char *filename = dir->GetFile(r);
      std::string ext = ist::GetFilenameExtension(filename);
      if (ext==".fib" || ext==".trk")
        file_list.push_back(path + '/' + filename);
    }
  }
  return file_list;
}

/*!
\brief Fits the tractogram to the input peak image by assigning a weight to each fiber (similar to https://doi.org/10.1016/j.neuroimage.2015.06.092).
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Fit Fibers To Image");
  parser.setCategory("Fiber Tracking Evaluation");
  parser.setDescription("");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i1", mitkCommandLineParser::StringList, "Input tractograms:", "input tractograms (.fib, vtk ascii file format)", us::Any(), false);
  parser.addArgument("", "i2", mitkCommandLineParser::InputFile, "Input peaks:", "input peak image", us::Any(), false);
  parser.addArgument("", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output root", us::Any(), false);

  parser.addArgument("max_iter", "", mitkCommandLineParser::Int, "Max. iterations:", "maximum number of optimizer iterations", 20);
  parser.addArgument("bundle_based", "", mitkCommandLineParser::Bool, "Bundle based fit:", "fit one weight per input tractogram/bundle, not for each fiber", false);
  parser.addArgument("min_g", "", mitkCommandLineParser::Float, "Min. g:", "lower termination threshold for gradient magnitude", 1e-5);
  parser.addArgument("lambda", "", mitkCommandLineParser::Float, "Lambda:", "modifier for regularization", 0.1);
  parser.addArgument("save_res", "", mitkCommandLineParser::Bool, "Residuals:", "save residual images", false);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  mitkCommandLineParser::StringContainerType fib_files = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["i1"]);
  string peak_file_name = us::any_cast<string>(parsedArgs["i2"]);
  string outRoot = us::any_cast<string>(parsedArgs["o"]);

  bool single_fib = true;
  if (parsedArgs.count("bundle_based"))
    single_fib = !us::any_cast<bool>(parsedArgs["bundle_based"]);

  bool save_residuals = false;
  if (parsedArgs.count("save_res"))
    save_residuals = us::any_cast<bool>(parsedArgs["save_res"]);

  int max_iter = 20;
  if (parsedArgs.count("max_iter"))
    max_iter = us::any_cast<int>(parsedArgs["max_iter"]);

  float g_tol = 1e-5;
  if (parsedArgs.count("min_g"))
    g_tol = us::any_cast<float>(parsedArgs["min_g"]);

  float lambda = 0.1;
  if (parsedArgs.count("lambda"))
    lambda = us::any_cast<float>(parsedArgs["lambda"]);

  try
  {
    std::vector< mitk::FiberBundle::Pointer > input_tracts;

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Peak Image", "Fiberbundles"}, {});
    mitk::Image::Pointer inputImage = dynamic_cast<mitk::PeakImage*>(mitk::IOUtil::Load(peak_file_name, &functor)[0].GetPointer());

    float minSpacing = 1;
    if(inputImage->GetGeometry()->GetSpacing()[0]<inputImage->GetGeometry()->GetSpacing()[1] && inputImage->GetGeometry()->GetSpacing()[0]<inputImage->GetGeometry()->GetSpacing()[2])
      minSpacing = inputImage->GetGeometry()->GetSpacing()[0];
    else if (inputImage->GetGeometry()->GetSpacing()[1] < inputImage->GetGeometry()->GetSpacing()[2])
      minSpacing = inputImage->GetGeometry()->GetSpacing()[1];
    else
      minSpacing = inputImage->GetGeometry()->GetSpacing()[2];

    std::vector< std::string > fib_names;
    for (auto item : fib_files)
    {
      if ( ist::FileIsDirectory(item) )
      {
        for ( auto fibFile : get_file_list(item) )
        {
          mitk::FiberBundle::Pointer inputTractogram = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(fibFile)[0].GetPointer());
          if (inputTractogram.IsNull())
            continue;
          inputTractogram->ResampleLinear(minSpacing/10);
          input_tracts.push_back(inputTractogram);
          fib_names.push_back(fibFile);
        }
      }
      else
      {
        mitk::FiberBundle::Pointer inputTractogram = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(item)[0].GetPointer());
        if (inputTractogram.IsNull())
          continue;
        inputTractogram->ResampleLinear(minSpacing/10);
        input_tracts.push_back(inputTractogram);
        fib_names.push_back(item);
      }
    }

    vnl_sparse_matrix<double> A;
    vnl_vector<double> b;
    vnl_vector<double> x = FitFibers(outRoot, input_tracts, inputImage, A, b, single_fib, max_iter, g_tol, lambda);

    MITK_INFO << "Weighting fibers";
    if (single_fib)
    {
      unsigned int fiber_count = 0;
      for (unsigned int bundle=0; bundle<input_tracts.size(); bundle++)
      {
        for (int i=0; i<input_tracts.at(bundle)->GetNumFibers(); i++)
        {
          input_tracts.at(bundle)->SetFiberWeight(i, x[fiber_count]);
          ++fiber_count;
        }
      }
    }
    else
    {
      for (unsigned int i=0; i<fib_names.size(); ++i)
        input_tracts.at(i)->SetFiberWeights(x[i]);
    }


    // OUTPUT IMAGES
    MITK_INFO << "Generating output images ...";
    typedef mitk::ImageToItk< PeakImgType > CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(inputImage);
    caster->Update();
    PeakImgType::Pointer peak_image = caster->GetOutput();

    itk::ImageDuplicator< PeakImgType >::Pointer duplicator = itk::ImageDuplicator< PeakImgType >::New();
    duplicator->SetInputImage(peak_image);
    duplicator->Update();
    PeakImgType::Pointer underexplained_image = duplicator->GetOutput();
    underexplained_image->FillBuffer(0.0);

    duplicator->SetInputImage(underexplained_image);
    duplicator->Update();
    PeakImgType::Pointer overexplained_image = duplicator->GetOutput();
    overexplained_image->FillBuffer(0.0);

    duplicator->SetInputImage(overexplained_image);
    duplicator->Update();
    PeakImgType::Pointer residual_image = duplicator->GetOutput();
    residual_image->FillBuffer(0.0);

    duplicator->SetInputImage(residual_image);
    duplicator->Update();
    PeakImgType::Pointer fitted_image = duplicator->GetOutput();
    fitted_image->FillBuffer(0.0);

    vnl_sparse_matrix_linear_system<double> S(A, b);
    vnl_vector<double> fitted_b; fitted_b.set_size(b.size());
    S.multiply(x, fitted_b);

    unsigned int* image_size = inputImage->GetDimensions();
    int sz_x = image_size[0];
    int sz_y = image_size[1];
    int sz_z = image_size[2];
    int sz_peaks = image_size[3]/3 + 1; // +1 for zero - peak
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
        peak_dir[0] = peak_image->GetPixel(idx);
        idx[3] += 1;
        peak_dir[1] = peak_image->GetPixel(idx);
        idx[3] += 1;
        peak_dir[2] = peak_image->GetPixel(idx);

        peak_dir.normalize();
        peak_dir *= fitted_b[r];

        idx[3] = peak_id*3;
        fitted_image->SetPixel(idx, peak_dir[0]);

        idx[3] += 1;
        fitted_image->SetPixel(idx, peak_dir[1]);

        idx[3] += 1;
        fitted_image->SetPixel(idx, peak_dir[2]);
      }
    }

    double FD = 0;
    double coverage = 0;
    double overshoot = 0;

    itk::Index<4> idx;
    for (idx[0]=0; idx[0]<sz_x; ++idx[0])
      for (idx[1]=0; idx[1]<sz_y; ++idx[1])
        for (idx[2]=0; idx[2]<sz_z; ++idx[2])
        {
          vnl_vector_fixed<float,3> peak_dir;
          vnl_vector_fixed<float,3> fitted_dir;
          vnl_vector_fixed<float,3> overshoot_dir;
          for (idx[3]=0; idx[3]<image_size[3]; ++idx[3])
          {
            peak_dir[idx[3]%3] = peak_image->GetPixel(idx);
            fitted_dir[idx[3]%3] = fitted_image->GetPixel(idx);
            residual_image->SetPixel(idx, peak_image->GetPixel(idx) - fitted_image->GetPixel(idx));

            if (idx[3]%3==2)
            {
              FD += peak_dir.magnitude();

              itk::Index<4> tidx= idx;
              if (peak_dir.magnitude()>fitted_dir.magnitude())
              {
                coverage += fitted_dir.magnitude();
                underexplained_image->SetPixel(tidx, peak_dir[2]-fitted_dir[2]); tidx[3]--;
                underexplained_image->SetPixel(tidx, peak_dir[1]-fitted_dir[1]); tidx[3]--;
                underexplained_image->SetPixel(tidx, peak_dir[0]-fitted_dir[0]);
              }
              else
              {
                overshoot_dir[0] = fitted_dir[0]-peak_dir[0];
                overshoot_dir[1] = fitted_dir[1]-peak_dir[1];
                overshoot_dir[2] = fitted_dir[2]-peak_dir[2];
                coverage += peak_dir.magnitude();
                overshoot += overshoot_dir.magnitude();
                overexplained_image->SetPixel(tidx, overshoot_dir[2]); tidx[3]--;
                overexplained_image->SetPixel(tidx, overshoot_dir[1]); tidx[3]--;
                overexplained_image->SetPixel(tidx, overshoot_dir[0]);
              }
            }
          }
        }

    MITK_INFO << std::fixed << "Coverage: " << setprecision(1) << 100.0*coverage/FD << "%";
    MITK_INFO << std::fixed << "Overshoot: " << setprecision(1) << 100.0*overshoot/FD << "%";

    if (save_residuals)
    {
      itk::ImageFileWriter< PeakImgType >::Pointer writer = itk::ImageFileWriter< PeakImgType >::New();
      writer->SetInput(fitted_image);
      writer->SetFileName(outRoot + "fitted_image.nrrd");
      writer->Update();

      writer->SetInput(residual_image);
      writer->SetFileName(outRoot + "residual_image.nrrd");
      writer->Update();

      writer->SetInput(overexplained_image);
      writer->SetFileName(outRoot + "overexplained_image.nrrd");
      writer->Update();

      writer->SetInput(underexplained_image);
      writer->SetFileName(outRoot + "underexplained_image.nrrd");
      writer->Update();
    }

    for (unsigned int bundle=0; bundle<input_tracts.size(); bundle++)
    {
      input_tracts.at(bundle)->Compress(0.1);
      std::string name = fib_names.at(bundle);
      name = ist::GetFilenameWithoutExtension(name);
      mitk::IOUtil::Save(input_tracts.at(bundle), outRoot + name + "_fitted.fib");
    }
  }
  catch (itk::ExceptionObject e)
  {
    std::cout << e;
    return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
    std::cout << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cout << "ERROR!?!";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
