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

using namespace std;
typedef itksys::SystemTools ist;
typedef itk::Image<unsigned char, 3>    ItkUcharImgType;
typedef std::tuple< ItkUcharImgType::Pointer, std::string > MaskType;
typedef mitk::DiffusionPropertyHelper DPH;
typedef itk::Point<float, 3> PointType;
typedef itk::Point<float, 4> PointType4;
typedef mitk::StickModel<> ModelType;
typedef mitk::BallModel<> BallModelType;
typedef itk::Image< float, 4 >  PeakImgType;

vnl_vector_fixed<float,3> GetCLosestPeak(itk::Index<4> idx, PeakImgType::Pointer peak_image , vnl_vector_fixed<float,3> fiber_dir, bool flip_x, bool flip_y, bool flip_z )
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

    if (flip_x)
      dir[0] *= -1;
    if (flip_y)
      dir[1] *= -1;
    if (flip_z)
      dir[2] *= -1;

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
      out_dir *= 1000;
    }
  }

  return out_dir;
}

std::vector<float> SolveLinear( std::string , std::vector< mitk::FiberBundle::Pointer > inputTractogram, mitk::Image::Pointer inputImage, bool flip_x=false, bool flip_y=false, bool flip_z=false )
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
  int num_voxels = sz_x*sz_y*sz_z;

  unsigned int num_unknowns = inputTractogram.size(); //inputTractogram->GetNumFibers();
  unsigned int number_of_residuals = num_voxels * 3;

  // create linear system
  MITK_INFO << "Num. unknowns: " << num_unknowns;
  MITK_INFO << "Num. residuals: " << number_of_residuals;

  vigra::MultiArray<2, double> test_m(vigra::Shape2(number_of_residuals, 1000000), 0.0);
  MITK_INFO << test_m.height();

  MITK_INFO << "Creating matrices ...";
  vigra::MultiArray<2, double> A(vigra::Shape2(number_of_residuals, num_unknowns), 0.0);
  vigra::MultiArray<2, double> b(vigra::Shape2(number_of_residuals, 1), 0.0);
  vigra::MultiArray<2, double> x(vigra::Shape2(num_unknowns, 1), 1.0);

  MITK_INFO << "Filling matrices ...";
  for (unsigned int bundle=0; bundle<inputTractogram.size(); bundle++)
  {
    vtkSmartPointer<vtkPolyData> polydata = inputTractogram.at(bundle)->GetFiberPolyData();

    for (int i=0; i<inputTractogram.at(bundle)->GetNumFibers(); ++i)
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

        vnl_vector_fixed<float,3> odf_peak = GetCLosestPeak(idx4, itkImage, fiber_dir, flip_x, flip_y, flip_z);
        if (odf_peak.magnitude()<0.001)
          continue;

        int x = idx4[0];
        int y = idx4[1];
        int z = idx4[2];

        unsigned int linear_index = x + sz_x*y + sz_x*sz_y*z;

        for (unsigned int k=0; k<3; ++k)
        {
          b(3*linear_index + k, 0) = (double)odf_peak[k];
          A(3*linear_index + k, bundle) = A(3*linear_index + k, bundle) + (double)fiber_dir[k];
        }

      }
    }
  }

  MITK_INFO << "Solving linear system";
  vigra::linalg::nonnegativeLeastSquares(A, b, x);

  std::vector<float> weights;
  for (unsigned int i=0; i<num_unknowns; ++i)
    weights.push_back(x[i]);

  return weights;
}

