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

#ifndef _MITKSHADERREPOSITORY_H_
#define _MITKSHADERREPOSITORY_H_

#include <MitkExports.h>

#include "mitkIShaderRepository.h"

class vtkXMLDataElement;
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
 *
 * \deprecatedSince{2013_03} Use the micro service interface IShaderRepository instead.
 */
class MITK_CORE_EXPORT ShaderRepository : public itk::LightObject, public IShaderRepository
{
public:

  mitkClassMacro( ShaderRepository, itk::LightObject )

  itkFactorylessNewMacro( Self )

  DEPRECATED(static ShaderRepository *GetGlobalShaderRepository());

  /**
   * \deprecatedSince{2013_03} Use IShaderRepository::Shader instead.
   */
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

    // DEPRECATED since 2013.03
    std::string name;
    // DEPRECATED since 2013.03
    std::string path;

    DEPRECATED(void LoadPropertiesFromPath());

    Uniform *GetUniform(char * /*id*/) { return 0; }

    std::list<Uniform::Pointer> *GetUniforms()
    {
      return &uniforms;
    }

  private:

    friend class ShaderRepository;

    void LoadProperties(vtkProperty* prop);
    void LoadProperties(const std::string& path);
    void LoadProperties(std::istream& stream);

  };



protected:

  std::list<Shader::Pointer> shaders;

  void LoadShaders();

  Shader::Pointer GetShaderImpl(const std::string& name) const;

  /**
   * Constructor
   */
  ShaderRepository();

  /**
   * Destructor
   */
  ~ShaderRepository();

private:

  static int shaderId;
  static const bool debug;

public:

  DEPRECATED(std::list<Shader::Pointer> *GetShaders())
  {
    return &shaders;
  }

  DEPRECATED(Shader *GetShader(const char *id) const);

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
  void ApplyProperties(mitk::DataNode* node, vtkActor *actor, mitk::BaseRenderer* renderer,itk::TimeStamp &MTime) const;

  /** \brief Loads a shader from a given file. Make sure that this file is in the XML shader format.
   */
  DEPRECATED(int LoadShader(const std::string& filename));

  int LoadShader(std::istream& stream, const std::string& name);

  bool UnloadShader(int id);


};

} //end of namespace mitk
#endif
