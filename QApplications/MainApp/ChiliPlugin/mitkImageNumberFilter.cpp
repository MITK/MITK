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

#include "mitkImageNumberFilter.h"

// MITK-Includes
#include "mitkProgressBar.h"
#include "mitkIpPicUnmangle.h"

// CHILI-Includes
#include <chili/isg.h>
#include <chili/plugin.h>
#include <ipDicom/ipDicom.h>
#include <ipPic/ipPicTags.h>

#include <algorithm>  //needed for "sort" (windows)

//help-functions
bool mitk::ImageNumberFilter::NumberSort( const Slice elem1, const Slice elem2 )
{
  if( elem1.imageNumber == elem2.imageNumber )
    return elem1.origin*elem1.normal < elem2.origin*elem2.normal; // projection of origin on inter-slice-direction
  else
    return elem1.imageNumber < elem2.imageNumber;
}

bool mitk::ImageNumberFilter::PositionSort( const Slice elem1, const Slice elem2 )
{
  return elem1.origin*elem1.normal < elem2.origin*elem2.normal; // projection of origin on inter-slice-direction
}

// constructor
mitk::ImageNumberFilter::ImageNumberFilter()
{
}

// destructor
mitk::ImageNumberFilter::~ImageNumberFilter()
{
}

// the "main"-function
void mitk::ImageNumberFilter::Update()
{
  m_Output.clear();
  m_GroupList.clear();
  m_ImageInstanceUIDs.clear();
  m_Abort = false;

  if( m_PicDescriptorList.size() > 0 && m_SeriesOID != "" )
  {
    ProgressBar::GetInstance()->AddStepsToDo( 6 );
    SortPicsToGroup();
    ProgressBar::GetInstance()->Progress();
    SortSlicesByImageNumber();
    ProgressBar::GetInstance()->Progress();
    SeperateBySpacing();
    ProgressBar::GetInstance()->Progress();
    SeperateByTime();
    ProgressBar::GetInstance()->Progress();
    SplitDummiVolumes();
    ProgressBar::GetInstance()->Progress();
    GenerateImages();
    ProgressBar::GetInstance()->Progress();
  }
  else std::cout<<"ImageNumberFilter-WARNING: No SeriesOID or PicDescriptorList set."<<std::endl;
}

