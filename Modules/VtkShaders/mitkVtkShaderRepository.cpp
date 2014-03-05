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

#define SR_INFO MITK_INFO("shader.repository")
#define SR_WARN MITK_WARN("shader.repository")
#define SR_ERROR MITK_ERROR("shader.repository")

#include "mitkVtkShaderRepository.h"
#include "mitkShaderProperty.h"
#include "mitkProperties.h"
#include "mitkDataNode.h"

#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkXMLMaterial.h>
#include <vtkXMLShader.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLMaterialParser.h>

#include <itkDirectory.h>
#include <itksys/SystemTools.hxx>

int mitk::VtkShaderRepository::shaderId = 0;
const bool mitk::VtkShaderRepository::debug = false;

mitk::VtkShaderRepository::VtkShaderRepository()
{
  LoadShaders();
}

mitk::VtkShaderRepository::~VtkShaderRepository()
{
}

void mitk::VtkShaderRepository::LoadShaders()
{
  itk::Directory::Pointer dir = itk::Directory::New();

  std::string dirPath = "./vtk_shader";

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

        element->SetName(itksys::SystemTools::GetFilenameWithoutExtension(filename));
        std::string filePath = dirPath + std::string("/") + element->GetName() + std::string(".xml");

        SR_INFO(debug) << "found shader '" << element->GetName() << "'";

        std::ifstream fileStream(filePath.c_str());
        element->LoadProperties(fileStream);

        shaders.push_back(element);
      }
    }
  }
}

mitk::VtkShaderRepository::Shader::Pointer mitk::VtkShaderRepository::GetShaderImpl(const std::string &name) const
{
  std::list<Shader::Pointer>::const_iterator i = shaders.begin();

  while( i != shaders.end() )
  {
    if( (*i)->GetName() == name)
      return (*i);

    i++;
  }

  return Shader::Pointer();
}

int mitk::VtkShaderRepository::LoadShader(std::istream& stream, const std::string& filename)
{
  Shader::Pointer element=Shader::New();
  element->SetName(filename);
  element->SetId(shaderId++);
  element->LoadProperties(stream);
  shaders.push_back(element);
  SR_INFO(debug) << "found shader '" << element->GetName() << "'";
  return element->GetId();
}

bool mitk::VtkShaderRepository::UnloadShader(int id)
{
  for (std::list<Shader::Pointer>::iterator i = shaders.begin();
       i != shaders.end(); ++i)
  {
    if ((*i)->GetId() == id)
    {
      shaders.erase(i);
      return true;
    }
  }
  return false;
}

mitk::VtkShaderRepository::Shader::Shader()
{
}

mitk::VtkShaderRepository::Shader::~Shader()
{
}

/*void mitk::VtkShaderRepository::Shader::LoadProperties(vtkPropertyXMLParser* p)
{
  vtkXMLMaterial *m=p->GetMaterial();
  if (m == NULL) return;

  // Vertexshader uniforms
  {
    vtkXMLShader *s=m->GetVertexShader();
    if (s)
    {
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
  }

  // Fragmentshader uniforms
  {
    vtkXMLShader *s=m->GetFragmentShader();
    if (s)
    {
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
  }
}*/

void mitk::VtkShaderRepository::Shader::LoadProperties(std::istream& stream)
{
  std::string content;
  content.reserve(2048);
  char buffer[2048];
  while (stream.read(buffer, sizeof(buffer)))
  {
    content.append(buffer, sizeof(buffer));
  }
  content.append(buffer, static_cast<std::size_t>(stream.gcount()));

  if (content.empty()) return;

  this->SetMaterialXml(content);

//  vtkPropertyXMLParser *p = vtkPropertyXMLParser::New(); SHADERTODO (move parsercode to here)
//  p->LoadMaterialFromString(content.c_str());
//  LoadProperties(p);
//  p->Delete();
}

mitk::VtkShaderRepository::Shader::Uniform::Uniform()
{
}

mitk::VtkShaderRepository::Shader::Uniform::~Uniform()
{
}

void mitk::VtkShaderRepository::Shader::Uniform::LoadFromXML(vtkXMLDataElement *y)
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
  }
  */
}

