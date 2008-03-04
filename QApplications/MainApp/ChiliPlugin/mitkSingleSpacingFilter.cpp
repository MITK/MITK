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

#include "mitkSingleSpacingFilter.h"

// Chili-Includes
#include <chili/isg.h>
#include <chili/plugin.h>
#include <ipPic/ipPic.h>
#include <ipPic/ipPicTags.h>
#include <ipDicom/ipDicom.h>
// MITK-Includes
#include "mitkFrameOfReferenceUIDManager.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkProperties.h"

#include "math.h"
#include <stdlib.h>

//helpfunction to sort
bool mitk::SingleSpacingFilter::PositionSort( const Position& elem1, const Position& elem2 )
{
  return elem1.origin*elem1.normal < elem2.origin*elem2.normal; // projection of origin on inter-slice-direction
}

bool mitk::SingleSpacingFilter::SliceSort( ipPicDescriptor* elem1, ipPicDescriptor* elem2 )
{
  int imageNumberPic1 = 0, imageNumberPic2 = 0;

  ipPicTSV_t* imagenumberTag1 = ipPicQueryTag( elem1, (char*)tagIMAGE_NUMBER );
  if( imagenumberTag1 && imagenumberTag1->type == ipPicInt )
    imageNumberPic1 = *( (int*)(imagenumberTag1->value) );
  else
  {
    ipPicTSV_t *tsv;
    void* data = NULL;
    ipUInt4_t len = 0;
    tsv = ipPicQueryTag( elem1, (char*)"SOURCE HEADER" );
    if( tsv )
      if( dicomFindElement( (unsigned char*) tsv->value, 0x0020, 0x0013, &data, &len ) )
        sscanf( (char *) data, "%d", &imageNumberPic1 );
  }

  ipPicTSV_t* imagenumberTag2 = ipPicQueryTag( elem2, (char*)tagIMAGE_NUMBER );
  if( imagenumberTag2 && imagenumberTag2->type == ipPicInt )
    imageNumberPic2 = *( (int*)(imagenumberTag2->value) );
  else
  {
    ipPicTSV_t *tsv;
    void* data = NULL;
    ipUInt4_t len = 0;
    tsv = ipPicQueryTag( elem2, (char*)"SOURCE HEADER" );
    if( tsv )
      if( dicomFindElement( (unsigned char*) tsv->value, 0x0020, 0x0013, &data, &len ) )
        sscanf( (char *) data, "%d", &imageNumberPic2 );
  }

  return imageNumberPic1 < imageNumberPic2;
}

//sort
void mitk::SingleSpacingFilter::SortGroupsAndSlices()
{
  for( std::vector< Group >::iterator groupIter = m_GroupVector.begin(); groupIter != m_GroupVector.end(); groupIter++ )
  {
    std::sort( groupIter->includedPositions.begin(), groupIter->includedPositions.end(), PositionSort );
    for( std::vector< Position >::iterator positionIter = groupIter->includedPositions.begin(); positionIter != groupIter->includedPositions.end(); positionIter++ )
    {
      std::sort( positionIter->includedPics.begin(), positionIter->includedPics.end(), SliceSort );
    }
  }
}

// function to round
double mitk::SingleSpacingFilter::Round(double number, unsigned int decimalPlaces)
{
  double d = pow( (long double)10.0, (int)decimalPlaces );
  double x;
  if( number > 0 )
    x = floor( number * d + 0.5 ) / d;
  else
    x = floor( number * d - 0.5 ) / d;
  return x;
}

// constructor
mitk::SingleSpacingFilter::SingleSpacingFilter()
{
}

// destructor
mitk::SingleSpacingFilter::~SingleSpacingFilter()
{
}

// set-function
void mitk::SingleSpacingFilter::SetInput( std::list< ipPicDescriptor* > inputPicDescriptorList, std::string inputSeriesOID )
{
  m_SeriesOID = inputSeriesOID;
  m_PicDescriptorList = inputPicDescriptorList;
}

