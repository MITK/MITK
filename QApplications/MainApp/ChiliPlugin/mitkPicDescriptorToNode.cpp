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

// CHILI-Includes
#include <chili/isg.h>
#include <chili/plugin.h>
#include <ipPic/ipPicTags.h>
#include <ipDicom/ipDicom.h>
//MITK-Includes
#define ipPicDescriptor mitkIpPicDescriptor
#include "mitkPicDescriptorToNode.h"
#include "mitkPACSPlugin.h"
#include "mitkChiliPluginEvents.h"
#include "mitkFrameOfReferenceUIDManager.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkProperties.h"
#include "math.h"
#undef ipPicDescriptor
#include <itkCommand.h>

// constructor
mitk::PicDescriptorToNode::PicDescriptorToNode()
{
  if( mitk::PACSPlugin::GetInstance() )
  {
    itk::ReceptorMemberCommand<PicDescriptorToNode>::Pointer command = itk::ReceptorMemberCommand<PicDescriptorToNode>::New();
    command->SetCallbackFunction( this, &PicDescriptorToNode::Abort );
    m_ObserverTag = mitk::PACSPlugin::GetInstance()->AddObserver( mitk::PluginAbortFilter(), command );
  }
}

// destructor
mitk::PicDescriptorToNode::~PicDescriptorToNode()
{
  if( mitk::PACSPlugin::GetInstance() )
    mitk::PACSPlugin::GetInstance()->RemoveObserver( m_ObserverTag );
}

// set-function
void mitk::PicDescriptorToNode::SetInput( std::list< mitkIpPicDescriptor* > inputPicDescriptorList, std::string inputSeriesOID )
{
  m_SeriesOID = inputSeriesOID;
  m_PicDescriptorList = inputPicDescriptorList;
  m_ImageInstanceUIDs.clear();
  m_Output.clear();
}

// get-function
std::vector< mitk::DataTreeNode::Pointer > mitk::PicDescriptorToNode::GetOutput()
{
  return m_Output;
}

std::vector< std::list< std::string > > mitk::PicDescriptorToNode::GetImageInstanceUIDs()
{
  return m_ImageInstanceUIDs;
}

// the "main"-function
void mitk::PicDescriptorToNode::Update()
{
}

// stop the filter
void mitk::PicDescriptorToNode::Abort( const itk::EventObject& )
{
  m_Abort = true;
}

// function to round
double mitk::PicDescriptorToNode::Round(double number, unsigned int decimalPlaces)
{
  double d = pow( (long double)10.0, (int)decimalPlaces );
  double x;
  if( number > 0 )
    x = floor( number * d + 0.5 ) / d;
  else
    x = floor( number * d - 0.5 ) / d;
  return x;
}

const mitk::PropertyList::Pointer mitk::PicDescriptorToNode::CreatePropertyListFromPicTags( mitkIpPicDescriptor* imageToExtractTagsFrom )
{
  if( !imageToExtractTagsFrom || !imageToExtractTagsFrom->info || !imageToExtractTagsFrom->info->tags_head )
    return NULL;

  PropertyList::Pointer resultPropertyList = PropertyList::New();
  _mitkIpPicTagsElement_t* currentTagNode = imageToExtractTagsFrom->info->tags_head;

  // Extract ALL tags from the PIC header
  while (currentTagNode)
  {
    mitkIpPicTSV_t* currentTag = currentTagNode->tsv;

    std::string propertyName = "CHILI: ";
    propertyName += currentTag->tag;

    //The currentTag->tag ends with a lot of ' ', so you find nothing if you search for the properties.
    while( propertyName[ propertyName.length() -1 ] == ' ' )
      propertyName.erase( propertyName.length() -1 );

    switch( currentTag->type )
    {
      case mitkIpPicASCII:
      {
        resultPropertyList->SetProperty( propertyName.c_str(), mitk::StringProperty::New( static_cast<char*>( currentTag->value ) ) );
        break;
      }
      case mitkIpPicInt:
      {
        resultPropertyList->SetProperty( propertyName.c_str(), mitk::IntProperty::New( *static_cast<int*>( currentTag->value ) ) );
        break;
      }
      case mitkIpPicUInt:
      {
        resultPropertyList->SetProperty( propertyName.c_str(), mitk::IntProperty::New( (int)*( (char*)( currentTag->value ) ) ) );
        break;
      }
      default:  //mitkIpPicUnknown, mitkIpPicBool, mitkIpPicFloat, mitkIpPicNonUniform, mitkIpPicTSV, _mitkIpPicTypeMax
      {
        break;
      }
    }
    // proceed to the next tag
    currentTagNode = currentTagNode->next;
  }
  return resultPropertyList;
}

std::string mitk::PicDescriptorToNode::GetImageInstanceUID( mitkIpPicDescriptor* input )
{
  std::string singleUID = "";
  mitkIpPicTSV_t* missingImageTagQuery = mitkIpPicQueryTag( input, (char*)tagIMAGE_INSTANCE_UID );
  if( missingImageTagQuery )
    singleUID = static_cast<char*>( missingImageTagQuery->value );
  else
  {
    mitkIpPicTSV_t *dicomHeader = mitkIpPicQueryTag( input, (char*)"SOURCE HEADER" );
    void* data = NULL;
    ipUInt4_t len = 0;
    if( dicomHeader && dicomFindElement( (unsigned char*) dicomHeader->value, 0x0008, 0x0018, &data, &len ) && data != NULL )
      singleUID = static_cast<char*>( data );
  }
  return singleUID;
}

