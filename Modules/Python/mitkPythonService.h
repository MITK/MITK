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
#include "mitkSurface.h"
#include <MitkPythonExports.h>

namespace mitk
{
  ///
  /// implementation of the IPythonService using ctkabstractpythonmanager
  /// \see IPythonService
  class MITK_PYTHON_EXPORT PythonService: public itk::LightObject, public mitk::IPythonService
  {
  public:
      ///
      /// instantiate python manager here
      PythonService();
      ///
      /// empty implementation...
      ~PythonService();
      ///
      /// \see IPythonService::Execute()
      std::string Execute( const std::string& pythonCommand, int commandType = SINGLE_LINE_COMMAND );
      ///
      /// \see IPythonService::ExecuteScript()
      void ExecuteScript(const std::string &pathToPythonScript);
      ///
      /// \see IPythonService::PythonErrorOccured()
      bool PythonErrorOccured() const;
      ///
      /// \see IPythonService::GetVariableStack()
      std::vector<PythonVariable> GetVariableStack() const;
      ///
      /// \see IPythonService::DoesVariableExist()
      bool DoesVariableExist(const std::string& name) const;
      ///
      /// \see IPythonService::AddPythonCommandObserver()
      void AddPythonCommandObserver( PythonCommandObserver* observer );
      ///
      /// \see IPythonService::RemovePythonCommandObserver()
      void RemovePythonCommandObserver( PythonCommandObserver* observer );
      ///
      /// \see IPythonService::NotifyObserver()
      void NotifyObserver( const std::string& command );
      ///
      /// \see IPythonService::IsItkPythonWrappingAvailable()
      bool IsSimpleItkPythonWrappingAvailable();
      ///
      /// \see IPythonService::CopyToPythonAsItkImage()
      bool CopyToPythonAsSimpleItkImage( mitk::Image* image, const std::string& varName );
      ///
      /// \see IPythonService::CopyItkImageFromPython()
      mitk::Image::Pointer CopySimpleItkImageFromPython( const std::string& varName );
      ///
      /// \see IPythonService::IsOpenCvPythonWrappingAvailable()
      bool IsOpenCvPythonWrappingAvailable();
      ///
      /// \see IPythonService::CopyToPythonAsCvImage()
      bool CopyToPythonAsCvImage( mitk::Image* image, const std::string& varName );
      ///
      /// \see IPythonService::CopyCvImageFromPython()
      mitk::Image::Pointer CopyCvImageFromPython( const std::string& varName );
      ///
      /// \see IPythonService::IsVtkPythonWrappingAvailable()
      bool IsVtkPythonWrappingAvailable();
      ///
      /// \see IPythonService::CopyToPythonAsVtkPolyData()
      bool CopyToPythonAsVtkPolyData( mitk::Surface* surface, const std::string& varName );
      ///
      /// \see IPythonService::CopyVtkPolyDataFromPython()
      mitk::Surface::Pointer CopyVtkPolyDataFromPython( const std::string& varName );
      ///
      /// \return the ctk abstract python manager instance
      ctkAbstractPythonManager* GetPythonManager();
  protected:
      QString GetTempDataFileName(const std::string &ext) const;
  private:
      QList<PythonCommandObserver*> m_Observer;
      ctkAbstractPythonManager m_PythonManager;
      static const QString m_TmpDataFileName;
      bool m_ItkWrappingAvailable;
      bool m_OpenCVWrappingAvailable;
      bool m_VtkWrappingAvailable;
      bool m_ErrorOccured;
  };
}
#endif