// get-function
std::vector< mitk::DataTreeNode::Pointer > mitk::SingleSpacingFilter::GetOutput()
{
  return m_Output;
}

std::vector< std::list< std::string > > mitk::SingleSpacingFilter::GetImageInstanceUIDs()
{
  return m_ImageInstanceUIDs;
}

// the "main"-function
void mitk::SingleSpacingFilter::Update()
{
  m_GroupVector.clear();
  m_Output.clear();
  m_ImageInstanceUIDs.clear();

  if( m_PicDescriptorList.size() > 0 && m_SeriesOID != "" )
  {
    SortSlicesToGroup();
    SortGroupsAndSlices();
    CreateResults();
  }
  else std::cout<<"SingleSpacingFilter-WARNING: No SeriesOID or PicDescriptorList set."<<std::endl;
}

void mitk::SingleSpacingFilter::SortSlicesToGroup()
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  for( std::list< ipPicDescriptor* >::iterator currentPic = m_PicDescriptorList.begin(); currentPic != m_PicDescriptorList.end(); currentPic ++ )
  {
    //check intersliceGeomtry
    interSliceGeometry_t* isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
    if( !pFetchSliceGeometryFromPic( (*currentPic), isg ) )
    {
      //PicDescriptor without a geometry not able to sort in a volume
      std::cout<<"SingleSpacingFilter-WARNING: Found image without SliceGeometry. Image ignored."<<std::endl;
      free( isg );
      continue;
    }

    //dimension
    unsigned int currentDimension = (*currentPic)->dim;
    if( currentDimension < 2 || currentDimension > 4 )
    {
      std::cout<<"SingleSpacingFilter-WARNING: Wrong PicDescriptor-Dimension. Image ignored."<<std::endl;
      free( isg );
      continue;
    }

    //SliceOrigin
    Vector3D sliceOrigin;
    vtk2itk( isg->o, sliceOrigin );
    //pixelSize
    Vector3D currentPixelSize;
    vtk2itk( isg->ps, currentPixelSize );
    //seriesDescription
    std::string currentSeriesDescription;
    ipPicTSV_t* seriesDescriptionTag = ipPicQueryTag( (*currentPic), (char*)tagSERIES_DESCRIPTION );
    if( seriesDescriptionTag )
      currentSeriesDescription = static_cast<char*>( seriesDescriptionTag->value );
    else
    {
      ipPicTSV_t *tsv;
      void* data = NULL;
      ipUInt4_t len = 0;
      tsv = ipPicQueryTag( (*currentPic), (char*)"SOURCE HEADER" );
      if( tsv && dicomFindElement( (unsigned char*) tsv->value, 0x0008, 0x103e, &data, &len ) )
        if( data != NULL )
          currentSeriesDescription = (char*)data;
      else
        currentSeriesDescription = "no Description";
    }
    //normale with ImageSize
    Vector3D rightVector, downVector;
    vtk2itk( isg->u, rightVector );
    vtk2itk( isg->v, downVector );
    rightVector = rightVector * (*currentPic)->n[0];
    downVector = downVector * (*currentPic)->n[1];
    Vector3D currentNormalWithImageSize;
    currentNormalWithImageSize[0] = Round( ( ( rightVector[1]*downVector[2] ) - ( rightVector[2]*downVector[1] ) ), 2 );
    currentNormalWithImageSize[1] = Round( ( ( rightVector[2]*downVector[0] ) - ( rightVector[0]*downVector[2] ) ), 2);
    currentNormalWithImageSize[2] = Round( ( ( rightVector[0]*downVector[1] ) - ( rightVector[1]*downVector[0] ) ), 2);

    bool foundMatch;
    int curCount, maxCount;

    if( currentDimension == 4 )
    {
      //with this combination, no search initialize and a new group created
      foundMatch = false;
      curCount = 0;
      maxCount = 0;
    }
    else
    {
      //initialize searching
      foundMatch = false;
      curCount = 0;
      maxCount = m_GroupVector.size();
    }

    // searching for equal output
    while( curCount < maxCount && !foundMatch )
    {
      //checking referenceUID, seriesDescription, pixelSize and NormaleWithSize
      if( m_GroupVector[ curCount ].referenceUID == isg->forUID && m_GroupVector[ curCount ].seriesDescription == currentSeriesDescription && m_GroupVector[ curCount ].pixelSize == currentPixelSize && m_GroupVector[ curCount ].normalWithImageSize == currentNormalWithImageSize )
      {
        if( m_GroupVector[ curCount ].dimension == 2 || ( m_GroupVector[ curCount ].dimension == 3 && m_GroupVector[ curCount ].includedPositions.front().origin == sliceOrigin && m_GroupVector[ curCount ].includedPositions.front().includedPics.front()->n[2] == (*currentPic)->n[2] ) )
          foundMatch = true;
        else
          curCount++;
      }
      else
        curCount++;
    }

    //if a matching output found, searching for PositionAtSpace else create a new group and PositionAtSpace
    if( foundMatch )
    {
      //searching for PositionAtSpace, therefore using the origin
      bool foundPositionMatch = false;
      int curPosCount = 0;
      int maxPosCount = m_GroupVector[ curCount ].includedPositions.size();
      while( curPosCount < maxPosCount && !foundPositionMatch )
      {
        if( m_GroupVector[ curCount ].includedPositions[curPosCount].origin == sliceOrigin )
          foundPositionMatch = true;
        else
          curPosCount++;
      }
      //PositionAtSpace always exist
      if( foundPositionMatch )
        m_GroupVector[ curCount ].includedPositions[curPosCount].includedPics.push_back( (*currentPic) );
      else
      //dont exist yet
      {
        Position newPosition;
        newPosition.origin = sliceOrigin;
        Vector3D rightVector, downVector;
        vtk2itk( isg->u, rightVector );
        vtk2itk( isg->v, downVector );
        newPosition.normal[0] = Round( ( ( rightVector[1]*downVector[2] ) - ( rightVector[2]*downVector[1] ) ), 2 );
        newPosition.normal[1] = Round( ( ( rightVector[2]*downVector[0] ) - ( rightVector[0]*downVector[2] ) ), 2);
        newPosition.normal[2] = Round( ( ( rightVector[0]*downVector[1] ) - ( rightVector[1]*downVector[0] ) ), 2);
        newPosition.includedPics.clear();
        newPosition.includedPics.push_back( (*currentPic) );
        m_GroupVector[ curCount ].includedPositions.push_back( newPosition );
      }
    }
    else
    //create a new Group and PositionAtSpace
    {
      Group newGroup;
      newGroup.referenceUID = isg->forUID;
      newGroup.seriesDescription = currentSeriesDescription;
      newGroup.dimension = currentDimension;
      newGroup.pixelSize = currentPixelSize;
      newGroup.normalWithImageSize = currentNormalWithImageSize;
      newGroup.includedPositions.clear();

      Position newPosition;
      newPosition.origin = sliceOrigin;
      Vector3D rightVector, downVector;
      vtk2itk( isg->u, rightVector );
      vtk2itk( isg->v, downVector );
      newPosition.normal[0] = Round( ( ( rightVector[1]*downVector[2] ) - ( rightVector[2]*downVector[1] ) ), 2 );
      newPosition.normal[1] = Round( ( ( rightVector[2]*downVector[0] ) - ( rightVector[0]*downVector[2] ) ), 2);
      newPosition.normal[2] = Round( ( ( rightVector[0]*downVector[1] ) - ( rightVector[1]*downVector[0] ) ), 2);
      newPosition.includedPics.clear();
      newPosition.includedPics.push_back( (*currentPic) );
      newGroup.includedPositions.push_back( newPosition );
      m_GroupVector.push_back( newGroup );
    }
  free( isg );
  }