void mitk::PicDescriptorToNode::GenerateData( std::list<mitkIpPicDescriptor*> slices, int sliceSteps, int timeSteps, Vector3D spacing, std::string seriesDescription )
{
  Image::Pointer resultImage = Image::New();
  std::list< std::string > ListOfUIDs;
  ListOfUIDs.clear();

  if( slices.front()->dim == 4 )
  {
    sliceSteps = slices.front()->n[2];
    timeSteps = slices.front()->n[3];
    resultImage->Initialize( slices.front() );
    resultImage->SetPicChannel( slices.front() );
    ListOfUIDs.push_back( GetImageInstanceUID( slices.front() ) );
  }
  else
  if( slices.front()->dim == 2 && (unsigned int)( timeSteps * sliceSteps ) != slices.size() )
    std::cout<<"ERROR: Calculated slicecount is not equal to the existing slices."<<std::endl;
  else
  {
    //intialize image
    if( slices.front()->dim == 3 )
    {
      sliceSteps = slices.front()->n[2];
      timeSteps = slices.size();
      resultImage->Initialize( slices.front(), 1, timeSteps, sliceSteps );
    }
    else
    {
      mitkIpPicDescriptor* header = mitkIpPicCopyHeader( slices.front(), NULL );

      if( sliceSteps == 1 )  //2D
      {
        if( timeSteps == 1 )
        {
          header->dim = 2;
          header->n[2] = 0;
          header->n[3] = 0;
        }
        else  //+t
        {
          header->dim = 4;
          header->n[2] = 1;
          header->n[3] = timeSteps;
        }
      }
      else  //3D
      {
        if( timeSteps == 1 )
        {
          header->dim = 3;
          header->n[2] = sliceSteps;
          header->n[3] = 0;
        }
        else  //+t
        {
          header->dim = 4;
          header->n[2] = sliceSteps;
          header->n[3] = timeSteps;
        }
      }
      resultImage->Initialize( header );
    }

    //get interslicegeometry
    ipPicDescriptor* chiliPic = reinterpret_cast<ipPicDescriptor*>(slices.front());
    interSliceGeometry_t* isg = ( interSliceGeometry_t* ) malloc ( sizeof( interSliceGeometry_t ) );
    if( !chiliPic || !pFetchSliceGeometryFromPic( chiliPic, isg ) )
    {
      free( isg );
      return;
    }

    Point3D origin;
    Vector3D rightVector, downVector;
    vtk2itk( isg->u, rightVector );
    vtk2itk( isg->v, downVector );
    vtk2itk( isg->o, origin );
    // its possible that a 2D-Image have no right- or down-Vector,but its not possible to initialize a [0,0,0] vector
    if( rightVector[0] == 0 && rightVector[1] == 0 && rightVector[2] == 0 )
      rightVector[0] = 1;
    if( downVector[0] == 0 && downVector[1] == 0 && downVector[2] == 0 )
      downVector[2] = -1;

    // set the timeBounds
    ScalarType timeBounds[] = {0.0, 1.0};
    // set the planeGeomtry
    PlaneGeometry::Pointer planegeometry = PlaneGeometry::New();
    planegeometry->InitializeStandardPlane( resultImage->GetDimension(0), resultImage->GetDimension(1), rightVector, downVector, &spacing );
    planegeometry->SetOrigin( origin );
    planegeometry->SetFrameOfReferenceID( FrameOfReferenceUIDManager::AddFrameOfReferenceUID( isg->forUID ) );
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
    //delete isg
    free( isg );

    // add the slices to the created mitk::Image
    if( slices.front()->dim == 3 )
    {
      int time = 0;
      for( std::list<mitkIpPicDescriptor*>::iterator picIter = slices.begin(); picIter != slices.end(); picIter++ )
      {
        resultImage->SetPicVolume( (*picIter), time );
        ListOfUIDs.push_back( GetImageInstanceUID( (*picIter) ) );
        time++;
      }
    }
    else
    {
      std::list<mitkIpPicDescriptor*>::iterator currentSlice = slices.begin();
      for( int s = 0; s < sliceSteps; s++ )
      {
        for( int t = 0; t < timeSteps; t++ )
        {
          resultImage->SetPicSlice( (*currentSlice), s, t );
          ListOfUIDs.push_back( GetImageInstanceUID( (*currentSlice) ) );
          currentSlice++;
        }
      }
    }
  }

  // if all okay create a node
  if( resultImage->IsInitialized() && resultImage.IsNotNull() )
  {
    DataTreeNode::Pointer node = mitk::DataTreeNode::New();
    node->SetData( resultImage );
    DataTreeNodeFactory::SetDefaultImageProperties( node );

    if( m_SeriesOID != "" )
      node->SetProperty( "SeriesOID", StringProperty::New( m_SeriesOID ) );
    if( seriesDescription != "" )
      node->SetProperty( "name", StringProperty::New( seriesDescription ) );
    else
      node->SetProperty( "name", StringProperty::New( "no SeriesDescription" ) );
    node->SetProperty( "NumberOfSlices", IntProperty::New( sliceSteps ) );
    node->SetProperty( "NumberOfTimeSlices", IntProperty::New( timeSteps ) );

    mitk::PropertyList::Pointer tempPropertyList = CreatePropertyListFromPicTags( slices.front() );
    for( mitk::PropertyList::PropertyMap::const_iterator iter = tempPropertyList->GetMap()->begin(); iter != tempPropertyList->GetMap()->end(); iter++ )
      node->SetProperty( iter->first.c_str(), iter->second.first );

    m_Output.push_back( node );
    m_ImageInstanceUIDs.push_back( ListOfUIDs );
  }
}
