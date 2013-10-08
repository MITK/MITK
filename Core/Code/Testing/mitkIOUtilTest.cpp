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

#include "mitkTestingMacros.h"
#include <mitkTestingConfig.h>
#include <mitkIOUtil.h>
#include <mitkImageGenerator.h>

#include <itksys/SystemTools.hxx>

void TestTempMethods()
{
  std::string tmpPath = mitk::IOUtil::GetTempPath();
  MITK_TEST_CONDITION_REQUIRED(!tmpPath.empty(), "Get temporary path")

  std::ofstream tmpFile;
  std::string tmpFilePath = mitk::IOUtil::CreateTemporaryFile(tmpFile);
  MITK_TEST_CONDITION_REQUIRED(tmpFile && tmpFile.is_open(), "Temp file stream");
  MITK_TEST_CONDITION_REQUIRED(tmpFilePath.size() > tmpPath.size(), "Temp file path size")
  MITK_TEST_CONDITION(tmpFilePath.substr(0, tmpPath.size()) == tmpPath, "Temp file is in temp path")

  tmpFile.close();
  MITK_TEST_CONDITION(std::remove(tmpFilePath.c_str()) == 0, "Remove temp file")

  std::string programPath = mitk::IOUtil::GetProgramPath();
  MITK_TEST_CONDITION_REQUIRED(!programPath.empty(), "Get program file path")
  std::ofstream tmpFile2;
  std::string tmpFilePath2 = mitk::IOUtil::CreateTemporaryFile(tmpFile2, "my-XXXXXX", programPath);
  MITK_TEST_CONDITION_REQUIRED(tmpFile2 && tmpFile2.is_open(), "Temp file 2 stream")
  MITK_TEST_CONDITION_REQUIRED(tmpFilePath2.size() > programPath.size(), "Temp file 2 path size")
  MITK_TEST_CONDITION(tmpFilePath2.substr(0, programPath.size()) == programPath, "Temp file 2 is in program path")
  tmpFile2.close();
  MITK_TEST_CONDITION(std::remove(tmpFilePath2.c_str()) == 0, "Remove temp file 2")

  std::ofstream tmpFile3;
  std::string tmpFilePath3 = mitk::IOUtil::CreateTemporaryFile(tmpFile3, std::ios_base::binary,
                                                               "my-XXXXXX.TXT", programPath);
  MITK_TEST_CONDITION_REQUIRED(tmpFile3 && tmpFile3.is_open(), "Temp file 3 stream")
  MITK_TEST_CONDITION_REQUIRED(tmpFilePath3.size() > programPath.size(), "Temp file 3 path size")
  MITK_TEST_CONDITION(tmpFilePath3.substr(0, programPath.size()) == programPath, "Temp file 23 is in program path")
  MITK_TEST_CONDITION(tmpFilePath3.substr(tmpFilePath3.size() - 13, 3) == "my-", "Temp file 3 starts with 'my-'")
  MITK_TEST_CONDITION(tmpFilePath3.substr(tmpFilePath3.size() - 4) == ".TXT", "Temp file 3 ends with '.TXT'")
  tmpFile3.close();
  //MITK_TEST_CONDITION(std::remove(tmpFilePath3.c_str()) == 0, "Remove temp file 3")

  MITK_TEST_FOR_EXCEPTION(mitk::Exception&, mitk::IOUtil::CreateTemporaryFile(tmpFile2, "XX"))

  std::string tmpDir = mitk::IOUtil::CreateTemporaryDirectory();
  MITK_TEST_CONDITION_REQUIRED(tmpDir.size() > tmpPath.size(), "Temp dir size")
  MITK_TEST_CONDITION(tmpDir.substr(0, tmpPath.size()) == tmpPath, "Temp dir is in temp path")
  MITK_TEST_CONDITION(itksys::SystemTools::RemoveADirectory(tmpDir.c_str()), "Remove temp dir")

  std::string tmpDir2 = mitk::IOUtil::CreateTemporaryDirectory("my-XXXXXX", programPath);
  MITK_TEST_CONDITION_REQUIRED(tmpDir2.size() > programPath.size(), "Temp dir 2 size")
  MITK_TEST_CONDITION(tmpDir2.substr(0, programPath.size()) == programPath, "Temp dir 2 is in temp path")
  MITK_TEST_CONDITION(itksys::SystemTools::RemoveADirectory(tmpDir2.c_str()), "Remove temp dir 2")
}

