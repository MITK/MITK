/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-04-18 12:03:52 +0200 (Sat, 18 Apr 2009) $
Version:   $Revision: 16883 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#define SR_INFO MITK_INFO("shader.repository")
#define SR_WARN MITK_WARN("shader.repository")
#define SR_ERROR MITK_ERROR("shader.repository")

#include "mitkShaderRepository.h"
#include "mitkShaderEnumProperty.h"
#include "mitkProperties.h"

#include <vtkProperty.h>
#include <vtkXMLMaterial.h>
#include <vtkXMLShader.h>
#include <vtkXMLDataElement.h>

#include <itkDirectory.h>
#include <itksys/SystemTools.hxx>

#include "mitkStandardFileLocations.h"

mitk::ShaderRepository::ShaderRepository()
{
  LoadShaders();
}

mitk::ShaderRepository::~ShaderRepository()
{
}

mitk::ShaderRepository *mitk::ShaderRepository::GetGlobalShaderRepository()
{
  static mitk::ShaderRepository::Pointer i;
  
  if(i.IsNull())
  {
    i=mitk::ShaderRepository::New();
  }
  
  return i;
}


void mitk::ShaderRepository::LoadShaders()
{
  itk::Directory::Pointer dir = itk::Directory::New();
  
  std::string mitkLighting = mitk::StandardFileLocations::GetInstance()->FindFile("mitkShaderLighting.xml", "Core/Code/Rendering");

  std::string dirPath = "./vtk_shader"; 
  
  
  if(mitkLighting.size() > 0)
  {
    // we found the default shader
    dirPath = itksys::SystemTools::GetFilenamePath( mitkLighting );
    
    SR_INFO << "found default mitk shader at '" << dirPath << "'";
  }
  
  
  
  if( dir->Load( dirPath.c_str() ) )
  {  
    int n = dir->GetNumberOfFiles();
    for(int r=0;r<n;r++)
    {
      const char *filename = dir->GetFile( r );
      
      std::string extension = itksys::SystemTools::GetFilenameExtension(filename);
      
      if(extension.compare(".xml")==0)
      {
        Shader::Pointer element=Shader::New();

        element->name = itksys::SystemTools::GetFilenameWithoutExtension(filename);
        element->path = dirPath + std::string("/") + element->name + std::string(".xml");
        
        SR_INFO << "found shader '" << element->name << "'";
        
        element->LoadPropertiesFromPath();
        
        shaders.push_back(element);
      }
    }
  }
}

mitk::ShaderRepository::Shader::Shader()
{
}

mitk::ShaderRepository::Shader::~Shader()
{
}

void mitk::ShaderRepository::Shader::LoadPropertiesFromPath()
{
  vtkProperty *p;
  
  p = vtkProperty::New();
  
  p->LoadMaterial(path.c_str());
  
  vtkXMLMaterial *m=p->GetMaterial();

  // Vertexshader uniforms
  {  
    vtkXMLShader *s=m->GetVertexShader();
    vtkXMLDataElement *x=s->GetRootElement();
    int n=x->GetNumberOfNestedElements();
    for(int r=0;r<n;r++)
    {
      vtkXMLDataElement *y=x->GetNestedElement(r);
      if(!strcmp(y->GetName(),"ApplicationUniform"))
      {
          Uniform::Pointer element=Uniform::New();
          element->LoadFromXML(y);
          uniforms.push_back(element);
      }
    }
  }
  
  // Fragmentshader uniforms
  {  
    vtkXMLShader *s=m->GetFragmentShader();
    vtkXMLDataElement *x=s->GetRootElement();
    int n=x->GetNumberOfNestedElements();
    for(int r=0;r<n;r++)
    {
      vtkXMLDataElement *y=x->GetNestedElement(r);
      if(!strcmp(y->GetName(),"ApplicationUniform"))
      {
          Uniform::Pointer element=Uniform::New();
          element->LoadFromXML(y);
          uniforms.push_back(element);
      }
    }
  }
    
  p->Delete();
}




mitk::ShaderRepository::Shader::Uniform::Uniform()
{
}

mitk::ShaderRepository::Shader::Uniform::~Uniform()
{
}

