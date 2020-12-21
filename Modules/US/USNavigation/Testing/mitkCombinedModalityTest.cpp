/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSCombinedModality.h"
#include <mitkVirtualTrackingDevice.h>
#include <mitkUSVideoDevice.h>
#include <mitkTestingMacros.h>
#include <mitkTrackingDeviceSource.h>

class mitkCombinedModalityTestClass
{
public:

  static bool CompareDoubles (double A, double B)
  {
    return std::abs(A - B) < 0.0001;
  }

  /*
  * \brief Creates a dummy combined modality.
  */
  static mitk::USCombinedModality::Pointer CreateModality()
  {
    mitk::USVideoDevice::Pointer  usDevice =  mitk::USVideoDevice::New("IllegalPath", "Manufacturer", "Model");
    mitk::VirtualTrackingDevice::Pointer tracker = mitk::VirtualTrackingDevice::New();
    tracker->AddTool("tool1");
    tracker->AddTool("tool2");
    mitk::TrackingDeviceSource::Pointer source = mitk::TrackingDeviceSource::New();
    source->SetTrackingDevice(tracker);
    source->Connect();
    source->StartTracking();
    return mitk::USCombinedModality::New(usDevice.GetPointer(), source.GetPointer(), false);
  }

  static void TestInstantiation()
  {
    mitk::USCombinedModality::Pointer combinedModality = CreateModality();
    MITK_TEST_CONDITION_REQUIRED(combinedModality.IsNotNull(), "CombinedModality should not be null after instantiation");
  }

  static void TestSerialization()
  {
    auto modality = CreateModality();

    mitk::AffineTransform3D::MatrixType matrix;
    matrix[0][0] = 1.1234;
    matrix[0][1] = 1.2234;
    matrix[0][2] = 1.3234;
    matrix[1][0] = 1.4234;
    matrix[1][1] = 1.5234;
    matrix[1][2] = 1.6234;
    matrix[2][0] = 1.7234;
    matrix[2][1] = 1.8234;
    matrix[2][2] = 1.9234;

    mitk::AffineTransform3D::OffsetType offset;
    offset[0] = 2.1234;
    offset[1] = 2.2234;
    offset[2] = 2.3234;

    mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
    transform->SetMatrix(matrix);
    transform->SetOffset(offset);

    modality->SetCalibration(transform);

    const auto xmlString = modality->SerializeCalibration();
    auto otherModality = CreateModality();
    otherModality->DeserializeCalibration(xmlString);
    auto calibration = otherModality->GetCalibration();
    const auto& otherMatrix = calibration->GetMatrix();
    const auto& otherOffset = calibration->GetOffset();

    bool identical = true;

    for (size_t i = 0; i < 3; ++i)
    {
      for (size_t j = 0; j < 3; ++j)
      {
        if (!CompareDoubles(matrix[i][j], otherMatrix[i][j]))
        {
          identical = false;
          break;
        }
      }

      if (!identical)
        break;
    }

    if (identical)
    {
      for (size_t i = 0; i < 3; ++i)
      {
        if (!CompareDoubles(offset[i], otherOffset[i]))
        {
          identical = false;
          break;
        }
      }
    }

    MITK_TEST_CONDITION_REQUIRED(identical, "Testing if deserialized calibration is identical to serialized one...");

    // test if invalid strings cause exceptions
    MITK_TEST_FOR_EXCEPTION(mitk::Exception, modality->DeserializeCalibration("invalid-string"));
    MITK_TEST_FOR_EXCEPTION(mitk::Exception, modality->DeserializeCalibration("<xml><test></xml>", false));
  }

  static void TestFilterPipeline()
  {
    /*mitk::USCombinedModality::Pointer combinedModality = mitkCombinedModalityTestClass::CreateModality();
    MITK_INFO << combinedModality->GetNavigationDataSource()->GetNameOfClass();
    MITK_TEST_CONDITION(strcmp(combinedModality->GetNavigationDataSource()->GetNameOfClass(), "TrackingDeviceSource") == 0,
                        "")*/

    mitk::USVideoDevice::Pointer  usDevice =  mitk::USVideoDevice::New("IllegalPath", "Manufacturer", "Model");
    mitk::VirtualTrackingDevice::Pointer tracker = mitk::VirtualTrackingDevice::New();
    tracker->AddTool("tool1");
    tracker->AddTool("tool2");
    mitk::TrackingDeviceSource::Pointer source = mitk::TrackingDeviceSource::New();
    source->SetTrackingDevice(tracker);
    source->Connect();
    source->StartTracking();
    mitk::USCombinedModality::Pointer modality = mitk::USCombinedModality::New(usDevice.GetPointer(), source.GetPointer(), false);

    MITK_TEST_CONDITION(source->GetOutput(0) == modality->GetNavigationDataSource()->GetOutput(0),
                        "Navigation data output of the Combined Modality should be the same as the source output as no filters are active.")

    modality->SetNumberOfSmoothingValues(2);

    mitk::NavigationDataSource::Pointer smoothingFilter = modality->GetNavigationDataSource();
    MITK_TEST_CONDITION(source->GetOutput(0) != smoothingFilter->GetOutput(0),
                        "Navigation data output of the Combined Modality should be different to the source output as smoothing filter is active.")

    modality->SetNumberOfSmoothingValues(0);
    MITK_TEST_CONDITION(source->GetOutput(0) == modality->GetNavigationDataSource()->GetOutput(0),
                        "Navigation data output of the Combined Modality should be the same as the source output again.")

    modality->SetDelayCount(5);
    MITK_TEST_CONDITION(source->GetOutput(0) != smoothingFilter->GetOutput(0),
                        "Navigation data output of the Combined Modality should be different to the source output as delay filter is active.")

    modality->SetDelayCount(0);
    MITK_TEST_CONDITION(source->GetOutput(0) == modality->GetNavigationDataSource()->GetOutput(0),
                        "Navigation data output of the Combined Modality should be the same as the source output again.")
  }
};

/**
* This function is testing methods of the class USDevice.
*/
int mitkCombinedModalityTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkCombinedModalityTest");

  mitkCombinedModalityTestClass::TestInstantiation();
  mitkCombinedModalityTestClass::TestSerialization();
  mitkCombinedModalityTestClass::TestFilterPipeline();

  MITK_TEST_END();
}
