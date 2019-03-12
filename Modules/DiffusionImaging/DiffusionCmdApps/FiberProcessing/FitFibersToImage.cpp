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
#include <mitkFiberBundle.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <itkImageFileWriter.h>
#include <mitkPeakImage.h>
#include <itkFitFibersToImageFilter.h>
#include <mitkTensorModel.h>
#include <mitkITKImageImport.h>
#include <mitkDiffusionDataIOHelper.h>

typedef itk::Point<float, 4> PointType4;
typedef itk::Image< float, 4 >  PeakImgType;

/*!
\brief Fits the tractogram to the input peak image by assigning a weight to each fiber (similar to https://doi.org/10.1016/j.neuroimage.2015.06.092).
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Fit Fibers To Image");
  parser.setCategory("Fiber Tracking and Processing Methods");
  parser.setDescription("Assigns a weight to each fiber in order to optimally explain the input peak image");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i1", mitkCommandLineParser::StringList, "Input tractograms:", "input tractograms (files or folder)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "i2", mitkCommandLineParser::String, "Input image:", "input image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output root", us::Any(), false, false, false, mitkCommandLineParser::Output);

  parser.addArgument("max_iter", "", mitkCommandLineParser::Int, "Max. iterations:", "maximum number of optimizer iterations", 20);
  parser.addArgument("bundle_based", "", mitkCommandLineParser::Bool, "Bundle based fit:", "fit one weight per input tractogram/bundle, not for each fiber", false);
  parser.addArgument("min_g", "", mitkCommandLineParser::Float, "Min. g:", "lower termination threshold for gradient magnitude", 1e-5);
  parser.addArgument("lambda", "", mitkCommandLineParser::Float, "Lambda:", "modifier for regularization", 0.1);
  parser.addArgument("save_res", "", mitkCommandLineParser::Bool, "Save Residuals:", "save residual images", false);
  parser.addArgument("save_weights", "", mitkCommandLineParser::Bool, "Save Weights:", "save fiber weights in a separate text file", false);
  parser.addArgument("filter_outliers", "", mitkCommandLineParser::Bool, "Filter outliers:", "perform second optimization run with an upper weight bound based on the first weight estimation (99% quantile)", false);
  parser.addArgument("join_tracts", "", mitkCommandLineParser::Bool, "Join output tracts:", "outout tracts are merged into a single tractogram", false);
  parser.addArgument("regu", "", mitkCommandLineParser::String, "Regularization:", "MSM; Variance; VoxelVariance; Lasso; GroupLasso; GroupVariance; NONE", std::string("VoxelVariance"));

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  mitkCommandLineParser::StringContainerType fib_files = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["i1"]);
  std::string input_image_name = us::any_cast<std::string>(parsedArgs["i2"]);
  std::string outRoot = us::any_cast<std::string>(parsedArgs["o"]);

  bool single_fib = true;
  if (parsedArgs.count("bundle_based"))
    single_fib = !us::any_cast<bool>(parsedArgs["bundle_based"]);

  bool save_residuals = false;
  if (parsedArgs.count("save_res"))
    save_residuals = us::any_cast<bool>(parsedArgs["save_res"]);

  bool save_weights = false;
  if (parsedArgs.count("save_weights"))
    save_weights = us::any_cast<bool>(parsedArgs["save_weights"]);

  std::string regu = "VoxelVariance";
  if (parsedArgs.count("regu"))
    regu = us::any_cast<std::string>(parsedArgs["regu"]);

  bool join_tracts = false;
  if (parsedArgs.count("join_tracts"))
    join_tracts = us::any_cast<bool>(parsedArgs["join_tracts"]);

  int max_iter = 20;
  if (parsedArgs.count("max_iter"))
    max_iter = us::any_cast<int>(parsedArgs["max_iter"]);

  float g_tol = 1e-5;
  if (parsedArgs.count("min_g"))
    g_tol = us::any_cast<float>(parsedArgs["min_g"]);

  float lambda = 0.1;
  if (parsedArgs.count("lambda"))
    lambda = us::any_cast<float>(parsedArgs["lambda"]);

  bool filter_outliers = false;
  if (parsedArgs.count("filter_outliers"))
    filter_outliers = us::any_cast<bool>(parsedArgs["filter_outliers"]);

  try
  {
    MITK_INFO << "Loading data";

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Peak Image", "Fiberbundles"}, {});

    std::vector< std::string > fib_names;
    auto input_tracts = mitk::DiffusionDataIOHelper::load_fibs(fib_files, &fib_names);

    itk::FitFibersToImageFilter::Pointer fitter = itk::FitFibersToImageFilter::New();

    mitk::BaseData::Pointer inputData = mitk::IOUtil::Load(input_image_name, &functor)[0].GetPointer();
    mitk::Image::Pointer mitk_image = dynamic_cast<mitk::Image*>(inputData.GetPointer());
    mitk::PeakImage::Pointer mitk_peak_image = dynamic_cast<mitk::PeakImage*>(inputData.GetPointer());
    if (mitk_peak_image.IsNotNull())
    {
      typedef mitk::ImageToItk< mitk::PeakImage::ItkPeakImageType > CasterType;
      CasterType::Pointer caster = CasterType::New();
      caster->SetInput(mitk_peak_image);
      caster->Update();
      mitk::PeakImage::ItkPeakImageType::Pointer peak_image = caster->GetOutput();
      fitter->SetPeakImage(peak_image);
    }
    else if (mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(mitk_image))
    {
      fitter->SetDiffImage(mitk::DiffusionPropertyHelper::GetItkVectorImage(mitk_image));
      mitk::TensorModel<>* model = new mitk::TensorModel<>();
      model->SetBvalue(1000);
      model->SetDiffusivity1(0.0010);
      model->SetDiffusivity2(0.00015);
      model->SetDiffusivity3(0.00015);
      model->SetGradientList(mitk::DiffusionPropertyHelper::GetGradientContainer(mitk_image));
      fitter->SetSignalModel(model);
    }
    else if (mitk_image->GetDimension()==3)
    {
      itk::FitFibersToImageFilter::DoubleImgType::Pointer scalar_image = itk::FitFibersToImageFilter::DoubleImgType::New();
      mitk::CastToItkImage(mitk_image, scalar_image);
      fitter->SetScalarImage(scalar_image);
    }
    else
    {
      MITK_INFO << "Input image invalid. Valid options are peak image, 3D scalar image or raw diffusion-weighted image.";
      return EXIT_FAILURE;
    }

    fitter->SetTractograms(input_tracts);
    fitter->SetFitIndividualFibers(single_fib);
    fitter->SetMaxIterations(max_iter);
    fitter->SetGradientTolerance(g_tol);
    fitter->SetLambda(lambda);
    fitter->SetFilterOutliers(filter_outliers);

    if (regu=="MSM")
      fitter->SetRegularization(VnlCostFunction::REGU::MSM);
    else if (regu=="Variance")
      fitter->SetRegularization(VnlCostFunction::REGU::VARIANCE);
    else if (regu=="Lasso")
      fitter->SetRegularization(VnlCostFunction::REGU::LASSO);
    else if (regu=="VoxelVariance")
      fitter->SetRegularization(VnlCostFunction::REGU::VOXEL_VARIANCE);
    else if (regu=="GroupLasso")
      fitter->SetRegularization(VnlCostFunction::REGU::GROUP_LASSO);
    else if (regu=="GroupVariance")
      fitter->SetRegularization(VnlCostFunction::REGU::GROUP_VARIANCE);
    else if (regu=="NONE")
      fitter->SetRegularization(VnlCostFunction::REGU::NONE);

    fitter->Update();

    mitk::LocaleSwitch localeSwitch("C");
    if (save_residuals && mitk_peak_image.IsNotNull())
    {
      itk::ImageFileWriter< PeakImgType >::Pointer writer = itk::ImageFileWriter< PeakImgType >::New();
      writer->SetInput(fitter->GetFittedImage());
      writer->SetFileName(outRoot + "_fitted.nii.gz");
      writer->Update();

      writer->SetInput(fitter->GetResidualImage());
      writer->SetFileName(outRoot + "_residual.nii.gz");
      writer->Update();

      writer->SetInput(fitter->GetOverexplainedImage());
      writer->SetFileName(outRoot + "_overexplained.nii.gz");
      writer->Update();

      writer->SetInput(fitter->GetUnderexplainedImage());
      writer->SetFileName(outRoot + "_underexplained.nii.gz");
      writer->Update();
    }
    else if (save_residuals && mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(mitk_image))
    {
      {
        mitk::Image::Pointer outImage = mitk::GrabItkImageMemory( fitter->GetFittedImageDiff().GetPointer() );
        mitk::DiffusionPropertyHelper::CopyProperties(mitk_image, outImage, true);
        mitk::DiffusionPropertyHelper::InitializeImage( outImage );
        mitk::IOUtil::Save(outImage, "application/vnd.mitk.nii.gz", outRoot + "_fitted_image.nii.gz");
      }

      {
        mitk::Image::Pointer outImage = mitk::GrabItkImageMemory( fitter->GetResidualImageDiff().GetPointer() );
        mitk::DiffusionPropertyHelper::CopyProperties(mitk_image, outImage, true);
        mitk::DiffusionPropertyHelper::InitializeImage( outImage );
        mitk::IOUtil::Save(outImage, "application/vnd.mitk.nii.gz", outRoot + "_residual_image.nii.gz");
      }

      {
        mitk::Image::Pointer outImage = mitk::GrabItkImageMemory( fitter->GetOverexplainedImageDiff().GetPointer() );
        mitk::DiffusionPropertyHelper::CopyProperties(mitk_image, outImage, true);
        mitk::DiffusionPropertyHelper::InitializeImage( outImage );
        mitk::IOUtil::Save(outImage, "application/vnd.mitk.nii.gz", outRoot + "_overexplained_image.nii.gz");
      }

      {
        mitk::Image::Pointer outImage = mitk::GrabItkImageMemory( fitter->GetUnderexplainedImageDiff().GetPointer() );
        mitk::DiffusionPropertyHelper::CopyProperties(mitk_image, outImage, true);
        mitk::DiffusionPropertyHelper::InitializeImage( outImage );
        mitk::IOUtil::Save(outImage, "application/vnd.mitk.nii.gz", outRoot + "_underexplained_image.nii.gz");
      }
    }
    else if (save_residuals)
    {
      itk::ImageFileWriter< itk::FitFibersToImageFilter::DoubleImgType >::Pointer writer = itk::ImageFileWriter< itk::FitFibersToImageFilter::DoubleImgType >::New();
      writer->SetInput(fitter->GetFittedImageScalar());
      writer->SetFileName(outRoot + "_fitted_image.nii.gz");
      writer->Update();

      writer->SetInput(fitter->GetResidualImageScalar());
      writer->SetFileName(outRoot + "_residual_image.nii.gz");
      writer->Update();

      writer->SetInput(fitter->GetOverexplainedImageScalar());
      writer->SetFileName(outRoot + "_overexplained_image.nii.gz");
      writer->Update();

      writer->SetInput(fitter->GetUnderexplainedImageScalar());
      writer->SetFileName(outRoot + "_underexplained_image.nii.gz");
      writer->Update();
    }

    std::vector< mitk::FiberBundle::Pointer > output_tracts = fitter->GetTractograms();

    if (!join_tracts)
    {
      for (unsigned int bundle=0; bundle<output_tracts.size(); bundle++)
      {
        std::string name = fib_names.at(bundle);
        name = ist::GetFilenameWithoutExtension(name);
        mitk::IOUtil::Save(output_tracts.at(bundle), outRoot + name + "_fitted.fib");

        if (save_weights)
        {
          ofstream logfile;
          logfile.open (outRoot + name + "_weights.txt");
          for (unsigned int f=0; f<output_tracts.at(bundle)->GetNumFibers(); ++f)
            logfile << output_tracts.at(bundle)->GetFiberWeight(f) << "\n";
          logfile.close();
        }
      }
    }
    else
    {
      mitk::FiberBundle::Pointer out = mitk::FiberBundle::New();
      out = out->AddBundles(output_tracts);
      out->ColorFibersByFiberWeights(false, true);
      mitk::IOUtil::Save(out, outRoot + "_fitted.fib");

      if (save_weights)
      {
        ofstream logfile;
        logfile.open (outRoot + "_weights.txt");
        for (unsigned int f=0; f<out->GetNumFibers(); ++f)
          logfile << out->GetFiberWeight(f) << "\n";
        logfile.close();
      }
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