mitk::ShaderRepository::Shader *mitk::ShaderRepository::GetShader(const char *id) 
{
  std::list<Shader::Pointer>::const_iterator i = shaders.begin();
  
  while( i != shaders.end() )
  {
    if( (*i)->name.compare(id) == 0)
      return (*i);
    
    i++;
  }
  
  return 0;
}
  

void mitk::ShaderRepository::Shader::Uniform::LoadFromXML(vtkXMLDataElement *y)
{
  //MITK_INFO << "found uniform '" << y->GetAttribute("name") << "' type=" << y->GetAttribute("type");// << " default=" << y->GetAttribute("value");          

  name = y->GetAttribute("name");
  
  const char *sType=y->GetAttribute("type");

  if(!strcmp(sType,"float"))
    type=glsl_float;
  else if(!strcmp(sType,"vec2"))
    type=glsl_vec2;
  else if(!strcmp(sType,"vec3"))
    type=glsl_vec3;
  else if(!strcmp(sType,"vec4"))
    type=glsl_vec4;
  else if(!strcmp(sType,"int"))
    type=glsl_int;
  else if(!strcmp(sType,"ivec2"))
    type=glsl_ivec2;
  else if(!strcmp(sType,"ivec3"))
    type=glsl_ivec3;
  else if(!strcmp(sType,"ivec4"))
    type=glsl_ivec4;
  else
  {
    type=glsl_none;
    SR_WARN << "unknown type for uniform '" << name << "'" ;
  }
              
              
  defaultFloat[0]=defaultFloat[1]=defaultFloat[2]=defaultFloat[3]=0;
              
                                     /*
  const char *sDefault=y->GetAttribute("value");
  
  switch(type)
  {
    case glsl_float:
      sscanf(sDefault,"%f",&defaultFloat[0]);
      break;
    
    case glsl_vec2:
      sscanf(sDefault,"%f %f",&defaultFloat[0],&defaultFloat[1]);
      break;
      
    case glsl_vec3:
      sscanf(sDefault,"%f %f %f",&defaultFloat[0],&defaultFloat[1],&defaultFloat[2]);
      break;
      
    case glsl_vec4:
      sscanf(sDefault,"%f %f %f %f",&defaultFloat[0],&defaultFloat[1],&defaultFloat[2],&defaultFloat[3]);
      break;
  }           */
}



void mitk::ShaderRepository::AddDefaultProperties(mitk::DataTreeNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "shader", mitk::ShaderEnumProperty::New(), renderer, overwrite );
    
  std::list<Shader::Pointer>::const_iterator i = shaders.begin();
  
  while( i != shaders.end() )
  {
    std::list<Shader::Uniform::Pointer> *l = (*i)->GetUniforms();
    
    std::string shaderName = (*i)->name;
    
    std::list<Shader::Uniform::Pointer>::const_iterator j = l->begin();
  
    while( j != l->end() )
    {
      std::string propertyName = "shader." + shaderName + "." + (*j)->name;
      
      switch( (*j)->type )
      {
        case Shader::Uniform::glsl_float:
          node->AddProperty( propertyName.c_str(), mitk::FloatProperty::New( (*j)->defaultFloat[0] ), renderer, overwrite );
          break;

        case Shader::Uniform::glsl_vec2:
          node->AddProperty( (propertyName+".x").c_str(), mitk::FloatProperty::New( (*j)->defaultFloat[0] ), renderer, overwrite );
          node->AddProperty( (propertyName+".y").c_str(), mitk::FloatProperty::New( (*j)->defaultFloat[1] ), renderer, overwrite );
          break;

        case Shader::Uniform::glsl_vec3:
          node->AddProperty( (propertyName+".x").c_str(), mitk::FloatProperty::New( (*j)->defaultFloat[0] ), renderer, overwrite );
          node->AddProperty( (propertyName+".y").c_str(), mitk::FloatProperty::New( (*j)->defaultFloat[1] ), renderer, overwrite );
          node->AddProperty( (propertyName+".z").c_str(), mitk::FloatProperty::New( (*j)->defaultFloat[2] ), renderer, overwrite );
          break;

       case Shader::Uniform::glsl_vec4:
          node->AddProperty( (propertyName+".x").c_str(), mitk::FloatProperty::New( (*j)->defaultFloat[0] ), renderer, overwrite );
          node->AddProperty( (propertyName+".y").c_str(), mitk::FloatProperty::New( (*j)->defaultFloat[1] ), renderer, overwrite );
          node->AddProperty( (propertyName+".z").c_str(), mitk::FloatProperty::New( (*j)->defaultFloat[2] ), renderer, overwrite );
          node->AddProperty( (propertyName+".w").c_str(), mitk::FloatProperty::New( (*j)->defaultFloat[3] ), renderer, overwrite );
          break;

       default:
          break;

      }
  
      j++;
      
    }
    
    i++;
  }

  
}
                                   
