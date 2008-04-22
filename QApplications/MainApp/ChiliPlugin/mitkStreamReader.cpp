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
#include "mitkProgressBar.h"

#ifdef CHILI_PLUGIN_VERSION_CODE

// constructor
mitk::StreamReader::StreamReader()
{
  m_SeriesDescription = "";
  m_Geometry = NULL;
}

// destructor
mitk::StreamReader::~StreamReader()
{
}

// input-function
void mitk::StreamReader::SetSecondInput( interSliceGeometry_t* geometry, std::string seriesDescription )
{
  m_Geometry = geometry;
  m_SeriesDescription = seriesDescription;
}

// the "main"-function
void mitk::StreamReader::Update()
{
  m_Output.clear();

  if( m_SeriesOID != "" && !m_PicDescriptorList.empty() && m_Geometry != NULL )
  {
    ProgressBar::GetInstance()->AddStepsToDo( 3 );
    Image::Pointer resultImage = Image::New();
    Point3D origin;
    Vector3D rightVector, downVector, spacing;
    mitkIpPicDescriptor* header;
    header = mitkIpPicCopyHeader( m_PicDescriptorList.front(), NULL );

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
    ProgressBar::GetInstance()->Progress();

    // add the slices to the created mitk::Image
    unsigned int x = 0;
    for( std::list< mitkIpPicDescriptor* >::iterator iter = m_PicDescriptorList.begin(); iter != m_PicDescriptorList.end(); iter++)
    {
      resultImage->SetPicSlice( (*iter), 0, x);
      x++;
    }
    ProgressBar::GetInstance()->Progress();

    // if all okay create a node, add the NumberOfSlices, NumberOfTimeSlices, SeriesOID, name, data and all pic-tags as properties
    if( resultImage->IsInitialized() && resultImage.IsNotNull() )
    {
      DataTreeNode::Pointer node = mitk::DataTreeNode::New();
      node->SetData( resultImage );
      DataTreeNodeFactory::SetDefaultImageProperties( node );

      if( m_SeriesDescription == "" )
        m_SeriesDescription = "no SeriesDescription";
      node->SetProperty( "name", StringProperty::New( m_SeriesDescription ) );
      node->SetProperty( "NumberOfSlices", IntProperty::New( 1 ) );
      node->SetProperty( "NumberOfTimeSlices", IntProperty::New( m_PicDescriptorList.size() ) );
      if( m_SeriesOID != "" )
        node->SetProperty( "SeriesOID", StringProperty::New( m_SeriesOID ) );

      m_Output.push_back( node );
    }
    ProgressBar::GetInstance()->Progress();
  }
  else std::cout<<"StreamReader-WARNING: No SeriesOID, PicDescriptorList or Geometry set."<<std::endl;
}

#endif
