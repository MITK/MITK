/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPythonContext.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <memory>

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

  // The affine handed to nibabel has to agree with the index-to-world transform
  // of the image itself, which pins down the convention of the direction matrix,
  // the order of the axes, and the conversion from LPS to RAS.
  void TestRasAffineFollowsGeometry()
  {
    m_Context->Execute(
      "angle = np.radians(30.0)\n"
      "rotation = np.array([[np.cos(angle), -np.sin(angle), 0.0], [np.sin(angle), np.cos(angle), 0.0], [0.0, 0.0, 1.0]])\n"
      "coronal = np.array([[1.0, 0.0, 0.0], [0.0, 0.0, -1.0], [0.0, 1.0, 0.0]])\n"
      "image = mitk.Image(np.zeros((5, 7, 11), dtype=np.float32), spacing=(0.7, 0.9, 2.5),\n"
      "                   origin=(-12.5, 33.0, 101.25), direction=rotation @ coronal)\n"
      "geometry = image.get_geometry(time_step=0)\n"
      "affine = voxtell_ras_affine(image)\n"
      "geometry_ok = True\n"
      "for index in [(0, 0, 0), (3, 1, 2), (10, 6, 4)]:\n"
      "    world = mitk.Point3D()\n"
      "    geometry.index_to_world(mitk.Point3D(*index), world)\n"
      "    ras = affine @ np.array([*index, 1.0])\n"
      "    expected = np.array([-world[0], -world[1], world[2]])\n"
      "    geometry_ok = geometry_ok and np.allclose(ras[:3], expected)\n");

    CPPUNIT_ASSERT_MESSAGE("The affine should map voxel indices to the same RAS positions as the geometry of the image",
                           m_Context->GetVariableAsBool("geometry_ok").value_or(false));
  }

private:
  std::unique_ptr<mitk::PythonContext> m_Context;
};

MITK_TEST_SUITE_REGISTRATION(mitkVoxTellBridge)
