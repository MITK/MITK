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
      std::string programPath = mitk::IOUtil::GetProgramPath();
      QDir programmDir( QString( programPath.c_str() ).append("/Python") );
      QString pythonCommand;
      // Set the pythonpath variable depending if
      // we have an installer or development environment
      if ( programmDir.exists() ) {
        // runtime directory used in installers
        pythonCommand.append( QString("import sys\n") );
        pythonCommand.append( QString("sys.path.append('')\n") );
        pythonCommand.append( QString("sys.path.append('%1')\n").arg(programPath.c_str()) );
        pythonCommand.append( QString("sys.path.append('%1/Python')\n").arg(programPath.c_str()) );
        pythonCommand.append( QString("sys.path.append('%1/Python/SimpleITK')").arg(programPath.c_str()) );
        // set python home if own runtime is deployed
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


      /*
      //system("export LD_LIBRARY_PATH=/local/muellerm/mitk/bugsquashing/bin-debug/VTK-build/bin:$LD_LIBRARY_PATH");
      //m_PythonManager.executeString( "print sys.path", ctkAbstractPythonManager::SingleInput );
      //MITK_DEBUG("mitk::PythonService") << "result of 'sys.path': " << result.toString().toStdString();

      //m_PythonManager.executeString( "sys.path.append('/usr/share/pyshared/numpy')", ctkAbstractPythonManager::SingleInput );
      //m_PythonManager.executeString( "import numpy", ctkAbstractPythonManager::SingleInput );

      MITK_DEBUG("mitk::PythonService") << "Trying to import ITK";
      m_PythonManager.executeString( "import itk", ctkAbstractPythonManager::SingleInput );
      m_ItkWrappingAvailable = !m_PythonManager.pythonErrorOccured();
      MITK_DEBUG("mitk::PythonService") << "m_ItkWrappingAvailable: " << (m_ItkWrappingAvailable? "yes": "no");
      if( !m_ItkWrappingAvailable )
      {
        MITK_WARN << "ITK Python wrapping not available. Please check build settings or PYTHONPATH settings.";
      }

      {
        MITK_DEBUG("mitk::PythonService") << "Trying to import OpenCv";
        PyRun_SimpleString("import cv2\n");
        if (PyErr_Occurred())
        {
          PyErr_Print();
        }
        else
          m_OpenCVWrappingAvailable = true;

        //m_PythonManager.executeString( "import cv2", ctkAbstractPythonManager::SingleInput );
        MITK_DEBUG("mitk::PythonService") << "Investigate if an error occured while importing cv2";
        //m_OpenCVWrappingAvailable = !m_PythonManager.pythonErrorOccured();
        MITK_DEBUG("mitk::PythonService") << "m_OpenCVWrappingAvailable: " << (m_OpenCVWrappingAvailable? "yes": "no");
        if( !m_OpenCVWrappingAvailable )
        {
          MITK_WARN << "OpenCV Python wrapping not available. Please check build settings or PYTHONPATH settings.";
        }
      }

      MITK_DEBUG("mitk::PythonService") << "Trying to import VTK";
      m_PythonManager.executeString( "import vtk", ctkAbstractPythonManager::SingleInput );
      m_VtkWrappingAvailable = !m_PythonManager.pythonErrorOccured();
      MITK_DEBUG("mitk::PythonService") << "m_VtkWrappingAvailable: " << (m_VtkWrappingAvailable? "yes": "no");
      if( !m_VtkWrappingAvailable )
      {
        MITK_WARN << "VTK Python wrapping not available. Please check build settings or PYTHONPATH settings.";
      }
      */
    }
    catch (...)
    {
      MITK_DEBUG("PythonService") << "exception initalizing python";
    }

  }

  //m_PythonManager.executeString( "for path in sys.path:\n  print path\n", ctkAbstractPythonManager::SingleInput );
  //m_PythonManager.executeString( "for k, v in os.environ.items():\n  print \"%s=%s\" % (k, v)", ctkAbstractPythonManager::SingleInput );
  //m_PythonManager.executeFile("/local/muellerm/Dropbox/13-02-11-python-wrapping/interpreterInfo.py");
  //std::string result = m_PythonManager.executeString( "5+5", ctkAbstractPythonManager::EvalInput );
  //MITK_DEBUG("mitk::PythonService") << "result of '5+5': " << result.toString().toStdString();
}

