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

#include "mitkShowSegmentationAsSurface.h"
#include "mitkManualSegmentationToSurfaceFilter.h"
#include "mitkVtkRepresentationProperty.h"
#include <mitkCoreObjectFactory.h>

#include <vtkPolyDataNormals.h>

namespace mitk
{

ShowSegmentationAsSurface::ShowSegmentationAsSurface()
:m_UIDGeneratorSurfaces("Surface_"),
 m_AddToTree(false)
{
}


ShowSegmentationAsSurface::~ShowSegmentationAsSurface()
{
}

void ShowSegmentationAsSurface::Initialize(const NonBlockingAlgorithm* other)
{
  Superclass::Initialize(other);

  bool syncVisibility(false);

  if (other)
  {
    other->GetParameter("Sync visibility", syncVisibility);
  }

  SetParameter("Sync visibility", syncVisibility );
  SetParameter("Median kernel size", 3u);
  SetParameter("Apply median", true );
  SetParameter("Smooth", true );
  SetParameter("Gaussian SD", 1.5f );
  SetParameter("Decimate mesh", true );
  SetParameter("Decimation rate", 0.8f );
  SetParameter("Wireframe", false );
  SetParameter("Creation type", 0u );
}


bool ShowSegmentationAsSurface::ReadyToRun()
{
  try
  {
    Image::Pointer image;
    GetPointerParameter("Input", image);

    return image.IsNotNull() && GetGroupNode();
  }
  catch (std::invalid_argument&)
  {
    return false;
  }
}


bool ShowSegmentationAsSurface::ThreadedUpdateFunction()
{
  Image::Pointer image;
  GetPointerParameter("Input", image);

  bool smooth(true);
  GetParameter("Smooth", smooth);

  bool applyMedian(true);
  GetParameter("Apply median", applyMedian);

  bool decimateMesh(true);
  GetParameter("Decimate mesh", decimateMesh);

  unsigned int medianKernelSize(3);
  GetParameter("Median kernel size", medianKernelSize);

  float gaussianSD(1.5);
  GetParameter("Gaussian SD", gaussianSD );

  float reductionRate(0.8);
  GetParameter("Decimation rate", reductionRate );

  MITK_INFO << "Creating polygon model with smoothing " << smooth << " gaussianSD " << gaussianSD
                                         << " median " << applyMedian << " median kernel " << medianKernelSize
                                         << " mesh reduction " << decimateMesh << " reductionRate " << reductionRate;

  ManualSegmentationToSurfaceFilter::Pointer surfaceFilter = ManualSegmentationToSurfaceFilter::New();
  surfaceFilter->SetInput( image );
  surfaceFilter->SetThreshold( 0.5 ); //expects binary image with zeros and ones

  surfaceFilter->SetUseGaussianImageSmooth(smooth); // apply gaussian to thresholded image ?
  surfaceFilter->SetSmooth(smooth);
  if (smooth)
  {
    surfaceFilter->InterpolationOn();
    surfaceFilter->SetGaussianStandardDeviation( gaussianSD );
  }

  surfaceFilter->SetMedianFilter3D(applyMedian); // apply median to segmentation before marching cubes ?
  if (applyMedian)
  {
    surfaceFilter->SetMedianKernelSize(medianKernelSize, medianKernelSize, medianKernelSize); // apply median to segmentation before marching cubes
  }

  //fix to avoid vtk warnings see bug #5390
  if ( image->GetDimension() > 3 )
    decimateMesh = false;

  if (decimateMesh)
  {
    surfaceFilter->SetDecimate( ImageToSurfaceFilter::QuadricDecimation );
    surfaceFilter->SetTargetReduction( reductionRate );
  }
  else
  {
    surfaceFilter->SetDecimate( ImageToSurfaceFilter::NoDecimation );
  }

  surfaceFilter->UpdateLargestPossibleRegion();

  // calculate normals for nicer display
  m_Surface = surfaceFilter->GetOutput();

  vtkPolyData* polyData = m_Surface->GetVtkPolyData();

  if (!polyData) throw std::logic_error("Could not create polygon model");

  polyData->SetVerts(0);
  polyData->SetLines(0);

  if ( smooth || applyMedian || decimateMesh)
  {
    vtkPolyDataNormals* normalsGen = vtkPolyDataNormals::New();

    normalsGen->SetInputData( polyData );
    normalsGen->Update();

    m_Surface->SetVtkPolyData( normalsGen->GetOutput() );

    normalsGen->Delete();
  }
  else
  {
    m_Surface->SetVtkPolyData( polyData );
  }

  return true;
}

void ShowSegmentationAsSurface::ThreadedUpdateSuccessful()
{
  m_Node = DataNode::New();

  bool wireframe(false);
  GetParameter("Wireframe", wireframe );
  if (wireframe)
  {
    VtkRepresentationProperty *np = dynamic_cast<VtkRepresentationProperty*>(m_Node->GetProperty("material.representation"));
    if (np)
      np->SetRepresentationToWireframe();
  }

  m_Node->SetProperty("opacity", FloatProperty::New(0.3) );
  m_Node->SetProperty("line width", IntProperty::New(1) );
  m_Node->SetProperty("scalar visibility", BoolProperty::New(false) );

  std::string groupNodesName ("surface");

  DataNode* groupNode = GetGroupNode();
  if (groupNode)
  {
    groupNode->GetName( groupNodesName );
    //if parameter smooth is set add extension to node name
    bool smooth(true);
    GetParameter("Smooth", smooth);
    if(smooth) groupNodesName.append("_smoothed");
  }
  m_Node->SetProperty( "name", StringProperty::New(groupNodesName) );

  unsigned int creationType(0u);
  GetParameter("Creation type", creationType );

  m_Node->SetIntProperty( "Creation type", creationType );

  // synchronize this object's color with the parent's color
  //surfaceNode->SetProperty( "color", parentNode->GetProperty( "color" ) );
  //surfaceNode->SetProperty( "visible", parentNode->GetProperty( "visible" ) );

  m_Node->SetData( m_Surface );

  BaseProperty* colorProp = groupNode->GetProperty("color");
  if (colorProp)
    m_Node->ReplaceProperty("color", colorProp->Clone());
  else
    m_Node->SetProperty("color", ColorProperty::New(1.0, 1.0, 0.0));

  bool showResult(true);
  GetParameter("Show result", showResult );

  bool syncVisibility(false);
  GetParameter("Sync visibility", syncVisibility );

  Image::Pointer image;
  GetPointerParameter("Input", image);

  BaseProperty* organTypeProp = image->GetProperty("organ type");
  if (organTypeProp)
    m_Surface->SetProperty("organ type", organTypeProp);

  BaseProperty* visibleProp = groupNode->GetProperty("visible");
  if (visibleProp && syncVisibility)
    m_Node->ReplaceProperty("visible", visibleProp->Clone());
  else
    m_Node->SetProperty("visible", BoolProperty::New(showResult));

  InsertBelowGroupNode(m_Node);

  Superclass::ThreadedUpdateSuccessful();
}

} // namespace