void mitk::ShaderRepository::ApplyProperties(mitk::DataTreeNode* node, vtkActor *actor, mitk::BaseRenderer* renderer,itk::TimeStamp &MTime)
{
  bool setMTime = false;

  vtkProperty* property = actor->GetProperty();
  
  unsigned long ts = MTime.GetMTime();
  
  mitk::ShaderEnumProperty *sep=(mitk::ShaderEnumProperty *)node->GetProperty("shader",renderer);

  if(!sep)
  {
    property->ShadingOff();
    return;
  }

  std::string shader=sep->GetValueAsString();
  
  // Need update pipeline mode
  if(sep->GetMTime() > ts)
  {
    if(shader.compare("fixed")==0)
    {
      //MITK_INFO << "disabling shader";
      property->ShadingOff();
    }
    else
    {
      Shader *s=GetShader(shader.c_str());
      if(s)
      {
        //MITK_INFO << "enabling shader";
        property->ShadingOn(); 
        property->LoadMaterial(s->path.c_str());
      }
    }
    setMTime = true;
  }    

   if(shader.compare("fixed")!=0)
  {
    Shader *s=GetShader(shader.c_str());
    
    if(!s)
      return;
    
    std::list<Shader::Uniform::Pointer>::const_iterator j = s->uniforms.begin();
  
    while( j != s->uniforms.end() )
    {
      std::string propertyName = "shader." + s->name + "." + (*j)->name;

    //  MITK_INFO << "querying property: " << propertyName;
      
    //  mitk::BaseProperty *p = node->GetProperty( propertyName.c_str(), renderer );
      
    //  if( p && p->GetMTime() > MTime.GetMTime() )
      {
        float fval[4];
        
       // MITK_INFO << "copying property " << propertyName << " ->->- " << (*j)->name << " type=" << (*j)->type ;
        
               
        switch( (*j)->type )
        {
          case Shader::Uniform::glsl_float:
            node->GetFloatProperty( propertyName.c_str(), fval[0], renderer );
            property->AddShaderVariable( (*j)->name.c_str(), 1 , fval );
            break;
                                      
          case Shader::Uniform::glsl_vec2:
            node->GetFloatProperty( (propertyName+".x").c_str(), fval[0], renderer );
            node->GetFloatProperty( (propertyName+".y").c_str(), fval[1], renderer );
            property->AddShaderVariable( (*j)->name.c_str(), 2 , fval );
            break;

          case Shader::Uniform::glsl_vec3:
            node->GetFloatProperty( (propertyName+".x").c_str(), fval[0], renderer );
            node->GetFloatProperty( (propertyName+".y").c_str(), fval[1], renderer );
            node->GetFloatProperty( (propertyName+".z").c_str(), fval[2], renderer );
            
            property->AddShaderVariable( (*j)->name.c_str(), 3 , fval );
            break;

         case Shader::Uniform::glsl_vec4:
            node->GetFloatProperty( (propertyName+".x").c_str(), fval[0], renderer );
            node->GetFloatProperty( (propertyName+".y").c_str(), fval[1], renderer );
            node->GetFloatProperty( (propertyName+".z").c_str(), fval[2], renderer );
            node->GetFloatProperty( (propertyName+".w").c_str(), fval[3], renderer );
            property->AddShaderVariable( (*j)->name.c_str(), 4 , fval );
            break;
         
         default:
          break;

        }
      
        //setMTime=true;
      }
        
      j++;
    }
  }
  
  if(setMTime)
    MTime.Modified();
}


