/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkDataNode.h>
#include <mitkImage.h>
#include <mitkImageVtkMapper2D.h>
#include <mitkLookupTable.h>
#include <mitkRenderingModeProperty.h>
#include <mitkVtkResliceInterpolationProperty.h>

#include <vtkImageReslice.h>

#include <cmath>
#include <numbers>

class mitkImageVtkMapper2DApplyLookupTableTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkImageVtkMapper2DApplyLookupTableTestSuite);
  MITK_TEST(Multilabel_SetsNearestAndLookupTableColor);
  MITK_TEST(LeavingMultilabel_RestoresCubicForRotatedImage);
  MITK_TEST(LeavingMultilabel_RestoresNearestForUnrotatedImage);
  MITK_TEST(SwitchingBetweenOtherTypes_KeepsInterpolation);
  MITK_TEST(NodeWithoutImage_GetsOnlyLookupTable);
  CPPUNIT_TEST_SUITE_END();

private:
  static mitk::DataNode::Pointer CreateImageNode(bool rotated)
  {
    auto image = mitk::Image::New();
    const unsigned int dimensions[] = { 2, 2, 2 };
    image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dimensions);

    if (rotated)
    {
      const auto radians = 30.0 * std::numbers::pi / 180.0;
      auto transform = mitk::AffineTransform3D::New();
      auto matrix = transform->GetMatrix();
      matrix(0, 0) = std::cos(radians);
      matrix(0, 1) = -std::sin(radians);
      matrix(1, 0) = std::sin(radians);
      matrix(1, 1) = std::cos(radians);
      transform->SetMatrix(matrix);
      image->GetGeometry()->SetIndexToWorldTransform(transform);
    }

    auto node = mitk::DataNode::New();
    node->SetData(image);
    return node;
  }

  static mitk::LookupTable::Pointer CreateLookupTable(mitk::LookupTable::LookupTableType type)
  {
    auto lookupTable = mitk::LookupTable::New();
    lookupTable->SetType(type);
    return lookupTable;
  }

  static int GetInterpolation(const mitk::DataNode* node)
  {
    auto property = dynamic_cast<mitk::VtkResliceInterpolationProperty*>(node->GetProperty("reslice interpolation"));
    CPPUNIT_ASSERT(nullptr != property);
    return property->GetInterpolation();
  }

  static int GetRenderingMode(const mitk::DataNode* node)
  {
    auto property = dynamic_cast<mitk::RenderingModeProperty*>(node->GetProperty("Image Rendering.Mode"));
    CPPUNIT_ASSERT(nullptr != property);
    return property->GetRenderingMode();
  }

public:
  void Multilabel_SetsNearestAndLookupTableColor()
  {
    auto node = CreateImageNode(true);
    node->SetProperty("reslice interpolation", mitk::VtkResliceInterpolationProperty::New(VTK_RESLICE_LINEAR));

    mitk::ImageVtkMapper2D::ApplyLookupTable(node, CreateLookupTable(mitk::LookupTable::MULTILABEL));

    CPPUNIT_ASSERT_EQUAL(VTK_RESLICE_NEAREST, GetInterpolation(node));
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(mitk::RenderingModeProperty::LOOKUPTABLE_COLOR), GetRenderingMode(node));
  }

  void LeavingMultilabel_RestoresCubicForRotatedImage()
  {
    auto node = CreateImageNode(true);
    mitk::ImageVtkMapper2D::ApplyLookupTable(node, CreateLookupTable(mitk::LookupTable::MULTILABEL));

    mitk::ImageVtkMapper2D::ApplyLookupTable(node, CreateLookupTable(mitk::LookupTable::GRAYSCALE));

    CPPUNIT_ASSERT_EQUAL(VTK_RESLICE_CUBIC, GetInterpolation(node));
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR), GetRenderingMode(node));
  }

  void LeavingMultilabel_RestoresNearestForUnrotatedImage()
  {
    auto node = CreateImageNode(false);
    mitk::ImageVtkMapper2D::ApplyLookupTable(node, CreateLookupTable(mitk::LookupTable::MULTILABEL));

    mitk::ImageVtkMapper2D::ApplyLookupTable(node, CreateLookupTable(mitk::LookupTable::JET));

    CPPUNIT_ASSERT_EQUAL(VTK_RESLICE_NEAREST, GetInterpolation(node));
  }

  void SwitchingBetweenOtherTypes_KeepsInterpolation()
  {
    auto node = CreateImageNode(true);
    mitk::ImageVtkMapper2D::ApplyLookupTable(node, CreateLookupTable(mitk::LookupTable::GRAYSCALE));
    node->SetProperty("reslice interpolation", mitk::VtkResliceInterpolationProperty::New(VTK_RESLICE_LINEAR));

    mitk::ImageVtkMapper2D::ApplyLookupTable(node, CreateLookupTable(mitk::LookupTable::JET));

    CPPUNIT_ASSERT_EQUAL(VTK_RESLICE_LINEAR, GetInterpolation(node));
  }

  void NodeWithoutImage_GetsOnlyLookupTable()
  {
    auto node = mitk::DataNode::New();

    mitk::ImageVtkMapper2D::ApplyLookupTable(node, CreateLookupTable(mitk::LookupTable::MULTILABEL));

    CPPUNIT_ASSERT(nullptr != node->GetProperty("LookupTable"));
    CPPUNIT_ASSERT(nullptr == node->GetProperty("reslice interpolation"));
    CPPUNIT_ASSERT(nullptr == node->GetProperty("Image Rendering.Mode"));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkImageVtkMapper2DApplyLookupTable)
