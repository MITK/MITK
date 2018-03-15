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

#include "mitkUSCombinedModality.h"
#include <mitkVirtualTrackingDevice.h>
#include <mitkUSVideoDevice.h>
#include <mitkTestingMacros.h>
#include <mitkTrackingDeviceSource.h>

class mitkCombinedModalityTestClass
{
public:

  /*
  * \brief Returns a reference string for serialized calibrations.
  */

  static std::string GetSerializedReference()
  {

    return std::string("<calibrations>\n<") + mitk::USCombinedModality::DefaultProbeIdentifier + mitk::USCombinedModality::ProbeAndDepthSeperator
                       + "0 M00=\"1.1234\" M01=\"1.2234\" M02=\"1.3234\" M10=\"1.4234\" M11=\"1.5234\" M12=\"1.6234\" M20=\"1.7234\" M21=\"1.8234\" M22=\"1.9234\" T0=\"2.1234\" T1=\"2.2234\" T2=\"2.3234\" />\n</calibrations>\n";
  }

  static bool CompareDoubles (double A, double B)
  {
    float diff = A - B;
    return (diff < 0.0001) && (-diff < 0.0001);
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
    mitk::USCombinedModality::Pointer modality = CreateModality();

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

    MITK_TEST_CONDITION_REQUIRED(modality->SerializeCalibration() == GetSerializedReference(), "Testing correct Serialization...");
  }

  static void TestDeserialization()
  {
    mitk::USCombinedModality::Pointer modality = CreateModality();
    modality->DeserializeCalibration(GetSerializedReference());
    mitk::AffineTransform3D::Pointer transform = modality->GetCalibration();

    mitk::AffineTransform3D::MatrixType matrix = transform->GetMatrix();
    mitk::AffineTransform3D::OffsetType offset = transform->GetOffset();;

    bool identical = true;

    if (! CompareDoubles(matrix[0][0], 1.1234)) identical = false;
    if (! CompareDoubles(matrix[0][1], 1.2234)) identical = false;
    if (! CompareDoubles(matrix[0][2], 1.3234)) identical = false;
    if (! CompareDoubles(matrix[1][0], 1.4234)) identical = false;
    if (! CompareDoubles(matrix[1][1], 1.5234)) identical = false;
    if (! CompareDoubles(matrix[1][2], 1.6234)) identical = false;
    if (! CompareDoubles(matrix[2][0], 1.7234)) identical = false;
    if (! CompareDoubles(matrix[2][1], 1.8234)) identical = false;
    if (! CompareDoubles(matrix[2][2], 1.9234)) identical = false;

    if (! CompareDoubles(offset[0], 2.1234)) identical = false;
    if (! CompareDoubles(offset[1], 2.2234)) identical = false;
    if (! CompareDoubles(offset[2], 2.3234)) identical = false;

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
  mitkCombinedModalityTestClass::TestDeserialization();
  mitkCombinedModalityTestClass::TestFilterPipeline();

  MITK_TEST_END();
}
