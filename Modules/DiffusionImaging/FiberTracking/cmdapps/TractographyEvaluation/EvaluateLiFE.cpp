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
#include <vtkTransformPolyDataFilter.h>
#include <fstream>
#include <chrono>
#include <boost/progress.hpp>
#include <itkFiberExtractionFilter.h>

typedef itksys::SystemTools ist;
typedef itk::Image<unsigned char, 3>    ItkFloatImgType;
typedef std::tuple< ItkFloatImgType::Pointer, std::string > MaskType;

ItkFloatImgType::Pointer LoadItkImage(const std::string& filename)
{
  mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(filename)[0].GetPointer());
  ItkFloatImgType::Pointer itkMask = ItkFloatImgType::New();
  mitk::CastToItkImage(img, itkMask);
  return itkMask;
}

std::vector< MaskType > get_file_list(const std::string& path)
{
  std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
  std::srand(ms.count());

  std::vector< MaskType > mask_list;

  itk::Directory::Pointer dir = itk::Directory::New();

  if (dir->Load(path.c_str()))
  {
    int n = dir->GetNumberOfFiles();
    int num_images = 0;

    std::vector< int > im_indices;
    for (int r = 0; r < n; r++)
    {
      const char *filename = dir->GetFile(r);
      std::string ext = ist::GetFilenameExtension(filename);
      if (ext==".nii" || ext==".nii.gz" || ext==".nrrd")
      {
        ++num_images;
        im_indices.push_back(r);
      }
    }

    int c = -1;
    for (int r : im_indices)
    {
      c++;
      const char *filename = dir->GetFile(r);

      MITK_INFO << "Loading " << ist::GetFilenameWithoutExtension(filename);
      std::streambuf *old = cout.rdbuf(); // <-- save
      std::stringstream ss;
      std::cout.rdbuf (ss.rdbuf());       // <-- redirect
      MaskType m(LoadItkImage(path + '/' + filename), ist::GetFilenameName(filename));
      mask_list.push_back(m);
      std::cout.rdbuf (old);              // <-- restore
    }
  }

  return mask_list;
}