std::vector<float> SolveEvo(mitk::FiberBundle* inputTractogram, mitk::Image::Pointer inputImage, ModelType signalModel, BallModelType ballModel, float start_weight, int num_iterations=1000)
{
  std::vector<float> out_weights;
  DPH::ImageType::Pointer itkImage = DPH::GetItkVectorImage(inputImage);
  itk::VectorImage< double, 3>::Pointer simulatedImage = itk::VectorImage< double, 3>::New();
  simulatedImage->SetSpacing(itkImage->GetSpacing());
  simulatedImage->SetOrigin(itkImage->GetOrigin());
  simulatedImage->SetDirection(itkImage->GetDirection());
  simulatedImage->SetRegions(itkImage->GetLargestPossibleRegion());
  simulatedImage->SetVectorLength(itkImage->GetVectorLength());
  simulatedImage->Allocate();
  DPH::ImageType::PixelType zero_signal;
  zero_signal.SetSize(itkImage->GetVectorLength());
  zero_signal.Fill(0);
  simulatedImage->FillBuffer(zero_signal);

  MITK_INFO << "start_weight: " << start_weight;
  double step = start_weight/10;

  vtkSmartPointer<vtkPolyData> polydata = inputTractogram->GetFiberPolyData();

  unsigned int* image_size = inputImage->GetDimensions();
  int sz_x = image_size[0];
  int sz_y = image_size[1];

  MITK_INFO << "INITIALIZING";
  std::vector< std::vector< ModelType::PixelType > > fiber_model_signals;
  std::vector< std::vector< DPH::ImageType::IndexType > > fiber_image_indices;
  std::map< unsigned int, std::vector<int> > image_index_to_fiber_indices;

  std::vector< int > fiber_indices;
  int f = 0;
  for (int i=0; i<inputTractogram->GetNumFibers(); ++i)
  {
    vtkCell* cell = polydata->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

//    if (numPoints<2)
//      continue;

    std::vector< ModelType::PixelType > model_signals;
    std::vector< DPH::ImageType::IndexType > image_indices;

    for (int j=0; j<numPoints-1; ++j)
    {
      double* p1 = points->GetPoint(j);
      PointType p;
      p[0]=p1[0];
      p[1]=p1[1];
      p[2]=p1[2];

      DPH::ImageType::IndexType idx;
      itkImage->TransformPhysicalPointToIndex(p, idx);
      if (!itkImage->GetLargestPossibleRegion().IsInside(idx))
        continue;

      double* p2 = points->GetPoint(j+1);
      ModelType::GradientType d;
      d[0] = p[0]-p2[0];
      d[1] = p[1]-p2[1];
      d[2] = p[2]-p2[2];
      signalModel.SetFiberDirection(d);

      ModelType::PixelType sig = signalModel.SimulateMeasurement();
      simulatedImage->SetPixel(idx, simulatedImage->GetPixel(idx) + start_weight * sig);

      model_signals.push_back(step*sig);
      image_indices.push_back(idx);
      unsigned int linear_index = idx[0] + sz_x*idx[1] + sz_x*sz_y*idx[2];

      if (image_index_to_fiber_indices.count(linear_index)==0)
      {
        image_index_to_fiber_indices[linear_index] = {i};
      }
      else
      {
        std::vector< int > index_fiber_indices = image_index_to_fiber_indices[linear_index];
        if(std::find(index_fiber_indices.begin(), index_fiber_indices.end(), i) == index_fiber_indices.end())
        {
           image_index_to_fiber_indices[linear_index].push_back(i);
        }
      }
    }

    fiber_model_signals.push_back(model_signals);
    fiber_image_indices.push_back(image_indices);
    fiber_indices.push_back(f);
    out_weights.push_back(start_weight);
    ++f;
  }

  BallModelType::PixelType ballSignal = step*ballModel.SimulateMeasurement();

  // SINGLE WEIGHT ONLY ANISO FIT


  // FILL WITH ISO

  // ACTUAL WEIGHT ADAPTATION

  double T_start = 0.01;
  double T_end = 0.001;
  double alpha = log(T_end/T_start);

  unsigned int num_gradients = signalModel.GetGradientList().size();
  for (int i=0; i<num_iterations; ++i)
  {
    double T = T_start * exp(alpha*(double)i/num_iterations);
//    MITK_INFO << "Iteration " << i;
//    MITK_INFO << "Temp " << T;
    std::random_shuffle(fiber_indices.begin(), fiber_indices.end());


    int accepted = 0;
    for (auto f : fiber_indices)
    {
      std::vector< ModelType::PixelType > model_signals = fiber_model_signals.at(f);
      std::vector< DPH::ImageType::IndexType > image_indices = fiber_image_indices.at(f);

      double E_ext_old = 0;
      double E_ext_new = 0;
      double E_int_old = 0;
      double E_int_new = 0;

      int add = std::rand()%2;
//      add = 1;
      int use_ball = std::rand()%2;
      use_ball = 0;

      if (add==0 && use_ball==0 && out_weights[f]<step)
        add = 1;

      // possible weight change
      float old_weight = out_weights[f];
      float new_weight = old_weight;
      if (add==1 && use_ball==0)
        new_weight = old_weight+step;
      else if (use_ball==0)
        new_weight = old_weight-step;

      int c = 0;
      for (auto idx : image_indices)
      {
        DPH::ImageType::PixelType measured_val = itkImage->GetPixel(idx);
        ModelType::PixelType simulated_val = simulatedImage->GetPixel(idx);
        ModelType::PixelType model_val = model_signals.at(c); // value of fiber model at currnet fiber position
        if (use_ball==1)
          model_val = ballSignal;

        // EXTERNAL ENERGY
        for (unsigned int g=0; g<num_gradients; ++g)
        {
          double delta = (double)measured_val[g] - simulated_val[g];
          MITK_INFO << fabs(delta) << " g " << g;

          E_ext_old += fabs(delta)/num_gradients; // external energy is simply the difference between sim and meas

          double delta2 = 0;
          if (add==1)
          {
            delta2 = (double)measured_val[g] - (simulated_val[g] + model_val[g]);
            E_ext_new += fabs(delta2)/num_gradients;
          }
          else
          {
            delta2 = (double)measured_val[g] - (simulated_val[g] - model_val[g]);
            E_ext_new += fabs(delta2)/num_gradients;
          }
          MITK_INFO << fabs(delta2) << " g " << g;
        }
        ++c;

        // INTERNAL ENERGY
        unsigned int linear_index = idx[0] + sz_x*idx[1] + sz_x*sz_y*idx[2];
        std::vector< int > index_fiber_indices = image_index_to_fiber_indices[linear_index];

        if (index_fiber_indices.size()>1)
        {
          float mean_weight = 0;
          for (auto neighbor_index : index_fiber_indices)
          {
            if (neighbor_index!=f)
              mean_weight += out_weights[neighbor_index];
          }
          mean_weight /= (index_fiber_indices.size()-1);

          E_int_old += fabs(mean_weight-old_weight);
          E_int_new += fabs(mean_weight-new_weight);
        }
      }

      E_ext_old /= image_indices.size();
      E_ext_new /= image_indices.size();
      E_int_old /= image_indices.size();
      E_int_new /= image_indices.size();

//      MITK_INFO << "EXT: " << E_ext_old << " --> " << E_ext_new;
//      MITK_INFO << "INT: " << E_int_old-E_int_new;

      double R = exp( (E_ext_old-E_ext_new)/T + 0*(E_int_old-E_int_new)/T );
//      MITK_INFO << R;

      float p = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

      p = E_ext_new;
      R = E_ext_old;

      MITK_INFO << add << " - " << i;
      if (p<R)
      {
//        MITK_INFO << "EXT: " << E_ext_old - E_ext_new;
        MITK_INFO << "W: " << new_weight - old_weight;
        accepted++;
        c = 0;
        for (auto idx : image_indices)
        {
          ModelType::PixelType sVal = simulatedImage->GetPixel(idx);
          ModelType::PixelType dVal = model_signals.at(c);
          if (use_ball==1)
            dVal = ballSignal;

          if (add==1)
          {
            sVal += dVal;
          }
          else
          {
            sVal -= dVal;
          }

          simulatedImage->SetPixel(idx, sVal);
          ++c;
        }

        out_weights[f] = new_weight;
      }

    }
//    MITK_INFO << "Accepted: " << (float)accepted/fiber_indices.size();
  }

  typedef  itk::ImageFileWriter< itk::VectorImage< double, 3> > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName("/home/neher/Projects/TractPlausibility/model_signal.nrrd");
  writer->SetInput(simulatedImage);
  writer->Update();

  return out_weights;
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
  parser.addArgument("input_tractograms", "i1", mitkCommandLineParser::StringList, "Input tractograms:", "input tractograms (.fib, vtk ascii file format)", us::Any(), false);
  parser.addArgument("input_peaks", "i2", mitkCommandLineParser::InputFile, "Input peaks:", "input peak image", us::Any(), false);
  parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output root", us::Any(), false);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  mitkCommandLineParser::StringContainerType fib_files = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["input_tractograms"]);
  string dwiFile = us::any_cast<string>(parsedArgs["input_peaks"]);
  string outRoot = us::any_cast<string>(parsedArgs["out"]);

  try
  {
    std::vector< mitk::FiberBundle::Pointer > bundles;

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images", "Fiberbundles"}, {});
    mitk::Image::Pointer inputImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(dwiFile, &functor)[0].GetPointer());

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
          bundles.push_back(inputTractogram);
          fib_names.push_back(fibFile);
        }
      }
      else
      {
        mitk::FiberBundle::Pointer inputTractogram = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(item)[0].GetPointer());
        if (inputTractogram.IsNull())
          continue;
        inputTractogram->ResampleLinear(minSpacing/4);
        bundles.push_back(inputTractogram);
        fib_names.push_back(item);
      }
    }

    std::vector<float> weights = SolveLinear(outRoot, bundles, inputImage);

    for (unsigned int i=0; i<fib_names.size(); ++i)
    {
      std::string name = fib_names.at(i);
      name = ist::GetFilenameWithoutExtension(name);
      MITK_INFO << name << ": " << weights.at(i);
      mitk::FiberBundle::Pointer bundle = bundles.at(i);
//      inputTractogram->SetFiberWeight(i, weights.at(i));
      bundle->SetFiberWeights(weights.at(i));
      mitk::IOUtil::Save(bundle, outRoot + name + "_fitted.fib");
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
