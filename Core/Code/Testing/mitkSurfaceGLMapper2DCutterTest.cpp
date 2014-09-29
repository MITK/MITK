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

//MITK
#include "mitkTestingMacros.h"
#include "mitkRenderingTestHelper.h"
#include <mitkNodePredicateDataType.h>
#include "mitkCoreServices.h"

#include "mitkISurfaceCutter.h"
#include "mitkISurfaceCutterFactory.h"
#include "mitkSurfaceCutterCGAL.h"
#include "mitkSurfaceCutterVtk.h"

#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include "usServiceReference.h"
#include "usServiceProperties.h"

//VTK
#include <vtkRegressionTestImage.h>

#include <mitkSurface.h>
#include <vtkPolyData.h>


template<class CutterType>
class SimpleSurfaceCutterFactory: public mitk::ISurfaceCutterFactory
{
public:
    virtual ~SimpleSurfaceCutterFactory() {}

    /** Create the surface cutter instance
    */
    mitk::ISurfaceCutter* createSurfaceCutter() const { 
        return new CutterType; 
    }
};


int mitkSurfaceGLMapper2DCutterTest(int argc, char* argv[])
{
    SimpleSurfaceCutterFactory<mitk::SurfaceCutterVtk> vtkFactory;
    SimpleSurfaceCutterFactory<mitk::SurfaceCutterCGAL> cgalFactory;
    std::vector<mitk::ISurfaceCutterFactory*> factories = { &vtkFactory, &cgalFactory };

  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkSurfaceGLMapper2DCutterTest")

  for (int i = 0; i < factories.size(); ++i) {
      us::ServiceProperties props;
      props[us::ServiceConstants::SERVICE_RANKING()] = i + 1;
      us::GetModuleContext()->RegisterService<mitk::ISurfaceCutterFactory>(factories[i], props);

      mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);
      renderingHelper.GetDataStorage()->GetNode(mitk::NodePredicateDataType::New("Surface"))->SetProperty("color", mitk::ColorProperty::New(255.0f, 0.0f, 0.0f));

      MITK_TEST_CONDITION(renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "CompareRenderWindowAgainstReference test result positive?");

      //####################
      //Use this to generate a reference screenshot or save the file.
      //(Only in your local version of the test!)
      if (false)
      {
          renderingHelper.SaveReferenceScreenShot("output.png");
      }
  }

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper

  //####################
  MITK_TEST_END();
}
