/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <filesystem>
#include <mitkCoreServices.h>
#include <mitkIOUtil.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <swigpyrun.h>

class mitkPyMITKTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPyMITKTestSuite);

  MITK_TEST(TestImportPyMITK);
  MITK_TEST(TestLoadImage);
  MITK_TEST(TestTransferImage);
  MITK_TEST(TestSaveImage);
  MITK_TEST(TestConvertToNumpy);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp() override { m_ImagePath = GetTestDataFilePath("Pic3D.nrrd"); }

  void tearDown() override {}

  void TestImportPyMITK()
  {
    std::string pySysPath = "sys.path.append('" + mitk::IOUtil::GetProgramPath() + "')";
    if (!Py_IsInitialized())
      Py_Initialize();
    std::string pyCommand = "print('Hello World from Embedded Python!!!')\n"
                            "import sys\n"
                            "sys.path.append('" +
                            mitk::IOUtil::GetProgramPath() + "')\n"
                            "import pyMITK\n"
                            "print(pyMITK.SayHi(), flush=True)";
    if (PyRun_SimpleString(pyCommand.c_str()) != 0)
    {
      CPPUNIT_FAIL("Error importing pyMITK.");
    }
  }

  void TestLoadImage()
  {
    PyGILState_Ensure();
    std::string pyLoadCommand = "image_list = pyMITK.IOUtil.imread('" + m_ImagePath + "')";
    PyRun_SimpleString(pyLoadCommand.c_str());
    PyRun_SimpleString("image = image_list[0]");
    swig_type_info *pTypeInfo = nullptr;
    pTypeInfo = SWIG_TypeQuery("_p_mitk__Image");
    PyObject *main = PyImport_AddModule("__main__");
    PyObject *globals = PyModule_GetDict(main);
    PyObject *pyImage = PyDict_GetItemString(globals, "image");
    if (pyImage == NULL)
    {
      MITK_INFO << "Could not get image from Python" << std::endl;
    }
    mitk::Image *imagePtrReceived;
    int res = 0;
    void *voidPtr;
    res = SWIG_ConvertPtr(pyImage, &voidPtr, pTypeInfo, 0);
    if (!SWIG_IsOK(res))
    {
      MITK_INFO << "Could not cast pyImage to C++ type";
    }
    imagePtrReceived = reinterpret_cast<mitk::Image *>(voidPtr);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), imagePtrReceived->GetDimension());
    PyRun_SimpleString("del image");
  }

  void TestTransferImage()
  {
    auto mitkImage = mitk::IOUtil::Load<mitk::Image>(m_ImagePath);
    PyRun_SimpleString("from pyMITK import Image");
    PyRun_SimpleString("py_image = None");
    PyRun_SimpleString("def receive(image_from_cxx):\n"
                       "    print ('receive called with %s', image_from_cxx)\n"
                       "    print('Python transferred image has area', image_from_cxx.GetDimension())\n"
                       "    global py_image\n"
                       "    py_image = image_from_cxx\n");
    PyGILState_Ensure();
    int owned = 0;
    swig_type_info *pTypeInfo = nullptr;
    pTypeInfo = SWIG_TypeQuery("_p_mitk__Image");
    PyObject *pInstance = SWIG_NewPointerObj(reinterpret_cast<void *>(mitkImage.GetPointer()), pTypeInfo, owned);
    if (pInstance == NULL)
    {
      std::cout << "Something went wrong creating the Python instance of the image\n";
    }
    PyObject *main = PyImport_AddModule("__main__");
    PyObject *receive = PyObject_GetAttrString(main, "receive");
    PyObject *result = PyObject_CallFunctionObjArgs(receive, pInstance, NULL);
    if (result == NULL)
    {
      std::cout << "Something went wrong setting the circle in Python\n";
    }
    PyRun_SimpleString("py_image_dimension = py_image.GetDimension()");
    PyObject *globals = PyModule_GetDict(main);
    PyObject *pyDim = PyDict_GetItemString(globals, "py_image_dimension");
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(PyLong_AsLong(pyDim)), mitkImage->GetDimension());
    PyRun_SimpleString("del py_image_dimension");
  }

  void TestSaveImage()
  {
    std::ofstream tmpStream;
    std::string imagePath = mitk::IOUtil::CreateTemporaryFile(tmpStream, "pyMITK-XXXXXX.nrrd");
    tmpStream.close();
    std::replace(imagePath.begin(), imagePath.end(), '\\', '/');

    std::string pyLoadCommand = "image_list = pyMITK.IOUtil.imread('" + m_ImagePath + "')";
    PyRun_SimpleString(pyLoadCommand.c_str());
    PyRun_SimpleString("print(image_list[0].GetDimension())");
    PyRun_SimpleString("image = image_list[0]");

    std::string imsaveCommand = "pyMITK.IOUtil.imsave(image,'" + imagePath + "')";
    PyRun_SimpleString(imsaveCommand.c_str());
    CPPUNIT_ASSERT(std::filesystem::file_size(imagePath) > 0);
    std::remove(imagePath.c_str());
    PyRun_SimpleString("del image");
  }

  void TestConvertToNumpy()
  {
    auto mitkImage = mitk::IOUtil::Load<mitk::Image>(m_ImagePath);
    PyRun_SimpleString("import numpy as np");
    std::string pyLoadCommand = "image_list = pyMITK.IOUtil.imread('" + m_ImagePath + "')";
    PyRun_SimpleString(pyLoadCommand.c_str());
    PyRun_SimpleString("temp = image_list[0].GetAsNumpy()");
    PyRun_SimpleString("shape_ = temp.shape");
    PyObject *main = PyImport_AddModule("__main__");
    PyObject *globals = PyModule_GetDict(main);
    PyObject *pyTuple = PyDict_GetItemString(globals, "shape_");
    if (PyTuple_Check(pyTuple))
    {
      Py_ssize_t n_dims = PyTuple_Size(pyTuple);
      for (Py_ssize_t i = 0; i < n_dims; i++)
      {
        PyObject *value = PyTuple_GetItem(pyTuple, i);
        CPPUNIT_ASSERT_EQUAL(mitkImage->GetDimension(n_dims - i - 1), static_cast<unsigned int>(PyLong_AsLong(value)));
      }
    }
  }

private:
  std::string m_ImagePath;
};

MITK_TEST_SUITE_REGISTRATION(mitkPyMITK)