void mitk::ImageNumberFilter::SortPicsToGroup()
{
  for( std::list< mitkIpPicDescriptor* >::iterator currentPic = m_PicDescriptorList.begin(); currentPic != m_PicDescriptorList.end(); currentPic ++ )
  {
    if( m_Abort ) return;
    //check intersliceGeomtry
    ipPicDescriptor* chiliPic = reinterpret_cast<ipPicDescriptor*>((*currentPic));
    interSliceGeometry_t* isg = ( interSliceGeometry_t* ) malloc ( sizeof( interSliceGeometry_t ) );
    if( !chiliPic || !pFetchSliceGeometryFromPic( chiliPic, isg ) )
    {
      //PicDescriptor without a geometry not able to sort in a volume
      std::cout<<"ImageNumberFilter-WARNING: Found image without SliceGeometry. Image ignored."<<std::endl;
      free( isg );
      continue;
    }

    //new Slice
    Slice newSlice;
    newSlice.currentPic = (*currentPic);
    vtk2itk( isg->o, newSlice.origin );
    //set normale
    Vector3D rightVector, downVector;
    vtk2itk( isg->u, rightVector );
    vtk2itk( isg->v, downVector );
    newSlice.normal[0] = Round( ( ( rightVector[1]*downVector[2] ) - ( rightVector[2]*downVector[1] ) ), 2 );
    newSlice.normal[1] = Round( ( ( rightVector[2]*downVector[0] ) - ( rightVector[0]*downVector[2] ) ), 2);
    newSlice.normal[2] = Round( ( ( rightVector[0]*downVector[1] ) - ( rightVector[1]*downVector[0] ) ), 2);
    //set imageNumber
    mitkIpPicTSV_t* imagenumberTag = mitkIpPicQueryTag( newSlice.currentPic, (char*)tagIMAGE_NUMBER );
    if( imagenumberTag && imagenumberTag->type == mitkIpPicInt )
      newSlice.imageNumber = *( (int*)(imagenumberTag->value) );
    else
    {
      mitkIpPicTSV_t *tsv;
      void* data = NULL;
      ipUInt4_t len = 0;
      tsv = mitkIpPicQueryTag( newSlice.currentPic, (char*)"SOURCE HEADER" );
      if( tsv )
      {
        if( dicomFindElement( (unsigned char*) tsv->value, 0x0020, 0x0013, &data, &len ) )
          if( data != NULL )
            sscanf( (char *) data, "%d", &newSlice.imageNumber );
      }
      else
        newSlice.imageNumber = 0;
    }

    //search and set group, therefore some more attributes have to generated
    //pixelSize
    Vector3D currentPixelSize;
    vtk2itk( isg->ps, currentPixelSize );
    //seriesDescription
    std::string currentSeriesDescription;
    mitkIpPicTSV_t* seriesDescriptionTag = mitkIpPicQueryTag( (*currentPic), (char*)tagSERIES_DESCRIPTION );
    if( seriesDescriptionTag )
      currentSeriesDescription = static_cast<char*>( seriesDescriptionTag->value );
    else
    {
      mitkIpPicTSV_t *tsv;
      void* data = NULL;
      ipUInt4_t len = 0;
      tsv = mitkIpPicQueryTag( (*currentPic), (char*)"SOURCE HEADER" );
      if( tsv && dicomFindElement( (unsigned char*) tsv->value, 0x0008, 0x103e, &data, &len ) )
        if( data != NULL )
          currentSeriesDescription = (char*)data;
      else
        currentSeriesDescription = "no Description";
    }
    //dimension
    int currentDimension = (*currentPic)->dim;

    if( currentDimension < 2 || currentDimension > 4 )
    {
      std::cout<<"ImageNumberFilter-WARNING: Wrong PicDescriptor-Dimension. Image ignored."<<std::endl;
      free( isg );
      continue;
    }

    //normale with ImageSize
    Vector3D currentNormalWithImageSize;
    rightVector = rightVector * newSlice.currentPic->n[0];
    downVector = downVector * newSlice.currentPic->n[1];
    currentNormalWithImageSize[0] = Round( ( ( rightVector[1]*downVector[2] ) - ( rightVector[2]*downVector[1] ) ), 2 );
    currentNormalWithImageSize[1] = Round( ( ( rightVector[2]*downVector[0] ) - ( rightVector[0]*downVector[2] ) ), 2);
    currentNormalWithImageSize[2] = Round( ( ( rightVector[0]*downVector[1] ) - ( rightVector[1]*downVector[0] ) ), 2);

    bool foundMatch;
    int curCount, maxCount;

    if( currentDimension == 4 )
    {
      //with this combination, no search initialize and a new group created --> 4D-datasets are seperate groups
      foundMatch = false;
      curCount = 0;
      maxCount = 0;
    }
    else
    {
      //initialize searching
      foundMatch = false;
      curCount = 0;
      maxCount = m_GroupList.size();
    }

    // searching for equal output
    while( curCount < maxCount && !foundMatch )
    {
      //checking referenceUID, seriesDescription, pixelSize and NormaleWithSize
      if( m_GroupList[ curCount ].referenceUID == isg->forUID && m_GroupList[ curCount ].seriesDescription == currentSeriesDescription && m_GroupList[ curCount ].pixelSize == currentPixelSize && m_GroupList[ curCount ].normalWithImageSize == currentNormalWithImageSize )
      {
        if( m_GroupList[ curCount ].dimension == 2 || ( m_GroupList[ curCount ].dimension == 3 && m_GroupList[ curCount ].includedSlices.front().origin == newSlice.origin && m_GroupList[ curCount ].includedSlices.front().currentPic->n[2] == (*currentPic)->n[2] ) )
          foundMatch = true;
        else
          curCount++;
      }
      else
        curCount++;
    }

    //if a matching output found, add slice or create a new group
    if( foundMatch )
      m_GroupList[ curCount ].includedSlices.push_back( newSlice );
    else
    {
      Group newGroup;
      newGroup.includedSlices.clear();
      newGroup.includedSlices.push_back( newSlice );
      newGroup.referenceUID = isg->forUID;
      newGroup.seriesDescription = currentSeriesDescription;
      newGroup.dimension = currentDimension;
      newGroup.pixelSize = currentPixelSize;
      newGroup.normalWithImageSize = currentNormalWithImageSize;
      m_GroupList.push_back( newGroup );
    }
  free( isg );
  }
}

void mitk::ImageNumberFilter::SortSlicesByImageNumber()
{
  for( std::vector< Group >::iterator iter = m_GroupList.begin(); iter != m_GroupList.end(); iter++ )
  {
    std::sort( (*iter).includedSlices.begin(), (*iter).includedSlices.end(), NumberSort );
  }
}

