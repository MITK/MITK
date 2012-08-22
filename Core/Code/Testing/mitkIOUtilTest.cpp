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
#include <mitkIOUtil.h>

int mitkIOUtilTest(int  argc , char* argv[])
{
    // always start with this!
    MITK_TEST_BEGIN("mitkIOUtilTest")
            MITK_TEST_CONDITION_REQUIRED( argc >= 4, "Testing if input parameters are set.");

    std::string pathToImage = argv[1];
    std::string pathToPointSet = argv[2];
    std::string pathToSurface = argv[3];

    mitk::Image::Pointer img1 = mitk::IOUtil::LoadImage(pathToImage);
    MITK_TEST_CONDITION( img1.IsNotNull(), "Testing if image 1 could be loaded");
    mitk::PointSet::Pointer pointset = mitk::IOUtil::LoadPointSet(pathToPointSet);
    MITK_TEST_CONDITION( pointset.IsNotNull(), "Testing if pointset could be loaded");
    mitk::Surface::Pointer surface = mitk::IOUtil::LoadSurface(pathToSurface);
    MITK_TEST_CONDITION( surface.IsNotNull(), "Testing if surface could be loaded");

    MITK_TEST_CONDITION(mitk::IOUtil::SaveImage(img1, "/media/hdd/thomasHdd/Pictures/MITK-Data/diffpic3d.nrrd"), "Testing if the image could be saved");
    MITK_TEST_CONDITION(mitk::IOUtil::SavePointSet(pointset, "/media/hdd/thomasHdd/Pictures/MITK-Data/diffpointset.mps"), "Testing if the pointset could be saved");
    MITK_TEST_CONDITION(mitk::IOUtil::SaveSurface(surface, "/media/hdd/thomasHdd/Pictures/MITK-Data/diffsurface.stl"), "Testing if the surface could be saved");
            // always end with this!
            MITK_TEST_END();
}
