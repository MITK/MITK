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

using namespace std;
typedef itksys::SystemTools ist;
typedef itk::Point<float, 4> PointType4;
typedef itk::Image< float, 4 >  PeakImgType;
const float UPSCALE = 1000.0;

vnl_vector_fixed<float,3> GetClosestPeak(itk::Index<4> idx, PeakImgType::Pointer peak_image , vnl_vector_fixed<float,3> fiber_dir, int& id )
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
      if (a<0)
        out_dir = -dir;
      else
        out_dir = dir;
      out_dir *= mag;
      out_dir *= UPSCALE;  // for the fit it's better if the magnitude is larger since many fibers pass each voxel and otherwise the individual contributions would be very small
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
  vnl_vector< double > m_b;

  void SetProblem(vnl_sparse_matrix< double >& A, vnl_vector<double>& b)
  {
    S = new vnl_sparse_matrix_linear_system<double>(A, b);
    m_A = A;
    m_b = b;
  }

  VnlCostFunction(const int NumVars) : vnl_cost_function(NumVars)
  {
  }

  double f(vnl_vector<double> const &x)
  {
    double cost = S->get_rms_error(x);

    double regu = x.squared_magnitude()/x.size();

//    unsigned int norm = 0;
//    for (unsigned int i=0; i<m_b.size(); ++i)
//    {
//      if (m_A.get_row(i).empty())
//        continue;

//      float mean = 0;
//      for (auto el : m_A.get_row(i))
//        mean += x[el.first];

//      mean /= m_A.get_row(i).size();

//      for (auto el : m_A.get_row(i))
//      {
//       float d = x[el.first] - mean;
//       {
//       regu += d*d;
//       norm++;
//       }
//      }
//    }
//    regu /= norm;

    cost += regu;
    return cost;
  }

  void gradf(vnl_vector<double> const &x, vnl_vector<double> &dx)
  {
    fdgradf(x, dx);
  }
};

std::vector<float> FitFibers( std::string , std::vector< mitk::FiberBundle::Pointer > input_tracts, mitk::Image::Pointer inputImage, bool single_fiber_fit, int max_iter, float g_tol, bool lb )
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
  int sz_peaks = image_size[3];
  int num_voxels = sz_x*sz_y*sz_z;

  unsigned int num_unknowns = input_tracts.size(); //inputTractogram->GetNumFibers();
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
  vnl_sparse_matrix< double > A_vnl; A_vnl.set_size(number_of_residuals, num_unknowns);
  vnl_vector<double> b_vnl; b_vnl.set_size(number_of_residuals); b_vnl.fill(0.0);

  float max_peak_mag = 0;
  int max_peak_idx = -1;

  float min_peak_mag = 999999999;
  int min_peak_idx = -1;

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

        int peak_id = -1;
        vnl_vector_fixed<float,3> odf_peak = GetClosestPeak(idx4, itkImage, fiber_dir, peak_id);
        float peak_mag = odf_peak.magnitude();
        if (peak_id<0)
          continue;

        int x = idx4[0];
        int y = idx4[1];
        int z = idx4[2];

        unsigned int linear_index = sz_peaks*(x + sz_x*y + sz_x*sz_y*z);

        if (peak_mag>max_peak_mag)
        {
          max_peak_mag = peak_mag;
          max_peak_idx = linear_index + 3*peak_id;
        }

        if (peak_mag<min_peak_mag)
        {
          min_peak_mag = peak_mag;
          min_peak_idx = linear_index + 3*peak_id;
        }

        for (unsigned int k=0; k<3; ++k)
        {
          if (single_fiber_fit)
          {
            b_vnl[linear_index + 3*peak_id + k] = (double)odf_peak[k];
            A_vnl.put(linear_index + 3*peak_id + k, fiber_count, A_vnl.get(linear_index + 3*peak_id + k, fiber_count) + (double)fiber_dir[k]);
          }
          else
          {
            b_vnl[linear_index + 3*peak_id + k] = (double)odf_peak[k];
            A_vnl.put(linear_index + 3*peak_id + k, bundle, A_vnl.get(linear_index + 3*peak_id + k, bundle) + (double)fiber_dir[k]);
          }
        }

      }

      ++fiber_count;
    }
  }

  vnl_vector_fixed<float,3> max_corr_fiber_dir; max_corr_fiber_dir.fill(0.0);
  vnl_vector_fixed<float,3> min_corr_fiber_dir; min_corr_fiber_dir.fill(0.0);
  for (unsigned int i=0; i<fiber_count; ++i)
  {
    max_corr_fiber_dir[0] += A_vnl.get(max_peak_idx, i);
    max_corr_fiber_dir[1] += A_vnl.get(max_peak_idx+1, i);
    max_corr_fiber_dir[2] += A_vnl.get(max_peak_idx+2, i);

    min_corr_fiber_dir[0] += A_vnl.get(min_peak_idx, i);
    min_corr_fiber_dir[1] += A_vnl.get(min_peak_idx+1, i);
    min_corr_fiber_dir[2] += A_vnl.get(min_peak_idx+2, i);
  }

  float upper_bound = max_peak_mag/max_corr_fiber_dir.magnitude();
  float lower_bound = min_peak_mag/min_corr_fiber_dir.magnitude();

  if (!lb || lower_bound>=upper_bound)
    lower_bound = 0;

  MITK_INFO << "Lower bound: " << lower_bound;
  MITK_INFO << "Upper bound: " << upper_bound;

  MITK_INFO << "Fitting fibers";
  VnlCostFunction cost(num_unknowns);
  cost.SetProblem(A_vnl, b_vnl);
  vnl_lbfgsb minimizer(cost);

  vnl_vector<double> l; l.set_size(num_unknowns); l.fill(lower_bound);
  vnl_vector<double> u; u.set_size(num_unknowns); u.fill(upper_bound);
  vnl_vector<long> bound_selection; bound_selection.set_size(num_unknowns); bound_selection.fill(2);
  vnl_vector<double> x_vnl; x_vnl.set_size(num_unknowns); x_vnl.fill( (upper_bound-lower_bound)/2 );

  minimizer.set_bound_selection(bound_selection);
  minimizer.set_lower_bound(l);
  minimizer.set_upper_bound(u);
  minimizer.set_trace(true);
  minimizer.set_projected_gradient_tolerance(g_tol);
  if (max_iter>0)
    minimizer.set_max_function_evals(max_iter);

  itk::TimeProbe clock;
  clock.Start();
  minimizer.minimize(x_vnl);
  clock.Stop();
  int h = clock.GetTotal()/3600;
  int m = ((int)clock.GetTotal()%3600)/60;
  int s = (int)clock.GetTotal()%60;
  MITK_INFO << "Optimization took " << h << "h, " << m << "m and " << s << "s";

  MITK_INFO << "Residual error: " << minimizer.get_end_error();
  MITK_INFO << "NumEvals: " << minimizer.get_num_evaluations();
  MITK_INFO << "NumIterations: " << minimizer.get_num_iterations();

  std::vector<float> weights;
