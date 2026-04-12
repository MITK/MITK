/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCommon.h>
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkIOUtil.h>
#include <mitkPythonContext.h>

class mitkPythonTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPythonTestSuite);
  MITK_TEST(TestExecuteAndGetVariable);
  MITK_TEST(TestBindImageToPython);
  MITK_TEST(TestImageAsNumpyDirect);
  MITK_TEST(TestImageAsNumpyAccessor);
  MITK_TEST(TestPythonContextExclusivity);
  MITK_TEST(TestCreateImage);
  MITK_TEST(TestImageGeometry);
  MITK_TEST(TestImageFromNumpy);
  MITK_TEST(TestImageArrayProtocol);
  MITK_TEST(TestImageLoadSaveRoundtrip);
  MITK_TEST(TestIOUtilReaderPreferences);
  MITK_TEST(TestImageConstructor);
  MITK_TEST(TestPointVectorTypes);
  MITK_TEST(TestPixelType);
  MITK_TEST(TestAutoloadModules);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp()
  {
  }

  void TestExecuteAndGetVariable()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    pythonContext.Execute("result = 5 + 5");
    auto result = pythonContext.GetVariableAsInt("result");

    CPPUNIT_ASSERT_MESSAGE("Variable 'result' should exist", result.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Result should be 10", 10, result.value());
  }

  void TestBindImageToPython()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    auto image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Pic3D.nrrd"));

    try
    {
      pythonContext.BindImage(image, "test_image");
    }
    catch (const mitk::Exception& e)
    {
      MITK_ERROR << e.GetDescription();
      CPPUNIT_FAIL("Error in binding MITK image to Python");
    }

    CPPUNIT_ASSERT_MESSAGE("test_image should exist in context", pythonContext.HasVariable("test_image"));

    pythonContext.Execute("dims = test_image.get_dimension()");
    auto dims = pythonContext.GetVariableAsInt("dims");

    CPPUNIT_ASSERT_MESSAGE("Variable 'dims' should exist", dims.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image should have 3 dimensions", 3, dims.value());
  }

  void TestImageAsNumpyDirect()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    auto image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Pic3D.nrrd"));
    pythonContext.BindImage(image, "test_image");

    // Default: as_numpy() returns a writeable view (numpy convention).
    // The direct-access path used by default holds no MITK accessor lock,
    // so there is no cost to defaulting to writeable. For a read-only
    // view, callers should use the .array property or np.asarray(img),
    // or pass writeable=False explicitly (see below).
    pythonContext.Execute(
      "import gc\n"
      "arr_default = test_image.as_numpy()\n"
      "default_writeable = arr_default.flags.writeable\n"
      "del arr_default\n"
    );

    auto defaultWriteable = pythonContext.GetVariableAsBool("default_writeable");
    CPPUNIT_ASSERT_MESSAGE("default_writeable should exist", defaultWriteable.has_value());
    CPPUNIT_ASSERT_MESSAGE("as_numpy() default should be writeable", defaultWriteable.value());

    // Explicit read-only: as_numpy(writeable=False) returns a non-writeable view.
    pythonContext.Execute(
      "arr_ro = test_image.as_numpy(writeable=False)\n"
      "ro_shape = arr_ro.shape\n"
      "ro_writeable = arr_ro.flags.writeable\n"
      "del arr_ro\n"
    );

    auto roWriteable = pythonContext.GetVariableAsBool("ro_writeable");
    CPPUNIT_ASSERT_MESSAGE("ro_writeable should exist", roWriteable.has_value());
    CPPUNIT_ASSERT_MESSAGE("as_numpy(writeable=False) must not be writeable", !roWriteable.value());

    // Explicit writeable: round-trip a write through as_numpy(writeable=True).
    pythonContext.Execute(
      "arr_rw = test_image.as_numpy(writeable=True)\n"
      "rw_writeable = arr_rw.flags.writeable\n"
      "original_value = int(arr_rw[0, 0, 0])\n"
      "arr_rw[0, 0, 0] = 42\n"
      "modified_value = int(arr_rw[0, 0, 0])\n"
      "del arr_rw\n"
    );

    auto rwWriteable = pythonContext.GetVariableAsBool("rw_writeable");
    CPPUNIT_ASSERT_MESSAGE("rw_writeable should exist", rwWriteable.has_value());
    CPPUNIT_ASSERT_MESSAGE("as_numpy(writeable=True) must be writeable", rwWriteable.value());

    auto modifiedValue = pythonContext.GetVariableAsInt("modified_value");
    CPPUNIT_ASSERT_MESSAGE("modified_value should exist", modifiedValue.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Modified value should be 42", 42, modifiedValue.value());
  }

  void TestImageAsNumpyAccessor()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    auto image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Pic3D.nrrd"));
    pythonContext.BindImage(image, "test_image");

    // Accessor read-only: uses ImageReadAccessor, returns non-writeable view.
    pythonContext.Execute(
      "arr_acc_ro = test_image.as_numpy(use_accessor=True, writeable=False)\n"
      "acc_ro_writeable = arr_acc_ro.flags.writeable\n"
      "del arr_acc_ro\n"
    );

    auto accRoWriteable = pythonContext.GetVariableAsBool("acc_ro_writeable");
    CPPUNIT_ASSERT_MESSAGE("acc_ro_writeable should exist", accRoWriteable.has_value());
    CPPUNIT_ASSERT_MESSAGE("as_numpy(use_accessor=True, writeable=False) must not be writeable",
                           !accRoWriteable.value());

    // Accessor writeable: uses ImageWriteAccessor, verify write round-trip.
    pythonContext.Execute(
      "arr_acc_rw = test_image.as_numpy(use_accessor=True, writeable=True)\n"
      "acc_rw_writeable = arr_acc_rw.flags.writeable\n"
      "arr_acc_rw[0, 0, 0] = 99\n"
      "acc_modified = int(arr_acc_rw[0, 0, 0])\n"
      "del arr_acc_rw\n"
    );

    auto accRwWriteable = pythonContext.GetVariableAsBool("acc_rw_writeable");
    CPPUNIT_ASSERT_MESSAGE("acc_rw_writeable should exist", accRwWriteable.has_value());
    CPPUNIT_ASSERT_MESSAGE("as_numpy(use_accessor=True, writeable=True) must be writeable",
                           accRwWriteable.value());

    auto accModified = pythonContext.GetVariableAsInt("acc_modified");
    CPPUNIT_ASSERT_MESSAGE("acc_modified should exist", accModified.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Accessor-written value should be 99", 99, accModified.value());
  }

  void TestPythonContextExclusivity()
  {
    mitk::PythonContext pythonContext_1;
    mitk::PythonContext pythonContext_2;

    try
    {
      pythonContext_1.Execute("test_var_context_1 = 10");
      pythonContext_2.Execute("test_var_context_2 = 20");
    }
    catch (const mitk::Exception& e)
    {
      MITK_ERROR << e.GetDescription();
      CPPUNIT_FAIL("Error in executing commands in Python");
    }

    CPPUNIT_ASSERT_MESSAGE("test_var_context_1 should be found in context 1",
                           pythonContext_1.HasVariable("test_var_context_1"));
    CPPUNIT_ASSERT_MESSAGE("test_var_context_2 should not be found in context 1",
                           !pythonContext_1.HasVariable("test_var_context_2"));
    CPPUNIT_ASSERT_MESSAGE("test_var_context_2 should be found in context 2",
                           pythonContext_2.HasVariable("test_var_context_2"));
    CPPUNIT_ASSERT_MESSAGE("test_var_context_1 should not be found in context 2",
                           !pythonContext_2.HasVariable("test_var_context_1"));
  }

  void TestCreateImage()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    pythonContext.Execute(
      "img = mitk.Image()\n"
      "img.initialize('float32', [64, 64, 64])\n"
      "create_ndim = img.get_dimension()\n"
      "create_dim0 = img.get_dimension(0)\n"
      "arr = img.as_numpy()\n"
      "create_shape_ok = arr.shape == (64, 64, 64)\n"
      "create_dtype_ok = arr.dtype == np.float32\n"
    );

    auto ndim = pythonContext.GetVariableAsInt("create_ndim");
    CPPUNIT_ASSERT_MESSAGE("create_ndim should exist", ndim.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image should have 3 dimensions", 3, ndim.value());

    auto dim0 = pythonContext.GetVariableAsInt("create_dim0");
    CPPUNIT_ASSERT_MESSAGE("create_dim0 should exist", dim0.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Dimension 0 should be 64", 64, dim0.value());

    auto shapeOk = pythonContext.GetVariableAsBool("create_shape_ok");
    CPPUNIT_ASSERT_MESSAGE("create_shape_ok should exist", shapeOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("as_numpy() shape should be (64, 64, 64)", shapeOk.value());

    auto dtypeOk = pythonContext.GetVariableAsBool("create_dtype_ok");
    CPPUNIT_ASSERT_MESSAGE("create_dtype_ok should exist", dtypeOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("as_numpy() dtype should be float32", dtypeOk.value());
  }

  void TestImageGeometry()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    pythonContext.Execute(
      "img = mitk.Image()\n"
      "img.initialize('float32', [10, 20, 30])\n"
      "\n"
      "# shape is reversed wrt MITK dim order (numpy convention)\n"
      "geom_shape_ok = img.shape == (30, 20, 10)\n"
      "geom_ndim = img.ndim\n"
      "geom_dtype_ok = img.dtype == np.float32\n"
      "\n"
      "# default spacing\n"
      "geom_default_spacing_ok = img.spacing == (1.0, 1.0, 1.0)\n"
      "img.spacing = (0.5, 0.7, 1.5)\n"
      "geom_spacing_x = img.spacing[0]\n"
      "geom_spacing_y = img.spacing[1]\n"
      "geom_spacing_z = img.spacing[2]\n"
      "\n"
      "# origin\n"
      "img.origin = (10.0, 20.0, 30.0)\n"
      "geom_origin_x = img.origin[0]\n"
      "geom_origin_y = img.origin[1]\n"
      "geom_origin_z = img.origin[2]\n"
      "\n"
      "# direction: default identity\n"
      "geom_dir_identity = np.allclose(img.direction, np.eye(3))\n"
      "# set flipped LPS variant\n"
      "flipped = np.diag([-1.0, -1.0, 1.0])\n"
      "img.direction = flipped\n"
      "geom_dir_flipped = np.allclose(img.direction, flipped)\n"
      "# spacing must be preserved across direction set\n"
      "geom_spacing_preserved = img.spacing == (0.5, 0.7, 1.5)\n"
      "\n"
      "# time geometry\n"
      "geom_time_steps = img.time_steps\n"
      "geom_has_time_geom = img.time_geometry is not None\n"
      "geom_tg_count = img.time_geometry.count_time_steps()\n"
      "geom_has_geom_t0 = img.get_geometry(time_step=0) is not None\n"
    );

    auto shapeOk = pythonContext.GetVariableAsBool("geom_shape_ok");
    CPPUNIT_ASSERT_MESSAGE("geom_shape_ok should exist", shapeOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("shape should be (30, 20, 10)", shapeOk.value());

    auto ndim = pythonContext.GetVariableAsInt("geom_ndim");
    CPPUNIT_ASSERT_MESSAGE("geom_ndim should exist", ndim.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("ndim should be 3", 3, ndim.value());

    auto dtypeOk = pythonContext.GetVariableAsBool("geom_dtype_ok");
    CPPUNIT_ASSERT_MESSAGE("geom_dtype_ok should exist", dtypeOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("dtype should be float32", dtypeOk.value());

    auto defaultSpacingOk = pythonContext.GetVariableAsBool("geom_default_spacing_ok");
    CPPUNIT_ASSERT_MESSAGE("geom_default_spacing_ok should exist", defaultSpacingOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("default spacing should be (1, 1, 1)", defaultSpacingOk.value());

    auto sx = pythonContext.GetVariableAsDouble("geom_spacing_x");
    auto sy = pythonContext.GetVariableAsDouble("geom_spacing_y");
    auto sz = pythonContext.GetVariableAsDouble("geom_spacing_z");
    CPPUNIT_ASSERT_MESSAGE("spacing x should exist", sx.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("spacing x", 0.5, sx.value(), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("spacing y", 0.7, sy.value(), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("spacing z", 1.5, sz.value(), 1e-9);

    auto ox = pythonContext.GetVariableAsDouble("geom_origin_x");
    auto oy = pythonContext.GetVariableAsDouble("geom_origin_y");
    auto oz = pythonContext.GetVariableAsDouble("geom_origin_z");
    CPPUNIT_ASSERT_MESSAGE("origin x should exist", ox.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("origin x", 10.0, ox.value(), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("origin y", 20.0, oy.value(), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("origin z", 30.0, oz.value(), 1e-9);

    auto dirIdentity = pythonContext.GetVariableAsBool("geom_dir_identity");
    CPPUNIT_ASSERT_MESSAGE("geom_dir_identity should exist", dirIdentity.has_value());
    CPPUNIT_ASSERT_MESSAGE("default direction should be identity", dirIdentity.value());

    auto dirFlipped = pythonContext.GetVariableAsBool("geom_dir_flipped");
    CPPUNIT_ASSERT_MESSAGE("geom_dir_flipped should exist", dirFlipped.has_value());
    CPPUNIT_ASSERT_MESSAGE("direction should match flipped LPS", dirFlipped.value());

    auto spacingPreserved = pythonContext.GetVariableAsBool("geom_spacing_preserved");
    CPPUNIT_ASSERT_MESSAGE("geom_spacing_preserved should exist", spacingPreserved.has_value());
    CPPUNIT_ASSERT_MESSAGE("spacing must survive direction change", spacingPreserved.value());

    auto timeSteps = pythonContext.GetVariableAsInt("geom_time_steps");
    CPPUNIT_ASSERT_MESSAGE("geom_time_steps should exist", timeSteps.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("time_steps should be 1", 1, timeSteps.value());

    auto hasTimeGeom = pythonContext.GetVariableAsBool("geom_has_time_geom");
    CPPUNIT_ASSERT_MESSAGE("geom_has_time_geom should exist", hasTimeGeom.has_value());
    CPPUNIT_ASSERT_MESSAGE("time_geometry should not be None", hasTimeGeom.value());

    auto tgCount = pythonContext.GetVariableAsInt("geom_tg_count");
    CPPUNIT_ASSERT_MESSAGE("geom_tg_count should exist", tgCount.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("time_geometry count should be 1", 1, tgCount.value());

    auto hasGeomT0 = pythonContext.GetVariableAsBool("geom_has_geom_t0");
    CPPUNIT_ASSERT_MESSAGE("geom_has_geom_t0 should exist", hasGeomT0.has_value());
    CPPUNIT_ASSERT_MESSAGE("get_geometry(time_step=0) should not be None", hasGeomT0.value());
  }

  void TestImageFromNumpy()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    pythonContext.Execute(
      "arr = np.arange(2 * 3 * 4, dtype=np.float32).reshape(4, 3, 2)\n"
      "\n"
      "# from_numpy classmethod\n"
      "img = mitk.Image.from_numpy(arr, spacing=(0.5, 1.0, 2.0), origin=(1.0, 2.0, 3.0))\n"
      "fn_shape_ok = img.shape == arr.shape\n"
      "fn_spacing_ok = img.spacing == (0.5, 1.0, 2.0)\n"
      "fn_origin_ok = img.origin == (1.0, 2.0, 3.0)\n"
      "fn_data_ok = np.array_equal(img.as_numpy(), arr)\n"
      "\n"
      "# constructor overload\n"
      "img2 = mitk.Image(arr, spacing=(0.5, 1.0, 2.0))\n"
      "fn_ctor_shape_ok = img2.shape == arr.shape\n"
      "fn_ctor_spacing_ok = img2.spacing == (0.5, 1.0, 2.0)\n"
    );

    auto shapeOk = pythonContext.GetVariableAsBool("fn_shape_ok");
    CPPUNIT_ASSERT_MESSAGE("fn_shape_ok should exist", shapeOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("from_numpy shape should match", shapeOk.value());

    auto spacingOk = pythonContext.GetVariableAsBool("fn_spacing_ok");
    CPPUNIT_ASSERT_MESSAGE("fn_spacing_ok should exist", spacingOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("from_numpy spacing should match", spacingOk.value());

    auto originOk = pythonContext.GetVariableAsBool("fn_origin_ok");
    CPPUNIT_ASSERT_MESSAGE("fn_origin_ok should exist", originOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("from_numpy origin should match", originOk.value());

    auto dataOk = pythonContext.GetVariableAsBool("fn_data_ok");
    CPPUNIT_ASSERT_MESSAGE("fn_data_ok should exist", dataOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("from_numpy data round-trip should match", dataOk.value());

    auto ctorShapeOk = pythonContext.GetVariableAsBool("fn_ctor_shape_ok");
    CPPUNIT_ASSERT_MESSAGE("fn_ctor_shape_ok should exist", ctorShapeOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("Image(arr) shape should match", ctorShapeOk.value());

    auto ctorSpacingOk = pythonContext.GetVariableAsBool("fn_ctor_spacing_ok");
    CPPUNIT_ASSERT_MESSAGE("fn_ctor_spacing_ok should exist", ctorSpacingOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("Image(arr) spacing should match", ctorSpacingOk.value());
  }

  void TestImageArrayProtocol()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    pythonContext.Execute(
      "img = mitk.Image(np.zeros((4, 5, 6), dtype=np.uint8))\n"
      "arr = np.asarray(img)\n"
      "ap_shape_ok = arr.shape == (4, 5, 6)\n"
      "ap_dtype_ok = arr.dtype == np.uint8\n"
      "\n"
      "arr2 = np.asarray(img, dtype=np.float32)\n"
      "ap_conv_dtype_ok = arr2.dtype == np.float32\n"
    );

    auto shapeOk = pythonContext.GetVariableAsBool("ap_shape_ok");
    CPPUNIT_ASSERT_MESSAGE("ap_shape_ok should exist", shapeOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("np.asarray shape should be (4, 5, 6)", shapeOk.value());

    auto dtypeOk = pythonContext.GetVariableAsBool("ap_dtype_ok");
    CPPUNIT_ASSERT_MESSAGE("ap_dtype_ok should exist", dtypeOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("np.asarray dtype should be uint8", dtypeOk.value());

    auto convOk = pythonContext.GetVariableAsBool("ap_conv_dtype_ok");
    CPPUNIT_ASSERT_MESSAGE("ap_conv_dtype_ok should exist", convOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("np.asarray dtype conversion to float32 should work", convOk.value());
  }

  void TestImageLoadSaveRoundtrip()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    pythonContext.Execute(
      "import tempfile, os\n"
      "from pathlib import Path\n"
      "\n"
      "arr = np.arange(60, dtype=np.float32).reshape(3, 4, 5)\n"
      "img = mitk.Image(arr, spacing=(0.5, 0.7, 1.1))\n"
      "\n"
      "tmp = tempfile.mkdtemp()\n"
      "path = os.path.join(tmp, 'out.nrrd')\n"
      "img.save(path)\n"
      "\n"
      "# classmethod load\n"
      "loaded = mitk.Image.load(path)\n"
      "ls_data_ok = np.array_equal(np.asarray(loaded), arr)\n"
      "ls_spacing_ok = loaded.spacing == (0.5, 0.7, 1.1)\n"
      "\n"
      "# constructor from str path\n"
      "loaded2 = mitk.Image(path)\n"
      "ls_ctor_str_ok = loaded2.spacing == (0.5, 0.7, 1.1)\n"
      "\n"
      "# constructor from pathlib.Path\n"
      "loaded3 = mitk.Image(Path(path))\n"
      "ls_ctor_path_ok = loaded3.spacing == (0.5, 0.7, 1.1)\n"
      "\n"
      "# IOUtil.load\n"
      "results = mitk.IOUtil.load(path)\n"
      "ls_ioutil_count = len(results)\n"
      "ls_ioutil_data_ok = np.array_equal(np.asarray(results[0]), arr)\n"
      "\n"
      "# cleanup\n"
      "import shutil\n"
      "shutil.rmtree(tmp)\n"
    );

    auto dataOk = pythonContext.GetVariableAsBool("ls_data_ok");
    CPPUNIT_ASSERT_MESSAGE("ls_data_ok should exist", dataOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("Image.load data should match", dataOk.value());

    auto spacingOk = pythonContext.GetVariableAsBool("ls_spacing_ok");
    CPPUNIT_ASSERT_MESSAGE("ls_spacing_ok should exist", spacingOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("Image.load spacing should match", spacingOk.value());

    auto ctorStrOk = pythonContext.GetVariableAsBool("ls_ctor_str_ok");
    CPPUNIT_ASSERT_MESSAGE("ls_ctor_str_ok should exist", ctorStrOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("Image(str_path) spacing should match", ctorStrOk.value());

    auto ctorPathOk = pythonContext.GetVariableAsBool("ls_ctor_path_ok");
    CPPUNIT_ASSERT_MESSAGE("ls_ctor_path_ok should exist", ctorPathOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("Image(Path) spacing should match", ctorPathOk.value());

    auto ioutilCount = pythonContext.GetVariableAsInt("ls_ioutil_count");
    CPPUNIT_ASSERT_MESSAGE("ls_ioutil_count should exist", ioutilCount.has_value());
    CPPUNIT_ASSERT_MESSAGE("IOUtil.load should return at least 1 result", ioutilCount.value() >= 1);

    auto ioutilDataOk = pythonContext.GetVariableAsBool("ls_ioutil_data_ok");
    CPPUNIT_ASSERT_MESSAGE("ls_ioutil_data_ok should exist", ioutilDataOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("IOUtil.load data should match", ioutilDataOk.value());
  }

  void TestIOUtilReaderPreferences()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    pythonContext.Execute(
      "import tempfile, os, shutil\n"
      "\n"
      "arr = np.arange(60, dtype=np.float32).reshape(3, 4, 5)\n"
      "img = mitk.Image(arr, spacing=(0.5, 0.7, 1.1))\n"
      "\n"
      "tmp = tempfile.mkdtemp()\n"
      "path = os.path.join(tmp, 'prefs.nrrd')\n"
      "img.save(path)\n"
      "\n"
      "def assert_loads(**kwargs):\n"
      "    results = mitk.IOUtil.load(path, **kwargs)\n"
      "    assert len(results) >= 1\n"
      "    assert np.array_equal(np.asarray(results[0]), arr)\n"
      "\n"
      "assert_loads(reader_preferences=None, reader_blacklist=None)\n"
      "assert_loads(reader_preferences=[], reader_blacklist=[])\n"
      "assert_loads(reader_preferences=['ITK NrrdImageIO'])\n"
      "assert_loads(reader_preferences=['NonExistentReader'])\n"
      "assert_loads(reader_blacklist=['NonExistentReader'])\n"
      "assert_loads(reader_preferences=['ITK NrrdImageIO'], reader_blacklist=['NonExistentReader'])\n"
      "assert_loads(reader_preferences=('ITK NrrdImageIO',), reader_blacklist=('NonExistentReader',))\n"
      "\n"
      "rp_all_ok = True\n"
      "\n"
      "shutil.rmtree(tmp)\n"
    );

    auto allOk = pythonContext.GetVariableAsBool("rp_all_ok");
    CPPUNIT_ASSERT_MESSAGE("rp_all_ok should exist", allOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("all reader_preferences/reader_blacklist combos should work", allOk.value());
  }

  void TestImageConstructor()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    pythonContext.Execute(
      "import tempfile, os, shutil\n"
      "from pathlib import Path\n"
      "\n"
      "# empty constructor\n"
      "empty = mitk.Image()\n"
      "ic_is_image = isinstance(empty, mitk.Image)\n"
      "ic_exact_type = type(empty) is mitk.Image\n"
      "\n"
      "# from numpy + geometry kwargs\n"
      "arr = np.zeros((2, 3, 4), dtype=np.uint8)\n"
      "img2 = mitk.Image(arr, spacing=(1.0, 2.0, 3.0))\n"
      "ic_isinstance2 = isinstance(img2, mitk.Image)\n"
      "ic_shape_ok = img2.shape == (2, 3, 4)\n"
      "ic_spacing_ok = img2.spacing == (1.0, 2.0, 3.0)\n"
      "\n"
      "# from path\n"
      "tmp = tempfile.mkdtemp()\n"
      "fpath = os.path.join(tmp, 'x.nrrd')\n"
      "img2.save(fpath)\n"
      "img3 = mitk.Image(fpath)\n"
      "img4 = mitk.Image(Path(fpath))\n"
      "ic_path_data_ok = np.array_equal(np.asarray(img3), arr)\n"
      "ic_pathlib_data_ok = np.array_equal(np.asarray(img4), arr)\n"
      "\n"
      "# NativeImage alias should not exist\n"
      "ic_no_native = not hasattr(mitk, 'NativeImage')\n"
      "\n"
      "shutil.rmtree(tmp)\n"
    );

    auto isImage = pythonContext.GetVariableAsBool("ic_is_image");
    CPPUNIT_ASSERT_MESSAGE("ic_is_image should exist", isImage.has_value());
    CPPUNIT_ASSERT_MESSAGE("empty Image() should be instance of mitk.Image", isImage.value());

    auto exactType = pythonContext.GetVariableAsBool("ic_exact_type");
    CPPUNIT_ASSERT_MESSAGE("ic_exact_type should exist", exactType.has_value());
    CPPUNIT_ASSERT_MESSAGE("type(Image()) should be exactly mitk.Image", exactType.value());

    auto isinstance2 = pythonContext.GetVariableAsBool("ic_isinstance2");
    CPPUNIT_ASSERT_MESSAGE("ic_isinstance2 should exist", isinstance2.has_value());
    CPPUNIT_ASSERT_MESSAGE("Image(arr) should be instance of mitk.Image", isinstance2.value());

    auto shapeOk = pythonContext.GetVariableAsBool("ic_shape_ok");
    CPPUNIT_ASSERT_MESSAGE("ic_shape_ok should exist", shapeOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("Image(arr) shape should be (2, 3, 4)", shapeOk.value());

    auto spacingOk = pythonContext.GetVariableAsBool("ic_spacing_ok");
    CPPUNIT_ASSERT_MESSAGE("ic_spacing_ok should exist", spacingOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("Image(arr) spacing should be (1, 2, 3)", spacingOk.value());

    auto pathDataOk = pythonContext.GetVariableAsBool("ic_path_data_ok");
    CPPUNIT_ASSERT_MESSAGE("ic_path_data_ok should exist", pathDataOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("Image(str_path) data should match", pathDataOk.value());

    auto pathlibDataOk = pythonContext.GetVariableAsBool("ic_pathlib_data_ok");
    CPPUNIT_ASSERT_MESSAGE("ic_pathlib_data_ok should exist", pathlibDataOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("Image(Path) data should match", pathlibDataOk.value());

    auto noNative = pythonContext.GetVariableAsBool("ic_no_native");
    CPPUNIT_ASSERT_MESSAGE("ic_no_native should exist", noNative.has_value());
    CPPUNIT_ASSERT_MESSAGE("mitk.NativeImage should not exist", noNative.value());
  }

  void TestPointVectorTypes()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    pythonContext.Execute(
      "p2 = mitk.Point2D(1.0, 2.0)\n"
      "pv_p2x = p2.x\n"
      "pv_p2y = p2.y\n"
      "\n"
      "p3 = mitk.Point3D(1.0, 2.0, 3.0)\n"
      "pv_p3z = p3.z\n"
      "\n"
      "v2 = mitk.Vector2D(4.0, 5.0)\n"
      "pv_v2x = v2.x\n"
      "\n"
      "v3 = mitk.Vector3D(4.0, 5.0, 6.0)\n"
      "pv_v3z = v3.z\n"
    );

    auto p2x = pythonContext.GetVariableAsDouble("pv_p2x");
    CPPUNIT_ASSERT_MESSAGE("pv_p2x should exist", p2x.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Point2D.x", 1.0, p2x.value(), 1e-9);

    auto p2y = pythonContext.GetVariableAsDouble("pv_p2y");
    CPPUNIT_ASSERT_MESSAGE("pv_p2y should exist", p2y.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Point2D.y", 2.0, p2y.value(), 1e-9);

    auto p3z = pythonContext.GetVariableAsDouble("pv_p3z");
    CPPUNIT_ASSERT_MESSAGE("pv_p3z should exist", p3z.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Point3D.z", 3.0, p3z.value(), 1e-9);

    auto v2x = pythonContext.GetVariableAsDouble("pv_v2x");
    CPPUNIT_ASSERT_MESSAGE("pv_v2x should exist", v2x.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Vector2D.x", 4.0, v2x.value(), 1e-9);

    auto v3z = pythonContext.GetVariableAsDouble("pv_v3z");
    CPPUNIT_ASSERT_MESSAGE("pv_v3z should exist", v3z.has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Vector3D.z", 6.0, v3z.value(), 1e-9);
  }

  void TestPixelType()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    pythonContext.Execute(
      "pt = mitk.make_pixel_type('float32')\n"
      "pt_ok = pt is not None\n"
    );

    auto ptOk = pythonContext.GetVariableAsBool("pt_ok");
    CPPUNIT_ASSERT_MESSAGE("pt_ok should exist", ptOk.has_value());
    CPPUNIT_ASSERT_MESSAGE("make_pixel_type('float32') should return non-None", ptOk.value());
  }

  void TestAutoloadModules()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    pythonContext.Execute(
      "modules = mitk.get_loaded_modules()\n"
      "al_has_core = 'MitkCore' in modules\n"
      "al_has_dicom = 'MitkDICOMImageIO' in modules\n"
      "al_has_ioext = 'MitkIOExt' in modules\n"
      "al_has_multilabel = 'MitkMultilabelIO' in modules\n"
      "al_module_count = len(modules)\n"
    );

    auto hasCore = pythonContext.GetVariableAsBool("al_has_core");
    CPPUNIT_ASSERT_MESSAGE("al_has_core should exist", hasCore.has_value());
    CPPUNIT_ASSERT_MESSAGE("MitkCore should be loaded", hasCore.value());

    auto hasDicom = pythonContext.GetVariableAsBool("al_has_dicom");
    CPPUNIT_ASSERT_MESSAGE("al_has_dicom should exist", hasDicom.has_value());
    CPPUNIT_ASSERT_MESSAGE("MitkDICOMImageIO should be auto-loaded", hasDicom.value());

    auto hasIoExt = pythonContext.GetVariableAsBool("al_has_ioext");
    CPPUNIT_ASSERT_MESSAGE("al_has_ioext should exist", hasIoExt.has_value());
    CPPUNIT_ASSERT_MESSAGE("MitkIOExt should be auto-loaded", hasIoExt.value());

    auto hasMultilabel = pythonContext.GetVariableAsBool("al_has_multilabel");
    CPPUNIT_ASSERT_MESSAGE("al_has_multilabel should exist", hasMultilabel.has_value());
    CPPUNIT_ASSERT_MESSAGE("MitkMultilabelIO should be auto-loaded", hasMultilabel.value());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPython)