#endif
}

void mitk::SingleSpacingFilter::CreateResults()
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  for( unsigned int x = 0; x < m_GroupVector.size(); x++)
    while( !m_GroupVector[x].includedPositions.empty() )
      SearchParameter( x );
#endif
}

void mitk::SingleSpacingFilter::SearchParameter( unsigned int currentGroup )
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  std::vector<Position*> usedPos;
  usedPos.clear();
  unsigned int timeCount = 0;
  Vector3D spacing;

  if( ( m_GroupVector[currentGroup].dimension == 3 ) || ( m_GroupVector[currentGroup].dimension == 4 ) )
  {
    timeCount = m_GroupVector[currentGroup].includedPositions.size();
    usedPos.push_back( &(m_GroupVector[currentGroup].includedPositions.front()) );

    interSliceGeometry_t* isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
    pFetchSliceGeometryFromPic( usedPos.front()->includedPics.front(), isg );

    vtk2itk( isg->ps, spacing );
    if( spacing[0] == 0 && spacing[1] == 0 && spacing[2] == 0 )
      spacing.Fill(1.0);
    free( isg );
  }
  else
  if( ( m_GroupVector[currentGroup].includedPositions.size() == 1 ) || ( m_GroupVector[currentGroup].includedPositions.size() == 2 ) ) //2D or 2D+t
  {
    timeCount = m_GroupVector[currentGroup].includedPositions.front().includedPics.size();
    usedPos.push_back( &(m_GroupVector[currentGroup].includedPositions.front()) );

    interSliceGeometry_t* isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
    pFetchSliceGeometryFromPic( usedPos.front()->includedPics.front(), isg );

    vtk2itk( isg->ps, spacing );
    if( spacing[0] == 0 && spacing[1] == 0 && spacing[2] == 0 )
      spacing.Fill(1.0);
  }
  else  //3D or 3D+t
  {
    std::vector< Position >::iterator iterEnd = m_GroupVector[currentGroup].includedPositions.end();
    iterEnd--;

    for( std::vector< Position >::iterator iterRoot = m_GroupVector[currentGroup].includedPositions.begin(); iterRoot != iterEnd; iterRoot++ )
    {
      for( std::vector< Position >::iterator iterWalk = iterRoot; iterWalk != m_GroupVector[currentGroup].includedPositions.end(); iterWalk++ )
      {
        if( !Equal( iterWalk->origin, iterRoot->origin ) )
        {
          std::vector<Position*> tempUsedPos;
          tempUsedPos.clear();
          tempUsedPos.push_back( &(*iterRoot) );
          tempUsedPos.push_back( &(*iterWalk) );
          unsigned int tempTimeCount = iterRoot->includedPics.size();

          //calculate spacing
          Vector3D tempDistance = iterWalk->origin - iterRoot->origin;
          double tempRoundedSpacing = Round( tempDistance.GetNorm(), 2 );

          std::vector< Position >::iterator iterRef = iterWalk;
          for( std::vector< Position >::iterator iterSpacingWalk = iterRef; iterSpacingWalk != m_GroupVector[currentGroup].includedPositions.end(); iterSpacingWalk++ )
          {
            if( !Equal( iterRef->origin, iterSpacingWalk->origin ) )
            {
              tempDistance = iterSpacingWalk->origin - iterRef->origin;
              double tempReferenceSpacing = Round( tempDistance.GetNorm(), 2 );

              if( tempReferenceSpacing == tempRoundedSpacing )
              {
                if( iterSpacingWalk->includedPics.size() < tempTimeCount )
                  tempTimeCount = iterSpacingWalk->includedPics.size();
                iterRef = iterSpacingWalk;
                tempUsedPos.push_back( &(*iterSpacingWalk) );
              }
              else
                if( tempReferenceSpacing > tempRoundedSpacing )
                  break;
            }
          }

          if( tempUsedPos.size() > usedPos.size() )
          {
            usedPos = tempUsedPos;
            timeCount = tempTimeCount;
          }
        }
      }
    }

    interSliceGeometry_t* isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
    pFetchSliceGeometryFromPic( usedPos.front()->includedPics.front(), isg );

    vtk2itk( isg->ps, spacing );
    if( spacing[0] == 0 && spacing[1] == 0 && spacing[2] == 0 )
      spacing.Fill(1.0);

    free( isg );

    //get the most counted not rounded spacing
    std::list<Spacing> SpacingList;
    std::vector<Position*>::iterator walkIter = usedPos.begin();
    walkIter++;

    for( ; walkIter != usedPos.end(); walkIter ++)
    {
      std::vector<Position*>::iterator iterB4 = walkIter;
      iterB4--;
      Vector3D tempDistance = (*walkIter)->origin - (*iterB4)->origin;
      spacing[2] = tempDistance.GetNorm();

      //search for spacing
      std::list<Spacing>::iterator searchIter = SpacingList.begin();
      while( searchIter != SpacingList.end() )
      {
        if( searchIter->spacing == spacing )
        {
          searchIter->count++;
          break;
        }
        else
          searchIter++;
      }

      //dont exist, create new entry
      if( searchIter == SpacingList.end() )
      {
        Spacing newElement;
        newElement.spacing = spacing;
        newElement.count = 1;
        SpacingList.push_back( newElement );
      }
    }

    //get maximum spacing
    int count = 0;
    for( std::list<Spacing>::iterator searchIter = SpacingList.begin(); searchIter != SpacingList.end(); searchIter++ )
    {
      if( searchIter->count > count )
      {
        spacing = searchIter->spacing;
        count = searchIter->count;
      }
    }
  }

  //create mitk::images
  GenerateNodes( usedPos, spacing, timeCount, currentGroup );

  //delete used slices and positions
  std::vector<Position*>::iterator deleteGroup = usedPos.end();
  do
  {
    deleteGroup--;
    std::vector< Position >::iterator searchGroup = m_GroupVector[currentGroup].includedPositions.begin();

    while( !Equal( searchGroup->origin, (*deleteGroup)->origin ) && searchGroup != m_GroupVector[currentGroup].includedPositions.end() )
      searchGroup++;

    if( Equal( searchGroup->origin, (*deleteGroup)->origin ) )
    {
      std::vector<ipPicDescriptor*>::iterator deleteSlice = (*deleteGroup)->includedPics.end();

      //dont delete the slices over the timeCount
      for( unsigned int x = (*deleteGroup)->includedPics.size(); x > timeCount; x-- )
        deleteSlice--;

      do
      {
        deleteSlice--;
        searchGroup->includedPics.erase( deleteSlice );
      }
      while( deleteSlice != (*deleteGroup)->includedPics.begin() );

      if( searchGroup->includedPics.empty() )
        m_GroupVector[currentGroup].includedPositions.erase( searchGroup );
    }
    else std::cout<<"SingleSpacingFilter-WARNING: Logical Error. Groups dont match."<<std::endl;
  }
  while( deleteGroup != usedPos.begin() );
