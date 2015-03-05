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


#include "mitkIShaderRepository.h"

#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include "usServiceReference.h"

#include "mitkTestingMacros.h"

#include <sstream>


int mitkShaderRepositoryTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ShaderRepository")

  us::ModuleContext* context = us::GetModuleContext();
  us::ServiceReference<mitk::IShaderRepository> serviceRef = context->GetServiceReference<mitk::IShaderRepository>();
  MITK_TEST_CONDITION_REQUIRED(serviceRef, "IShaderRepository service ref")

  mitk::IShaderRepository* shaderRepo = context->GetService(serviceRef);
  MITK_TEST_CONDITION_REQUIRED(shaderRepo, "Check non-empty IShaderRepositry")

  mitk::IShaderRepository::Shader::Pointer shader = shaderRepo->GetShader("mitkShaderLighting");
  MITK_TEST_CONDITION_REQUIRED(shader.IsNotNull(), "Non-null mitkShaderLighting shader")

  MITK_TEST_CONDITION(shader->GetName() == "mitkShaderLighting", "Shader name")
  MITK_TEST_CONDITION(!shader->GetMaterialXml().empty(), "Shader content")

   const std::string testShader =
      "<Material name=\"SmoothPlastic\" number_of_properties=\"1\" number_of_vertex_shaders=\"0\" number_of_fragment_shaders=\"0\">"
      "<Property name=\"ShinyProperty\">"
      "<Member name=\"DiffuseColor\" number_of_elements=\"3\" type=\"Double\" value=\"1.0 0.0 0.0\"/>"
      "</Property>"
      "</Material>";

  const std::size_t shaderCount = shaderRepo->GetShaders().size();

  std::stringstream testShaderStream(testShader);
  const std::string testShaderName = "SmoothPlastic";
  int id = shaderRepo->LoadShader(testShaderStream, testShaderName);
  MITK_TEST_CONDITION_REQUIRED(id > -1, "New shader id")
  MITK_TEST_CONDITION(shaderRepo->GetShaders().size() == shaderCount+1, "Shader count")
  mitk::IShaderRepository::Shader::Pointer shader2 = shaderRepo->GetShader(testShaderName);
  MITK_TEST_CONDITION_REQUIRED(shader2.IsNotNull(), "Non-null shader")
  MITK_TEST_CONDITION(shader2->GetId() == id, "Shader id")
  MITK_TEST_CONDITION(shader2->GetName() == testShaderName, "Shader name")
  mitk::IShaderRepository::Shader::Pointer shader3 = shaderRepo->GetShader(id);
  MITK_TEST_CONDITION_REQUIRED(shader3.IsNotNull(), "Non-null shader")
  MITK_TEST_CONDITION(shader3->GetId() == id, "Shader id")
  MITK_TEST_CONDITION(shader3->GetName() == testShaderName, "Shader name")

  MITK_TEST_CONDITION_REQUIRED(shaderRepo->UnloadShader(id), "Unload shader")
  MITK_TEST_CONDITION(shaderRepo->GetShader(testShaderName).IsNull(), "Null shader")
  MITK_TEST_CONDITION(shaderRepo->GetShader(id).IsNull(), "Null shader")

  MITK_TEST_END()
}
