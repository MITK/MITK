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

#ifndef _MITKVTKSHADERREPOSITORY_H_
#define _MITKVTKSHADERREPOSITORY_H_

#include "mitkIShaderRepository.h"
#include <vtkShaderProgram2.h>
#include <vtkXMLShader.h>


class vtkXMLDataElement;
class vtkXMLMaterial;
class vtkProperty;

namespace mitk {

/**
 * \brief Management class for vtkShader XML descriptions.
 *
 * Looks for all XML shader files in a given directory and adds default properties
 * for each shader object (shader uniforms) to the specified mitk::DataNode.
 *
 * Additionally, it provides a utility function for applying properties for shaders
 * in mappers.
 */
class VtkShaderRepository : public IShaderRepository
{

protected:

  class Shader : public IShaderRepository::Shader
  {
    public:

    mitkClassMacro( Shader, itk::Object )
    itkFactorylessNewMacro( Self )

    class Uniform : public itk::Object
    {
      public:

      mitkClassMacro( Uniform, itk::Object )
      itkFactorylessNewMacro( Self )

      enum Type
      {
        glsl_none,
        glsl_float,
        glsl_vec2,
        glsl_vec3,
        glsl_vec4,
        glsl_int,
        glsl_ivec2,
        glsl_ivec3,
        glsl_ivec4
      };

      /**
       * Constructor
       */
      Uniform();

      /**
       * Destructor
       */
      ~Uniform();

      Type type;
      std::string name;

      int defaultInt[4];
      float defaultFloat[4];

      void LoadFromXML(vtkXMLDataElement *e);
    };

    std::list<Uniform::Pointer> uniforms;

    /**
     * Constructor
     */
    Shader();

    /**
     * Destructor
     */
    ~Shader();

    void SetVertexShaderCode(const std::string& code);
    std::string GetVertexShaderCode() const;

    void SetFragmentShaderCode(const std::string& code);
    std::string GetFragmentShaderCode() const;

    void SetGeometryShaderCode(const std::string& code);
    std::string GetGeometryShaderCode() const;

    std::list<Uniform::Pointer> GetUniforms() const;

  private:

    friend class VtkShaderRepository;

    std::string m_VertexShaderCode;
    std::string m_FragmentShaderCode;
    std::string m_GeometryShaderCode;

    void LoadXmlShader(std::istream& stream);

  };

  void LoadShaders();

  Shader::Pointer GetShaderImpl(const std::string& name) const;

private:

  std::list<Shader::Pointer> shaders;

  static int shaderId;
  static const bool debug;

  public:

  /**
   * Constructor
   */
  VtkShaderRepository();

  /**
   * Destructor
   */
  ~VtkShaderRepository();

  ShaderProgram::Pointer CreateShaderProgram();

  std::list<IShaderRepository::Shader::Pointer> GetShaders() const;

  IShaderRepository::Shader::Pointer GetShader(const std::string& name) const;

  IShaderRepository::Shader::Pointer GetShader(int id) const;

  /** \brief Adds all parsed shader uniforms to property list of the given DataNode;
   * used by mappers.
   */
  void AddDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite) const;

  /** \brief Applies shader and shader specific variables of the specified DataNode
   * to the VTK object by updating the shader variables of its vtkProperty.
   */

  int LoadShader(std::istream& stream, const std::string& name);

  bool UnloadShader(int id);

  void UpdateShaderProgram(mitk::IShaderRepository::ShaderProgram* shaderProgram,
                           DataNode* node, BaseRenderer* renderer) const;
};

} //end of namespace mitk
#endif