int mitkIOUtilTest(int  argc , char* argv[])
{
    // always start with this!
    MITK_TEST_BEGIN("mitkIOUtilTest");
    MITK_TEST_CONDITION_REQUIRED( argc >= 4, "Testing if input parameters are set.");

    TestTempMethods();

    std::string pathToImage = argv[1];
    std::string pathToPointSet = argv[2];
    std::string pathToSurface = argv[3];

    mitk::Image::Pointer img1 = mitk::IOUtil::LoadImage(pathToImage);
    MITK_TEST_CONDITION( img1.IsNotNull(), "Testing if image 1 could be loaded");
    mitk::PointSet::Pointer pointset = mitk::IOUtil::LoadPointSet(pathToPointSet);
    MITK_TEST_CONDITION( pointset.IsNotNull(), "Testing if pointset could be loaded");
    mitk::Surface::Pointer surface = mitk::IOUtil::LoadSurface(pathToSurface);
    MITK_TEST_CONDITION( surface.IsNotNull(), "Testing if surface could be loaded");

    std::string outDir = MITK_TEST_OUTPUT_DIR;
    std::string imagePath = outDir+"/diffpic3d.nrrd";
    std::string imagePath2 = outDir+"/diffpic3d.nii.gz";
    std::string pointSetPath = outDir + "/diffpointset.mps";
    std::string surfacePath = outDir + "/diffsurface.stl";
    std::string pointSetPathWithDefaultExtension = outDir + "/diffpointset2.mps";
    std::string pointSetPathWithoutDefaultExtension = outDir + "/diffpointset2.xXx";

    // the cases where no exception should be thrown
    MITK_TEST_CONDITION(mitk::IOUtil::SaveImage(img1, imagePath), "Testing if the image could be saved");
    MITK_TEST_CONDITION(mitk::IOUtil::SaveBaseData(img1.GetPointer(), imagePath2), "Testing if the image could be saved");
    MITK_TEST_CONDITION(mitk::IOUtil::SavePointSet(pointset, pointSetPath), "Testing if the pointset could be saved");
    MITK_TEST_CONDITION(mitk::IOUtil::SaveSurface(surface, surfacePath), "Testing if the surface could be saved");

    // test if defaultextension is inserted if no extension is present
    MITK_TEST_CONDITION(mitk::IOUtil::SavePointSet(pointset, pointSetPathWithoutDefaultExtension.c_str()), "Testing if the pointset could be saved");

    // test if exception is thrown as expected on unknown extsension
    MITK_TEST_FOR_EXCEPTION(mitk::Exception, mitk::IOUtil::SaveSurface(surface,"testSurface.xXx"));
    //load data which does not exist
    MITK_TEST_FOR_EXCEPTION(mitk::Exception, mitk::IOUtil::LoadImage("fileWhichDoesNotExist.nrrd"));

    //delete the files after the test is done
    remove(imagePath.c_str());
    remove(pointSetPath.c_str());
    remove(surfacePath.c_str());
    //remove the pointset with default extension and not the one without
    remove(pointSetPathWithDefaultExtension.c_str());

    mitk::Image::Pointer relativImage = mitk::ImageGenerator::GenerateGradientImage<float>(4,4,4,1);
    mitk::IOUtil::SaveImage(relativImage, "tempfile.nrrd");
    try
    {
      mitk::IOUtil::LoadImage("tempfile.nrrd");
      MITK_TEST_CONDITION(true, "Temporary image is in right place");
      remove("tempfile.nrrd");
    }
    catch (mitk::Exception &e)
    {
      MITK_TEST_CONDITION(false, "Temporary image is in right place");
    }

    MITK_TEST_END();
}
