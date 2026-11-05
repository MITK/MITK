/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarFigure.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itkObjectFactoryBase.h>

class mitkPlanarFigureSceneSerializerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPlanarFigureSceneSerializerTestSuite);
  MITK_TEST(EveryType_HasOneSerializer);
  CPPUNIT_TEST_SUITE_END();

public:
  void EveryType_HasOneSerializer()
  {
    for (const std::string typeId : { "line", "arrow", "angle", "four_point_angle", "cross", "circle", "ellipse",
                                      "double_ellipse", "rectangle", "polygon", "path", "subdivision_polygon",
                                      "bezier_curve" })
    {
      const auto serializerName = std::string(mitk::CreatePlanarFigure(typeId)->GetNameOfClass()) + "Serializer";
      const auto serializers = itk::ObjectFactoryBase::CreateAllInstance(serializerName.c_str());

      CPPUNIT_ASSERT_EQUAL_MESSAGE(serializerName, std::size_t(1), serializers.size());
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureSceneSerializer)
