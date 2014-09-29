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

#include "mitkPythonService.h"
#include <Python.h>
#include <mitkIOUtil.h>
#include <QFile>
#include <QDir>
#include <PythonQt.h>
#include "PythonPath.h"
#include <vtkPolyData.h>
#include <mitkRenderingManager.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>
#include <numpy/arrayobject.h>

#ifndef WIN32
  #include <dlfcn.h>
#endif

const QString mitk::PythonService::m_TmpDataFileName("temp_mitk_data_file");
#ifdef USE_MITK_BUILTIN_PYTHON
  static char* pHome = NULL;
#endif

mitk::PythonService::PythonService()
  : m_ItkWrappingAvailable( true ), m_OpenCVWrappingAvailable( true ), m_VtkWrappingAvailable( true ), m_ErrorOccured( false )
{
  {
    MITK_DEBUG << "will init python if necessary";
  }
  bool pythonInitialized = static_cast<bool>( Py_IsInitialized() ); //m_PythonManager.isPythonInitialized() );
  {
    MITK_DEBUG << "pythonInitialized " << pythonInitialized;
    MITK_DEBUG << "m_PythonManager.isPythonInitialized() " << m_PythonManager.isPythonInitialized();
  }

  // due to strange static var behaviour on windows Py_IsInitialized() returns correct value while
  // m_PythonManager.isPythonInitialized() does not because it has been constructed and destructed again
  if( !m_PythonManager.isPythonInitialized() )
  {
    try
    {
//TODO a better way to do this
#ifndef WIN32
#if defined (__APPLE__) || defined(MACOSX)
  const char* library = "libpython2.7.dylib";
#else
  const char* library = "libpython2.7.so";
#endif
      dlerror();
      if(dlopen(library, RTLD_NOW | RTLD_GLOBAL) == 0 )
      {
        mitkThrow() << "Python runtime could not be loaded: " << dlerror();
      }
#endif

      std::string programPath = mitk::IOUtil::GetProgramPath();
      QDir programmDir( QString( programPath.c_str() ).append("/Python") );
      QString pythonCommand;
      // Set the pythonpath variable depending if
      // we have an installer or development environment
      if ( programmDir.exists() ) {
        // runtime directory used in installers
        pythonCommand.append( QString("import site, sys\n") );
        pythonCommand.append( QString("sys.path.append('')\n") );
        pythonCommand.append( QString("sys.path.append('%1')\n").arg(programPath.c_str()) );
        pythonCommand.append( QString("sys.path.append('%1/Python')").arg(programPath.c_str()) );
#ifndef USE_MITK_BUILTIN_PYTHON
        pythonCommand.append( QString("\nsite.addsitedir('%1/Python/SimpleITK')").arg(programPath.c_str()) );
#endif
      } else {
        pythonCommand.append(PYTHONPATH_COMMAND);
      }

      if( pythonInitialized )
        m_PythonManager.setInitializationFlags(PythonQt::RedirectStdOut|PythonQt::PythonAlreadyInitialized);
      else
        m_PythonManager.setInitializationFlags(PythonQt::RedirectStdOut);

// set python home if own runtime is used
#ifdef USE_MITK_BUILTIN_PYTHON
      QString pythonHome;
      if ( programmDir.exists() )
        pythonHome.append(QString("%1/Python").arg(programPath.c_str()));
      else
        pythonHome.append(PYTHONHOME);

      if(pHome) delete[] pHome;
      pHome = new char[pythonHome.toStdString().length() + 1];

      strcpy(pHome,pythonHome.toStdString().c_str());
      Py_SetPythonHome(pHome);
      MITK_DEBUG("PythonService") << "PythonHome: " << pHome;
#endif

      MITK_DEBUG("PythonService") << "initalizing python";
      m_PythonManager.initialize();

#ifdef USE_MITK_BUILTIN_PYTHON
      PyObject* dict = PyDict_New();
      // Import builtin modules
      if (PyDict_GetItemString(dict, "__builtins__") == NULL)
      {
         PyObject* builtinMod = PyImport_ImportModule("__builtin__");
         if (builtinMod == NULL ||
             PyDict_SetItemString(dict, "__builtins__", builtinMod) != 0)
         {
           Py_DECREF(dict);
           Py_XDECREF(dict);
           return;
         }
         Py_DECREF(builtinMod);
      }
#endif

      MITK_DEBUG("PythonService")<< "Python Search paths: " << Py_GetPath();
      MITK_DEBUG("PythonService") << "python initalized";

      MITK_DEBUG("PythonService") << "registering python paths" << PYTHONPATH_COMMAND;
      m_PythonManager.executeString( pythonCommand, ctkAbstractPythonManager::FileInput );
    }
    catch (...)
    {
      MITK_DEBUG("PythonService") << "exception initalizing python";
    }
  }
}

mitk::PythonService::~PythonService()
{
  MITK_DEBUG("mitk::PythonService") << "destructing PythonService";

#ifdef USE_MITK_BUILTIN_PYTHON
  if(pHome)
    delete[] pHome;
#endif
}

