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
 * @brief Abstract Class to Setup a python virtual environment and pip install required packages.
 * Derive this class for creating installer for the respective tool.
 */
class MITKSEGMENTATIONUI_EXPORT QmitkSetupVirtualEnvUtil
{
public:
  QmitkSetupVirtualEnvUtil(const QString& baseDir);
  QmitkSetupVirtualEnvUtil();

  /**
   * @brief Implement the method in child class 
   * to setup the virtual environment.
   */
  virtual bool SetupVirtualEnv(const QString& venvName) = 0;

  /**
   * @brief Get the Virtual Env Path object. Override this method in the respective
   * tool installer class.
   * 
   * @return QString 
   */
  virtual QString GetVirtualEnvPath();

  /**
   * @brief Function to Pip install a library package given the location of 
   * pip3 executable.
   * Any callback function can be passed to process the output.
   * 
   * @param library 
   * @param workingDir 
   * @param callback 
   * @param command 
   */
  void PipInstall(const std::string &library,
                  const std::string &workingDir,
                  void (*callback)(itk::Object *, const itk::EventObject &, void *),
                  const std::string &command = "pip3");

  /**
   * @brief Overloaded function to Pip install a library function.
   * 
   * @param library 
   * @param callback 
   * @param command 
   */
  void PipInstall(const std::string &library,
                  void (*callback)(itk::Object *, const itk::EventObject &, void *),
                  const std::string &command = "pip3");


  /**
   * @brief Function to execute any python code given a python path.
   * Any callback function can be passed to process the output.
   * 
   * @param args 
   * @param pythonPath 
   * @param callback 
   * @param command 
   */
  void ExecutePython(const std::string &args,
                     const std::string &pythonPath,
                     void (*callback)(itk::Object *, const itk::EventObject &, void *),
                     const std::string &command = "python");

  /**
   * @brief Overloaded function to Execute Python code.
   * Any callback function can be passed to process the output.
   * 
   * @param args 
   * @param callback 
   * @param command 
   */
  void ExecutePython(const std::string &args,
                     void (*callback)(itk::Object *, const itk::EventObject &, void *),
                     const std::string &command = "python");

  /**
   * @brief Installs pytorch using light-the-torch package, correctly identifying cuda version.
   * Requires location of pip3 executable.
   * Any callback function can be passed to process the output.
   * 
   * @param workingDir 
   * @param callback 
   */
  void InstallPytorch(const std::string &workingDir, void (*callback)(itk::Object *, const itk::EventObject &, void *));

  /**
   * @brief Overloaded function to install pytorch using light-the-torch package, correctly 
   * identifying cuda version.
   */
  void InstallPytorch();

  /**
   * @brief Get the Base Dir object
   * 
   * @return QString& 
   */
  QString& GetBaseDir();

  /**
   * @brief Get the System Python Path object
   * 
   * @return QString& 
   */
  QString& GetSystemPythonPath();

  /**
   * @brief Get the Python Path object
   * 
   * @return QString& 
   */
  QString& GetPythonPath();

  /**
   * @brief Get the Pip Path object
   * 
   * @return QString& 
   */
  QString& GetPipPath();

  /**
   * @brief Set the System Python Path object
   * 
   * @param path 
   */
  void SetSystemPythonPath(const QString& path);

  /**
   * @brief Set the Python Path object
   * 
   * @param path 
   */
  void SetPythonPath(const QString& path);

  /**
   * @brief Set the Pip Path object
   * 
   * @param path 
   */
  void SetPipPath(const QString& path);

  /**
   * @brief Set the Virtual Env Path object
   * 
   * @param path 
   */
  void SetVirtualEnvPath(const QString &path);

  /**
   * @brief Check if the path provide has python executable or not.
   * 
   * @param pythonPath 
   * @return true 
   * @return false 
   */
  bool IsPythonPath(const QString &pythonPath);

  /**
   * @brief Function can be used as callback to simply print out all the process execution output
   * parsed out from itk::EventObject.
   * 
   */
  static void PrintProcessEvent(itk::Object *, const itk::EventObject &e, void *);

private:
  QString m_PythonPath;
  QString m_PipPath;
  QString m_BaseDir;
  QString m_venvPath;
  QString m_SysPythonPath;
};
#endif
