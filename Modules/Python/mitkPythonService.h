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
#ifndef mitkPythonService_h
#define mitkPythonService_h

#include "mitkIPythonService.h"
#include <itkLightObject.h>
#include <ctkAbstractPythonManager.h>

namespace mitk
{
  class PythonService: public itk::LightObject, public mitk::IPythonService
  {
  public:
      PythonService();
      ~PythonService(); // leer in mitkIPythonService.cpp implementieren
      QVariant Execute( const QString& pythonCommand, int commandType = SINGLE_LINE_COMMAND );
      QList<PythonVariable> GetVariableStack() const;

      void AddPythonCommandObserver( PythonCommandObserver* observer );
      void RemovePythonCommandObserver( PythonCommandObserver* observer );
      void NotifyObserver( const QString& command );

      bool CopyToPythonAsItkImage( mitk::Image* image, const QString& varName );
      mitk::Image::Pointer CopyItkImageFromPython( const QString& varName );
  private:
      QList<PythonCommandObserver*> m_Observer;
      ctkAbstractPythonManager m_PythonManager;

  };
}
#endif
