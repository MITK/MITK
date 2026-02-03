/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPythonContext_h
#define mitkPythonContext_h

#include <MitkPythonExports.h>

#include <memory>
#include <optional>
#include <string>

namespace mitk
{
  class Image;

  /**
   * \brief Provides a Python interpreter context for executing Python code and
   *        interacting with MITK data like images.
   *
   * This class wraps a Python interpreter and maintains separate global and
   * local dictionaries for executing Python code. It allows binding MITK data
   * like images to Python variables and retrieving Python variables from the
   * context in a type-safe manner.
   */
  class MITKPYTHON_EXPORT PythonContext
  {
  public:
    /**
     * \brief Constructs a PythonContext and optionally creates or activates a
     *        virtual environment.
     *
     * \param venvName Name of the Python virtual environment to create or
                       activate. If empty, no virtual environment is used.
     */
    explicit PythonContext(const std::string& venvName = {});

    /**
     * \brief Destructor. Clears internal Python dictionaries.
     */
    ~PythonContext();

    /**
     * \brief Initializes the Python interpreter context and sets up module
     *        paths.
     *
     * Adds the application path and the active virtual environment's
     * site-packages to the Python sys.path. Also imports NumPy and MITK Python
     * modules.
     */
    void Activate();

    /**
     * \brief Checks whether a Python variable with the given name exists.
     *
     * \param varName Name of the Python variable to check.
     *
     * \return True if the variable exists in either the local or global
     *         dictionary.
     */
    bool HasVariable(const std::string &varName);

    /**
     * \brief Retrieves a Python variable as a bool.
     *
     * \param varName Name of the Python variable.
     *
     * \return std::optional<bool> containing the value if it exists and can be
     *         cast, std::nullopt otherwise.
     */
    std::optional<bool> GetVariableAsBool(const std::string& varName);

    /**
     * \brief Retrieves a Python variable as an int.
     *
     * \param varName Name of the Python variable.
     *
     * \return std::optional<int> containing the value if it exists and can be
     *         cast, std::nullopt otherwise.
     */
    std::optional<int> GetVariableAsInt(const std::string& varName);

    /**
     * \brief Retrieves a Python variable as a string.
     *
     * \param varName Name of the Python variable.
     *
     * \return std::optional<std::string> containing the value if it exists and
     *         can be cast, std::nullopt otherwise.
     */
    std::optional<std::string> GetVariableAsString(const std::string& varName);

    /**
     * \brief Binds an MITK image to a Python variable in the global dictionary.
     *
     * \param image Pointer to the Image to bind. If nullptr, the variable is
     *              set to None.
     * \param varName Name of the Python variable to assign the image to.
     */
    void BindImage(mitk::Image* image, const std::string& varName);

    /**
     * \brief Executes arbitrary Python code with the context's dictionaries.
     *
     * \param expression The Python code to execute.
     *
     * \throws mitk::Exception if execution fails.
     */
    void Execute(const std::string &expression);

  private:
    struct Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif

