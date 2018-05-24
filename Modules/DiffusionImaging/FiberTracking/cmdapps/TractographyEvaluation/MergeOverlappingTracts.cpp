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
#include <mitkLexicalCast.h>
#include <itksys/SystemTools.hxx>
#include <itkDirectory.h>
#include <mitkFiberBundle.h>
#include <vtkTransformPolyDataFilter.h>
#include <fstream>
#include <chrono>
#include <boost/progress.hpp>
#include <itkTractsToFiberEndingsImageFilter.h>
#include <itkTractDensityImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageFileWriter.h>

typedef itksys::SystemTools ist;
typedef itk::Image<unsigned char, 3>    ItkFloatImgType;
typedef itk::Image<unsigned int, 3>    ItkUIntImgType;

std::vector< std::string > get_file_list(const std::string& path, std::vector< std::string > extensions={".fib", ".trk"})
{
  std::vector< std::string > file_list;
  itk::Directory::Pointer dir = itk::Directory::New();

  if (dir->Load(path.c_str()))
  {
    int n = dir->GetNumberOfFiles();
    for (int r = 0; r < n; r++)
    {
      const char *filename = dir->GetFile(r);
      std::string ext = ist::GetFilenameExtension(filename);
      for (auto e : extensions)
      {
        if (ext==e)
        {
          file_list.push_back(path + '/' + filename);
          break;
        }
      }
    }
  }
  return file_list;
}

/*!
\brief
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Merge Overlapping Tracts");
  parser.setCategory("Fiber Tracking Evaluation");
  parser.setDescription("");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("in", "i", mitkCommandLineParser::InputFile, "Input Folder:", "input folder", us::Any(), false);
  parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output Folder:", "output folder", us::Any(), false);
  parser.addArgument("overlap", "", mitkCommandLineParser::Float, "Overlap threshold:", "Tracts with overlap larger than this threshold are merged", 0.8, false);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string input_folder = us::any_cast<std::string>(parsedArgs["in"]);
  std::string out_folder = us::any_cast<std::string>(parsedArgs["out"]);

  float overlap = 0.8;
  if (parsedArgs.count("overlap"))
    overlap = us::any_cast<float>(parsedArgs["overlap"]);

  try
  {
    if (!ist::PathExists(out_folder))
      ist::MakeDirectory(out_folder);

    std::vector< std::string > fib_files = get_file_list(input_folder, {".fib", ".trk", ".tck"});
    if (fib_files.empty())
      return EXIT_FAILURE;

    std::streambuf *old = cout.rdbuf(); // <-- save
    std::stringstream ss;
    std::cout.rdbuf (ss.rdbuf());       // <-- redirect

    std::vector< mitk::FiberBundle::Pointer > fibs;
    for (std::string f : fib_files)
    {
      mitk::FiberBundle::Pointer fib = mitk::IOUtil::Load<mitk::FiberBundle>(f);
      fibs.push_back(fib);
    }

    mitk::FiberBundle::Pointer combined = mitk::FiberBundle::New();
    combined = combined->AddBundles(fibs);

    itk::TractsToFiberEndingsImageFilter< ItkFloatImgType >::Pointer endings = itk::TractsToFiberEndingsImageFilter< ItkFloatImgType >::New();
    endings->SetFiberBundle(combined);
    endings->SetUpsamplingFactor(0.25);
    endings->Update();
    ItkFloatImgType::Pointer ref_image = endings->GetOutput();

    std::cout.rdbuf (old);              // <-- restore

    for (int its = 0; its<3; its++)
    {
      std::streambuf *old = cout.rdbuf(); // <-- save
      std::stringstream ss;
      std::cout.rdbuf (ss.rdbuf());       // <-- redirect

      std::vector< ItkFloatImgType::Pointer > mask_images;
      for (auto fib : fibs)
      {
        itk::TractDensityImageFilter< ItkFloatImgType >::Pointer masks = itk::TractDensityImageFilter< ItkFloatImgType >::New();
        masks->SetInputImage(ref_image);
        masks->SetBinaryOutput(true);
        masks->SetFiberBundle(fib);
        masks->SetUseImageGeometry(true);
        masks->Update();
        mask_images.push_back(masks->GetOutput());
      }

      int r=0;
      vnl_matrix< int > mat; mat.set_size(mask_images.size(), mask_images.size()); mat.fill(0);
      for (auto m1 : mask_images)
      {
        float max_overlap = overlap;
        int c = 0;
        for (auto m2 : mask_images)
        {
          if (c<=r)
          {
            ++c;
            continue;
          }

          itk::ImageRegionConstIterator<ItkFloatImgType> it1(m1, m1->GetLargestPossibleRegion());
          itk::ImageRegionConstIterator<ItkFloatImgType> it2(m2, m2->GetLargestPossibleRegion());

          unsigned int c1 = 0;
          unsigned int c2 = 0;
          unsigned int intersect = 0;
          while( !it1.IsAtEnd() )
          {
            if( it1.Get()>0 && it2.Get()>0)
              ++intersect;
            if(it1.Get()>0)
              ++c1;
            if(it2.Get()>0)
              ++c2;
            ++it1;
            ++it2;
          }

          if ( (float)intersect/c1>max_overlap )
          {
            max_overlap = (float)intersect/c1;
            mat.put(r,c, 1);
          }
          if ( (float)intersect/c2>max_overlap )
          {
            max_overlap = (float)intersect/c2;
            mat.put(r,c, 1);
          }
          ++c;
        }

        ++r;
      }

      std::vector< mitk::FiberBundle::Pointer > out_fibs;
      std::vector< bool > used;
      for (unsigned int i=0; i<fibs.size(); i++)
        used.push_back(false);
      for (unsigned int r=0; r<mask_images.size(); r++)
      {
        if (used.at(r))
          continue;

        mitk::FiberBundle::Pointer fib = fibs.at(r);
        for (unsigned int c=r+1; c<mask_images.size(); c++)
        {
          if (mat.get(r,c)>0)
          {
            fib = fib->AddBundle(fibs.at(c));
            MITK_INFO << c;
            used[c] = true;
          }
        }

        out_fibs.push_back(fib);
      }
      std::cout.rdbuf (old);              // <-- restore

      MITK_INFO << fibs.size() << " --> " << out_fibs.size();

      if (fibs.size()==out_fibs.size())
        break;
      fibs = out_fibs;
    }

    int c = 0;
    for (auto fib : fibs)
    {
      std::streambuf *old = cout.rdbuf(); // <-- save
      std::stringstream ss;
      std::cout.rdbuf (ss.rdbuf());       // <-- redirect
      mitk::IOUtil::Save(fib, out_folder + "/bundle_" + boost::lexical_cast<std::string>(c) + ".trk");
      std::cout.rdbuf (old);              // <-- restore

      ++c;
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
