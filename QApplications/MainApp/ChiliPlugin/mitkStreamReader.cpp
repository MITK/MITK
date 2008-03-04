/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-06-15 14:28:00 +0200 (Fr, 15 Jun 2007) $
Version:   $Revision: 10778 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkStreamReader.h"

// CHILI-Includes
#include <chili/plugin.h>
// MITK-Includes
#include "mitkDataTreeNodeFactory.h"
#include "mitkProperties.h"

// constructor
mitk::StreamReader::StreamReader()
{
  m_SeriesOID = "";
  m_SeriesDescription = "";
  m_PicDescriptorList.clear();
  m_Geometry = NULL;
  m_Output.clear();
}

// destructor
mitk::StreamReader::~StreamReader()
{
}

// set-function
void mitk::StreamReader::SetInput( std::list< ipPicDescriptor* > inputPicDescriptorList, std::string inputSeriesOID )
{
  m_SeriesOID = inputSeriesOID;
  m_PicDescriptorList = inputPicDescriptorList;
}

void mitk::StreamReader::SetSecondInput( interSliceGeometry_t* geometry, std::string seriesDescription )
{
  m_Geometry = geometry;
  m_SeriesDescription = seriesDescription;
}

// get-function
std::vector< mitk::DataTreeNode::Pointer > mitk::StreamReader::GetOutput()
{
  return m_Output;
}

std::vector< std::list< std::string > > mitk::StreamReader::GetImageInstanceUIDs()
{
  std::vector< std::list< std::string > > imageInstanceUIDs;
  imageInstanceUIDs.clear();
  return imageInstanceUIDs;
}

// the "main"-function
void mitk::StreamReader::Update()
{
  m_Output.clear();

  if( m_SeriesOID != "" && !m_PicDescriptorList.empty() && m_Geometry != NULL )
  {
    #ifdef CHILI_PLUGIN_VERSION_CODE

    Image::Pointer resultImage = Image::New();
    Point3D origin;
    Vector3D rightVector, downVector, spacing;
    ipPicDescriptor* header;
    header = ipPicCopyHeader( m_PicDescriptorList.front(), NULL );

    //2D+t
    header->dim = 4;
    header->n[2] = 1;
    header->n[3] = m_PicDescriptorList.size();

    resultImage->Initialize( header );

    vtk2itk( m_Geometry->u, rightVector );
    vtk2itk( m_Geometry->v, downVector );
    vtk2itk( m_Geometry->ps, spacing );
    vtk2itk( m_Geometry->o, origin );

    // its possible that a 2D-Image have no right- or down-Vector,but its not possible to initialize a [0,0,0] vector
    if( rightVector[0] == 0 && rightVector[1] == 0 && rightVector[2] == 0 )
      rightVector[0] = 1;
    if( downVector[0] == 0 && downVector[1] == 0 && downVector[2] == 0 )
      downVector[2] = -1;

    // set the timeBounds
    ScalarType timeBounds[] = {0.0, 1.0};
    // set the planeGeomtry
    PlaneGeometry::Pointer planegeometry = PlaneGeometry::New();

    //spacing
    if( spacing[0] == 0 && spacing[1] == 0 && spacing[2] == 0 )
      spacing.Fill(1.0);

    planegeometry->InitializeStandardPlane( resultImage->GetDimension(0), resultImage->GetDimension(1), rightVector, downVector, &spacing );
    planegeometry->SetOrigin( origin );
    //planegeometry->SetFrameOfReferenceID( FrameOfReferenceUIDManager::AddFrameOfReferenceUID( m_ReferenceUID.c_str() ) );
    planegeometry->SetTimeBounds( timeBounds );
    // slicedGeometry
    SlicedGeometry3D::Pointer slicedGeometry = SlicedGeometry3D::New();
    slicedGeometry->InitializeEvenlySpaced( planegeometry, resultImage->GetDimension(2) );
    // timeSlicedGeometry
    TimeSlicedGeometry::Pointer timeSliceGeometry = TimeSlicedGeometry::New();
    timeSliceGeometry->InitializeEvenlyTimed( slicedGeometry, resultImage->GetDimension(3) );
    timeSliceGeometry->TransferItkToVtkTransform();
    // Image->SetGeometry
    resultImage->SetGeometry( timeSliceGeometry );

    // add the slices to the created mitk::Image
    unsigned int x = 0;
    for( std::list< ipPicDescriptor* >::iterator iter = m_PicDescriptorList.begin(); iter != m_PicDescriptorList.end(); iter++)
    {
      resultImage->SetPicSlice( (*iter), 0, x);
      x++;
    }

    // if all okay create a node, add the NumberOfSlices, NumberOfTimeSlices, SeriesOID, name, data and all pic-tags as properties
    if( resultImage->IsInitialized() && resultImage.IsNotNull() )
    {
      DataTreeNode::Pointer node = mitk::DataTreeNode::New();
      node->SetData( resultImage );
      DataTreeNodeFactory::SetDefaultImageProperties( node );

      if( m_SeriesDescription == "" )
        m_SeriesDescription = "no SeriesDescription";
      node->SetProperty( "name", new StringProperty( m_SeriesDescription ) );
      node->SetProperty( "NumberOfSlices", new IntProperty( 1 ) );
      node->SetProperty( "NumberOfTimeSlices", new IntProperty( m_PicDescriptorList.size() ) );
      if( m_SeriesOID != "" )
        node->SetProperty( "SeriesOID", new StringProperty( m_SeriesOID ) );

      m_Output.push_back( node );
    }
    #endif
  }
  else std::cout<<"StreamReader-WARNING: No SeriesOID, PicDescriptorList or Geometry set."<<std::endl;
}