// separation on spacing and set minimum of timslices and slices
void mitk::ImageNumberFilter::SeperateBySpacing()
{
  ProgressBar::GetInstance()->AddStepsToDo( m_GroupList.size() );
  for( unsigned int n = 0; n < m_GroupList.size(); n++)
  {
    if( m_Abort )
    {
      ProgressBar::GetInstance()->Progress( m_GroupList.size() - n );
      return;
    }
    if( m_GroupList[n].dimension == 2 && m_GroupList[n].includedSlices.size() > 2 )
    {
      int currentCount = 0;  //used to split list
      //get reference-spacing
      double referenceSpacing;
      std::vector< Slice >::iterator originIter = m_GroupList[n].includedSlices.begin();
      while( originIter != m_GroupList[n].includedSlices.end() && Equal( m_GroupList[n].includedSlices.begin()->origin, originIter->origin ) )
      {
        originIter++;
        currentCount++;
      }

      if( originIter != m_GroupList[n].includedSlices.end() )
      {
        Vector3D tempDistance = originIter->origin - m_GroupList[n].includedSlices.begin()->origin;
        referenceSpacing = Round( tempDistance.GetNorm(), 2 );

        std::vector< Slice >::iterator iterB4 = originIter;
        while( originIter != m_GroupList[n].includedSlices.end() )
        {
          ProgressBar::GetInstance()->AddStepsToDo( 1 );
          if( Equal ( iterB4->origin, originIter->origin ) )
          {
            originIter++;
            currentCount++;
          }
          else
          {
            tempDistance = originIter->origin - iterB4->origin;
            double tmpSpacing = Round( tempDistance.GetNorm(), 2 );
            if( tmpSpacing == referenceSpacing )
            {
              iterB4 = originIter;
              originIter++;
              currentCount++;
            }
            else
            {
              Group newGroup;
              newGroup.referenceUID = m_GroupList[n].referenceUID;
              newGroup.seriesDescription = m_GroupList[n].seriesDescription;
              newGroup.dimension = m_GroupList[n].dimension;
              newGroup.pixelSize = m_GroupList[n].pixelSize;
              newGroup.includedSlices.clear();
              newGroup.includedSlices.assign( originIter, m_GroupList[n].includedSlices.end() );
              m_GroupList[n].includedSlices.resize( currentCount );
              m_GroupList.push_back( newGroup );
              originIter = m_GroupList[n].includedSlices.end();
            }
          }
          ProgressBar::GetInstance()->Progress();
        }
      }
    }
    ProgressBar::GetInstance()->Progress();
  }
}

void mitk::ImageNumberFilter::SeperateByTime()
{
  ProgressBar::GetInstance()->AddStepsToDo( m_GroupList.size() );
  for( unsigned int n = 0; n < m_GroupList.size(); n++)
  {
    if( m_Abort )
    {
      ProgressBar::GetInstance()->Progress( m_GroupList.size() - n );
      return;
    }
    if( m_GroupList[n].dimension == 2 && m_GroupList[n].includedSlices.size() > 2 )
    {
      bool differentTimeSliced = false;
      unsigned int minTimeSteps = 0;

      std::vector< Slice >::iterator walkIter = m_GroupList[n].includedSlices.begin();
      walkIter++;
      unsigned int timeSteps = 0;
      Vector3D tmpOrigin = m_GroupList[n].includedSlices.front().origin;

      for( ; walkIter != m_GroupList[n].includedSlices.end(); walkIter++ )
      {
        ProgressBar::GetInstance()->AddStepsToDo( 1 );
        if( Equal ( tmpOrigin, walkIter->origin ) )
          timeSteps++;
        else
        {
          if( timeSteps < minTimeSteps || minTimeSteps == 0 )
          {
            minTimeSteps = timeSteps;
            if( timeSteps < minTimeSteps )
              differentTimeSliced = true;
          }
        }
        tmpOrigin = walkIter->origin;
        ProgressBar::GetInstance()->Progress();
      }

      if( differentTimeSliced )
      {
        Group newGroup;
        newGroup.referenceUID = m_GroupList[n].referenceUID;
        newGroup.seriesDescription = m_GroupList[n].seriesDescription;
        newGroup.dimension = m_GroupList[n].dimension;
        newGroup.pixelSize = m_GroupList[n].pixelSize;
        newGroup.includedSlices.clear();

        unsigned int currentTimeStep = 0;
        tmpOrigin = m_GroupList[n].includedSlices.front().origin;
        std::vector< Slice >::iterator delIter = m_GroupList[n].includedSlices.begin();
        delIter++;
        while( delIter != m_GroupList[n].includedSlices.end() )
        {
          if( Equal( delIter->origin, tmpOrigin ) )
            currentTimeStep++;
          else currentTimeStep = 0;

          if( currentTimeStep > minTimeSteps )
          {
            newGroup.includedSlices.push_back( (*delIter) );
            delIter = m_GroupList[n].includedSlices.erase( delIter );
          }
          else
          {
            tmpOrigin = delIter->origin;
            delIter++;
          }
        }
        m_GroupList.push_back( newGroup );
      }
    }
    ProgressBar::GetInstance()->Progress();
  }
}

