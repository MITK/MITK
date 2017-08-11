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
#include <vigra/regression.hxx>

//vigra::linalg::nonnegativeLeastSquares

using namespace std;
typedef itksys::SystemTools ist;
typedef itk::Image<unsigned char, 3>    ItkUcharImgType;
typedef std::tuple< ItkUcharImgType::Pointer, std::string > MaskType;
typedef mitk::DiffusionPropertyHelper DPH;
typedef itk::Point<float, 3> PointType;
typedef mitk::StickModel<> ModelType;

std::vector<float> SolveLinear(mitk::FiberBundle* inputTractogram, mitk::Image::Pointer inputImage, ModelType signalModel)
{
  unsigned int num_gradients = signalModel.GetGradientList().size();

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
  Eigen::MatrixXf A = Eigen::MatrixXf::Constant(number_of_residuals, num_unknowns, 0);
  Eigen::VectorXf b = Eigen::VectorXf::Constant(number_of_residuals, 0);

  MITK_INFO << "Filling matrices ...";
  unsigned int point_counter = 0;
  DPH::ImageType::Pointer itkImage = DPH::GetItkVectorImage(inputImage);
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
      d[0] = p1[0]-p2[0];
      d[1] = p1[1]-p2[1];
      d[2] = p1[2]-p2[2];
      signalModel.SetFiberDirection(d);
      ModelType::PixelType model_signal = signalModel.SimulateMeasurement();
      DPH::ImageType::PixelType measured_signal = itkImage->GetPixel(idx);

      int x = idx[0];
      int y = idx[1];
      int z = idx[2];

      unsigned int linear_index = x + sz_x*y + sz_x*sz_y*z;

      for (unsigned int k=0; k<num_gradients; ++k)
      {
        b(num_gradients*linear_index + k) = (float)measured_signal[k];
        A(num_gradients*linear_index + k, i) += (float)model_signal[k];
      }

      point_counter++;
    }
  }

  unsigned int num_used_residuals = 0;
  for (unsigned int i=0; i<number_of_residuals; i++)
    if (b(i)>0)
      num_used_residuals++;

  Eigen::MatrixXf A2 = Eigen::MatrixXf::Constant(num_used_residuals, num_unknowns, 0);
  Eigen::VectorXf b2 = Eigen::VectorXf::Constant(num_used_residuals, 0);

  unsigned int c = 0;
  for (unsigned int i=0; i<number_of_residuals; i++)
    if (b(i)>0)
    {
      b2(c)=b(i);
      for (unsigned int j=0; j<num_unknowns; j++)
        A2(c, j) = A(i,j);
      ++c;
    }

  MITK_INFO << "Num. used residuals: " << num_used_residuals;
  MITK_INFO << "Solving linear system";
  Eigen::VectorXf x = (A2.transpose() * A2).ldlt().solve(A2.transpose() * b2);
//  Eigen::VectorXf x = A.colPivHouseholderQr().solve(b);
//  Eigen::VectorXf x = A.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);

  std::vector<float> weights;
  for (unsigned int i=0; i<num_unknowns; ++i)
    weights.push_back(x[i]);
  return weights;
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

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  string fibFile = us::any_cast<string>(parsedArgs["input_tractogram"]);
  string dwiFile = us::any_cast<string>(parsedArgs["input_dMRI"]);
  string outRoot = us::any_cast<string>(parsedArgs["out"]);

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
    inputTractogram->ResampleLinear(minSpacing/4);

    // set up signal model
    GradientContainerType* gradients = static_cast<mitk::GradientDirectionsProperty*>( inputImage->GetProperty(DPH::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();

    float b_value = 0;
    inputImage->GetPropertyList()->GetFloatProperty(DPH::REFERENCEBVALUEPROPERTYNAME.c_str(), b_value);

    ModelType signalModel;
    signalModel.SetDiffusivity(0.001);
    signalModel.SetBvalue(b_value);
    signalModel.SetGradientList(gradients);
    MITK_INFO << b_value;

    std::vector<float> weights = SolveLinear(inputTractogram, inputImage, signalModel);

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