#endif
}

void mitk::SingleSpacingFilter::GenerateNodes( std::vector<Position*> usedPos, Vector3D spacing, unsigned int timeCount, unsigned int currentGroup )
{
#ifdef CHILI_PLUGIN_VERSION_CODE

  Image::Pointer resultImage = Image::New();
  std::list< std::string > ListOfUIDs;
  ListOfUIDs.clear();

  if( m_GroupVector[currentGroup].dimension == 4 )
  {
    resultImage->Initialize( usedPos.front()->includedPics.front() );
    resultImage->SetPicChannel( usedPos.front()->includedPics.front() );

    //get ImageInstanceUID
    std::string SingleUID;
    ipPicTSV_t* missingImageTagQuery = ipPicQueryTag( usedPos.front()->includedPics.front(), (char*)tagIMAGE_INSTANCE_UID );
    if( missingImageTagQuery )
      SingleUID = static_cast<char*>( missingImageTagQuery->value );
    else
    {
      ipPicTSV_t *dicomHeader = ipPicQueryTag( usedPos.front()->includedPics.front(), (char*)"SOURCE HEADER" );
      void* data = NULL;
      ipUInt4_t len = 0;
      if( dicomHeader && dicomFindElement( (unsigned char*) dicomHeader->value, 0x0008, 0x0018, &data, &len ) && data != NULL )
        SingleUID = static_cast<char*>( data );
    }
    ListOfUIDs.push_back( SingleUID );
  }
  else
  {
    //create mitk::Image
    Vector3D rightVector, downVector;
    Point3D origin;
    ipPicDescriptor* header;

    header = ipPicCopyHeader( usedPos.front()->includedPics.front(), NULL );

    if( m_GroupVector[currentGroup].dimension == 3 )
    {
      if( usedPos.size() == 1 )
        header->dim = 3;
      else
        header->dim = 4;
      header->n[2] = usedPos.front()->includedPics.front()->n[2];
      header->n[3] = usedPos.front()->includedPics.size();
    }
    else
    if( usedPos.size() == 1 && m_GroupVector[currentGroup].dimension == 2 )
    {
      if( usedPos.front()->includedPics.size() == 1 )   //2D
      {
        header->dim = 2;
        header->n[2] = 0;
        header->n[3] = 0;
      }
      else  //2D + t
      {
        header->dim = 4;
        header->n[2] = 1;
        header->n[3] = timeCount;
      }
    }
    else
    {
      header->n[2] = usedPos.size();

      if( usedPos.front()->includedPics.size() == 1 )
      {
        header->dim = 3;
        header->n[3] = 0;
      }
      else
      {
        header->dim = 4;
        header->n[3] = timeCount;
      }
    }

    resultImage->Initialize( header );

    interSliceGeometry_t* isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
    pFetchSliceGeometryFromPic( usedPos.front()->includedPics.front(), isg );

    //rightVector, downVector, origin
    vtk2itk( isg->u, rightVector );
    vtk2itk( isg->v, downVector );
    vtk2itk( isg->o, origin );
    free( isg );

    // its possible that a 2D-Image have no right- or down-Vector, but its not possible to initialize a [0,0,0] vector
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
    planegeometry->SetFrameOfReferenceID( FrameOfReferenceUIDManager::AddFrameOfReferenceUID( m_GroupVector[currentGroup].referenceUID.c_str() ) );
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
    unsigned int n = 0;
    for( std::vector< Position* >::iterator posIter = usedPos.begin(); posIter != usedPos.end(); posIter++ )
    {
      std::vector< ipPicDescriptor* >::iterator picIter = (*posIter)->includedPics.begin();
      for( unsigned int t = 0; t < timeCount; t++ )
      {
        //get ImageInstanceUID
        std::string SingleUID;
        ipPicTSV_t* missingImageTagQuery = ipPicQueryTag( (*picIter), (char*)tagIMAGE_INSTANCE_UID );
        if( missingImageTagQuery )
          SingleUID = static_cast<char*>( missingImageTagQuery->value );
        else
        {
          ipPicTSV_t *dicomHeader = ipPicQueryTag( (*picIter), (char*)"SOURCE HEADER" );
          void* data = NULL;
          ipUInt4_t len = 0;
          if( dicomHeader && dicomFindElement( (unsigned char*) dicomHeader->value, 0x0008, 0x0018, &data, &len ) && data != NULL )
            SingleUID = static_cast<char*>( data );
        }
        ListOfUIDs.push_back( SingleUID );

        //add to mitk::Image
        if( m_GroupVector[currentGroup].dimension == 3 )
          resultImage->SetPicVolume( (*picIter), t );
        else
          resultImage->SetPicSlice( (*picIter), n, t );

        picIter++;
      }
      n++;
    }
  }

  if( resultImage->IsInitialized() && resultImage.IsNotNull() )
  {
    DataTreeNode::Pointer node = mitk::DataTreeNode::New();
    node->SetData( resultImage );
    DataTreeNodeFactory::SetDefaultImageProperties( node );

    if( m_GroupVector[currentGroup].seriesDescription == "" )
      m_GroupVector[currentGroup].seriesDescription = "no SeriesDescription";
    node->SetProperty( "name", new StringProperty( m_GroupVector[currentGroup].seriesDescription ) );
    if( m_GroupVector[currentGroup].dimension == 4 )
    {
      node->SetProperty( "NumberOfSlices", new IntProperty( usedPos.front()->includedPics.front()->n[2] ) );
      node->SetProperty( "NumberOfTimeSlices", new IntProperty( usedPos.front()->includedPics.front()->n[3] ) );
    }
    else
    if( m_GroupVector[currentGroup].dimension == 3 )
    {
      node->SetProperty( "NumberOfSlices", new IntProperty( usedPos.front()->includedPics.front()->n[2] ) );
      node->SetProperty( "NumberOfTimeSlices", new IntProperty( usedPos.front()->includedPics.size() ) );
    }
    else
    {
      node->SetProperty( "NumberOfSlices", new IntProperty( usedPos.size() ) );
      node->SetProperty( "NumberOfTimeSlices", new IntProperty( timeCount ) );
    }
     if( m_SeriesOID != "" )
       node->SetProperty( "SeriesOID", new StringProperty( m_SeriesOID ) );

    mitk::PropertyList::Pointer tempPropertyList = CreatePropertyListFromPicTags( usedPos.front()->includedPics.front() );
    for( mitk::PropertyList::PropertyMap::const_iterator iter = tempPropertyList->GetMap()->begin(); iter !=  tempPropertyList->GetMap()->end(); iter++ )
    {
      node->SetProperty( iter->first.c_str(), iter->second.first );
    }

    m_Output.push_back( node );
    m_ImageInstanceUIDs.push_back( ListOfUIDs );
  }
#endif
}

