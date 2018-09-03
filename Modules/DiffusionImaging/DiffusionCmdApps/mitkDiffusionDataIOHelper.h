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

#ifndef __mitkDiffusionDataIOHelper_h_
#define __mitkDiffusionDataIOHelper_h_

#include <MitkDiffusionCmdAppsExports.h>
#include <itkImage.h>
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkFiberBundle.h>
#include <mitkImageToItk.h>

#include <itksys/SystemTools.hxx>
#include <itkDirectory.h>
#include <mitkLocaleSwitch.h>

typedef itksys::SystemTools ist;

namespace mitk{

class MITKDIFFUSIONCMDAPPS_EXPORT DiffusionDataIOHelper
{
public:

  static std::vector< std::string > get_file_list(const std::string& path, const std::vector< std::string > extensions={".fib", ".trk"});

  static std::vector< mitk::FiberBundle::Pointer > load_fibs(const std::vector<std::string> files, std::vector<std::string>* filenames=nullptr);

  static std::vector< mitk::Image::Pointer > load_mitk_images(const std::vector<std::string> files, std::vector<std::string>* filenames=nullptr);

  template< class TYPE >
  static typename TYPE::Pointer load_itk_image(const std::string file)
  {
    mitk::LocaleSwitch localeSwitch("C");
    std::streambuf *old = cout.rdbuf(); // <-- save
    std::stringstream ss;
    std::cout.rdbuf (ss.rdbuf());       // <-- redirect
    std::vector< typename TYPE::Pointer > out;
    if (file.compare("")!=0 && itksys::SystemTools::FileExists(file))
    {
      mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(file);  
      if (image.IsNotNull())
      {
        typedef mitk::ImageToItk< TYPE > CasterType;
        typename CasterType::Pointer caster = CasterType::New();
        caster->SetInput(image);
        caster->Update();
        typename TYPE::Pointer itk_image = caster->GetOutput();
        std::cout.rdbuf (old);              // <-- restore
        MITK_INFO << "Loaded 1 image";
        return itk_image;
      }
    }
    std::cout.rdbuf (old);              // <-- restore    // <-- restore
    MITK_INFO << "Loaded 0 images";
    return nullptr;
  }

  template< class TYPE >
  static std::vector< typename TYPE::Pointer > load_itk_images(const std::vector<std::string> files, std::vector<std::string>* filenames=nullptr)
  {
    mitk::LocaleSwitch localeSwitch("C");
    std::streambuf *old = cout.rdbuf(); // <-- save
    std::stringstream ss;
    std::cout.rdbuf (ss.rdbuf());       // <-- redirect
    std::vector< typename TYPE::Pointer > out;
    for (auto f : files)
    {
      if (itksys::SystemTools::FileExists(f, true))
      {
        mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(f);
        if (image.IsNotNull())
        {
          typedef mitk::ImageToItk< TYPE > CasterType;
          typename CasterType::Pointer caster = CasterType::New();
          caster->SetInput(image);
          caster->Update();
          typename TYPE::Pointer itk_image = caster->GetOutput();

          out.push_back(itk_image);
          if (filenames!=nullptr)
            filenames->push_back(f);
        }
      }
      else if (itksys::SystemTools::PathExists(f))
      {
        if (!f.empty() && f.back() != '/')
          f += "/";

        auto list = get_file_list(f, {".nrrd",".nii.gz",".nii"});
        for (auto file : list)
        {
          mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(file);
          if (image.IsNotNull())
          {
            typedef mitk::ImageToItk< TYPE > CasterType;
            typename CasterType::Pointer caster = CasterType::New();
            caster->SetInput(image);
            caster->Update();
            typename TYPE::Pointer itk_image = caster->GetOutput();

            out.push_back(itk_image);
            if (filenames!=nullptr)
              filenames->push_back(file);
          }
        }
      }
    }

    std::cout.rdbuf (old);              // <-- restore
    MITK_INFO << "Loaded " << out.size() << " images";
    return out;
  }

};

}

#endif //__mitkDiffusionDataIOHelper_h_

