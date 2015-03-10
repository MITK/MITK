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

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkXMLMaterial.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkXMLMaterial.h"

#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLMaterialParser.h"
#include "vtkXMLShader.h"

#include <vector>
#include <assert.h>

class vtkXMLMaterialInternals
{
public:
  typedef std::vector<vtkXMLDataElement*> VectorOfElements;
  typedef std::vector<vtkSmartPointer<vtkXMLShader> > VectorOfShaders;
  VectorOfElements Properties;
  VectorOfShaders VertexShaders;
  VectorOfShaders FragmentShaders;
  VectorOfShaders GeometryShaders;
  VectorOfElements Textures;
  void Initialize()
    {
    this->Properties.clear();
    this->VertexShaders.clear();
    this->FragmentShaders.clear();
    this->GeometryShaders.clear();
    this->Textures.clear();
    }
};

vtkStandardNewMacro(vtkXMLMaterial);

//-----------------------------------------------------------------------------
vtkXMLMaterial::vtkXMLMaterial()
{
  this->RootElement = 0;
  this->Internals = new vtkXMLMaterialInternals;
}

//-----------------------------------------------------------------------------
vtkXMLMaterial::~vtkXMLMaterial()
{
  this->SetRootElement(0);
  delete this->Internals;
}

vtkXMLMaterial* vtkXMLMaterial::CreateInstance(const char* name)
{
  if (!name)
    {
    return 0;
    }

  vtkXMLMaterialParser* parser = vtkXMLMaterialParser::New();
  vtkXMLMaterial* material = vtkXMLMaterial::New();
  parser->SetMaterial(material);

  // First, look for material library files.
  // Then, look for Repository files.

  char* filename = vtkXMLShader::LocateFile(name);
  if (filename)
    {
    parser->SetFileName( filename );
    delete [] filename;
    parser->Parse();
    parser->Delete();
    return material;
    }

  parser->Delete();
  material->Delete();
  return NULL;
}

//-----------------------------------------------------------------------------
void vtkXMLMaterial::SetRootElement(vtkXMLDataElement* root)
{
  this->Internals->Initialize();

  vtkSetObjectBodyMacro(RootElement, vtkXMLDataElement, root);
  if (this->RootElement)
    {
    // Update the internal data structure to
    // avoid repeated searches.
    int numElems = this->RootElement->GetNumberOfNestedElements();
    for (int i=0; i<numElems; i++)
      {
      vtkXMLDataElement* elem = this->RootElement->GetNestedElement(i);
      const char* name = elem->GetName();
      if (!name)
        {
        continue;
        }
      if (strcmp(name, "Property") == 0)
        {
        this->Internals->Properties.push_back(elem);
        }
      else if (strcmp(name, "Shader") == 0)
        {
        vtkXMLShader* shader = vtkXMLShader::New();
        shader->SetRootElement(elem);

        switch (shader->GetScope())
          {
        case vtkXMLShader::SCOPE_VERTEX:
          this->Internals->VertexShaders.push_back(shader);
          break;
        case vtkXMLShader::SCOPE_FRAGMENT:
          this->Internals->FragmentShaders.push_back(shader);
          break;
        case vtkXMLShader::SCOPE_GEOMETRY:
          this->Internals->GeometryShaders.push_back(shader);
          break;
        default:
          vtkErrorMacro("Invalid scope for shader: " << shader->GetName());
          }

        shader->Delete();
        }
      else if (strcmp(name, "Texture") == 0)
        {
        this->Internals->Textures.push_back(elem);
        }
      }
    }
}

//-----------------------------------------------------------------------------
int vtkXMLMaterial::GetNumberOfProperties()
{
  return static_cast<int>(this->Internals->Properties.size());
}

//-----------------------------------------------------------------------------
int vtkXMLMaterial::GetNumberOfTextures()
{
  return static_cast<int>(this->Internals->Textures.size());
}

//-----------------------------------------------------------------------------
int vtkXMLMaterial::GetNumberOfVertexShaders()
{
  return static_cast<int>(this->Internals->VertexShaders.size());
}

//-----------------------------------------------------------------------------
int vtkXMLMaterial::GetNumberOfFragmentShaders()
{
  return static_cast<int>(this->Internals->FragmentShaders.size());
}

int vtkXMLMaterial::GetNumberOfGeometryShaders()
{
  return static_cast<int>(this->Internals->GeometryShaders.size());
}

//-----------------------------------------------------------------------------
vtkXMLDataElement* vtkXMLMaterial::GetProperty(int id)
{
  if (id < this->GetNumberOfProperties())
    {
    return this->Internals->Properties[id];
    }
  return NULL;
}

//-----------------------------------------------------------------------------
vtkXMLDataElement* vtkXMLMaterial::GetTexture(int index)
{
  if (index < this->GetNumberOfTextures())
    {
    return this->Internals->Textures[index];
    }
  return NULL;
}

//-----------------------------------------------------------------------------
vtkXMLShader* vtkXMLMaterial::GetVertexShader(int id)
{
  if (id < this->GetNumberOfVertexShaders())
    {
    return this->Internals->VertexShaders[id].GetPointer();
    }
  return NULL;
}

//-----------------------------------------------------------------------------
vtkXMLShader* vtkXMLMaterial::GetFragmentShader(int id)
{
  if (id < this->GetNumberOfFragmentShaders())
    {
    return this->Internals->FragmentShaders[id].GetPointer();
    }
  return NULL;
}


vtkXMLShader* vtkXMLMaterial::GetGeometryShader(int id)
{
  if (id < this->GetNumberOfGeometryShaders())
    {
    return this->Internals->GeometryShaders[id].GetPointer();
    }
  return NULL;
}

//----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Description:
// Get the style the shaders.
// \post valid_result: result==1 || result==2
int vtkXMLMaterial::GetShaderStyle()
{
  int result = 1;
  int vStyle = 0;
  if (this->GetVertexShader())
  {
    vStyle = this->GetVertexShader()->GetStyle();
  }
  int fStyle = 0;
  if (this->GetFragmentShader())
  {
    fStyle=this->GetFragmentShader()->GetStyle();
  }
  int gStyle = 0;
  if (this->GetGeometryShader())
  {
    gStyle=this->GetGeometryShader()->GetStyle();
  }
  if (vStyle!=0 && fStyle!=0 && !gStyle && vStyle!=fStyle )
  {
    vtkErrorMacro(<<"vertex shader and fragment shader style differ.");
  }
  else
  {
    if (vStyle!=0)
    {
      result = vStyle;
    }
    else
    {
      result = fStyle;
    }
  }


  assert("post: valid_result" && (result==1 || result==2) );
  return result;
}

//-----------------------------------------------------------------------------
void vtkXMLMaterial::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Number of Properties: " << this->GetNumberOfProperties()
    << endl;
  os << indent << "Number of Vertex Shaders: "
    << this->GetNumberOfVertexShaders() << endl;
  os << indent << "Number of Fragment Shaders: "
    << this->GetNumberOfFragmentShaders() << endl;
  os << indent << "RootElement: ";
  if (this->RootElement)
    {
    os << endl;
    this->RootElement->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << "(null)" << endl;
    }
}
