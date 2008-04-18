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

// CHILI-Includes
#include <chili/isg.h>
#include <chili/plugin.h>
#include <ipPic/ipPicTags.h>
#include <ipDicom/ipDicom.h>
// MITK-Includes
#include "mitkChiliMacros.h"
#include "mitkProgressBar.h"

#include <algorithm>  //needed for "sort" (windows)

#ifdef CHILI_PLUGIN_VERSION_CODE

//helpfunction to sort
bool mitk::SingleSpacingFilter::PositionSort( const Position& elem1, const Position& elem2 )
{
  return elem1.origin*elem1.normal < elem2.origin*elem2.normal; // projection of origin on inter-slice-direction
}

bool mitk::SingleSpacingFilter::PicSort( ipPicDescriptor* elem1, ipPicDescriptor* elem2 )
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
void mitk::SingleSpacingFilter::SortPositionsAndPics()
{
  for( std::vector< Group >::iterator groupIter = m_GroupVector.begin(); groupIter != m_GroupVector.end(); groupIter++ )
  {
    std::sort( groupIter->includedPositions.begin(), groupIter->includedPositions.end(), PositionSort );
    for( std::vector< Position >::iterator positionIter = groupIter->includedPositions.begin(); positionIter != groupIter->includedPositions.end(); positionIter++ )
    {
      std::sort( positionIter->includedPics.begin(), positionIter->includedPics.end(), PicSort );
    }
  }
}

// constructor
mitk::SingleSpacingFilter::SingleSpacingFilter()
{
}

// destructor
mitk::SingleSpacingFilter::~SingleSpacingFilter()
{
}

// the "main"-function
void mitk::SingleSpacingFilter::Update()
{
  m_GroupVector.clear();
  m_Output.clear();
  m_ImageInstanceUIDs.clear();
  m_Abort = false;

  if( m_PicDescriptorList.size() > 0 && m_SeriesOID != "" )
  {
    ProgressBar::GetInstance()->AddStepsToDo( 30 );
    SortPicsToGroup();
    ProgressBar::GetInstance()->Progress( 10 );
    SortPositionsAndPics();
    ProgressBar::GetInstance()->Progress( 10 );
    CreateResults();
    ProgressBar::GetInstance()->Progress( 10 );
  }
  else std::cout<<"SingleSpacingFilter-WARNING: No SeriesOID or PicDescriptorList set."<<std::endl;
}

void mitk::SingleSpacingFilter::SortPicsToGroup()
{
for( std::list< ipPicDescriptor* >::iterator currentPic = m_PicDescriptorList.begin(); currentPic != m_PicDescriptorList.end(); currentPic ++ )
  {
    if( m_Abort ) return;
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
}

void mitk::SingleSpacingFilter::CreateResults()
{
  for( unsigned int x = 0; x < m_GroupVector.size(); x++)
  {
    while( !m_GroupVector[x].includedPositions.empty() )
    {
      if( m_Abort ) return;
      SearchParameter( x );
    }
  }
}

void mitk::SingleSpacingFilter::SearchParameter( unsigned int mitkHideIfNoVersionCode( currentGroup ) )
{
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
    ProgressBar::GetInstance()->AddStepsToDo( 2 );
    //search fot the most coherent slices with the same spacing
    std::vector< Position >::iterator iterEnd = m_GroupVector[currentGroup].includedPositions.end();
    iterEnd--;

    for( std::vector< Position >::iterator iterRoot = m_GroupVector[currentGroup].includedPositions.begin(); iterRoot != iterEnd; iterRoot++ )
    {
      for( std::vector< Position >::iterator iterWalk = iterRoot; iterWalk != m_GroupVector[currentGroup].includedPositions.end(); iterWalk++ )
      {
        if( m_Abort )
        {
          ProgressBar::GetInstance()->Progress( 2 );
          return;
        }
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
    ProgressBar::GetInstance()->Progress();

    //now search for the spacing between the slices
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
      if( !Equal( (*walkIter)->origin, (*iterB4)->origin ) )
      {
        Vector3D tempDistance = (*walkIter)->origin - (*iterB4)->origin;
        spacing[2] = tempDistance.GetNorm();
        //search for spacing
        std::list<Spacing>::iterator searchIter = SpacingList.begin();
        for( ; searchIter != SpacingList.end(); searchIter++ )
        {
          if( Equal( searchIter->spacing, spacing) )
          {
            searchIter->count++;
            break;
          }
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
    ProgressBar::GetInstance()->Progress();
  }

  //create mitk::images
  std::list<ipPicDescriptor*> usedPic;
  usedPic.clear();
  for( std::vector<Position*>::iterator walkPic = usedPos.begin(); walkPic != usedPos.end(); walkPic++ )
    for( unsigned int step = 0; step < timeCount; step++ )
      usedPic.push_back( (*walkPic)->includedPics[step] );
  GenerateData( usedPic, usedPos.size(), timeCount, spacing, m_GroupVector[currentGroup].seriesDescription );

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
}

#endif
