/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPythonService.h"
#include <Python.h>
#include <mitkIOUtil.h>
#include <QFile>
#include <QDir>

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 5208)
#endif

#include <PythonQt.h>

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

#include "PythonPath.h"
#include <vtkPolyData.h>
#include <mitkRenderingManager.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>
#include <QFileInfo>
#include <QCoreApplication>
#include <itksys/SystemTools.hxx>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include <mitkExceptionMacro.h>

#ifndef WIN32
#include <dlfcn.h>
#endif

typedef itksys::SystemTools ist;

mitk::PythonService::PythonService()
  : m_ItkWrappingAvailable( true )
  , m_OpenCVWrappingAvailable( true )
  , m_VtkWrappingAvailable( true )
  , m_ErrorOccured( false )
{
  bool pythonInitialized = static_cast<bool>( Py_IsInitialized() ); //m_PythonManager.isPythonInitialized() );

  // due to strange static var behaviour on windows Py_IsInitialized() returns correct value while
  // m_PythonManager.isPythonInitialized() does not because it has been constructed and destructed again
  if( !pythonInitialized )
  {
    MITK_INFO << "Initializing python service";
    //TODO a better way to do this
#ifndef WIN32
    dlerror();
    if(dlopen(PYTHON_LIBRARY, RTLD_NOW | RTLD_GLOBAL) == nullptr )
    {
      mitkThrow() << "Python runtime could not be loaded: " << dlerror();
    }
#endif

    std::string programPath = QCoreApplication::applicationDirPath().toStdString() + "/";

    QString pythonCommand;
    pythonCommand.append( QString("import site, sys\n") );
    pythonCommand.append( QString("import SimpleITK as sitk\n") );
    pythonCommand.append( QString("import SimpleITK._SimpleITK as _SimpleITK\n") );
    pythonCommand.append( QString("import numpy\n") );

    pythonCommand.append( QString("sys.path.append('')\n") );
    pythonCommand.append( QString("sys.path.append('%1')\n").arg(programPath.c_str()) );
    pythonCommand.append( QString("sys.path.append('%1')\n").arg(EXTERNAL_DIST_PACKAGES) );
    pythonCommand.append( QString("\nsite.addsitedir('%1')").arg(EXTERNAL_SITE_PACKAGES) );

    if( pythonInitialized )
      m_PythonManager.setInitializationFlags(PythonQt::RedirectStdOut|PythonQt::PythonAlreadyInitialized);
    else
      m_PythonManager.setInitializationFlags(PythonQt::RedirectStdOut);
    m_PythonManager.initialize();

    m_PythonManager.executeString( pythonCommand, ctkAbstractPythonManager::FileInput );
  }
}

mitk::PythonService::~PythonService()
{
  MITK_DEBUG("mitk::PythonService") << "destructing PythonService";
}

void mitk::PythonService::AddRelativeSearchDirs(std::vector< std::string > dirs)
{
  std::string programPath = QCoreApplication::applicationDirPath().toStdString() + "/";
  std::string cwd = ist::GetCurrentWorkingDirectory() + "/";

  for (auto dir : dirs)
  {
    m_PythonManager.executeString(QString("sys.path.append('%1')").arg((programPath + dir).c_str()), ctkAbstractPythonManager::SingleInput );
    m_PythonManager.executeString(QString("sys.path.append('%1')").arg((cwd + dir).c_str()), ctkAbstractPythonManager::SingleInput );
  }
}

void mitk::PythonService::AddAbsoluteSearchDirs(std::vector< std::string > dirs)
{
  for (auto dir : dirs)
  {
    m_PythonManager.executeString(QString("sys.path.append('%1')").arg(dir.c_str()), ctkAbstractPythonManager::SingleInput );
  }
}

std::string mitk::PythonService::Execute(const std::string &stdpythonCommand, int commandType)
{
  QString pythonCommand = QString::fromStdString(stdpythonCommand);
  QVariant result;

  bool commandIssued = true;
  if(commandType == IPythonService::SINGLE_LINE_COMMAND )
    result = m_PythonManager.executeString(pythonCommand, ctkAbstractPythonManager::SingleInput );
  else if(commandType == IPythonService::MULTI_LINE_COMMAND )
    result = m_PythonManager.executeString(pythonCommand, ctkAbstractPythonManager::FileInput );
  else if(commandType == IPythonService::EVAL_COMMAND )
    result = m_PythonManager.executeString(pythonCommand, ctkAbstractPythonManager::EvalInput );
  else
    commandIssued = false;

  if(commandIssued)
  {
    this->NotifyObserver(pythonCommand.toStdString());
    m_ErrorOccured = PythonQt::self()->hadError();
  }

  return result.toString().toStdString();
}

