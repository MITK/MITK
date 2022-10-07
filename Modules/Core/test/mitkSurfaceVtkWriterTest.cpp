/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkSurface.h>
#include <mitkTestingMacros.h>
#include <mitkTestManager.h>

#include <cstdio>
#include <string>
#include <fstream>

int mitkSurfaceVtkWriterTest(int, char *argv[])
{
  MITK_TEST_BEGIN("SurfaceVtkWriter")

  mitk::Surface::Pointer surface;

  try
  {
    MITK_TEST_OUTPUT(<< "Load surface from \"" << argv[1] << "\"")
    surface = mitk::IOUtil::Load<mitk::Surface>(argv[1]);
  }
  catch (...)
  {
  }

  if (surface.IsNotNull())
  {
    std::string path;

    try
    {
      std::ofstream stream;
      path = mitk::IOUtil::CreateTemporaryFile(stream, "XXXXXX.vtp");
    }
    catch (...)
    {
    }

    if (!path.empty())
    {
      try
      {
        MITK_TEST_OUTPUT(<< "Save surface as \"" << path << "\"")
        mitk::IOUtil::Save(surface, path);
      }
      catch (const std::exception& e)
      {
        std::remove(path.c_str());
        MITK_TEST_FAILED_MSG(<< "Caught exception: " << e.what() << " [FAILED]")
      }

      mitk::TestManager::GetInstance()->TestPassed();
    }
  }

  MITK_TEST_END()
}