mitk::PythonService::~PythonService()
{
  //QVariant result = m_PythonManager.executeString( "sys.getrefcount(cv2)", ctkAbstractPythonManager::EvalInput );
  //MITK_DEBUG("mitk::PythonService") << "sys.getrefcount(cv2): " << result.toString().toStdString();

  //m_PythonManager.executeString( "del sys.modules[\"cv2\"]", ctkAbstractPythonManager::SingleInput );
  //m_PythonManager.executeString( "del cv2", ctkAbstractPythonManager::SingleInput );
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
  for(int i = 0; i< allVars.size(); i++)
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
    for( size_t i=0; i< m_Observer.size(); ++i )
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
  // save image
  QString fileName = this->GetTempDataFileName( mitk::IOUtil::DEFAULTIMAGEEXTENSION );
  fileName = QDir::fromNativeSeparators( fileName );

  MITK_DEBUG("PythonService") << "Saving temporary file " << fileName.toStdString();
  if( !mitk::IOUtil::SaveImage(image, fileName.toStdString()) )
  {
    MITK_ERROR << "Temporary file could not be created.";
  }
  else
  {
    QString command;

    command.append( QString("reader = sitk.ImageFileReader()\n") );
    command.append( QString("reader.SetFileName(\"%1\")\n").arg(fileName) );
    command.append( QString("%1 = reader.Execute()\n").arg(varName) );
    command.append( QString("del reader") );

    MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();
    MITK_INFO << this->Execute( command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

    QFile file( fileName );
    MITK_DEBUG("PythonService") << "Removing file " << fileName.toStdString();
    file.remove();
    return true;
  }
  return false;
}

mitk::Image::Pointer mitk::PythonService::CopySimpleItkImageFromPython(const std::string &stdvarName)
{
  QString varName = QString::fromStdString( stdvarName );
    mitk::Image::Pointer mitkImage;
    QString command;
    QString fileName = GetTempDataFileName( mitk::IOUtil::DEFAULTIMAGEEXTENSION );
    fileName = QDir::fromNativeSeparators( fileName );

    MITK_DEBUG("PythonService") << "Saving temporary file with python itk code " << fileName.toStdString();

    command.append( QString("writer = sitk.ImageFileWriter()\n") );
    command.append( QString("writer.SetFileName(\"%1\")\n").arg(fileName) );
    command.append( QString("writer.Execute(%1)\n").arg(varName) );
    command.append( QString("del writer") );

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

  command.append( QString("surface_addr_str = %1.GetAddressAsString(\"vtkPolyData\")\n").arg(varName) );
  // remove 0x from the address
  command.append( QString("surface_addr = int(surface_addr_str[5:],16)\n") );

  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();
  this->Execute(command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  // get address of the object
  pyAddr = PyDict_GetItemString(pyDict,"surface_addr");

  // convert to long
  addr = PyInt_AsLong(pyAddr);

  MITK_DEBUG << "Python object address: " << addr;

  // get the object
  vtkPolyData* poly = (vtkPolyData*)((void*)addr);
  surface->SetVtkPolyData(poly);

  // delete helper variables from python stack
  command = "";
  command.append( QString("del surface_addr_str\n") );
  command.append( QString("del surface_addr\n") );

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
  //addr = addr.substr(2);

  address = QString::fromStdString(addr.substr(2));

  command.append( QString("%1 = vtk.vtkPolyData(\"%2\")\n").arg(varName).arg(address) );

  MITK_DEBUG("PythonService") << "Issuing python command " << command.toStdString();
  this->Execute(command.toStdString(), IPythonService::MULTI_LINE_COMMAND );

  return true;
}

bool mitk::PythonService::IsSimpleItkPythonWrappingAvailable()
{
  this->Execute( "import SimpleITK as sitk\n", IPythonService::SINGLE_LINE_COMMAND );
  //this->Execute( "import itk\n", IPythonService::SINGLE_LINE_COMMAND );
  //this->Execute( "print \"Using ITK version \" + itk.Version.GetITKVersion()\n", IPythonService::SINGLE_LINE_COMMAND );

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
  this->Execute( "print \"Using VTK version \" + vtk.vtkVersion.GetVTKVersion()\n", IPythonService::SINGLE_LINE_COMMAND );
  m_VtkWrappingAvailable = !this->PythonErrorOccured();

  return m_VtkWrappingAvailable;
}

bool mitk::PythonService::PythonErrorOccured() const
{
  return m_ErrorOccured;
}

