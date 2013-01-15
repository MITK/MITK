MITK_CREATE_MODULE_TESTS()

mitkAddCustomModuleTest(mitkKinectReconstructionTest_LegoPhantom mitkKinectReconstructionTest
                        /media/hdd/thomasHdd/CalibrationFIles/RGB_calib.xml #camera intrinsics
                        /media/hdd/thomasHdd/Pictures/KinectLegoPhantom/FlippedImages/Kinect_Lego_Phantom_MF0_IT0_10Images_DistanceImage-f.nrrd #kinect distance image
                        /media/hdd/thomasHdd/Pictures/KinectLegoPhantom/Surface/Surface_RGB_calib_ohneDistanz.vtp #ground truth surface
)

#${MITK_DATA_DIR}/ToF-Data/
#mitkAddCustomModuleTest(mitkToFImageDownsamplingFilterTest_20 mitkToFImageDownsamplingFilterTest PMDCamCube2_MF0_IT0_20Images_DistanceImage.pic)
#mitkAddCustomModuleTest(mitkToFImageDownsamplingFilterTest_1 mitkToFImageDownsamplingFilterTest PMDCamCube2_MF0_IT0_1Images_DistanceImage.pic)
