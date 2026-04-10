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
   *
   * A typical usage pattern is:
   * \code
   * mitk::PythonContext ctx("myenv");
   * ctx.Activate();
   * ctx.BindImage(image, "input_image");
   * ctx.Execute("result = process(input_image)");
   * auto value = ctx.GetVariableAsInt("result");
   * \endcode
   *
   * \sa mitk::nnInteractiveTool
   */
  class MITKPYTHON_EXPORT PythonContext
  {
  public:
    /**
     * \brief Constructs a PythonContext and optionally creates or activates a
     *        virtual environment.
     *
     * If the Python interpreter has not been initialized yet, it will be
     * initialized during construction.
     *
     * \param[in] venvName Name of the Python virtual environment to create or
     *                     activate. If empty, no virtual environment is used.
     *
     * \throw mitk::Exception if the virtual environment cannot be created or
     *        activated.
     *
     * \post The Python interpreter is initialized.
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
     *
     * \pre The PythonContext has been constructed.
     *
     * \throw mitk::Exception if any of the Python initialization commands fail.
     */
    void Activate();

    /**
     * \brief Checks whether a Python variable with the given name exists.
     *
     * \param[in] varName Name of the Python variable to check.
     *
     * \return \c true if the variable exists in either the local or global
     *         dictionary, \c false otherwise.
     */
    bool HasVariable(const std::string &varName);

    /**
     * \brief Retrieves a Python variable as a bool.
     *
     * \param[in] varName Name of the Python variable.
     *
     * \return The variable value if it exists and can be cast to \c bool,
     *         \c std::nullopt otherwise.
     */
    std::optional<bool> GetVariableAsBool(const std::string& varName);

    /**
     * \brief Retrieves a Python variable as an int.
     *
     * \param[in] varName Name of the Python variable.
     *
     * \return The variable value if it exists and can be cast to \c int,
     *         \c std::nullopt otherwise.
     */
    std::optional<int> GetVariableAsInt(const std::string& varName);

    /**
     * \brief Retrieves a Python variable as a double.
     *
     * \param[in] varName Name of the Python variable.
     *
     * \return The variable value if it exists and can be cast to \c double,
     *         \c std::nullopt otherwise.
     */
    std::optional<double> GetVariableAsDouble(const std::string& varName);

    /**
     * \brief Retrieves a Python variable as a string.
     *
     * \param[in] varName Name of the Python variable.
     *
     * \return The variable value if it exists and can be cast to \c std::string,
     *         \c std::nullopt otherwise.
     */
    std::optional<std::string> GetVariableAsString(const std::string& varName);

    /**
     * \brief Binds an MITK image to a Python variable in the global dictionary.
     *
     * The image is passed by reference to Python (no copy). If the image
     * pointer is \c nullptr, the variable is set to Python's \c None.
     *
     * \param[in] image Pointer to the Image to bind. If \c nullptr, the
     *                  variable is set to \c None.
     * \param[in] varName Name of the Python variable to assign the image to.
     *
     * \throw mitk::Exception if the image cannot be bound to the variable.
     */
    void BindImage(mitk::Image* image, const std::string& varName);

    /**
     * \brief Executes arbitrary Python code with the context's dictionaries.
     *
     * The code is executed using the context's global and local dictionaries,
     * so variables set in previous calls are available in subsequent ones.
     *
     * \param[in] expression The Python code to execute. May contain multiple
     *                       statements separated by newlines.
     *
     * \throw mitk::Exception if execution fails (e.g., due to a Python error).
     */
    void Execute(const std::string &expression);

  private:
    struct Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif

