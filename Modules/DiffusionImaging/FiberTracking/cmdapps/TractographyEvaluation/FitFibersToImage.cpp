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
typedef mitk::StickModel<> ModelType;
typedef mitk::BallModel<> BallModelType;

std::vector<float> SolveLinear(mitk::FiberBundle* inputTractogram, mitk::Image::Pointer inputImage, ModelType signalModel, BallModelType )
{
  unsigned int num_gradients = signalModel.GetGradientList().size();
  DPH::ImageType::Pointer itkImage = DPH::GetItkVectorImage(inputImage);

  unsigned int* image_size = inputImage->GetDimensions();
  int sz_x = image_size[0];
  int sz_y = image_size[1];
  int sz_z = image_size[2];
  int num_voxels = sz_x*sz_y*sz_z;

  unsigned int num_unknowns = inputTractogram->GetNumFibers();
  unsigned int number_of_residuals = num_voxels * num_gradients;


  // create linear system
  MITK_INFO << "Num. unknowns: " << num_unknowns;
  MITK_INFO << "Num. residuals: " << number_of_residuals;

  MITK_INFO << "Creating matrices ...";
  //  Eigen::MatrixXf A = Eigen::MatrixXf::Constant(number_of_residuals, num_unknowns, 0);
  //  Eigen::VectorXf b = Eigen::VectorXf::Constant(number_of_residuals, 0);

  vigra::MultiArray<2, double> A(vigra::Shape2(number_of_residuals, num_unknowns), 0.0);
  vigra::MultiArray<2, double> b(vigra::Shape2(number_of_residuals, 1), 0.0);
  vigra::MultiArray<2, double> x(vigra::Shape2(num_unknowns, 1), 1.0);


  itk::Image< double, 3>::Pointer temp_img1 = itk::Image< double, 3>::New();
  temp_img1->SetSpacing( itkImage->GetSpacing() );
  temp_img1->SetOrigin( itkImage->GetOrigin() );
  temp_img1->SetDirection( itkImage->GetDirection() );
  temp_img1->SetLargestPossibleRegion( itkImage->GetLargestPossibleRegion() );
  temp_img1->SetBufferedRegion( itkImage->GetBufferedRegion() );
  temp_img1->SetRequestedRegion( itkImage->GetRequestedRegion() );
  temp_img1->Allocate();
  temp_img1->FillBuffer(0.0);

  itk::Image< double, 3>::Pointer temp_img2 = itk::Image< double, 3>::New();
  temp_img2->SetSpacing( itkImage->GetSpacing() );
  temp_img2->SetOrigin( itkImage->GetOrigin() );
  temp_img2->SetDirection( itkImage->GetDirection() );
  temp_img2->SetLargestPossibleRegion( itkImage->GetLargestPossibleRegion() );
  temp_img2->SetBufferedRegion( itkImage->GetBufferedRegion() );
  temp_img2->SetRequestedRegion( itkImage->GetRequestedRegion() );
  temp_img2->Allocate();
  temp_img2->FillBuffer(0.0);

  MITK_INFO << "Filling matrices ...";
  unsigned int point_counter = 0;
  vtkSmartPointer<vtkPolyData> polydata = inputTractogram->GetFiberPolyData();
  for (int i=0; i<inputTractogram->GetNumFibers(); ++i)
  {
    vtkCell* cell = polydata->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    if (numPoints<2)
      MITK_INFO << "FIBER WITH ONLY ONE POINT ENCOUNTERED!";

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
      ModelType::PixelType model_signal = signalModel.SimulateMeasurement();
      DPH::ImageType::PixelType measured_signal = itkImage->GetPixel(idx);

      int x = idx[0];
      int y = idx[1];
      int z = idx[2];

      unsigned int linear_index = x + sz_x*y + sz_x*sz_y*z;

      for (unsigned int k=0; k<num_gradients; ++k)
      {
        b(num_gradients*linear_index + k, 0) = (double)measured_signal[k];
        A(num_gradients*linear_index + k, i) = A(num_gradients*linear_index + k, i) + (double)model_signal[k];

        temp_img1->SetPixel(idx, temp_img1->GetPixel(idx) + (double)model_signal[k]);
        A(num_gradients*linear_index + k, i) = temp_img1->GetPixel(idx);

//        temp_img2->SetPixel(idx, A(num_gradients*linear_index + k, i) );
      }

      point_counter++;
    }
  }

