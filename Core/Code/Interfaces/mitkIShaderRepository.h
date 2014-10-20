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

#ifndef MITKISHADERREPOSITORY_H
#define MITKISHADERREPOSITORY_H

#include <itkLightObject.h>

#include "mitkCommon.h"
#include "mitkServiceInterface.h"

#include <list>

class vtkActor;
class vtkShaderProgram2;

namespace mitk {

class DataNode;
class BaseRenderer;

/**
 * \brief Management class for vtkShader XML descriptions.
 *
 * Loads XML shader files from std::istream objects and adds default properties
 * for each shader object (shader uniforms) to the specified mitk::DataNode.
 *
 * Additionally, it provides a utility function for applying properties for shaders
 * in mappers.
 */
struct MITK_CORE_EXPORT IShaderRepository
{

  struct ShaderPrivate;

  class MITK_CORE_EXPORT Shader : public itk::LightObject
  {

  public:

    mitkClassMacro( Shader, itk::LightObject )
    itkFactorylessNewMacro( Self )

    ~Shader();

    int GetId() const;
    std::string GetName() const;
    std::string GetMaterialXml() const;

  protected:

    Shader();

    void SetId(int id);
    void SetName(const std::string& name);
    void SetMaterialXml(const std::string& xml);

  private:

    // not implemented
    Shader(const Shader&);
    Shader& operator=(const Shader&);

    ShaderPrivate* d;

  };

  class MITK_CORE_EXPORT ShaderProgram : public itk::LightObject
  {
  public:
    virtual void Activate() = 0;
    virtual void Deactivate() = 0;
    mitkClassMacro( ShaderProgram, itk::LightObject )
  };


  virtual ~IShaderRepository();

  virtual std::list<Shader::Pointer> GetShaders() const = 0;

  /**
   * \brief Return the named shader.
   *
   * \param name The shader name.
   * \return A Shader object.
   *
   * Names might not be unique. Use the shader id to uniquely identify a shader.
   */
  virtual Shader::Pointer GetShader(const std::string& name) const = 0;

  virtual ShaderProgram::Pointer CreateShaderProgram() = 0;

  /**
   * \brief Return the shader identified by the given id.
   * @param id The shader id.
   * @return The shader object or null if the id is unknown.
   */
  virtual Shader::Pointer GetShader(int id) const = 0;

  /** \brief Adds all parsed shader uniforms to property list of the given DataNode;
   * used by mappers.
   */
  virtual void AddDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer,
                                    bool overwrite) const = 0;

  /** \brief Applies shader and shader specific variables of the specified DataNode
   * to the VTK object by updating the shader variables of its vtkProperty.
   */
  virtual void UpdateShaderProgram(mitk::IShaderRepository::ShaderProgram* shaderProgram, mitk::DataNode* node,
                                                     mitk::BaseRenderer* renderer) const = 0;

  /** \brief Loads a shader from a given file. Make sure that this stream is in the XML shader format.
   *
   * \return A unique id for the loaded shader which can be used to unload it.
   */
  virtual int LoadShader(std::istream& stream, const std::string& name) = 0;

  /**
   * \brief Unload a previously loaded shader.
   * \param id The unique shader id returned by LoadShader.
   * \return \c true if the shader id was found and the shader was successfully unloaded,
   *         \c false otherwise.
   */
  virtual bool UnloadShader(int id) = 0;
};

}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IShaderRepository, "org.mitk.services.IShaderRepository/1.0")

#endif // MITKISHADERREPOSITORY_H
