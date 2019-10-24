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

#include "mitkUSVideoDevice.h"
#include "mitkTestingMacros.h"
#include "mitkUSImageToUSImageFilter.h"

// START TESTFILER
// This is an specialization of the USImageToUSImageFIlter

  class TestUSFilter : public mitk::USImageToUSImageFilter
    {
    protected:
      TestUSFilter() : mitk::USImageToUSImageFilter(){};
      virtual ~TestUSFilter(){};

    public:
      mitkClassMacro(TestUSFilter, mitk::USImageToUSImageFilter);
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

    virtual void GenerateOutputInformation()
      {
      mitk::Image::Pointer inputImage  = (mitk::Image*) this->GetInput(0);
      mitk::Image::Pointer output      = (mitk::Image*) this->GetPrimaryOutput();
     if(inputImage.IsNull()) return;
     };


    void GenerateData()
    {
      mitk::USImage::Pointer ni = this->GetInput(0);
      mitk::USImage::Pointer result = mitk::USImage::New();

      try
      {
        mitk::Image::Pointer image = ni.GetPointer();
        result->Initialize(image);
        result->SetImportVolume(imgA->GetVolumeData()->GetData());
        mitk::USImageMetadata::Pointer meta = ni->GetMetadata();
        meta->SetDeviceComment("Test");
        result->SetMetadata(meta);
        SetNthOutput(0, result);
      }
      catch(mitk::Exception& e)
      {
        std::stringstream msg;
        msg << "Cannot open image for reading: " << e.GetDescription();
        MITK_TEST_FAILED_MSG(<<msg.str());
      }
    };
  };


// END TESTFILTER

class mitkUSPipelineTestClass
{

public:

  static void TestPipelineUS(std::string videoFilePath)
  {
   // Set up a pipeline
    mitk::USVideoDevice::Pointer videoDevice = mitk::USVideoDevice::New(videoFilePath, "Manufacturer", "Model");
    TestUSFilter::Pointer filter = TestUSFilter::New();
    videoDevice->Update();
    filter->SetInput(videoDevice->GetOutput());
    filter->Update();
    MITK_TEST_CONDITION_REQUIRED(videoDevice.IsNotNull(), "videoDevice should not be null after instantiation");

    //mitk::USImage::Pointer result = dynamic_cast<mitk::USImage *> (filter->GetOutput());
    mitk::USImage::Pointer result = filter->GetOutput(0);
    MITK_TEST_CONDITION_REQUIRED(result.IsNotNull(), "resulting images should not be null");
    std::string model = result->GetMetadata()->GetDeviceModel();
    MITK_TEST_CONDITION_REQUIRED(model.compare("Model") == 0   , "Resulting images should have their metadata set correctly");
  }

};

/**
* This function is setting up a pipeline and checks the output for validity.
*/
int mitkUSPipelineTest(int  argc , char* argv[])
{
  MITK_TEST_BEGIN("mitkUSPipelineTest");

  #ifdef WIN32 // Video file compression is currently only supported under windows.
   // US Pipelines need to be reworked :(
   //mitkUSPipelineTestClass::TestPipelineUS(argv[1]);
  #endif

  MITK_TEST_END();
}