const mitk::PropertyList::Pointer mitk::SingleSpacingFilter::CreatePropertyListFromPicTags( ipPicDescriptor* imageToExtractTagsFrom )
{
  if( !imageToExtractTagsFrom || !imageToExtractTagsFrom->info || !imageToExtractTagsFrom->info->tags_head )
    return NULL;

  PropertyList::Pointer resultPropertyList = PropertyList::New();
  _ipPicTagsElement_t* currentTagNode = imageToExtractTagsFrom->info->tags_head;

  // Extract ALL tags from the PIC header
  while (currentTagNode)
  {
    ipPicTSV_t* currentTag = currentTagNode->tsv;

    std::string propertyName = "CHILI: ";
    propertyName += currentTag->tag;

    //The currentTag->tag ends with a lot of ' ', so you find nothing if you search for the properties.
    while( propertyName[ propertyName.length() -1 ] == ' ' )
      propertyName.erase( propertyName.length() -1 );

    switch( currentTag->type )
    {
      case ipPicASCII:
      {
        resultPropertyList->SetProperty( propertyName.c_str(), new mitk::StringProperty( static_cast<char*>( currentTag->value ) ) );
        break;
      }
      case ipPicInt:
      {
        resultPropertyList->SetProperty( propertyName.c_str(), new mitk::IntProperty( *static_cast<int*>( currentTag->value ) ) );
        break;
      }
      case ipPicUInt:
      {
        resultPropertyList->SetProperty( propertyName.c_str(), new mitk::IntProperty( (int)*( (char*)( currentTag->value ) ) ) );
        break;
      }
      default:  //ipPicUnknown, ipPicBool, ipPicFloat, ipPicNonUniform, ipPicTSV, _ipPicTypeMax
      {
        //every PicDescriptor have the following tags wich not get imported, but they are not so important that we have to throw messages
        if( propertyName != "CHILI: PIXEL SIZE" && propertyName != "CHILI: REAL PIXEL SIZE" && propertyName != "CHILI: ISG" && propertyName != "CHILI: SOURCE HEADER" && propertyName != "CHILI: PIXEL SPACING" )
        {
          std::cout << "WARNING: Type of PIC-Tag '" << currentTag->type << "'( " << propertyName << " ) not handled in mitkSingleSpacingFilter." << std::endl;
        }
        break;
      }
    }
    // proceed to the next tag
    currentTagNode = currentTagNode->next;
  }
  return resultPropertyList;
}

