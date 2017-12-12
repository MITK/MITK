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

typedef itksys::SystemTools ist;
typedef itk::Image<unsigned char, 3>    ItkUcharImgType;
typedef std::tuple< ItkUcharImgType::Pointer, std::string > MaskType;

void CreateFolderStructure(const std::string& path)
{
  if (ist::PathExists(path))
    ist::RemoveADirectory(path);

  ist::MakeDirectory(path);
  ist::MakeDirectory(path + "/anchor_tracts/");
  ist::MakeDirectory(path + "/candidate_tracts/");
  ist::MakeDirectory(path + "/implausible_tracts/");
  ist::MakeDirectory(path + "/skipped_masks/");
}

ItkUcharImgType::Pointer LoadItkMaskImage(const std::string& filename)
{
  mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(filename)[0].GetPointer());
  ItkUcharImgType::Pointer itkMask = ItkUcharImgType::New();
  mitk::CastToItkImage(img, itkMask);
  return itkMask;
}

std::vector< MaskType > get_file_list(const std::string& path, float anchor_fraction, const std::string& skipped_path)
{
  if (anchor_fraction<0)
    anchor_fraction = 0;
  else if (anchor_fraction>1.0)
    anchor_fraction = 1.0;

  std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
  std::srand(ms.count());

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
      MaskType m(LoadItkMaskImage(path + '/' + filename), ist::GetFilenameName(filename));
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

  float overlap = 0.8;
  if (parsedArgs.count("overlap"))
    overlap = us::any_cast<float>(parsedArgs["overlap"]);

  float subsample = 1.0;
  if (parsedArgs.count("subsample"))
    subsample = us::any_cast<float>(parsedArgs["subsample"]);

  try
  {
    CreateFolderStructure(out_folder);
    std::vector< MaskType > known_tract_masks = get_file_list(reference_mask_folder, anchor_fraction, out_folder + "/skipped_masks/");
    mitk::FiberBundle::Pointer inputTractogram = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(fibFile)[0].GetPointer());

    MITK_INFO << "Removing fibers not ending inside of GM";
    if (gray_matter_mask.compare("")!=0)
    {
      std::streambuf *old = cout.rdbuf(); // <-- save
      std::stringstream ss;
      std::cout.rdbuf (ss.rdbuf());       // <-- redirect
      ItkUcharImgType::Pointer gm_image = LoadItkMaskImage(gray_matter_mask);
      std::cout.rdbuf (old);              // <-- restore

      mitk::FiberBundle::Pointer not_gm_fibers = inputTractogram->ExtractFiberSubset(gm_image, false, true, true);

      old = cout.rdbuf(); // <-- save
      std::cout.rdbuf (ss.rdbuf());       // <-- redirect
      mitk::IOUtil::Save(not_gm_fibers, out_folder + "/implausible_tracts/no_gm_endings.trk");
      inputTractogram = inputTractogram->ExtractFiberSubset(gm_image, false, false, true);
      std::cout.rdbuf (old);              // <-- restore
    }

    if (subsample<1.0)
      inputTractogram = inputTractogram->SubsampleFibers(subsample);


    // resample fibers
    float minSpacing = 1;
    if (!known_tract_masks.empty())
    {
      if(std::get<0>(known_tract_masks.at(0))->GetSpacing()[0]<std::get<0>(known_tract_masks.at(0))->GetSpacing()[1] && std::get<0>(known_tract_masks.at(0))->GetSpacing()[0]<std::get<0>(known_tract_masks.at(0))->GetSpacing()[2])
        minSpacing = std::get<0>(known_tract_masks.at(0))->GetSpacing()[0];
      else if (std::get<0>(known_tract_masks.at(0))->GetSpacing()[1] < std::get<0>(known_tract_masks.at(0))->GetSpacing()[2])
        minSpacing = std::get<0>(known_tract_masks.at(0))->GetSpacing()[1];
      else
        minSpacing = std::get<0>(known_tract_masks.at(0))->GetSpacing()[2];
    }
    inputTractogram->ResampleLinear(minSpacing/5);

    mitk::FiberBundle::Pointer all_anchor_tracts = mitk::FiberBundle::New(nullptr);
    if (!known_tract_masks.empty())
    {
      MITK_INFO << "Find known tracts via overlap match";
      boost::progress_display disp(known_tract_masks.size());
      for ( MaskType mask : known_tract_masks )
      {
        ++disp;
        std::streambuf *old = cout.rdbuf(); // <-- save
        std::stringstream ss;
        std::cout.rdbuf (ss.rdbuf());       // <-- redirect

        ItkUcharImgType::Pointer mask_image = std::get<0>(mask);
        std::string mask_name = std::get<1>(mask);
        mitk::FiberBundle::Pointer known_tract = inputTractogram->ExtractFiberSubset(mask_image, true, false, false, overlap, false);
        mitk::IOUtil::Save(known_tract, out_folder + "/anchor_tracts/" + mask_name + ".trk");

        all_anchor_tracts = all_anchor_tracts->AddBundle(known_tract);

        std::cout.rdbuf (old);              // <-- restore
      }
    }

    mitk::IOUtil::Save(all_anchor_tracts, out_folder + "/anchor_tracts/anchor_tractogram.trk");
    mitk::FiberBundle::Pointer remaining_tracts = inputTractogram->SubtractBundle(all_anchor_tracts);
    mitk::IOUtil::Save(remaining_tracts, out_folder + "/candidate_tracts/candidate_tractogram.trk");
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
