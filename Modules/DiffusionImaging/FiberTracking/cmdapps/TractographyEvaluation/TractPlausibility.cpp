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

using namespace std;
typedef itksys::SystemTools ist;
typedef itk::Image<unsigned char, 3>    ItkUcharImgType;
typedef std::tuple< ItkUcharImgType::Pointer, std::string > MaskType;

void CreateFolderStructure(const std::string& path)
{
  if (ist::PathExists(path))
    ist::RemoveADirectory(path);

  ist::MakeDirectory(path);
  ist::MakeDirectory(path + "/known_tracts/");
  ist::MakeDirectory(path + "/plausible_tracts/");
  ist::MakeDirectory(path + "/implausible_tracts/");
}

ItkUcharImgType::Pointer LoadItkMaskImage(const std::string& filename)
{
  mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(filename)[0].GetPointer());
  ItkUcharImgType::Pointer itkMask = ItkUcharImgType::New();
  mitk::CastToItkImage(img, itkMask);
  return itkMask;
}

std::vector< MaskType > get_file_list(const std::string& path)
{
  std::vector< MaskType > mask_list;

  itk::Directory::Pointer dir = itk::Directory::New();

  if (dir->Load(path.c_str()))
  {
    int n = dir->GetNumberOfFiles();
    for (int r = 0; r < n; r++)
    {
      const char *filename = dir->GetFile(r);

      if (std::rand()%3==0 && ist::GetFilenameWithoutExtension(filename)!="CC")
      {
        MITK_INFO << "Dismissing " << ist::GetFilenameWithoutExtension(filename);
        continue;
      }

      std::string ext = ist::GetFilenameExtension(filename);
      if (ext==".nii" || ext==".nii.gz" || ext==".nrrd")
      {
        MITK_INFO << "Loading " << ist::GetFilenameWithoutExtension(filename);
        streambuf *old = cout.rdbuf(); // <-- save
        stringstream ss;
        std::cout.rdbuf (ss.rdbuf());       // <-- redirect

        MaskType m(LoadItkMaskImage(path + '/' + filename), ist::GetFilenameWithoutExtension(filename));
        mask_list.push_back(m);

        std::cout.rdbuf (old);              // <-- restore
      }
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

  parser.setTitle("Tract Plausibility");
  parser.setCategory("Fiber Tracking Evaluation");
  parser.setDescription("");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input:", "input tractogram (.fib, vtk ascii file format)", us::Any(), false);
  parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output root", us::Any(), false);
  parser.addArgument("reference_mask_folder", "m", mitkCommandLineParser::String, "Reference Mask Folder:", "reference masks of known bundles", false);
  parser.addArgument("gray_matter_mask", "gm", mitkCommandLineParser::String, "", "", false);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  string fibFile = us::any_cast<string>(parsedArgs["input"]);
  string gray_matter_mask = us::any_cast<string>(parsedArgs["gray_matter_mask"]);
  string reference_mask_folder = us::any_cast<string>(parsedArgs["reference_mask_folder"]);
  string outRoot = us::any_cast<string>(parsedArgs["out"]);

  try
  {
    CreateFolderStructure(outRoot);

    std::vector< MaskType > known_tract_masks = get_file_list(reference_mask_folder);
    if (known_tract_masks.empty())
      return EXIT_FAILURE;

    ItkUcharImgType::Pointer gm_image = LoadItkMaskImage(gray_matter_mask);

    mitk::FiberBundle::Pointer inputTractogram = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(fibFile)[0].GetPointer());

    // filter gray matter fibers
    mitk::FiberBundle::Pointer not_gm_fibers = inputTractogram->ExtractFiberSubset(gm_image, false, true, true);
    mitk::IOUtil::Save(not_gm_fibers, outRoot + "/implausible_tracts/no_gm_endings.trk");
    inputTractogram = inputTractogram->ExtractFiberSubset(gm_image, false, false, true);

    // resample fibers
    float minSpacing = 1;
    if(std::get<0>(known_tract_masks.at(0))->GetSpacing()[0]<std::get<0>(known_tract_masks.at(0))->GetSpacing()[1] && std::get<0>(known_tract_masks.at(0))->GetSpacing()[0]<std::get<0>(known_tract_masks.at(0))->GetSpacing()[2])
      minSpacing = std::get<0>(known_tract_masks.at(0))->GetSpacing()[0];
    else if (std::get<0>(known_tract_masks.at(0))->GetSpacing()[1] < std::get<0>(known_tract_masks.at(0))->GetSpacing()[2])
      minSpacing = std::get<0>(known_tract_masks.at(0))->GetSpacing()[1];
    else
      minSpacing = std::get<0>(known_tract_masks.at(0))->GetSpacing()[2];
    inputTractogram->ResampleLinear(minSpacing/5);

    // find known tracts via overlap match
    mitk::FiberBundle::Pointer all_known_tracts = nullptr;
    for ( MaskType mask : known_tract_masks )
    {
      ItkUcharImgType::Pointer mask_image = std::get<0>(mask);
      std::string mask_name = std::get<1>(mask);
      mitk::FiberBundle::Pointer known_tract = inputTractogram->ExtractFiberSubset(mask_image, true, false, false, 0.9, false);
      mitk::IOUtil::Save(known_tract, outRoot + "/known_tracts/" + mask_name + ".trk");

      if (all_known_tracts.IsNull())
        all_known_tracts = mitk::FiberBundle::New(known_tract->GetFiberPolyData());
      else
        all_known_tracts = all_known_tracts->AddBundle(known_tract);
    }
    mitk::IOUtil::Save(all_known_tracts, outRoot + "/known_tracts/all_known_tracts.trk");
    mitk::IOUtil::Save(TransformToMRtrixSpace(all_known_tracts), outRoot + "/known_tracts/all_known_tracts_mrtrixspace.fib");

    mitk::FiberBundle::Pointer remaining_tracts = inputTractogram->SubtractBundle(all_known_tracts);
    mitk::IOUtil::Save(remaining_tracts, outRoot + "/plausible_tracts/remaining_tracts.trk");
    mitk::IOUtil::Save(TransformToMRtrixSpace(remaining_tracts), outRoot + "/plausible_tracts/remaining_tracts_mrtrixspace.fib");
    MITK_INFO << "step_size: " << minSpacing/5;
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