//  typedef  itk::ImageFileWriter< itk::Image< double, 3>  > WriterType;
//  WriterType::Pointer writer = WriterType::New();
//  writer->SetFileName("/home/neher/Projects/TractPlausibility/model_signal.nrrd");
//  writer->SetInput(temp_img2);
//  writer->Update();

//  writer->SetFileName("/home/neher/Projects/TractPlausibility/measured_signal.nrrd");
//  writer->SetInput(temp_img1);
//  writer->Update();

  MITK_INFO << "Solving linear system";
  vigra::linalg::nonnegativeLeastSquares(A, b, x);

  std::vector<float> weights;
  for (unsigned int i=0; i<num_unknowns; ++i)
    weights.push_back(x[i]);
  return weights;
}

std::vector<float> SolveEvo(mitk::FiberBundle* inputTractogram, mitk::Image::Pointer inputImage, ModelType signalModel, BallModelType ballModel, int num_iterations=1000, double step=0.1)
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

  vtkSmartPointer<vtkPolyData> polydata = inputTractogram->GetFiberPolyData();

  MITK_INFO << "INITIALIZING";
  std::vector< std::vector< ModelType::PixelType > > fiber_model_signals;
  std::vector< std::vector< DPH::ImageType::IndexType > > fiber_image_indices;

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

      model_signals.push_back(step*signalModel.SimulateMeasurement());
      image_indices.push_back(idx);
    }

    fiber_model_signals.push_back(model_signals);
    fiber_image_indices.push_back(image_indices);
    fiber_indices.push_back(f);
    out_weights.push_back(0);
    ++f;
  }

  BallModelType::PixelType ballSignal = step/10*ballModel.SimulateMeasurement();

  double T_start = 0.0001;
  double T_end = 0.000001;
  double alpha = log(T_end/T_start);

  unsigned int num_gradients = signalModel.GetGradientList().size();
  for (int i=0; i<num_iterations; ++i)
  {
    MITK_INFO << "Iteration " << i;
    std::random_shuffle(fiber_indices.begin(), fiber_indices.end());

    double T = T_start * exp(alpha*(double)i/num_iterations);

    int accepted = 0;
    for (auto f : fiber_indices)
    {
      std::vector< ModelType::PixelType > model_signals = fiber_model_signals.at(f);
      std::vector< DPH::ImageType::IndexType > image_indices = fiber_image_indices.at(f);

      double E_old = 0;
      double E_new = 0;

      int add = std::rand()%2;
//      add = 1;
      int use_ball = std::rand()%2;

      if (add==0 && use_ball==0 && out_weights[f]<step)
        add = 1;

      int c = 0;
      for (auto idx : image_indices)
      {
        DPH::ImageType::PixelType mVal = itkImage->GetPixel(idx);
        ModelType::PixelType sVal = simulatedImage->GetPixel(idx);
        ModelType::PixelType dVal = model_signals.at(c);
        if (use_ball==1)
          dVal = ballSignal;

        for (unsigned int g=0; g<num_gradients; ++g)
        {
//          ModelType::GradientType d = signalModel.GetGradientDirection(g);
//          if (d.GetNorm()<0.0001)
//            continue;

          double delta = (double)mVal[g] - sVal[g];

          E_old += fabs(delta)/num_gradients;

          if (add==1)
          {
            delta = (double)mVal[g] - (sVal[g] + dVal[g]);
            E_new += fabs(delta)/num_gradients;
          }
          else
          {
            delta = (double)mVal[g] - (sVal[g] - dVal[g]);
            E_new += fabs(delta)/num_gradients;
          }
        }
        ++c;
      }

      E_old /= image_indices.size();
      E_new /= image_indices.size();
      double R = exp( (E_old-E_new)/T );

//      if (E_new < E_old)
      float p = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      if (p<R)
      {
        accepted++;
        c = 0;
        for (auto idx : image_indices)
        {
          ModelType::PixelType sVal = simulatedImage->GetPixel(idx);
          ModelType::PixelType dVal = model_signals.at(c);
          if (use_ball==1)
            dVal = ballSignal;

          if (add==1)
            sVal += dVal;
          else
            sVal -= dVal;

          simulatedImage->SetPixel(idx, sVal);

          ++c;
        }

        if (add==1 && use_ball==0)
          out_weights[f] += step;
        else if (use_ball==0)
          out_weights[f] -= step;

      }

    }
    MITK_INFO << "Accepted: " << (float)accepted/fiber_indices.size();
  }

  typedef  itk::ImageFileWriter< itk::VectorImage< double, 3> > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName("/home/neher/Projects/TractPlausibility/model_signal.nrrd");
  writer->SetInput(simulatedImage);
  writer->Update();

  return out_weights;
}