void mitk::VtkShaderRepository::AddDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite) const
{
  node->AddProperty( "shader", mitk::ShaderProperty::New(), renderer, overwrite );

  std::list<Shader::Pointer>::const_iterator i = shaders.begin();

  while( i != shaders.end() )
  {
    std::list<Shader::Uniform::Pointer> *l = (*i)->GetUniforms();

    std::string shaderName = (*i)->GetName();

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

void mitk::VtkShaderRepository::ApplyShaderProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, vtkXMLMaterial* xmlMaterial, itk::TimeStamp& MTime)
{
  bool setMTime = false;
  mitk::ShaderProperty *sep= dynamic_cast<mitk::ShaderProperty*>(node->GetProperty("shader",renderer));

  std::string shader=sep->GetValueAsString();
  if(shader.compare("fixed")!=0)
 {
   Shader::Pointer s=GetShaderImpl(shader);

   if(s.IsNull())
     return;

   std::list<Shader::Uniform::Pointer>::const_iterator j = s->uniforms.begin();

   while( j != s->uniforms.end() )
   {
     std::string propertyName = "shader." + s->GetName() + "." + (*j)->name;

   //  MITK_INFO << "querying property: " << propertyName;

   //  mitk::BaseProperty *p = node->GetProperty( propertyName.c_str(), renderer );

   //  if( p && p->GetMTime() > MTime.GetMTime() )
     {
       float fval[4];

      // MITK_INFO << "copying property " << propertyName << " ->->- " << (*j)->name << " type=" << (*j)->type ;


 /*      switch( (*j)->type )
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

       }*/

       //setMTime=true;
     }

     j++;
   }
 }

 if(setMTime)
   MTime.Modified();
}

vtkXMLMaterial* mitk::VtkShaderRepository::GetXMLMaterial(mitk::DataNode* node, mitk::BaseRenderer* renderer, itk::TimeStamp& MTime) const
{
  /* SHADERTODO invoke parser here
  bool setMTime = false;

  vtkPropertyXMLParser* property = new vtkPropertyXMLParser();

  unsigned long ts = MTime.GetMTime();

  mitk::ShaderProperty *sep= dynamic_cast<mitk::ShaderProperty*>(node->GetProperty("shader",renderer));

  if(!sep)
  {
    property->ShadingOff();
    return 0;
  }

  std::string shader=sep->GetValueAsString();

  // Need update pipeline mode
  if(sep->GetMTime() > ts)
  {
    if(shader.compare("fixed")==0)
    {
      //MITK_INFO << "disabling shader";
      property->ShadingOff();
      return 0;
    }
    else
    {
      Shader::Pointer s=GetShaderImpl(shader);
      if(s.IsNotNull())
      {
        //MITK_INFO << "enabling shader";
        property->ShadingOn();
        setMTime = true;
        return property->LoadMaterialFromString(s->GetMaterialXml().c_str());
      }
    }
    setMTime = true;
    return 0;
  }
*/
}

std::list<mitk::IShaderRepository::Shader::Pointer> mitk::VtkShaderRepository::GetShaders() const
{
  std::list<mitk::IShaderRepository::Shader::Pointer> result;
  for (std::list<Shader::Pointer>::const_iterator i = shaders.begin();
       i != shaders.end(); ++i)
  {
    result.push_back(i->GetPointer());
  }
  return result;
}

mitk::IShaderRepository::Shader::Pointer mitk::VtkShaderRepository::GetShader(const std::string& name) const
{
  for (std::list<Shader::Pointer>::const_iterator i = shaders.begin();
       i != shaders.end(); ++i)
  {
    if ((*i)->GetName() == name) return i->GetPointer();
  }
  return IShaderRepository::Shader::Pointer();
}

mitk::IShaderRepository::Shader::Pointer mitk::VtkShaderRepository::GetShader(int id) const
{
  for (std::list<Shader::Pointer>::const_iterator i = shaders.begin();
       i != shaders.end(); ++i)
  {
    if ((*i)->GetId() == id) return i->GetPointer();
  }
  return IShaderRepository::Shader::Pointer();
}

mitk::IShaderRepository::ShaderProgram::Pointer mitk::VtkShaderRepository::UpdateShaderProgram(mitk::IShaderRepository::ShaderProgram::Pointer & shaderProgram, DataNode* node, BaseRenderer* renderer, itk::TimeStamp& MTime) const
{
  vtkXMLMaterial* xmlMaterial = GetXMLMaterial(node,renderer,MTime);
  //SHADERTODO
}
