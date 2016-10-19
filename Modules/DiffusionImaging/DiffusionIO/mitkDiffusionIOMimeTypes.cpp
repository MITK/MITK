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

#include "mitkDiffusionIOMimeTypes.h"
#include "mitkIOMimeTypes.h"
#include <itksys/SystemTools.hxx>
#include <itkNrrdImageIO.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <mitkLogMacros.h>

namespace mitk
{

std::vector<CustomMimeType*> DiffusionIOMimeTypes::Get()
{
  std::vector<CustomMimeType*> mimeTypes;

  // order matters here (descending rank for mime types)

  mimeTypes.push_back(FIBERBUNDLE_VTK_MIMETYPE().Clone());
  mimeTypes.push_back(FIBERBUNDLE_TRK_MIMETYPE().Clone());

  mimeTypes.push_back(CONNECTOMICS_MIMETYPE().Clone());

  mimeTypes.push_back(PLANARFIGURECOMPOSITE_MIMETYPE().Clone());

  return mimeTypes;
}

// Mime Types
CustomMimeType DiffusionIOMimeTypes::PLANARFIGURECOMPOSITE_MIMETYPE()
{
  CustomMimeType mimeType(PLANARFIGURECOMPOSITE_MIMETYPE_NAME());
  std::string category = "Planar Figure Composite";
  mimeType.SetComment("Planar Figure Composite");
  mimeType.SetCategory(category);
  mimeType.AddExtension("pfc");
  return mimeType;
}

CustomMimeType DiffusionIOMimeTypes::FIBERBUNDLE_VTK_MIMETYPE()
{
  CustomMimeType mimeType(FIBERBUNDLE_VTK_MIMETYPE_NAME());
  std::string category = "VTK Fibers";
  mimeType.SetComment("VTK Fibers");
  mimeType.SetCategory(category);
  mimeType.AddExtension("fib");
//  mimeType.AddExtension("vtk");
  return mimeType;
}

CustomMimeType DiffusionIOMimeTypes::FIBERBUNDLE_TRK_MIMETYPE()
{
  CustomMimeType mimeType(FIBERBUNDLE_TRK_MIMETYPE_NAME());
  std::string category = "TrackVis Fibers";
  mimeType.SetComment("TrackVis Fibers");
  mimeType.SetCategory(category);
  mimeType.AddExtension("trk");
  return mimeType;
}

CustomMimeType DiffusionIOMimeTypes::CONNECTOMICS_MIMETYPE()
{
  CustomMimeType mimeType(CONNECTOMICS_MIMETYPE_NAME());
  std::string category = "Connectomics Networks";
  mimeType.SetComment("Connectomics Networks Files");
  mimeType.SetCategory(category);
  mimeType.AddExtension("cnf");
  return mimeType;
}

CustomMimeType DiffusionIOMimeTypes::CONNECTOMICS_MATRIX_MIMETYPE()
{
  CustomMimeType mimeType(CONNECTOMICS_MATRIX_MIMETYPE_NAME());
  std::string category = "Connectomics Networks export";
  mimeType.SetComment("Connectomics Matrix Files");
  mimeType.SetCategory(category);
  mimeType.AddExtension("mat");
  return mimeType;
}

CustomMimeType DiffusionIOMimeTypes::CONNECTOMICS_LIST_MIMETYPE()
{
  CustomMimeType mimeType(CONNECTOMICS_LIST_MIMETYPE_NAME());
  std::string category = "Connectomics Networks export";
  mimeType.SetComment("Connectomics Connection Lists");
  mimeType.SetCategory(category);
  mimeType.AddExtension("txt");
  return mimeType;
}

// Names
std::string DiffusionIOMimeTypes::FIBERBUNDLE_VTK_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".FiberBundle.vtk";
  return name;
}

std::string DiffusionIOMimeTypes::FIBERBUNDLE_TRK_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".FiberBundle.trk";
  return name;
}

std::string DiffusionIOMimeTypes::CONNECTOMICS_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".cnf";
  return name;
}

std::string DiffusionIOMimeTypes::CONNECTOMICS_MATRIX_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".mat";
  return name;
}

std::string DiffusionIOMimeTypes::CONNECTOMICS_LIST_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".txt";
  return name;
}

std::string DiffusionIOMimeTypes::PLANARFIGURECOMPOSITE_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".pfc";
  return name;
}

// Descriptions
std::string DiffusionIOMimeTypes::FIBERBUNDLE_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Fiberbundles";
  return description;
}

std::string DiffusionIOMimeTypes::CONNECTOMICS_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Connectomics Networks";
  return description;
}

std::string DiffusionIOMimeTypes::PLANARFIGURECOMPOSITE_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Planar Figure Composite";
  return description;
}

}