/*!
\brief
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Evaluate LiFE results");
  parser.setCategory("Fiber Tracking Evaluation");
  parser.setDescription("");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input:", "input tractogram (.fib, vtk ascii file format)", us::Any(), false);
  parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output folder", us::Any(), false);
  parser.addArgument("reference_mask_folder", "m", mitkCommandLineParser::String, "Reference Mask Folder:", "reference masks of known bundles", false);
  parser.addArgument("overlap", "", mitkCommandLineParser::Float, "Overlap threshold:", "Overlap threshold used to identify true positives", 0.8);
  parser.addArgument("steps", "", mitkCommandLineParser::Int, "Threshold steps:", "number of weight thresholds used to calculate the ROC curve", 100);
  parser.addArgument("pre_filter_zeros", "", mitkCommandLineParser::Bool, "Remove zero weights:", "remove fibers with zero weights before starting the evaluation");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string fibFile = us::any_cast<std::string>(parsedArgs["input"]);
  std::string reference_mask_folder = us::any_cast<std::string>(parsedArgs["reference_mask_folder"]);
  std::string out_folder = us::any_cast<std::string>(parsedArgs["out"]);

  float overlap = 0.8;
  if (parsedArgs.count("overlap"))
    overlap = us::any_cast<float>(parsedArgs["overlap"]);

  int steps = 10;
  if (parsedArgs.count("steps"))
    steps = us::any_cast<int>(parsedArgs["steps"]);

  bool pre_filter_zeros = false;
  if (parsedArgs.count("pre_filter_zeros"))
    pre_filter_zeros = us::any_cast<bool>(parsedArgs["pre_filter_zeros"]);

  try
  {
    std::vector< MaskType > known_tract_masks = get_file_list(reference_mask_folder);
    if (known_tract_masks.empty())
      return EXIT_FAILURE;

    mitk::FiberBundle::Pointer inputTractogram = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(fibFile)[0].GetPointer());

    // resample fibers
    float minSpacing = 1;
    if(std::get<0>(known_tract_masks.at(0))->GetSpacing()[0]<std::get<0>(known_tract_masks.at(0))->GetSpacing()[1] && std::get<0>(known_tract_masks.at(0))->GetSpacing()[0]<std::get<0>(known_tract_masks.at(0))->GetSpacing()[2])
      minSpacing = std::get<0>(known_tract_masks.at(0))->GetSpacing()[0];
    else if (std::get<0>(known_tract_masks.at(0))->GetSpacing()[1] < std::get<0>(known_tract_masks.at(0))->GetSpacing()[2])
      minSpacing = std::get<0>(known_tract_masks.at(0))->GetSpacing()[1];
    else
      minSpacing = std::get<0>(known_tract_masks.at(0))->GetSpacing()[2];
    inputTractogram->ResampleLinear(minSpacing/5);

    std::vector< float > weights;
    for (int i=0; i<inputTractogram->GetNumFibers(); ++i)
      weights.push_back(inputTractogram->GetFiberWeight(i));
    std::sort(weights.begin(), weights.end());

    if (pre_filter_zeros)
      inputTractogram = inputTractogram->FilterByWeights(0.0);
    mitk::FiberBundle::Pointer pred_positives = inputTractogram->GetDeepCopy();
    mitk::FiberBundle::Pointer pred_negatives = mitk::FiberBundle::New(nullptr);

    ofstream logfile;
    logfile.open (out_folder + "LiFE_ROC.txt");

    float fpr = 1.0;
    float tpr = 1.0;
    float step = weights.back()/steps;
    float w = 0;
    if (!pre_filter_zeros)
      w -= step;
    while (pred_positives->GetNumFibers()>0 && fpr>0.001 && tpr>0.001)
    {
      w += step;

      std::streambuf *old = cout.rdbuf(); // <-- save
      std::stringstream ss;
      std::cout.rdbuf (ss.rdbuf());       // <-- redirect

      mitk::FiberBundle::Pointer tp_tracts = mitk::FiberBundle::New(nullptr);
      mitk::FiberBundle::Pointer fn_tracts = mitk::FiberBundle::New(nullptr);

      for ( MaskType mask : known_tract_masks )
      {
        ItkFloatImgType::Pointer mask_image = std::get<0>(mask);

        mitk::FiberBundle::Pointer a;
        {
          itk::FiberExtractionFilter<unsigned char>::Pointer extractor = itk::FiberExtractionFilter<unsigned char>::New();
          extractor->SetInputFiberBundle(pred_positives);
          extractor->SetRoiImages({mask_image});
          extractor->SetOverlapFraction(overlap);
          extractor->SetDontResampleFibers(true);
          extractor->SetMode(itk::FiberExtractionFilter<unsigned char>::MODE::OVERLAP);
          extractor->Update();
          a = extractor->GetPositives().at(0);
        }
        tp_tracts = tp_tracts->AddBundle(a);

        mitk::FiberBundle::Pointer b;
        {
          itk::FiberExtractionFilter<unsigned char>::Pointer extractor = itk::FiberExtractionFilter<unsigned char>::New();
          extractor->SetInputFiberBundle(pred_negatives);
          extractor->SetRoiImages({mask_image});
          extractor->SetOverlapFraction(overlap);
          extractor->SetDontResampleFibers(true);
          extractor->SetMode(itk::FiberExtractionFilter<unsigned char>::MODE::OVERLAP);
          extractor->Update();
          b = extractor->GetPositives().at(0);
        }
        fn_tracts = fn_tracts->AddBundle(b);
      }
      mitk::FiberBundle::Pointer fp_tracts = pred_positives->SubtractBundle(tp_tracts);
      mitk::FiberBundle::Pointer tn_tracts = pred_negatives->SubtractBundle(fn_tracts);

      std::cout.rdbuf (old);              // <-- restore

      float positives = tp_tracts->GetNumFibers() + fn_tracts->GetNumFibers();
      float negatives = tn_tracts->GetNumFibers() + fp_tracts->GetNumFibers();
      fpr = (float)fp_tracts->GetNumFibers() / negatives;
      tpr = (float)tp_tracts->GetNumFibers() / positives;

      float accuracy = 1.0;
      if (pred_positives->GetNumFibers()>0)
        accuracy = (float)tp_tracts->GetNumFibers()/pred_positives->GetNumFibers();
      logfile << w << " " << fpr << " " << tpr << " " << accuracy << " \n";
      MITK_INFO << "#Fibers: " << pred_positives->GetNumFibers();
      MITK_INFO << "FPR/TPR: " << fpr << "/" << tpr;
      MITK_INFO << "Acc: " << accuracy;

      pred_positives = inputTractogram->FilterByWeights(w);
      pred_negatives = inputTractogram->FilterByWeights(w, true);
    }
    logfile.close();
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
