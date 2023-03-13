/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.s

============================================================================*/

#ifndef QmitkSetupVirtualEnvUtil_h_Included
#define QmitkSetupVirtualEnvUtil_h_Included

#include "mitkLogMacros.h"
#include "mitkProcessExecutor.h"
#include <MitkSegmentationUIExports.h>
#include <QString>
#include <QDir>
#include <QApplication>

/**
 * @brief Super Class to Setup a python virtual environment and pip install required packages
 */
class MITKSEGMENTATIONUI_EXPORT QmitkSetupVirtualEnvUtil
{
public:
  QmitkSetupVirtualEnvUtil(const QString& baseDir);
  QmitkSetupVirtualEnvUtil();

  virtual bool SetupVirtualEnv(const QString& venvName) = 0;
  virtual QString GetVirtualEnvPath();

  void PipInstall(const std::string &library,
                  const std::string &workingDir,
                  void (*callback)(itk::Object *, const itk::EventObject &, void *),
                  const std::string &command = "pip3");

  void PipInstall(const std::string &library,
                  void (*callback)(itk::Object *, const itk::EventObject &, void *),
                  const std::string &command = "pip3");


  void ExecutePython(const std::string &args,
                     const std::string &pythonPath,
                     void (*callback)(itk::Object *, const itk::EventObject &, void *),
                     const std::string &command = "python");

  void ExecutePython(const std::string &args,
                     void (*callback)(itk::Object *, const itk::EventObject &, void *),
                     const std::string &command = "python");

  void InstallPytorch(const std::string &workingDir, void (*callback)(itk::Object *, const itk::EventObject &, void *));
  void InstallPytorch();

  QString& GetBaseDir();
  QString& GetSystemPythonPath();
  QString& GetPythonPath();
  QString& GetPipPath();
  void SetSystemPythonPath(const QString& path);
  void SetPythonPath(const QString& path);
  void SetPipPath(const QString& path);
  void SetVirtualEnvPath(const QString &path);

  bool IsPythonPath(const QString &pythonPath);
  static void PrintProcessEvent(itk::Object * /*pCaller*/, const itk::EventObject &e, void *);

private:
  QString m_PythonPath;
  QString m_PipPath;
  QString m_BaseDir;
  QString m_venvPath;
  QString m_SysPythonPath;
};
#endif
