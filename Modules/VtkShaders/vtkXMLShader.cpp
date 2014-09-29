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
  Module:    vtkXMLShader.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkXMLShader.h"

#include "vtkObjectFactory.h"
#include "vtkXMLDataElement.h"

#include <vtksys/SystemTools.hxx>
#include <assert.h>

vtkStandardNewMacro(vtkXMLShader);
vtkCxxSetObjectMacro(vtkXMLShader, SourceLibraryElement, vtkXMLDataElement);
//-----------------------------------------------------------------------------
vtkXMLShader::vtkXMLShader()
  : Code(NULL),
    RootElement(NULL),
    SourceLibraryElement(NULL),
    Args(NULL)
{
}

//-----------------------------------------------------------------------------
vtkXMLShader::~vtkXMLShader()
{
  if (this->RootElement)
    {
    this->RootElement->UnRegister(this);
    this->RootElement = 0;
    }
  this->SetSourceLibraryElement(0);
  this->SetCode(0);
  this->CleanupArgs();
}

//-----------------------------------------------------------------------------
void vtkXMLShader::SetRootElement(vtkXMLDataElement* root)
{
  vtkSetObjectBodyMacro(RootElement, vtkXMLDataElement, root);
  this->SetCode(0);
  this->SetSourceLibraryElement(0); // release the SourceLibrary element.
}

//-----------------------------------------------------------------------------
// Note that this method allocates a new string which must be deleted by
// the caller.
char* vtkXMLShader::LocateFile(const char* filename)
{
  if(!filename)
    {
    return NULL;
    }

  // if filename is absolute path, return the same.
  if (vtksys::SystemTools::FileExists(filename))
    {
    return vtksys::SystemTools::DuplicateString(filename);
    }

  // Fetch any runtime defined user paths for materials
  std::vector<std::string> paths;
  std::string userpaths;
  vtksys::SystemTools::GetEnv("USER_MATERIALS_DIRS", userpaths);
  if (userpaths.size()>0)
    {
    vtksys::SystemTools::Split(userpaths.c_str(), paths, ';');
    }

#ifdef VTK_MATERIALS_DIRS
  // search thru default paths to locate file.
  vtksys::SystemTools::Split(VTK_MATERIALS_DIRS, paths, ';');
#endif
  for (unsigned int i =0; i < paths.size(); i++)
    {
    std::string path = paths[i];
    if (path.size() == 0)
      {
      continue;
      }
    vtksys::SystemTools::ConvertToUnixSlashes(path);
    if (path[path.size()-1] != '/')
      {
      path += "/";
      }
    path += filename;
    if (vtksys::SystemTools::FileExists(path.c_str()))
      {
      return vtksys::SystemTools::DuplicateString(path.c_str());
      }
    }
  return NULL;
}

//-----------------------------------------------------------------------------
int vtkXMLShader::GetScope()
{
  if (this->RootElement)
    {
    const char* scope = this->RootElement->GetAttribute("scope");
    if (!scope)
      {
      vtkErrorMacro("Shader description missing \"scope\" attribute.");
      }
    else if (strcmp(scope, "Vertex") == 0)
      {
      return vtkXMLShader::SCOPE_VERTEX;
      }
    else if (strcmp(scope, "Fragment") == 0)
      {
      return vtkXMLShader::SCOPE_FRAGMENT;
      }
    else if (strcmp(scope, "Geometry") == 0)
      {
      return vtkXMLShader::SCOPE_GEOMETRY;
      }
    }
  return vtkXMLShader::SCOPE_NONE;
}

// ----------------------------------------------------------------------------
// \post valid_result: result==1 || result==2
int vtkXMLShader::GetStyle()
{
  int result=1;
  if(this->RootElement)
    {
    const char *loc=this->RootElement->GetAttribute("style");
    if(loc==0)
      {
      // fine. this attribute is optional.
      }
    else
      {
      if(strcmp(loc,"1")==0)
        {
        // fine. default value.
        }
      else
        {
        if(strcmp(loc,"2")==0)
          {
          result=2; // new style
          }
        else
          {
          vtkErrorMacro(<<"style number not supported. Expect 1 or 2. We force it to be 1.");
          }
        }
      }
    }

  assert("post valid_result" && (result==1 || result==2) );
  return result;
}

//-----------------------------------------------------------------------------
const char* vtkXMLShader::GetName()
{
  return (this->RootElement)? this->RootElement->GetAttribute("name") : 0;
}

//-----------------------------------------------------------------------------
const char* vtkXMLShader::GetEntry()
{
  return (this->RootElement)? this->RootElement->GetAttribute("entry") : 0;
}

//-----------------------------------------------------------------------------
const char** vtkXMLShader::GetArgs()
{
  this->CleanupArgs();
  if (!this->RootElement || !this->RootElement->GetAttribute("args"))
    {
    return 0;
    }

  std::vector<std::string> args;
  vtksys::SystemTools::Split(this->RootElement->GetAttribute("args"), args, ' ');

  int i;
  int size = static_cast<int>(args.size());
  if (size == 0)
    {
    return 0;
    }
  this->Args = new char*[size+1];
  for (i=0; i < size; i++)
    {
    this->Args[i] = vtksys::SystemTools::DuplicateString(args[i].c_str());
    }
  this->Args[size] = 0;
  return const_cast<const char**>(this->Args);
}

//-----------------------------------------------------------------------------
const char* vtkXMLShader::GetCode()
{
  return this->RootElement->GetCharacterData();
}


//-----------------------------------------------------------------------------
void vtkXMLShader::CleanupArgs()
{
  if (this->Args)
    {
    char** a = this->Args;
    while (*a)
      {
      delete [] (*a);
      a++;
      }
    delete [] this->Args;
    this->Args = 0;
    }
}

//-----------------------------------------------------------------------------
void vtkXMLShader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Name: " << (this->GetName()? this->GetName() : "(none)")
                                                                    << endl;
  os << indent << "Scope: ";
  switch(this->GetScope())
    {
  case SCOPE_NONE:
    os << "None";
    break;
  case SCOPE_MIXED:
    os << "Mixed";
    break;
  case SCOPE_VERTEX:
    os << "Vertex";
    break;
  case SCOPE_FRAGMENT:
    os << "Fragment";
    break;
  case SCOPE_GEOMETRY:
    os << "Geometry";
    break;
    }
  os << endl;

  os << indent << "Entry: "
    <<  (this->GetEntry()? this->GetEntry() : "(none)") << endl;
  os << indent << "Args: ";
  const char** args = this->GetArgs();
  if (!args)
    {
    os << "(none)" << endl;
    }
  else
    {
    while (*args)
      {
      os << indent << *args << " ";
      args++;
      }
    os << endl;
    }

  os << indent << "RootElement: ";
  if (this->RootElement)
    {
    os << endl;
    this->RootElement->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << "(none)" << endl;
    }
}
