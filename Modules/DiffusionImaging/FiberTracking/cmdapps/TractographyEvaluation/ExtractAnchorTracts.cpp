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

void CreateFolderStructure(const std::string& path)
{
  if (ist::PathExists(path))
    ist::RemoveADirectory(path);

  ist::MakeDirectory(path);
  ist::MakeDirectory(path + "/anchor_tracts/");
  ist::MakeDirectory(path + "/candidate_tracts/");
  ist::MakeDirectory(path + "/skipped_masks/");
}

ItkFloatImgType::Pointer LoadItkImage(const std::string& filename)
{
  mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(filename)[0].GetPointer());
  ItkFloatImgType::Pointer itkMask = ItkFloatImgType::New();
  mitk::CastToItkImage(img, itkMask);
  return itkMask;
}

std::vector< MaskType > get_file_list(const std::string& path, float anchor_fraction, const std::string& skipped_path, int random_seed)
{
  if (anchor_fraction<0)
    anchor_fraction = 0;
  else if (anchor_fraction>1.0)
    anchor_fraction = 1.0;

  std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
  if (random_seed<0)
    std::srand(ms.count());
  else
    std::srand(random_seed);
  MITK_INFO << "random_seed: " << random_seed;

  std::vector< MaskType > mask_list;

  itk::Directory::Pointer dir = itk::Directory::New();

  int skipped = 0;
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

    int skipping_num = num_images * (1.0 - anchor_fraction);
    std::random_shuffle(im_indices.begin(), im_indices.end());
    MITK_INFO << "Skipping " << skipping_num << " images";
    MITK_INFO << "Number of anchors: " << num_images-skipping_num;

    int c = -1;
    for (int r : im_indices)
    {
      c++;
      const char *filename = dir->GetFile(r);

      if (c<skipping_num && (skipping_num==num_images || ist::GetFilenameWithoutExtension(filename)!="CC"))
      {
        MITK_INFO << "Skipping " << ist::GetFilenameWithoutExtension(filename);
        ist::CopyAFile(path + '/' + filename, skipped_path + ist::GetFilenameName(filename));
        skipped++;
        continue;
      }

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

mitk::FiberBundle::Pointer TransformToMRtrixSpace(mitk::FiberBundle::Pointer fib)
{
  mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
  vtkSmartPointer< vtkMatrix4x4 > matrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  matrix->Identity();
  matrix->SetElement(0,0,-matrix->GetElement(0,0));
  matrix->SetElement(1,1,-matrix->GetElement(1,1));
  geometry->SetIndexToWorldTransformByVtkMatrix(matrix);

  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformFilter->SetInputData(fib->GetFiberPolyData());
  transformFilter->SetTransform(geometry->GetVtkTransform());
  transformFilter->Update();
  mitk::FiberBundle::Pointer transformed_fib = mitk::FiberBundle::New(transformFilter->GetOutput());
  return transformed_fib;
}

/*!
\brief
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Extract Overlapping Tracts");
  parser.setCategory("Fiber Tracking Evaluation");
  parser.setDescription("");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input:", "input tractogram (.fib, vtk ascii file format)", us::Any(), false);
  parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output folder", us::Any(), false);
  parser.addArgument("reference_mask_folder", "m", mitkCommandLineParser::String, "Reference Mask Folder:", "reference masks of known bundles", us::Any(), false);
  parser.addArgument("gray_matter_mask", "gm", mitkCommandLineParser::String, "GM mask:", "remove fibers not ending in the gray matter");
  parser.addArgument("anchor_fraction", "", mitkCommandLineParser::Float, "Anchor fraction:", "Fraction of tracts used as anchors", 0.5);
  parser.addArgument("overlap", "", mitkCommandLineParser::Float, "Overlap threshold:", "Overlap threshold used to identify the anchor tracts", 0.8);
  parser.addArgument("subsample", "", mitkCommandLineParser::Float, "Subsampling factor:", "Only use specified fraction of input fibers for the analysis", 1.0);
  parser.addArgument("random_seed", "", mitkCommandLineParser::Int, ":", "", -1);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string fibFile = us::any_cast<std::string>(parsedArgs["input"]);
  std::string reference_mask_folder = us::any_cast<std::string>(parsedArgs["reference_mask_folder"]);
  std::string out_folder = us::any_cast<std::string>(parsedArgs["out"]);

  std::string gray_matter_mask = "";
  if (parsedArgs.count("gray_matter_mask"))
    gray_matter_mask = us::any_cast<std::string>(parsedArgs["gray_matter_mask"]);

  float anchor_fraction = 0.5;
  if (parsedArgs.count("anchor_fraction"))
    anchor_fraction = us::any_cast<float>(parsedArgs["anchor_fraction"]);

  int random_seed = -1;
  if (parsedArgs.count("random_seed"))
    random_seed = us::any_cast<int>(parsedArgs["random_seed"]);

  float overlap = 0.8;
  if (parsedArgs.count("overlap"))
    overlap = us::any_cast<float>(parsedArgs["overlap"]);

  float subsample = 1.0;
  if (parsedArgs.count("subsample"))
    subsample = us::any_cast<float>(parsedArgs["subsample"]);

  try
  {
    CreateFolderStructure(out_folder);
    std::vector< MaskType > known_tract_masks = get_file_list(reference_mask_folder, anchor_fraction, out_folder + "/skipped_masks/", random_seed);
    mitk::FiberBundle::Pointer inputTractogram = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(fibFile)[0].GetPointer());

    if (gray_matter_mask.compare("")!=0)
    {
      MITK_INFO << "Removing fibers not ending inside of GM";
      std::streambuf *old = cout.rdbuf(); // <-- save
      std::stringstream ss;
      std::cout.rdbuf (ss.rdbuf());       // <-- redirect
      ItkFloatImgType::Pointer gm_image = LoadItkImage(gray_matter_mask);
      std::cout.rdbuf (old);              // <-- restore

      itk::FiberExtractionFilter<unsigned char>::Pointer extractor = itk::FiberExtractionFilter<unsigned char>::New();
      extractor->SetInputFiberBundle(inputTractogram);
      extractor->SetRoiImages({gm_image});
      extractor->SetBothEnds(true);
      extractor->SetNoNegatives(true);
      extractor->SetMode(itk::FiberExtractionFilter<unsigned char>::MODE::ENDPOINTS);
      extractor->Update();
      inputTractogram = extractor->GetPositives().at(0);
    }

    std::srand(0);
    if (subsample<1.0)
      inputTractogram = inputTractogram->SubsampleFibers(subsample);

    mitk::FiberBundle::Pointer anchor_tractogram = mitk::FiberBundle::New(nullptr);
    mitk::FiberBundle::Pointer candidate_tractogram = mitk::FiberBundle::New(nullptr);
    if (!known_tract_masks.empty())
    {
      MITK_INFO << "Find known tracts via overlap match";
      std::vector< ItkFloatImgType::Pointer > mask_images;
      for (auto mask : known_tract_masks)
        mask_images.push_back(std::get<0>(mask));

      std::vector< ItkFloatImgType* > roi_images2;
      for (auto roi : mask_images)
        roi_images2.push_back(roi);

      itk::FiberExtractionFilter<unsigned char>::Pointer extractor = itk::FiberExtractionFilter<unsigned char>::New();
      extractor->SetInputFiberBundle(inputTractogram);
      extractor->SetRoiImages(roi_images2);
      extractor->SetOverlapFraction(overlap);
      extractor->SetMode(itk::FiberExtractionFilter<unsigned char>::MODE::OVERLAP);
      extractor->Update();
      std::vector< mitk::FiberBundle::Pointer > positives = extractor->GetPositives();
      candidate_tractogram = extractor->GetNegatives().at(0);

      for ( unsigned int i=0; i<known_tract_masks.size(); ++i )
      {
        std::streambuf *old = cout.rdbuf(); // <-- save
        std::stringstream ss;
        std::cout.rdbuf (ss.rdbuf());       // <-- redirect

        std::string mask_name = std::get<1>(known_tract_masks.at(i));
        mitk::IOUtil::Save(positives.at(i), out_folder + "/anchor_tracts/" + mask_name + ".trk");

        std::cout.rdbuf (old);              // <-- restore
      }
      anchor_tractogram = anchor_tractogram->AddBundles(positives);
    }

    mitk::IOUtil::Save(anchor_tractogram, out_folder + "/anchor_tracts/anchor_tractogram.trk");
    mitk::IOUtil::Save(candidate_tractogram, out_folder + "/candidate_tracts/candidate_tractogram.trk");
    mitk::IOUtil::Save(inputTractogram, out_folder + "/filtered_tractogram.trk");
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