/*!
\brief
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Fit Fibers To Image");
  parser.setCategory("Fiber Tracking Evaluation");
  parser.setDescription("");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("input_tractogram", "i1", mitkCommandLineParser::InputFile, "Input Tractogram:", "input tractogram (.fib, vtk ascii file format)", us::Any(), false);
  parser.addArgument("input_dMRI", "i2", mitkCommandLineParser::InputFile, "Input dMRI:", "input diffusion-weighted image", us::Any(), false);
  parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output root", us::Any(), false);
  parser.addArgument("step", "", mitkCommandLineParser::Float, "", "", us::Any());
  parser.addArgument("it", "", mitkCommandLineParser::Int, "", "", us::Any());

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  string fibFile = us::any_cast<string>(parsedArgs["input_tractogram"]);
  string dwiFile = us::any_cast<string>(parsedArgs["input_dMRI"]);
  string outRoot = us::any_cast<string>(parsedArgs["out"]);

  int it = 1000;
  if (parsedArgs.count("it"))
      it = us::any_cast<int>(parsedArgs["it"]);

  float step = 0.1;
  if (parsedArgs.count("step"))
      step = us::any_cast<float>(parsedArgs["step"]);



  typedef DPH::GradientDirectionsContainerType GradientContainerType;

  try
  {
    mitk::FiberBundle::Pointer inputTractogram = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(fibFile)[0].GetPointer());

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images", "Fiberbundles"}, {});
    mitk::Image::Pointer inputImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(dwiFile, &functor)[0].GetPointer());
    if (!DPH::IsDiffusionWeightedImage(inputImage))
      return EXIT_FAILURE;

    // resample fibers
    float minSpacing = 1;
    if(inputImage->GetGeometry()->GetSpacing()[0]<inputImage->GetGeometry()->GetSpacing()[1] && inputImage->GetGeometry()->GetSpacing()[0]<inputImage->GetGeometry()->GetSpacing()[2])
      minSpacing = inputImage->GetGeometry()->GetSpacing()[0];
    else if (inputImage->GetGeometry()->GetSpacing()[1] < inputImage->GetGeometry()->GetSpacing()[2])
      minSpacing = inputImage->GetGeometry()->GetSpacing()[1];
    else
      minSpacing = inputImage->GetGeometry()->GetSpacing()[2];
    inputTractogram->ResampleLinear(minSpacing/10);

    // set up signal model
    GradientContainerType* gradients = static_cast<mitk::GradientDirectionsProperty*>( inputImage->GetProperty(DPH::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();

    float b_value = 0;
    inputImage->GetPropertyList()->GetFloatProperty(DPH::REFERENCEBVALUEPROPERTYNAME.c_str(), b_value);

    ModelType signalModel;
    signalModel.SetDiffusivity(0.001);
    signalModel.SetBvalue(b_value);
    signalModel.SetGradientList(gradients);

    BallModelType ballModel;
    ballModel.SetDiffusivity(0.001);
    ballModel.SetBvalue(b_value);
    ballModel.SetGradientList(gradients);

//    MITK_INFO << it << " " << step;
//    std::vector<float> weights = SolveLinear(inputTractogram, inputImage, signalModel, ballModel);
    std::vector<float> weights = SolveEvo(inputTractogram, inputImage, signalModel, ballModel, it, step);

    for (unsigned int i=0; i<weights.size(); ++i)
    {
      MITK_INFO << weights.at(i);
      inputTractogram->SetFiberWeight(i, weights.at(i));
    }

    mitk::IOUtil::Save(inputTractogram, outRoot + "fitted.fib");
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