std::string mitk::PythonService::Execute(const std::string &stdpythonCommand, int commandType)
{
  QString pythonCommand = QString::fromStdString(stdpythonCommand);
  {
      MITK_DEBUG("mitk::PythonService") << "pythonCommand = " << pythonCommand.toStdString();
      MITK_DEBUG("mitk::PythonService") << "commandType = " << commandType;
  }

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
  m_PythonManager.executeFile(QString::fromStdString(pythonScript));
}

std::vector<mitk::PythonVariable> mitk::PythonService::GetVariableStack() const
{
  std::vector<mitk::PythonVariable> list;

  PyObject* dict = PyImport_GetModuleDict();
  PyObject* object = PyDict_GetItemString(dict, "__main__");
  PyObject* dirMain = PyObject_Dir(object);
  PyObject* tempObject = 0;
  PyObject* strTempObject = 0;

  if(dirMain)
  {
    std::string name, attrValue, attrType;

    for(int i = 0; i<PyList_Size(dirMain); i++)
    {
      tempObject = PyList_GetItem(dirMain, i);
      name = PyString_AsString(tempObject);
      tempObject = PyObject_GetAttrString( object, name.c_str() );
      attrType = tempObject->ob_type->tp_name;

      strTempObject = PyObject_Repr(tempObject);
      if(strTempObject && ( PyUnicode_Check(strTempObject) || PyString_Check(strTempObject) ) )
        attrValue = PyString_AsString(strTempObject);
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

QString mitk::PythonService::GetTempDataFileName(const std::string& ext) const
{
    QString tmpFolder = QDir::tempPath();
    QString fileName = tmpFolder + QDir::separator() + m_TmpDataFileName + QString::fromStdString(ext);
    return fileName;
}

bool mitk::PythonService::CopyToPythonAsSimpleItkImage(mitk::Image *image, const std::string &stdvarName)
{
  QString varName = QString::fromStdString( stdvarName );
  QString command;
  unsigned int* imgDim = image->GetDimensions();
  int npy_nd = 1;
  npy_intp* npy_dims = new npy_intp[1];
  npy_dims[0] = imgDim[0] * imgDim[1] * imgDim[2];
  // access python module
  PyObject *pyMod = PyImport_AddModule((char*)"__main__");
  // global dictionarry
  PyObject *pyDict = PyModule_GetDict(pyMod);
  const mitk::Vector3D spacing = image->GetGeometry()->GetSpacing();
  const mitk::Point3D origin = image->GetGeometry()->GetOrigin();
  mitk::PixelType pixelType = image->GetPixelType();
  itk::ImageIOBase::IOPixelType ioPixelType = image->GetPixelType().GetPixelType();
  PyObject* npyArray = NULL;
  mitk::ImageReadAccessor racc(image);
  void* array = (void*) racc.GetData();

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
  } else {
    MITK_WARN << "not a scalar pixeltype";
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

  command.append( QString("%1 = sitk.Image(%2,%3,%4,sitk.%5)\n").arg(varName)
                  .arg(QString::number(imgDim[0]))
                  .arg(QString::number(imgDim[1]))
                  .arg(QString::number(imgDim[2]))
                  .arg(QString(sitk_type.c_str())) );
  command.append( QString("%1.SetSpacing([%2,%3,%4])\n").arg(varName)
                  .arg(QString::number(spacing[0]))
                  .arg(QString::number(spacing[1]))
                  .arg(QString::number(spacing[2])) );
  command.append( QString("%1.SetOrigin([%2,%3,%4])\n").arg(varName)
                  .arg(QString::number(origin[0]))
                  .arg(QString::number(origin[1]))
                  .arg(QString::number(origin[2])) );
  // directly access the cpp api from the lib
  command.append( QString("_SimpleITK._SetImageFromArray(%1_numpy_array,%1)\n").arg(varName) );
  command.append( QString("del %1_numpy_array").arg(varName) );

  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();
  this->Execute( command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  return true;
}

mitk::Image::Pointer mitk::PythonService::CopySimpleItkImageFromPython(const std::string &stdvarName)
{
  double*ds = NULL;
  // access python module
  PyObject *pyMod = PyImport_AddModule((char*)"__main__");
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
  command.append( QString("%1_dtype = %1_numpy_array.dtype.name").arg(varName) );

  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();
  this->Execute(command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  PyObject* py_dtype = PyDict_GetItemString(pyDict,QString("%1_dtype").arg(varName).toStdString().c_str() );
  std::string dtype = PyString_AsString(py_dtype);
  PyArrayObject* py_data = (PyArrayObject*) PyDict_GetItemString(pyDict,QString("%1_numpy_array").arg(varName).toStdString().c_str() );
  PyArrayObject* py_spacing = (PyArrayObject*) PyDict_GetItemString(pyDict,QString("%1_spacing").arg(varName).toStdString().c_str() );
  PyArrayObject* py_origin = (PyArrayObject*) PyDict_GetItemString(pyDict,QString("%1_origin").arg(varName).toStdString().c_str() );

  size_t sz = sizeof(short);
  mitk::PixelType pixelType = MakeScalarPixelType<short>();
  if( dtype.compare("float64") == 0   ) {
    pixelType = MakeScalarPixelType<double>();
    sz = sizeof(double);
  } else if( dtype.compare("float32") == 0 ) {
    pixelType = MakeScalarPixelType<float>();
    sz = sizeof(float);
  } else if( dtype.compare("int16") == 0) {
    pixelType = MakeScalarPixelType<short>();
    sz = sizeof(short);
  } else if( dtype.compare("int8") == 0 ) {
    pixelType = MakeScalarPixelType<char>();
    sz = sizeof(char);
  } else if( dtype.compare("int32") == 0 ) {
    pixelType = MakeScalarPixelType<int>();
    sz = sizeof(int);
  } else if( dtype.compare("int64") == 0 ) {
    pixelType = MakeScalarPixelType<long>();
    sz = sizeof(long);
  } else if( dtype.compare("uint8") == 0 ) {
    pixelType = MakeScalarPixelType<unsigned char>();
    sz = sizeof(unsigned char);
  } else if( dtype.compare("uint32") == 0 ) {
    pixelType = MakeScalarPixelType<unsigned int>();
    sz = sizeof(unsigned int);
  } else if( dtype.compare("uint64") == 0 ) {
    pixelType = MakeScalarPixelType<unsigned long>();
    sz = sizeof(unsigned long);
  } else if( dtype.compare("uint16") == 0 ) {
    pixelType = MakeScalarPixelType<unsigned short>();
    sz = sizeof(unsigned short);
  }

  unsigned int* dimensions = new unsigned int[py_data->nd];
  // fill backwards , nd data saves dimensions in opposite direction
  for( int i = 0; i < py_data->nd; ++i )
  {
    dimensions[i] = py_data->dimensions[py_data->nd - 1 - i];
    sz *= dimensions[i];
  }

  mitkImage->Initialize(pixelType, py_data->nd, dimensions);
  mitkImage->SetChannel(py_data->data);

  ds = (double*)py_spacing->data;
  spacing[0] = ds[0];
  spacing[1] = ds[1];
  spacing[2] = ds[2];
  mitkImage->GetGeometry()->SetSpacing(spacing);

  ds = (double*)py_origin->data;
  origin[0] = ds[0];
  origin[1] = ds[1];
  origin[2] = ds[2];
  mitkImage->GetGeometry()->SetOrigin(origin);

  // cleanup
  command.clear();
  command.append( QString("del %1_numpy_array\n").arg(varName) );
  command.append( QString("del %1_dtype\n").arg(varName) );
  command.append( QString("del %1_spacing\n").arg(varName) );
  command.append( QString("del %1_origin").arg(varName) );
  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();
  this->Execute(command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  delete[] dimensions;

  return mitkImage;
}

bool mitk::PythonService::CopyToPythonAsCvImage( mitk::Image* image, const std::string& stdvarName )
{
  QString varName = QString::fromStdString( stdvarName );

  bool convert = false;
  if(image->GetDimension() != 2)
  {
    MITK_ERROR << "Only 2D images allowed for OpenCV images";
    return convert;
  }

  // try to save mitk image
  QString fileName = this->GetTempDataFileName( ".bmp" );
  fileName = QDir::fromNativeSeparators( fileName );
  MITK_DEBUG("PythonService") << "Saving temporary file " << fileName.toStdString();
  if( !mitk::IOUtil::SaveImage(image, fileName.toStdString()) )
  {
    MITK_ERROR << "Temporary file " << fileName.toStdString() << " could not be created.";
    return convert;
  }

  QString command;

  command.append( QString("%1 = cv2.imread(\"%2\")\n") .arg( varName ).arg( fileName ) );
  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();
  this->Execute(command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  MITK_DEBUG("PythonService") << "Removing file " << fileName.toStdString();
  QFile file(fileName);
  file.remove();
  convert = true;
  return convert;
}

mitk::Image::Pointer mitk::PythonService::CopyCvImageFromPython( const std::string& stdvarName )
{
  QString varName = QString::fromStdString( stdvarName );

  mitk::Image::Pointer mitkImage;
  QString command;
  QString fileName = GetTempDataFileName( ".bmp" );
  fileName = QDir::fromNativeSeparators( fileName );

  MITK_DEBUG("PythonService") << "run python command to save image with opencv to " << fileName.toStdString();

  command.append( QString( "cv2.imwrite(\"%1\", %2)\n").arg( fileName ).arg( varName ) );

  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();
  this->Execute(command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  try
  {
      MITK_DEBUG("PythonService") << "Loading temporary file " << fileName.toStdString() << " as MITK image";
      mitkImage = mitk::IOUtil::LoadImage( fileName.toStdString() );
  }
  catch(std::exception& e)
  {
    MITK_ERROR << e.what();
  }

  QFile file(fileName);
  if( file.exists() )
  {
      MITK_DEBUG("PythonService") << "Removing temporary file " << fileName.toStdString();
      file.remove();
  }

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
  PyObject *pyAddr = NULL;
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
  this->Execute( "import _SimpleITK\n", IPythonService::SINGLE_LINE_COMMAND );
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