void mitk::PythonService::ExecuteScript( const std::string& pythonScript )
{
  std::ifstream t(pythonScript.c_str());
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());
  t.close();

  m_PythonManager.executeString(QString::fromStdString(str));
}

std::vector<mitk::PythonVariable> mitk::PythonService::GetVariableStack() const
{
  std::vector<mitk::PythonVariable> list;

  PyObject* dict = PyImport_GetModuleDict();
  PyObject* object = PyDict_GetItemString(dict, "__main__");
  PyObject* dirMain = PyObject_Dir(object);
  PyObject* tempObject = nullptr;
  //PyObject* strTempObject = 0;

  if(dirMain)
  {
    std::string name, attrValue, attrType;

    for(int i = 0; i<PyList_Size(dirMain); i++)
    {
      tempObject = PyList_GetItem(dirMain, i);
      name = PyString_AsString(tempObject);
      tempObject = PyObject_GetAttrString( object, name.c_str() );
      attrType = tempObject->ob_type->tp_name;

      if(tempObject && ( PyUnicode_Check(tempObject) || PyString_Check(tempObject) ) )
        attrValue = PyString_AsString(tempObject);
      else
        attrValue = "";

      mitk::PythonVariable var;
      var.m_Name = name;
      var.m_Value = attrValue;
      var.m_Type = attrType;
      list.push_back(var);
    }
  }

  return list;
}

std::string mitk::PythonService::GetVariable(const std::string& name) const
{
  std::vector<mitk::PythonVariable> allVars = this->GetVariableStack();
  for(unsigned int i = 0; i< allVars.size(); i++)
  {
    if( allVars.at(i).m_Name == name )
      return allVars.at(i).m_Value;
  }

  return "";
}

bool mitk::PythonService::DoesVariableExist(const std::string& name) const
{
  bool varExists = false;

  std::vector<mitk::PythonVariable> allVars = this->GetVariableStack();
  for(unsigned int i = 0; i< allVars.size(); i++)
  {
    if( allVars.at(i).m_Name == name )
    {
      varExists = true;
      break;
    }
  }

  return varExists;
}

void mitk::PythonService::AddPythonCommandObserver(mitk::PythonCommandObserver *observer)
{
  if(!m_Observer.contains(observer))
    m_Observer.append(observer);
}

void mitk::PythonService::RemovePythonCommandObserver(mitk::PythonCommandObserver *observer)
{
  m_Observer.removeOne(observer);
}

void mitk::PythonService::NotifyObserver(const std::string &command)
{
  MITK_DEBUG("mitk::PythonService") << "number of observer " << m_Observer.size();
  for( int i=0; i< m_Observer.size(); ++i )
  {
    m_Observer.at(i)->CommandExecuted(command);
  }
}

