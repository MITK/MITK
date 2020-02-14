/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkPythonService_h
#define mitkPythonService_h

#include <ctkAbstractPythonManager.h>
#include "mitkIPythonService.h"
#include <itkLightObject.h>
#include "mitkSurface.h"

namespace mitk
{
  ///
  /// implementation of the IPythonService using ctkabstractpythonmanager
  /// \see IPythonService
  class PythonService: public itk::LightObject, public mitk::IPythonService
  {
  public:
      ///
      /// instantiate python manager here
      PythonService();
      ///
      /// empty implementation...
      ~PythonService() override;
      ///
      /// \see IPythonService::Execute()
      std::string Execute( const std::string& pythonCommand, int commandType = SINGLE_LINE_COMMAND ) override;
      ///
      /// \see IPythonService::ExecuteScript()
      void ExecuteScript(const std::string &pathToPythonScript) override;
      ///
      /// \see IPythonService::PythonErrorOccured()
      bool PythonErrorOccured() const override;
      ///
      /// \see IPythonService::GetVariableStack()
      std::vector<PythonVariable> GetVariableStack() const override;
      ///
      /// \see IPythonService::DoesVariableExist()
      bool DoesVariableExist(const std::string& name) const override;
      ///
      /// \see IPythonService::GetVariable()
      std::string GetVariable(const std::string& name) const override;
      ///
      /// \see IPythonService::AddPythonCommandObserver()
      void AddPythonCommandObserver( PythonCommandObserver* observer ) override;
      ///
      /// \see IPythonService::RemovePythonCommandObserver()
      void RemovePythonCommandObserver( PythonCommandObserver* observer ) override;
      ///
      /// \see IPythonService::NotifyObserver()
      void NotifyObserver( const std::string& command ) override;
      ///
      /// \see IPythonService::IsItkPythonWrappingAvailable()
      bool IsSimpleItkPythonWrappingAvailable() override;
      ///
      /// \see IPythonService::CopyToPythonAsItkImage()
      bool CopyToPythonAsSimpleItkImage( mitk::Image* image, const std::string& varName ) override;
      ///
      /// \see IPythonService::CopyItkImageFromPython()
      mitk::Image::Pointer CopySimpleItkImageFromPython( const std::string& varName ) override;
      ///
      /// \see IPythonService::IsOpenCvPythonWrappingAvailable()
      bool IsOpenCvPythonWrappingAvailable() override;
      ///
      /// \see IPythonService::CopyToPythonAsCvImage()
      bool CopyToPythonAsCvImage( mitk::Image* image, const std::string& varName ) override;
      ///
      /// \see IPythonService::CopyCvImageFromPython()
      mitk::Image::Pointer CopyCvImageFromPython( const std::string& varName ) override;
      ///
      /// \see IPythonService::IsVtkPythonWrappingAvailable()
      bool IsVtkPythonWrappingAvailable() override;
      ///
      /// \see IPythonService::CopyToPythonAsVtkPolyData()
      bool CopyToPythonAsVtkPolyData( mitk::Surface* surface, const std::string& varName ) override;
      ///
      /// \see IPythonService::CopyVtkPolyDataFromPython()
      mitk::Surface::Pointer CopyVtkPolyDataFromPython( const std::string& varName ) override;
      ///
      /// \return the ctk abstract python manager instance
      ctkAbstractPythonManager* GetPythonManager() override;

      void AddRelativeSearchDirs(std::vector< std::string > dirs) override;

      void AddAbsoluteSearchDirs(std::vector< std::string > dirs) override;

  protected:

  private:
      QList<PythonCommandObserver*> m_Observer;
      ctkAbstractPythonManager m_PythonManager;
      bool m_ItkWrappingAvailable;
      bool m_OpenCVWrappingAvailable;
      bool m_VtkWrappingAvailable;
      bool m_ErrorOccured;
  };
}
#endif
