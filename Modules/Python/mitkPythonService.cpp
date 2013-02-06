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

mitk::PythonService::PythonService()
{
    m_PythonManager.initialize();
}

mitk::PythonService::~PythonService()
{
}

QVariant mitk::PythonService::Execute(const QString &pythonCommand, int commandType)
{
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
        this->NotifyObserver(pythonCommand);

    return result;
}

QList<mitk::PythonVariable> mitk::PythonService::GetVariableStack() const
{
    QList<mitk::PythonVariable> list;

    PyObject* dict = PyImport_GetModuleDict();
    PyObject* object = PyDict_GetItemString(dict, "__main__");
    PyObject* dirMain = PyObject_Dir(object);
    PyObject* tempObject;

    if(dirMain)
    {
      QString attr, attrValue, attrType;

      for(int i = 0; i<PyList_Size(dirMain); i++)
      {
        tempObject = PyList_GetItem(dirMain, i);
        attr = PyString_AsString(tempObject);
        tempObject = PyObject_GetAttrString(object, attr.toLocal8Bit().data());
        attrType = tempObject->ob_type->tp_name;
        if(PyUnicode_Check(tempObject) || PyString_Check(tempObject))
          attrValue = PyString_AsString(tempObject);
        else
          attrValue = "";
        mitk::PythonVariable var;
        var.m_Name = attr;
        var.m_Value = attrValue;
        var.m_Type = attrType;
        list.append(var);
      }
    }

    return list;
}

void mitk::PythonService::AddPythonCommandObserver(mitk::PythonCommandObserver *observer)
{
    if(m_Observer.contains(observer))
        m_Observer.append(observer);
}

void mitk::PythonService::RemovePythonCommandObserver(mitk::PythonCommandObserver *observer)
{
    m_Observer.removeOne(observer);
}

void mitk::PythonService::NotifyObserver(const QString &command)
{
    foreach(mitk::PythonCommandObserver* observer, m_Observer)
    {
        observer->CommandExecuted(command);
    }
}

bool mitk::PythonService::CopyToPythonAsItkImage(mitk::Image *image, const QString &varName)
{
    // save image
    QString tmpFolder = QDir::tempPath();
    QString fileName = tmpFolder + QDir::separator() + varName + ".nrrd";

    MITK_DEBUG << "Saving temporary file " << fileName.toStdString();
    if( !mitk::IOUtil::SaveImage(image, fileName.toStdString()) )
    {
        MITK_ERROR << "Temporary file could not be created.";
    }
    else
    {
        // TODO CORRECT TYPE SETUP, MAKE MITK_DEBUG MITK_DEBUG
        int dim = 3;
        QString type = "US";
        QString command;
        command.append( "import itk\n" );
        command.append( "try:\n" );
        command.append( "  mitkImages" );
        command.append( "except NameError:\n" );
        command.append( "  mitkImages = {}\n" );
        //command.append( QString("mitkImages['%1_dim''] = %2\n").arg( varName ).arg(dim) );
        //command.append( QString("mitkImages['%1_pixelType'] = itk.%2\n").arg( varName ).argtype( type ) );
        command.append( QString("mitkImages['%1_imageType'] = itk.Image[%2, %3]\n").arg( varName ).arg( type ).arg( dim ) );
        command.append( QString("readerType = itk.ImageFileReader[mitkImages['%1_imageType']]\n").arg( varName ) );
        command.append( "reader = readerType.New()\n" );
        command.append( QString("reader.SetFileName( \"%1\" )\n").arg(fileName) );
        command.append( "reader.Update()\n" );
        command.append( QString("mitkImages['%1'] = reader.GetOutput()\n").arg( varName ));
        MITK_DEBUG << "Issuing python command " << command.toStdString();
        this->Execute(command, IPythonService::SINGLE_LINE_COMMAND );

        QFile file(fileName);
        MITK_DEBUG << "Removing file " << fileName.toStdString();
        file.remove();
        return true;
    }

    return false;
}

mitk::Image::Pointer mitk::PythonService::CopyItkImageFromPython(const QString &varName)
{
    return 0;
}