bool mitk::PythonService::CopyToPythonAsSimpleItkImage(mitk::Image *image, const std::string &stdvarName)
{
  QString varName = QString::fromStdString( stdvarName );
  QString command;
  unsigned int* imgDim = image->GetDimensions();
  int npy_nd = 1;

  // access python module
  PyObject *pyMod = PyImport_AddModule("__main__");
  // global dictionary
  PyObject *pyDict = PyModule_GetDict(pyMod);
  const mitk::Vector3D spacing = image->GetGeometry()->GetSpacing();
  const mitk::Point3D origin = image->GetGeometry()->GetOrigin();
  mitk::PixelType pixelType = image->GetPixelType();
  itk::ImageIOBase::IOPixelType ioPixelType = image->GetPixelType().GetPixelType();
  PyObject* npyArray = nullptr;
  mitk::ImageReadAccessor racc(image);
  void* array = const_cast<void*>(racc.GetData());

  mitk::Vector3D xDirection;
  mitk::Vector3D yDirection;
  mitk::Vector3D zDirection;
  const vnl_matrix_fixed<ScalarType, 3, 3> &transform =
      image->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix();

  mitk::Vector3D s = image->GetGeometry()->GetSpacing();

  // ToDo: Check if this is a collumn or row vector from the matrix.
  // right now it works but not sure for rotated geometries
  mitk::FillVector3D(xDirection, transform[0][0]/s[0], transform[0][1]/s[1], transform[0][2]/s[2]);
  mitk::FillVector3D(yDirection, transform[1][0]/s[0], transform[1][1]/s[1], transform[1][2]/s[2]);
  mitk::FillVector3D(zDirection, transform[2][0]/s[0], transform[2][1]/s[1], transform[2][2]/s[2]);

  // save the total number of elements here (since the numpy array is one dimensional)
  npy_intp* npy_dims = new npy_intp[1];
  npy_dims[0] = imgDim[0];

  /**
   * Build a string in the format [1024,1028,1]
   * to describe the dimensionality. This is needed for simple itk
   * to know the dimensions of the image
   */
  QString dimensionString;
  dimensionString.append(QString("["));
  dimensionString.append(QString::number(imgDim[0]));
  for (unsigned i = 1; i < 3; ++i)
    // always three because otherwise the 3d-geometry gets destroyed
    // (relevant for backtransformation of simple itk image to mitk.
  {
    dimensionString.append(QString(","));
    dimensionString.append(QString::number(imgDim[i]));
    npy_dims[0] *= imgDim[i];
  }
  dimensionString.append("]");


  // the next line is necessary for vectorimages
  npy_dims[0] *= pixelType.GetNumberOfComponents();

  // default pixeltype: unsigned short
  NPY_TYPES npy_type  = NPY_USHORT;
  std::string sitk_type = "sitkUInt8";
  if( ioPixelType == itk::ImageIOBase::SCALAR )
  {
    if( pixelType.GetComponentType() == itk::ImageIOBase::DOUBLE ) {
      npy_type = NPY_DOUBLE;
      sitk_type = "sitkFloat64";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::FLOAT ) {
      npy_type = NPY_FLOAT;
      sitk_type = "sitkFloat32";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::SHORT) {
      npy_type = NPY_SHORT;
      sitk_type = "sitkInt16";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::CHAR ) {
      npy_type = NPY_BYTE;
      sitk_type = "sitkInt8";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::INT ) {
      npy_type = NPY_INT;
      sitk_type = "sitkInt32";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::LONG ) {
      npy_type = NPY_LONG;
      sitk_type = "sitkInt64";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::UCHAR ) {
      npy_type = NPY_UBYTE;
      sitk_type = "sitkUInt8";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::UINT ) {
      npy_type = NPY_UINT;
      sitk_type = "sitkUInt32";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::ULONG ) {
      npy_type = NPY_LONG;
      sitk_type = "sitkUInt64";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::USHORT ) {
      npy_type = NPY_USHORT;
      sitk_type = "sitkUInt16";
    }
  }
  else if ( ioPixelType == itk::ImageIOBase::VECTOR ||
            ioPixelType == itk::ImageIOBase::RGB ||
            ioPixelType == itk::ImageIOBase::RGBA
            )
  {
    if( pixelType.GetComponentType() == itk::ImageIOBase::DOUBLE ) {
      npy_type = NPY_DOUBLE;
      sitk_type = "sitkVectorFloat64";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::FLOAT ) {
      npy_type = NPY_FLOAT;
      sitk_type = "sitkVectorFloat32";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::SHORT) {
      npy_type = NPY_SHORT;
      sitk_type = "sitkVectorInt16";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::CHAR ) {
      npy_type = NPY_BYTE;
      sitk_type = "sitkVectorInt8";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::INT ) {
      npy_type = NPY_INT;
      sitk_type = "sitkVectorInt32";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::LONG ) {
      npy_type = NPY_LONG;
      sitk_type = "sitkVectorInt64";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::UCHAR ) {
      npy_type = NPY_UBYTE;
      sitk_type = "sitkVectorUInt8";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::UINT ) {
      npy_type = NPY_UINT;
      sitk_type = "sitkVectorUInt32";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::ULONG ) {
      npy_type = NPY_LONG;
      sitk_type = "sitkVectorUInt64";
    } else if( pixelType.GetComponentType() == itk::ImageIOBase::USHORT ) {
      npy_type = NPY_USHORT;
      sitk_type = "sitkVectorUInt16";
    }
  }
  else {
    MITK_WARN << "not a recognized pixeltype";
    return false;
  }

  // creating numpy array
  import_array1 (true);
  npyArray = PyArray_SimpleNewFromData(npy_nd,npy_dims,npy_type,array);

  // add temp array it to the python dictionary to access it in python code
  const int status = PyDict_SetItemString( pyDict,QString("%1_numpy_array")
                                           .arg(varName).toStdString().c_str(),
                                           npyArray );


  // sanity check
  if ( status != 0 )
    return false;


  command.append( QString("%1 = sitk.Image(%2,sitk.%3,%4)\n").arg(varName)
                  .arg(dimensionString)
                  .arg(QString(sitk_type.c_str())).arg(QString::number(pixelType.GetNumberOfComponents())) );
  command.append( QString("%1.SetSpacing([%2,%3,%4])\n").arg(varName)
                  .arg(QString::number(spacing[0]))
      .arg(QString::number(spacing[1]))
      .arg(QString::number(spacing[2])) );
  command.append( QString("%1.SetOrigin([%2,%3,%4])\n").arg(varName)
                  .arg(QString::number(origin[0]))
      .arg(QString::number(origin[1]))
      .arg(QString::number(origin[2])) );
  command.append( QString("%1.SetDirection([%2,%3,%4,%5,%6,%7,%8,%9,%10])\n").arg(varName)
                  .arg(QString::number(xDirection[0]))
      .arg(QString::number(xDirection[1]))
      .arg(QString::number(xDirection[2]))
      .arg(QString::number(yDirection[0]))
      .arg(QString::number(yDirection[1]))
      .arg(QString::number(yDirection[2]))
      .arg(QString::number(zDirection[0]))
      .arg(QString::number(zDirection[1]))
      .arg(QString::number(zDirection[2]))
      );
  // directly access the cpp api from the lib
  command.append( QString("_SimpleITK._SetImageFromArray(%1_numpy_array,%1)\n").arg(varName) );
  command.append( QString("del %1_numpy_array").arg(varName) );

  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();


  this->Execute( command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  return true;
}


mitk::PixelType DeterminePixelType(const std::string& pythonPixeltype, unsigned long nrComponents, int dimensions)
{
  typedef itk::RGBPixel< unsigned char > UCRGBPixelType;
  typedef itk::RGBPixel< unsigned short > USRGBPixelType;
  typedef itk::RGBPixel< float > FloatRGBPixelType;
  typedef itk::RGBPixel< double > DoubleRGBPixelType;
  typedef itk::Image< UCRGBPixelType > UCRGBImageType;
  typedef itk::Image< USRGBPixelType > USRGBImageType;
  typedef itk::Image< FloatRGBPixelType > FloatRGBImageType;
  typedef itk::Image< DoubleRGBPixelType > DoubleRGBImageType;
  typedef itk::RGBAPixel< unsigned char > UCRGBAPixelType;
  typedef itk::RGBAPixel< unsigned short > USRGBAPixelType;
  typedef itk::RGBAPixel< float > FloatRGBAPixelType;
  typedef itk::RGBAPixel< double > DoubleRGBAPixelType;
  typedef itk::Image< UCRGBAPixelType > UCRGBAImageType;
  typedef itk::Image< USRGBAPixelType > USRGBAImageType;
  typedef itk::Image< FloatRGBAPixelType > FloatRGBAImageType;
  typedef itk::Image< DoubleRGBAPixelType > DoubleRGBAImageType;

  auto pixelType = mitk::MakePixelType<char, char >(nrComponents);

  if (nrComponents == 1)
  {
    if( pythonPixeltype.compare("float64") == 0   ) {
      pixelType = mitk::MakePixelType<double, double >(nrComponents);
    } else if( pythonPixeltype.compare("float32") == 0 ) {
      pixelType = mitk::MakePixelType<float, float >(nrComponents);
    } else if( pythonPixeltype.compare("int16") == 0) {
      pixelType = mitk::MakePixelType<short, short >(nrComponents);
    } else if( pythonPixeltype.compare("int8") == 0 ) {
      pixelType = mitk::MakePixelType<char, char >(nrComponents);
    } else if( pythonPixeltype.compare("int32") == 0 ) {
      pixelType = mitk::MakePixelType<int, int >(nrComponents);
    } else if( pythonPixeltype.compare("int64") == 0 ) {
      pixelType = mitk::MakePixelType<long, long >(nrComponents);
    } else if( pythonPixeltype.compare("uint8") == 0 ) {
      pixelType = mitk::MakePixelType<unsigned char, unsigned char >(nrComponents);
    } else if( pythonPixeltype.compare("uint32") == 0 ) {
      pixelType = mitk::MakePixelType<unsigned int, unsigned int >(nrComponents);
    } else if( pythonPixeltype.compare("uint64") == 0 ) {
      pixelType = mitk::MakePixelType<unsigned long, unsigned long >(nrComponents);
    } else if( pythonPixeltype.compare("uint16") == 0 ) {
      pixelType = mitk::MakePixelType<unsigned short, unsigned short >(nrComponents);
    }
    else
    {
      mitkThrow()<< "unknown scalar PixelType";
    }
  } else if(nrComponents == 3 && dimensions == 2) {
    if( pythonPixeltype.compare("float64") == 0   ) {
      pixelType = mitk::MakePixelType<DoubleRGBImageType>();
    } else if( pythonPixeltype.compare("float32") == 0 ) {
      pixelType = mitk::MakePixelType<FloatRGBImageType>();
    } else if( pythonPixeltype.compare("uint8") == 0 ) {
      pixelType = mitk::MakePixelType<UCRGBImageType>();
    } else if( pythonPixeltype.compare("uint16") == 0 ) {
      pixelType = mitk::MakePixelType<USRGBImageType>();
    }
  } else if( (nrComponents == 4) && dimensions == 2 ) {
    if( pythonPixeltype.compare("float64") == 0   ) {
      pixelType = mitk::MakePixelType<DoubleRGBAImageType>();
    } else if( pythonPixeltype.compare("float32") == 0 ) {
      pixelType = mitk::MakePixelType<FloatRGBAImageType>();
    } else if( pythonPixeltype.compare("uint8") == 0 ) {
      pixelType = mitk::MakePixelType<UCRGBAImageType>();
    } else if( pythonPixeltype.compare("uint16") == 0 ) {
      pixelType = mitk::MakePixelType<USRGBAImageType>();
    }
  }
  else {
    if( pythonPixeltype.compare("float64") == 0   ) {
      pixelType = mitk::MakePixelType<double, itk::Vector<double,3> >(nrComponents);
    } else if( pythonPixeltype.compare("float32") == 0 ) {
      pixelType = mitk::MakePixelType<float, itk::Vector<float,3> >(nrComponents);
    } else if( pythonPixeltype.compare("int16") == 0) {
      pixelType = mitk::MakePixelType<short, itk::Vector<short,3> >(nrComponents);
    } else if( pythonPixeltype.compare("int8") == 0 ) {
      pixelType = mitk::MakePixelType<char, itk::Vector<char,3> >(nrComponents);
    } else if( pythonPixeltype.compare("int32") == 0 ) {
      pixelType = mitk::MakePixelType<int, itk::Vector<int,3> >(nrComponents);
    } else if( pythonPixeltype.compare("int64") == 0 ) {
      pixelType = mitk::MakePixelType<long, itk::Vector<long,3> >(nrComponents);
    } else if( pythonPixeltype.compare("uint8") == 0 ) {
      pixelType = mitk::MakePixelType<unsigned char, itk::Vector<unsigned char,3> >(nrComponents);
    } else if( pythonPixeltype.compare("uint16") == 0 ) {
      pixelType = mitk::MakePixelType<unsigned short, itk::Vector<unsigned short,3> >(nrComponents);
    } else if( pythonPixeltype.compare("uint32") == 0 ) {
      pixelType = mitk::MakePixelType<unsigned int, itk::Vector<unsigned int,3> >(nrComponents);
    } else if( pythonPixeltype.compare("uint64") == 0 ) {
      pixelType = mitk::MakePixelType<unsigned long, itk::Vector<unsigned long,3> >(nrComponents);
    }  else {
      mitkThrow()<< "unknown vectorial PixelType";
    }
  }

  return pixelType;
}

mitk::Image::Pointer mitk::PythonService::CopySimpleItkImageFromPython(const std::string &stdvarName)
{
  double*ds = nullptr;
  // access python module
  PyObject *pyMod = PyImport_AddModule("__main__");
  // global dictionarry
  PyObject *pyDict = PyModule_GetDict(pyMod);
  mitk::Image::Pointer mitkImage = mitk::Image::New();
  mitk::Vector3D spacing;
  mitk::Point3D origin;
  QString command;
  QString varName = QString::fromStdString( stdvarName );

  command.append( QString("%1_numpy_array = sitk.GetArrayFromImage(%1)\n").arg(varName) );
  command.append( QString("%1_spacing = numpy.asarray(%1.GetSpacing())\n").arg(varName) );
  command.append( QString("%1_origin = numpy.asarray(%1.GetOrigin())\n").arg(varName) );
  command.append( QString("%1_dtype = %1_numpy_array.dtype.name\n").arg(varName) );
  command.append( QString("%1_direction = numpy.asarray(%1.GetDirection())\n").arg(varName) );
  command.append( QString("%1_nrComponents = numpy.asarray(%1.GetNumberOfComponentsPerPixel())\n").arg(varName));
  command.append( QString("%1_dtype = %1_numpy_array.dtype.name\n").arg(varName) );


  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();
  this->Execute(command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  PyObject* py_dtype = PyDict_GetItemString(pyDict,QString("%1_dtype").arg(varName).toStdString().c_str() );
  std::string dtype = PyString_AsString(py_dtype);
  PyArrayObject* py_data = (PyArrayObject*) PyDict_GetItemString(pyDict,QString("%1_numpy_array").arg(varName).toStdString().c_str() );
  PyArrayObject* py_spacing = (PyArrayObject*) PyDict_GetItemString(pyDict,QString("%1_spacing").arg(varName).toStdString().c_str() );
  PyArrayObject* py_origin = (PyArrayObject*) PyDict_GetItemString(pyDict,QString("%1_origin").arg(varName).toStdString().c_str() );
  PyArrayObject* py_direction = (PyArrayObject*) PyDict_GetItemString(pyDict,QString("%1_direction").arg(varName).toStdString().c_str() );

  PyArrayObject* py_nrComponents = (PyArrayObject*) PyDict_GetItemString(pyDict,QString("%1_nrComponents").arg(varName).toStdString().c_str() );

  unsigned int nr_Components = *(reinterpret_cast<unsigned int*>(PyArray_DATA(py_nrComponents)));

  unsigned int nr_dimensions = PyArray_NDIM(py_data);
  if (nr_Components > 1) // for VectorImages the last dimension in the numpy array are the vector components.
  {
    --nr_dimensions;
  }

  mitk::PixelType pixelType = DeterminePixelType(dtype, nr_Components, nr_dimensions);

  unsigned int* dimensions = new unsigned int[nr_dimensions];
  // fill backwards , nd data saves dimensions in opposite direction
  for( unsigned i = 0; i < nr_dimensions; ++i )
  {
    dimensions[i] = PyArray_DIMS(py_data)[nr_dimensions - 1 - i];
  }

  mitkImage->Initialize(pixelType, nr_dimensions, dimensions);


  mitkImage->SetChannel(PyArray_DATA(py_data));


  ds = reinterpret_cast<double*>(PyArray_DATA(py_spacing));
  spacing[0] = ds[0];
  spacing[1] = ds[1];
  spacing[2] = ds[2];

  mitkImage->GetGeometry()->SetSpacing(spacing);


  ds = reinterpret_cast<double*>(PyArray_DATA(py_origin));
  origin[0] = ds[0];
  origin[1] = ds[1];
  origin[2] = ds[2];
  mitkImage->GetGeometry()->SetOrigin(origin);


  itk::Matrix<double,3,3> py_transform;

  ds = reinterpret_cast<double*>(PyArray_DATA(py_direction));
  py_transform[0][0] = ds[0];
  py_transform[0][1] = ds[1];
  py_transform[0][2] = ds[2];

  py_transform[1][0] = ds[3];
  py_transform[1][1] = ds[4];
  py_transform[1][2] = ds[5];

  py_transform[2][0] = ds[6];
  py_transform[2][1] = ds[7];
  py_transform[2][2] = ds[8];

  mitk::AffineTransform3D::Pointer affineTransform = mitkImage->GetGeometry()->GetIndexToWorldTransform();

  itk::Matrix<double,3,3> transform = py_transform * affineTransform->GetMatrix();

  affineTransform->SetMatrix(transform);

  mitkImage->GetGeometry()->SetIndexToWorldTransform(affineTransform);

  // mitk::AffineTransform3D::New();
  //mitkImage->GetGeometry()->SetIndexToWorldTransform();

  // cleanup
  command.clear();
  command.append( QString("del %1_numpy_array\n").arg(varName) );
  command.append( QString("del %1_dtype\n").arg(varName) );
  command.append( QString("del %1_spacing\n").arg(varName) );
  command.append( QString("del %1_origin\n").arg(varName) );
  command.append( QString("del %1_direction\n").arg(varName) );
  command.append( QString("del %1_nrComponents\n").arg(varName) );
  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();
  this->Execute(command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  delete[] dimensions;


  return mitkImage;
}

bool mitk::PythonService::CopyToPythonAsCvImage( mitk::Image* image, const std::string& stdvarName )
{
  QString varName = QString::fromStdString( stdvarName );
  QString command;
  unsigned int* imgDim = image->GetDimensions();
  int npy_nd = 1;

  // access python module
  PyObject *pyMod = PyImport_AddModule((char*)"__main__");
  // global dictionary
  PyObject *pyDict = PyModule_GetDict(pyMod);
  mitk::PixelType pixelType = image->GetPixelType();
  PyObject* npyArray = nullptr;
  mitk::ImageReadAccessor racc(image);
  void* array = (void*) racc.GetData();

  // save the total number of elements here (since the numpy array is one dimensional)
  npy_intp* npy_dims = new npy_intp[1];
  npy_dims[0] = imgDim[0];

  /**
   * Build a string in the format [1024,1028,1]
   * to describe the dimensionality. This is needed for simple itk
   * to know the dimensions of the image
   */
  QString dimensionString;
  dimensionString.append(QString("["));
  dimensionString.append(QString::number(imgDim[0]));
  // ToDo: check if we need this
  for (unsigned i = 1; i < 3; ++i)
    // always three because otherwise the 3d-geometry gets destroyed
    // (relevant for backtransformation of simple itk image to mitk.
  {
    dimensionString.append(QString(","));
    dimensionString.append(QString::number(imgDim[i]));
    npy_dims[0] *= imgDim[i];
  }
  dimensionString.append("]");


  // the next line is necessary for vectorimages
  npy_dims[0] *= pixelType.GetNumberOfComponents();

  // default pixeltype: unsigned short
  NPY_TYPES npy_type  = NPY_USHORT;
  if( pixelType.GetComponentType() == itk::ImageIOBase::DOUBLE ) {
    npy_type = NPY_DOUBLE;
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::FLOAT ) {
    npy_type = NPY_FLOAT;
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::SHORT) {
    npy_type = NPY_SHORT;
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::CHAR ) {
    npy_type = NPY_BYTE;
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::INT ) {
    npy_type = NPY_INT;
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::LONG ) {
    npy_type = NPY_LONG;
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::UCHAR ) {
    npy_type = NPY_UBYTE;
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::UINT ) {
    npy_type = NPY_UINT;
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::ULONG ) {
    npy_type = NPY_LONG;
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::USHORT ) {
    npy_type = NPY_USHORT;
  }
  else {
    MITK_WARN << "not a recognized pixeltype";
    return false;
  }

  // creating numpy array
  import_array1 (true);
  npyArray = PyArray_SimpleNewFromData(npy_nd,npy_dims,npy_type,array);

  // add temp array it to the python dictionary to access it in python code
  const int status = PyDict_SetItemString( pyDict,QString("%1_numpy_array")
                                           .arg(varName).toStdString().c_str(),
                                           npyArray );
  // sanity check
  if ( status != 0 )
    return false;

  command.append( QString("import numpy as np\n"));
  //command.append( QString("if '%1' in globals():\n").arg(varName));
  //command.append( QString("  del %1\n").arg(varName));
  command.append( QString("%1_array_tmp=%1_numpy_array.copy()\n").arg(varName));
  command.append( QString("%1_array_tmp=%1_array_tmp.reshape(%2,%3,%4)\n").arg( varName,
                                                                                QString::number(imgDim[1]),
                  QString::number(imgDim[0]),
      QString::number(pixelType.GetNumberOfComponents())));

  command.append( QString("%1 = %1_array_tmp[:,...,::-1]\n").arg(varName));
  command.append( QString("del %1_numpy_array\n").arg(varName) );
  command.append( QString("del %1_array_tmp").arg(varName) );

  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();

  this->Execute( command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  return true;
}


mitk::Image::Pointer mitk::PythonService::CopyCvImageFromPython( const std::string& stdvarName )
{

  // access python module
  PyObject *pyMod = PyImport_AddModule((char*)"__main__");
  // global dictionarry
  PyObject *pyDict = PyModule_GetDict(pyMod);
  mitk::Image::Pointer mitkImage = mitk::Image::New();
  QString command;
  QString varName = QString::fromStdString( stdvarName );

  command.append( QString("import numpy as np\n"));
  command.append( QString("%1_dtype=%1.dtype.name\n").arg(varName) );
  command.append( QString("%1_shape=np.asarray(%1.shape)\n").arg(varName) );
  command.append( QString("%1_np_array=%1[:,...,::-1]\n").arg(varName));
  command.append( QString("%1_np_array=np.reshape(%1_np_array,%1.shape[0] * %1.shape[1] * %1.shape[2])").arg(varName) );

  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();
  this->Execute(command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  PyObject* py_dtype = PyDict_GetItemString(pyDict,QString("%1_dtype").arg(varName).toStdString().c_str() );
  std::string dtype = PyString_AsString(py_dtype);
  PyArrayObject* py_data = (PyArrayObject*) PyDict_GetItemString(pyDict,QString("%1_np_array").arg(varName).toStdString().c_str() );
  PyArrayObject* shape = (PyArrayObject*) PyDict_GetItemString(pyDict,QString("%1_shape").arg(varName).toStdString().c_str() );

  size_t* d = reinterpret_cast<size_t*>(PyArray_DATA(shape));

  unsigned int dimensions[3];
  dimensions[0] = d[1];
  dimensions[1] = d[0];
  dimensions[2] = d[2];

  unsigned int nr_dimensions = 2;

  // get number of components
  unsigned int nr_Components = (unsigned int) d[2];

  auto pixelType = DeterminePixelType(dtype, nr_Components, nr_dimensions);

  mitkImage->Initialize(pixelType, nr_dimensions, dimensions);
  //mitkImage->SetChannel(py_data->data);

  {
    mitk::ImageWriteAccessor ra(mitkImage);
    char* data = (char*)(ra.GetData());
    memcpy(data, PyArray_DATA(py_data), dimensions[0] * dimensions[1] * pixelType.GetSize());
  }

  command.clear();

  command.append( QString("del %1_shape\n").arg(varName) );
  command.append( QString("del %1_dtype\n").arg(varName) );
  command.append( QString("del %1_np_array").arg(varName));

  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();
  this->Execute(command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  return mitkImage;
}

ctkAbstractPythonManager *mitk::PythonService::GetPythonManager()
{
  return &m_PythonManager;
}

mitk::Surface::Pointer mitk::PythonService::CopyVtkPolyDataFromPython( const std::string& stdvarName )
{
  // access python module
  PyObject *pyMod = PyImport_AddModule((char*)"__main__");
  // global dictionarry
  PyObject *pyDict = PyModule_GetDict(pyMod);
  // python memory address
  PyObject *pyAddr = nullptr;
  // cpp address
  size_t addr = 0;
  mitk::Surface::Pointer surface = mitk::Surface::New();
  QString command;
  QString varName = QString::fromStdString( stdvarName );

  command.append( QString("%1_addr_str = %1.GetAddressAsString(\"vtkPolyData\")\n").arg(varName) );
  // remove 0x from the address
  command.append( QString("%1_addr = int(%1_addr_str[5:],16)").arg(varName) );

  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();
  this->Execute(command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  // get address of the object
  pyAddr = PyDict_GetItemString(pyDict,QString("%1_addr").arg(varName).toStdString().c_str());

  // convert to long
  addr = PyInt_AsLong(pyAddr);

  MITK_DEBUG << "Python object address: " << addr;

  // get the object
  vtkPolyData* poly = (vtkPolyData*)((void*)addr);
  surface->SetVtkPolyData(poly);

  // delete helper variables from python stack
  command = "";
  command.append( QString("del %1_addr_str\n").arg(varName) );
  command.append( QString("del %1_addr").arg(varName) );

  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();
  this->Execute(command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  return surface;
}

bool mitk::PythonService::CopyToPythonAsVtkPolyData( mitk::Surface* surface, const std::string& stdvarName )
{
  QString varName = QString::fromStdString( stdvarName );
  std::ostringstream oss;
  std::string addr = "";
  QString command;
  QString address;

  oss << (void*) ( surface->GetVtkPolyData() );

  // get the address
  addr = oss.str();

  // remove "0x"
  address = QString::fromStdString(addr.substr(2));

  command.append( QString("%1 = vtk.vtkPolyData(\"%2\")\n").arg(varName).arg(address) );

  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();
  this->Execute(command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  return true;
}

bool mitk::PythonService::IsSimpleItkPythonWrappingAvailable()
{
  this->Execute( "import SimpleITK as sitk\n", IPythonService::SINGLE_LINE_COMMAND );
  // directly access cpp lib
  this->Execute( "import SimpleITK._SimpleITK as _SimpleITK\n", IPythonService::SINGLE_LINE_COMMAND );
  m_ItkWrappingAvailable = !this->PythonErrorOccured();

  // check for numpy
  this->Execute( "import numpy\n", IPythonService::SINGLE_LINE_COMMAND );

  if ( this->PythonErrorOccured() )
    MITK_ERROR << "Numpy not found.";

  m_ItkWrappingAvailable = !this->PythonErrorOccured();

  return m_ItkWrappingAvailable;
}

bool mitk::PythonService::IsOpenCvPythonWrappingAvailable()
{
  this->Execute( "import cv2\n", IPythonService::SINGLE_LINE_COMMAND );
  m_OpenCVWrappingAvailable = !this->PythonErrorOccured();

  return m_OpenCVWrappingAvailable;
}

bool mitk::PythonService::IsVtkPythonWrappingAvailable()
{
  this->Execute( "import vtk", IPythonService::SINGLE_LINE_COMMAND );
  //this->Execute( "print \"Using VTK version \" + vtk.vtkVersion.GetVTKVersion()\n", IPythonService::SINGLE_LINE_COMMAND );
  m_VtkWrappingAvailable = !this->PythonErrorOccured();

  return m_VtkWrappingAvailable;
}

bool mitk::PythonService::PythonErrorOccured() const
{
  return m_ErrorOccured;
}