//  float max_w = 0;
  for (unsigned int i=0; i<num_unknowns; ++i)
  {
//    MITK_INFO << x_vnl[i];
//    if (x_vnl[i]>max_w)
//      max_w = x_vnl[i];
    weights.push_back(x_vnl[i]);
  }
//  MITK_INFO << "Max w: " << max_w;

  return weights;
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
  parser.addArgument("", "it", mitkCommandLineParser::Int, "", "");
  parser.addArgument("", "s", mitkCommandLineParser::Bool, "", "");
  parser.addArgument("", "lb", mitkCommandLineParser::Bool, "", "");
  parser.addArgument("", "g", mitkCommandLineParser::Float, "", "");
  parser.addArgument("", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output root", us::Any(), false);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  mitkCommandLineParser::StringContainerType fib_files = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["i1"]);
  string dwiFile = us::any_cast<string>(parsedArgs["i2"]);
  string outRoot = us::any_cast<string>(parsedArgs["o"]);

  bool single_fib = false;
  if (parsedArgs.count("s"))
    single_fib = us::any_cast<bool>(parsedArgs["s"]);

  int max_iter = 0;
  if (parsedArgs.count("it"))
    max_iter = us::any_cast<int>(parsedArgs["it"]);

  float g_tol = 0.1;
  if (parsedArgs.count("g"))
    g_tol = us::any_cast<float>(parsedArgs["g"]);

  bool lb = false;
  if (parsedArgs.count("lb"))
    lb = us::any_cast<bool>(parsedArgs["lb"]);

  try
  {
    std::vector< mitk::FiberBundle::Pointer > input_tracts;

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Peak Image", "Fiberbundles"}, {});
    mitk::Image::Pointer inputImage = dynamic_cast<mitk::PeakImage*>(mitk::IOUtil::Load(dwiFile, &functor)[0].GetPointer());

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
          inputTractogram->ResampleLinear(minSpacing/4);
          input_tracts.push_back(inputTractogram);
          fib_names.push_back(fibFile);
        }
      }
      else
      {
        mitk::FiberBundle::Pointer inputTractogram = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(item)[0].GetPointer());
        if (inputTractogram.IsNull())
          continue;
        inputTractogram->ResampleLinear(minSpacing/4);
        input_tracts.push_back(inputTractogram);
        fib_names.push_back(item);
      }
    }

    std::vector<float> weights = FitFibers(outRoot, input_tracts, inputImage, single_fib, max_iter, g_tol, lb);

    if (single_fib)
    {
      unsigned int fiber_count = 0;

      for (unsigned int bundle=0; bundle<input_tracts.size(); bundle++)
      {
        mitk::FiberBundle::Pointer fib = input_tracts.at(bundle);
        for (int i=0; i<fib->GetNumFibers(); i++)
        {
          fib->SetFiberWeight(i, weights.at(fiber_count));
          ++fiber_count;
        }

        std::string name = fib_names.at(bundle);
        name = ist::GetFilenameWithoutExtension(name);
        mitk::IOUtil::Save(fib, outRoot + name + "_fitted.fib");
      }
    }
    else
    {
      for (unsigned int i=0; i<fib_names.size(); ++i)
      {
        std::string name = fib_names.at(i);
        name = ist::GetFilenameWithoutExtension(name);
        MITK_INFO << name << ": " << weights.at(i);
        mitk::FiberBundle::Pointer bundle = input_tracts.at(i);
        bundle->SetFiberWeights(weights.at(i));
        mitk::IOUtil::Save(bundle, outRoot + name + "_fitted.fib");
      }
    }


    // OUTPUT IMAGES
    MITK_INFO << "Generating output images ...";
    typedef mitk::ImageToItk< PeakImgType > CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(inputImage);
    caster->Update();
    PeakImgType::Pointer itkImage = caster->GetOutput();

    itk::ImageDuplicator< PeakImgType >::Pointer duplicator = itk::ImageDuplicator< PeakImgType >::New();
    duplicator->SetInputImage(itkImage);
    duplicator->Update();
    PeakImgType::Pointer unexplained_image = duplicator->GetOutput();

    duplicator->SetInputImage(unexplained_image);
    duplicator->Update();
    PeakImgType::Pointer residual_image = duplicator->GetOutput();

    duplicator->SetInputImage(residual_image);
    duplicator->Update();
    PeakImgType::Pointer explained_image = duplicator->GetOutput();
    explained_image->FillBuffer(0.0);

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

        float w = input_tracts.at(bundle)->GetFiberWeight(i)/UPSCALE;

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

          int peak_id = -1;
          GetClosestPeak(idx4, itkImage, fiber_dir, peak_id);
          if (peak_id<0)
            continue;

          vnl_vector_fixed<float,3> unexplained_dir;
          vnl_vector_fixed<float,3> explained_dir;
          vnl_vector_fixed<float,3> res_dir;
          vnl_vector_fixed<float,3> peak_dir;

          idx4[3] = peak_id*3;
          unexplained_dir[0] = unexplained_image->GetPixel(idx4);
          explained_dir[0] = explained_image->GetPixel(idx4);
          peak_dir[0] = itkImage->GetPixel(idx4);
          res_dir[0] = residual_image->GetPixel(idx4);

          idx4[3] += 1;
          unexplained_dir[1] = unexplained_image->GetPixel(idx4);
          explained_dir[1] = explained_image->GetPixel(idx4);
          peak_dir[1] = itkImage->GetPixel(idx4);
          res_dir[1] = residual_image->GetPixel(idx4);

          idx4[3] += 1;
          unexplained_dir[2] = unexplained_image->GetPixel(idx4);
          explained_dir[2] = explained_image->GetPixel(idx4);
          peak_dir[2] = itkImage->GetPixel(idx4);
          res_dir[2] = residual_image->GetPixel(idx4);

          if (dot_product(peak_dir, fiber_dir)<0)
            fiber_dir *= -1;
          fiber_dir *= w;

          idx4[3] = peak_id*3;
          residual_image->SetPixel(idx4, res_dir[0] - fiber_dir[0]);

          idx4[3] += 1;
          residual_image->SetPixel(idx4, res_dir[1] - fiber_dir[1]);

          idx4[3] += 1;
          residual_image->SetPixel(idx4, res_dir[2] - fiber_dir[2]);


          if ( fabs(unexplained_dir[0]) - fabs(fiber_dir[0]) < 0 )  // did we "overexplain" stuff?
            fiber_dir = unexplained_dir;

          idx4[3] = peak_id*3;
          unexplained_image->SetPixel(idx4, unexplained_dir[0] - fiber_dir[0]);
          explained_image->SetPixel(idx4, explained_dir[0] + fiber_dir[0]);

          idx4[3] += 1;
          unexplained_image->SetPixel(idx4, unexplained_dir[1] - fiber_dir[1]);
          explained_image->SetPixel(idx4, explained_dir[1] + fiber_dir[1]);

          idx4[3] += 1;
          unexplained_image->SetPixel(idx4, unexplained_dir[2] - fiber_dir[2]);
          explained_image->SetPixel(idx4, explained_dir[2] + fiber_dir[2]);
        }

      }
    }

    itk::ImageFileWriter< PeakImgType >::Pointer writer = itk::ImageFileWriter< PeakImgType >::New();
    writer->SetInput(unexplained_image);
    writer->SetFileName(outRoot + "unexplained_image.nrrd");
    writer->Update();

    writer->SetInput(explained_image);
    writer->SetFileName(outRoot + "explained_image.nrrd");
    writer->Update();

    writer->SetInput(residual_image);
    writer->SetFileName(outRoot + "residual_image.nrrd");
    writer->Update();
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
