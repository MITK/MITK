/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkShowSegmentationAsSurface.h"
#include "mitkManualSegmentationToSurfaceFilter.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkMaterialProperty.h"
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

  LOG_INFO << "Creating polygon model with smoothing " << smooth << " gaussianSD " << gaussianSD 
                                         << " median " << applyMedian << " median kernel " << medianKernelSize 
                                         << " mesh reduction " << decimateMesh << " reductionRate " << reductionRate;

  ManualSegmentationToSurfaceFilter::Pointer surfaceFilter = ManualSegmentationToSurfaceFilter::New();
  surfaceFilter->SetInput( image );
  surfaceFilter->SetThreshold( 1 ); //expects binary image with zeros and ones

  surfaceFilter->SetUseGaussianImageSmooth(smooth); // apply gaussian to thresholded image ?
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
  
  if (decimateMesh)
  {
    surfaceFilter->SetDecimate( ImageToSurfaceFilter::DecimatePro );
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

    normalsGen->SetInput( polyData );
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
  m_Node = LookForPointerTargetBelowGroupNode("Surface representation");

  m_AddToTree = m_Node.IsNull();

  if (m_AddToTree)
  {
    m_Node = DataTreeNode::New();

    bool wireframe(false);
    GetParameter("Wireframe", wireframe );
    if (wireframe)
    {
      MaterialProperty* mp = dynamic_cast<MaterialProperty*>( m_Node->GetProperty("material"));
      if (mp)
      {
        mp->SetRepresentation( MaterialProperty::Wireframe );
      }
    }
  
    m_Node->SetProperty("opacity", FloatProperty::New(0.3) );
    m_Node->SetProperty("line width", IntProperty::New(1) );
    m_Node->SetProperty("scalar visibility", BoolProperty::New(false) );
    
    std::string uid = m_UIDGeneratorSurfaces.GetUID();
    m_Node->SetProperty( "FILENAME", StringProperty::New( uid + ".vtk" ) ); // undocumented feature of Image::WriteXMLData
    std::string groupNodesName ("surface");
    
    DataTreeNode* groupNode = GetGroupNode();
    if (groupNode)
    {
      groupNode->GetName( groupNodesName );
    }
    m_Node->SetProperty( "name", StringProperty::New(groupNodesName) );
   
    // synchronize this object's color with the parent's color
    //surfaceNode->SetProperty( "color", parentNode->GetProperty( "color" ) );
    //surfaceNode->SetProperty( "visible", parentNode->GetProperty( "visible" ) );
  }

  m_Node->SetData( m_Surface );

  if (m_AddToTree)
  {

    DataTreeNode* groupNode = GetGroupNode();
    if (groupNode)
    {
      groupNode->SetProperty( "Surface representation", SmartPointerProperty::New(m_Node) );
      BaseProperty* colorProp = groupNode->GetProperty("color");
      if (colorProp)
        m_Node->ReplaceProperty("color", colorProp);
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
        m_Node->ReplaceProperty("visible", visibleProp);
      else
        m_Node->SetProperty("visible", BoolProperty::New(showResult));
     }
    
    InsertBelowGroupNode(m_Node);
  }
    
  Superclass::ThreadedUpdateSuccessful();
}

} // namespace

