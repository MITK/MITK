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
    /// describes a python variable (data container)
    /// \see IPythonService::GetVariableStack()
    ///
    class MITK_PYTHON_EXPORT IPythonService
    {
    public:
        static const int SINGLE_LINE_COMMAND = 0;
        static const int MULTI_LINE_COMMAND = 1;
        static const int EVAL_COMMAND = 2;

        virtual ~IPythonService(); // leer in mitkIPythonService.cpp implementieren
        virtual QVariant Execute( const QString& pythonCommand, int commandType = SINGLE_LINE_COMMAND ) = 0;
        virtual QList<PythonVariable> GetVariableStack() const = 0;

        virtual void AddPythonCommandObserver( PythonCommandObserver* observer ) = 0;
        virtual void RemovePythonCommandObserver( PythonCommandObserver* observer ) = 0;
        virtual void NotifyObserver( const QString& command ) = 0;

        virtual bool CopyToPythonAsItkImage( mitk::Image* image, const QString& varName ) = 0;
        virtual mitk::Image::Pointer CopyItkImageFromPython( const QString& varName ) = 0;
    };
}

//US_DECLARE_SERVICE_INTERFACE(mitk::IPythonService, "org.mitk.services.IPythonService")

#endif
