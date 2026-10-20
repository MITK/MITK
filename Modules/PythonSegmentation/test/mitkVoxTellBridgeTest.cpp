/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkGeometry3D.h>
#include <mitkPythonContext.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkVoxTellTool.h>

#include <nlohmann/json.hpp>

#include <array>
#include <cmath>
#include <memory>
#include <vector>

class mitkVoxTellBridgeTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkVoxTellBridgeTestSuite);
  MITK_TEST(TestOrientationRoundTrip);
  MITK_TEST(TestRasAffineFollowsGeometry);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override
  {
    m_Context = std::make_unique<mitk::PythonContext>();
    m_Context->Activate();
    m_Context->ExecuteFile(MITK_VOXTELL_BRIDGE_FILE);
  }

  void tearDown() override
  {
    m_Context.reset();
  }

  // Every orientation an image can have (all permutations of the axes, each
  // flipped or not) must be undone exactly, for the image as well as for masks.
  void TestOrientationRoundTrip()
  {
    m_Context->Execute(
      "import itertools\n"
      "generator = np.random.default_rng(0)\n"
      "volume = generator.integers(0, 255, size=(5, 7, 11)).astype(np.float32)\n" // (Z, Y, X), no two extents equal
      "labels = generator.integers(0, 3, size=volume.shape).astype(np.uint16)\n"
      "round_trip_ok = True\n"
      "case_count = 0\n"
      "for permutation in itertools.permutations(range(3)):\n"
      "    for flips in itertools.product((1, -1), repeat=3):\n"
      "        ornt = np.array([[permutation[i], flips[i]] for i in range(3)], dtype=np.float64)\n"
      "        xyz = volume.transpose(2, 1, 0)\n"
      "        reoriented = voxtell_apply_orientation(xyz, ornt)\n"
      "        for i in range(3):\n" // axis i of the input has to land on axis permutation[i]
      "            round_trip_ok = round_trip_ok and reoriented.shape[permutation[i]] == xyz.shape[i]\n"
      "        restored = voxtell_apply_orientation(reoriented, voxtell_inverse_orientation(ornt)).transpose(2, 1, 0)\n"
      "        round_trip_ok = round_trip_ok and np.array_equal(restored, volume)\n"
      "        masks = np.stack([\n"
      "            voxtell_apply_orientation((labels == value).transpose(2, 1, 0), ornt).transpose(2, 1, 0).astype(np.uint8)\n"
      "            for value in (1, 2)])\n"
      "        for index, value in enumerate((1, 2)):\n"
      "            target = np.full(volume.shape, 9, dtype=np.uint8)\n" // stale content must not survive
      "            voxtell_write_mask(masks, index, ornt, target)\n"
      "            round_trip_ok = round_trip_ok and np.array_equal(target, (labels == value).astype(np.uint8))\n"
      "        case_count += 1\n");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("All 48 orientations should have been checked", 48,
                                 m_Context->GetVariableAsInt("case_count").value_or(0));
    CPPUNIT_ASSERT_MESSAGE("An orientation was not undone exactly",
                           m_Context->GetVariableAsBool("round_trip_ok").value_or(false));
  }

  // The tool hands VoxTell the index-to-world matrix of the image, from which
  // the bridge derives the affine for nibabel. Both have to agree with the
  // geometry itself, which pins down the order of the axes and the conversion
  // from LPS to RAS. The geometry is rotated, flipped and offset.
  void TestRasAffineFollowsGeometry()
  {
    const double angle = 30.0 * 3.14159265358979323846 / 180.0;
    const double c = std::cos(angle);
    const double s = std::sin(angle);

    // A rotation about z times a coronal orientation; the columns are the
    // directions of the index axes.
    const double direction[3][3] = { { c, 0.0, s }, { s, 0.0, -c }, { 0.0, 1.0, 0.0 } };
    const double spacing[3] = { 0.7, 0.9, 2.5 };

    mitk::AffineTransform3D::MatrixType matrix;

    for (unsigned int row = 0; row < 3; ++row)
    {
      for (unsigned int column = 0; column < 3; ++column)
        matrix[row][column] = direction[row][column] * spacing[column];
    }

    mitk::AffineTransform3D::OutputVectorType offset;
    offset[0] = -12.5;
    offset[1] = 33.0;
    offset[2] = 101.25;

    auto transform = mitk::AffineTransform3D::New();
    transform->SetMatrix(matrix);
    transform->SetOffset(offset);

    auto geometry = mitk::Geometry3D::New();
    geometry->SetIndexToWorldTransform(transform);

    const auto rows = mitk::VoxTell::IndexToWorld(geometry);
    const std::vector<std::array<double, 3>> indices = { { 0.0, 0.0, 0.0 }, { 3.0, 1.0, 2.0 }, { 10.0, 6.0, 4.0 } };
    auto expectedRas = nlohmann::json::array();

    for (const auto& index : indices)
    {
      mitk::Point3D indexPoint;

      for (unsigned int axis = 0; axis < 3; ++axis)
        indexPoint[axis] = index[axis];

      mitk::Point3D world;
      geometry->IndexToWorld(indexPoint, world);

      for (unsigned int row = 0; row < 3; ++row)
      {
        double mapped = rows[row][3].get<double>();

        for (unsigned int column = 0; column < 3; ++column)
          mapped += rows[row][column].get<double>() * index[column];

        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The matrix should map an index like the geometry", world[row], mapped, 1e-9);
      }

      expectedRas.push_back({ -world[0], -world[1], world[2] });
    }

    m_Context->Execute(
      "index_to_world = np.array(" + rows.dump() + ")\n"
      "indices = " + nlohmann::json(indices).dump() + "\n"
      "expected = np.array(" + expectedRas.dump() + ")\n"
      "affine = voxtell_ras_affine(index_to_world)\n"
      "geometry_ok = all(np.allclose((affine @ np.array([*index, 1.0]))[:3], ras) for index, ras in zip(indices, expected))\n");

    CPPUNIT_ASSERT_MESSAGE("The affine should map voxel indices to the same RAS positions as the geometry of the image",
                           m_Context->GetVariableAsBool("geometry_ok").value_or(false));
  }

private:
  std::unique_ptr<mitk::PythonContext> m_Context;
};

MITK_TEST_SUITE_REGISTRATION(mitkVoxTellBridge)
