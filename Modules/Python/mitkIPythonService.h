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
#ifndef mitkIPythonService_h
#define mitkIPythonService_h

// mitk
#include "mitkPythonExports.h"
#include "mitkImage.h"
//for microservices
#include <usServiceInterface.h>
// Qt
#include <QString>
#include <QVariant>
#include <QList>

namespace mitk
{
    ///
    /// describes a python variable (data container)
    /// \see IPythonService::GetVariableStack()
    ///
    struct PythonVariable
    {
      QString m_Name;
      QString m_Type;
      QString m_Value;
    };

    ///
    /// a PythonCommandObserver gets informed as soon as a python command was issued
    /// \see IPythonService::AddPythonCommandObserver()
    ///
    class PythonCommandObserver
    {
    public:
      virtual void CommandExecuted(const QString& pythonCommand) = 0;
    };

    ///
    /// The central service for issuing Python Code
    /// The class also enables to transfer mitk images to python as itk::Image and vice versa
    /// \see IPythonService::GetVariableStack()
    ///
    class MITK_PYTHON_EXPORT IPythonService
    {
    public:
        ///
        /// Constant representing a single line command
        /// \see IPythonService::Execute()
        static const int SINGLE_LINE_COMMAND = 0;
        ///
        /// Constant representing a command in which the commands are seperated by new lines, i.e. "\\n"
        /// \see IPythonService::Execute()
        static const int MULTI_LINE_COMMAND = 1;
        ///
        /// Constant representing a single line command x which is run as "eval(x)"
        /// \see IPythonService::Execute()
        static const int EVAL_COMMAND = 2;

        ///
        /// Executes a python command.
        /// \return A variant containing the return value of the python code (if any)
        virtual QVariant Execute( const QString& pythonCommand, int commandType = SINGLE_LINE_COMMAND ) = 0;
        ///
        /// \return The list of variables in the __main__ namespace
        virtual QList<PythonVariable> GetVariableStack() const = 0;
        ///
        /// adds a command observer which is informed after a command was issued with "Execute"
        virtual void AddPythonCommandObserver( PythonCommandObserver* observer ) = 0;
        ///
        /// removes a specific command observer
        virtual void RemovePythonCommandObserver( PythonCommandObserver* observer ) = 0;
        ///
        /// notify all observer. this should only be used if it can be garantueed that the
        /// current python interpreter instance got another command from anywhere else
        /// the the Execute() method of this service, e.g. the shell widget uses this function
        /// since it does not use Execute()
        virtual void NotifyObserver( const QString& command ) = 0;

        ///
        /// copies an mitk image as itk image into the python interpreter process
        /// the image will be available as "varName" in python if everythin worked
        /// \return true if image was copied, else false
        virtual bool CopyToPythonAsItkImage( mitk::Image* image, const QString& varName ) = 0;
        ///
        /// copies an itk image from the python process that is named "varName"
        /// \return the image or 0 if copying was not possible
        virtual mitk::Image::Pointer CopyItkImageFromPython( const QString& varName ) = 0;

        ///
        /// \see CopyToPythonAsItkImage()
        virtual bool CopyToPythonAsCvImage( mitk::Image* image, const QString& varName ) = 0;
        ///
        /// \see CopyCvImageFromPython()
        virtual mitk::Image::Pointer CopyCvImageFromPython( const QString& varName ) = 0;

        ///
        /// nothing to do here
        virtual ~IPythonService(); // leer in mitkIPythonService.cpp implementieren
    };
}

US_DECLARE_SERVICE_INTERFACE(mitk::IPythonService, "org.mitk.services.IPythonService")

#endif