void mitk::ImageNumberFilter::SplitDummiVolumes()
{
  for( unsigned int n = 0; n < m_GroupList.size(); n++)
  {
    if( m_Abort ) return;
    if( m_GroupList[n].dimension == 2 && m_GroupList[n].includedSlices.size() == 2 )
    {
      if( !Equal( m_GroupList[n].includedSlices.front().origin, m_GroupList[n].includedSlices.back().origin ) )  // 2D+t ?
      {
        Group newGroup;
        newGroup.referenceUID = m_GroupList[n].referenceUID;
        newGroup.seriesDescription = m_GroupList[n].seriesDescription;
        newGroup.dimension = m_GroupList[n].dimension;
        newGroup.pixelSize = m_GroupList[n].pixelSize;
        newGroup.includedSlices.clear();
        newGroup.includedSlices.push_back( m_GroupList[n].includedSlices.back() );
        m_GroupList.push_back( newGroup );
        m_GroupList[n].includedSlices.pop_back();
      }
    }
  }
}

void mitk::ImageNumberFilter::GenerateImages()
{
  for( std::vector< Group >::iterator groupIter = m_GroupList.begin(); groupIter != m_GroupList.end(); groupIter++ )
  {
    if( m_Abort ) return;
    //get time, count
    int timeSteps = 0, sliceSteps = 0;
    std::vector< Slice >::iterator originIter = groupIter->includedSlices.begin();
    while( originIter != groupIter->includedSlices.end() && Equal( groupIter->includedSlices.begin()->origin, originIter->origin ) )
    {
      timeSteps++;
      originIter++;
    }
    sliceSteps = groupIter->includedSlices.size()/timeSteps;

    //get spacing
    Vector3D spacing;
    //initialize
    ipPicDescriptor* chiliPic = reinterpret_cast<ipPicDescriptor*>(groupIter->includedSlices.begin()->currentPic);
    interSliceGeometry_t* isg = ( interSliceGeometry_t* ) malloc ( sizeof( interSliceGeometry_t ) );
    pFetchSliceGeometryFromPic( chiliPic, isg );
    vtk2itk( isg->ps, spacing );
    if( spacing[0] == 0 && spacing[1] == 0 && spacing[2] == 0 )
      spacing.Fill(1.0);
    free( isg );
    //get the not rounded spacing
    std::list<Spacing> SpacingList;
    SpacingList.clear();
    std::vector< Slice >::iterator walkIter = groupIter->includedSlices.begin();
    walkIter++;
    for( ; walkIter != groupIter->includedSlices.end(); walkIter ++)
    {
      std::vector<Slice>::iterator iterB4 = walkIter;
      iterB4--;
      if( !Equal( walkIter->origin, iterB4->origin ) )
      {
        Vector3D tempDistance = walkIter->origin - iterB4->origin;
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
    //get spacing
    int count = 0;
    for( std::list<Spacing>::iterator searchIter = SpacingList.begin(); searchIter != SpacingList.end(); searchIter++ )
    {
      if( searchIter->count > count )
      {
        spacing = searchIter->spacing;
        count = searchIter->count;
      }
    }

    //get all mitkIpPicDescriptor
    std::sort( groupIter->includedSlices.begin(), groupIter->includedSlices.end(),PositionSort );
    std::list<mitkIpPicDescriptor*> usedPic;
    usedPic.clear();
    for( std::vector< Slice >::iterator picIter = groupIter->includedSlices.begin(); picIter != groupIter->includedSlices.end(); picIter++ )
      usedPic.push_back( picIter->currentPic );

    GenerateData( usedPic, sliceSteps, timeSteps, spacing, groupIter->seriesDescription );
  }
}
