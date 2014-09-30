
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
#include <mitkImage.h>
#include <mitkSurface.h>
#include <mitkIOUtil.h>
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usServiceReference.h>
#include <usModuleRegistry.h>
#include <usModule.h>
#include <usServiceReference.h>
#include <mitkPythonService.h>
#include <mitkIPythonService.h>
#include <QmitkPythonSnippets.h>

namespace mitk
{

class CommonPythonTestSuite : public mitk::TestFixture
{

protected:
    mitk::PythonService* m_PythonService;
    mitk::Image::Pointer m_Image;
    mitk::Image::Pointer m_Image2D;
    mitk::Surface::Pointer m_Surface;
    QMap<QString, QString> m_Snippets;

public:

  void setUp()
  {
    //get the context of the python module
    us::Module* module = us::ModuleRegistry::GetModule("MitkPython");
    us::ModuleContext* context = module->GetModuleContext();
    //get the service which is generated in the PythonModuleActivator
    us::ServiceReference<mitk::IPythonService> serviceRef = context->GetServiceReference<mitk::IPythonService>();
    m_PythonService = dynamic_cast<mitk::PythonService*>( context->GetService(serviceRef) );

    m_Image = mitk::IOUtil::LoadImage(GetTestDataFilePath("Pic3D.nrrd"));
    m_Image2D = mitk::IOUtil::LoadImage(GetTestDataFilePath("Png2D-bw.png"));
    m_Surface = mitk::IOUtil::LoadSurface(GetTestDataFilePath("binary.stl"));

    QmitkPythonSnippets::LoadStringMap(QmitkPythonSnippets::DEFAULT_SNIPPET_FILE,m_Snippets);
  }

  void tearDown()
  {
    m_Image = NULL;
    m_Image2D = NULL;
    m_Surface = NULL;
  }
};

}
