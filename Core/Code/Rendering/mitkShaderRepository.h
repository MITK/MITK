/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-08 13:23:19 +0100 (Fri, 08 Feb 2008) $
Version:   $Revision: 13561 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef _MITKSHADERREPOSITORY_H_
#define _MITKSHADERREPOSITORY_H_

#include "mitkCommon.h"
#include "itkObject.h"
#include "itkObjectFactory.h"
#include <vtkXMLDataElement.h>

#include "mitkBaseRenderer.h"
#include "mitkDataTreeNode.h"

namespace mitk {

/**
 * \brief Management class for vtkShader XML descriptions.
 *
 * Looks for all XML shader files in a given directory and adds default properties
 * for each shader object (shader uniforms) to the specified mitk::DataTreeNode.
 *
 * Additionally, it provides a utility function for applying properties for shaders
 * in mappers.
 */
class MITK_CORE_EXPORT ShaderRepository : public itk::Object
{
public:

  mitkClassMacro( ShaderRepository, itk::Object );
  
  itkNewMacro( Self );

  static ShaderRepository *GetGlobalShaderRepository();  

  class Shader : public itk::Object
  {
    public:

    mitkClassMacro( Shader, itk::Object );
    itkNewMacro( Self );

    class Uniform : public itk::Object
    {
      public:

      mitkClassMacro( Uniform, itk::Object );
      itkNewMacro( Self );

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
    
    std::string name;
    std::string path;
    
    void LoadPropertiesFromPath();
    
    Uniform *GetUniform(char * /*id*/) { return 0; }
    
    std::list<Uniform::Pointer> *GetUniforms()
    {
      return &uniforms;
    }
  }; 
  
protected:

  std::list<Shader::Pointer> shaders;

  void LoadShaders();


  /**
   * Constructor
   */
  ShaderRepository();

  /**
   * Destructor
   */
  ~ShaderRepository();

public:

  std::list<Shader::Pointer> *GetShaders()
  {
    return &shaders;
  }
  
  Shader *GetShader(const char *id);
  
  
  /** \brief Adds all parsed shader uniforms to property list of the given DataTreeNode;
   * used by mappers.
   */
  void AddDefaultProperties(mitk::DataTreeNode* node, mitk::BaseRenderer* renderer, bool overwrite);

  /** \brief Applies shader and shader specific variables of the specified DataTreeNode
   * to the VTK object by updating the shader variables of its vtkProperty.
   */
  void ApplyProperties(mitk::DataTreeNode* node, vtkActor *actor, mitk::BaseRenderer* renderer,itk::TimeStamp &MTime);


};

} //end of namespace mitk
#endif


