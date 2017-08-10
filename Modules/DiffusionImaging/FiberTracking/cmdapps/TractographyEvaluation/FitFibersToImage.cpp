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

using namespace std;
typedef itksys::SystemTools ist;
typedef itk::Image<unsigned char, 3>    ItkUcharImgType;
typedef std::tuple< ItkUcharImgType::Pointer, std::string > MaskType;

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

  typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType GradientContainerType;

  try
  {
    mitk::FiberBundle::Pointer inputTractogram = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(fibFile)[0].GetPointer());

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images", "Fiberbundles"}, {});
    mitk::Image::Pointer inputImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(dwiFile, &functor)[0].GetPointer());
    if (!mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(inputImage))
      return EXIT_FAILURE;

    GradientContainerType* gradients = static_cast<mitk::GradientDirectionsProperty*>( inputImage->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();

    // resample fibers
    float minSpacing = 1;
    if(inputImage->GetGeometry()->GetSpacing()[0]<inputImage->GetGeometry()->GetSpacing()[1] && inputImage->GetGeometry()->GetSpacing()[0]<inputImage->GetGeometry()->GetSpacing()[2])
      minSpacing = inputImage->GetGeometry()->GetSpacing()[0];
    else if (inputImage->GetGeometry()->GetSpacing()[1] < inputImage->GetGeometry()->GetSpacing()[2])
      minSpacing = inputImage->GetGeometry()->GetSpacing()[1];
    else
      minSpacing = inputImage->GetGeometry()->GetSpacing()[2];
    inputTractogram->ResampleLinear(minSpacing/2);

    int num_unknowns = inputTractogram->GetNumFibers();
    int number_of_residuals = inputTractogram->GetNumberOfPoints() * gradients->Size();

    MITK_INFO << outRoot;
    MITK_INFO << num_unknowns;
    MITK_INFO << number_of_residuals;

//    vnl_matrix<double> matrix(number_of_residuals, num_unknowns);
    Eigen::MatrixXf A = Eigen::MatrixXf::Random(num_unknowns, num_unknowns);
    Eigen::VectorXf b = Eigen::VectorXf::Random(num_unknowns);

    // create linear system
    unsigned int point_counter = 0;
    vtkSmartPointer<vtkPolyData> polydata = inputTractogram->GetFiberPolyData();
    for (int i=0; i<inputTractogram->GetNumFibers(); i++)
    {
      vtkCell* cell = polydata->GetCell(i);
      int numPoints = cell->GetNumberOfPoints();
      vtkPoints* points = cell->GetPoints();

      for (int j=0; j<numPoints; j++)
      {
        double* p1 = points->GetPoint(j);
        MITK_INFO << p1;

        point_counter++;
      }
    }
//    vnl_linear_system system(matrix.rows(), matrix.cols());

    Eigen::VectorXf x = (A.transpose() * A).ldlt().solve(A.transpose() * b);
    for (int i=0; i<num_unknowns; ++i)
      MITK_INFO << x[i];
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
