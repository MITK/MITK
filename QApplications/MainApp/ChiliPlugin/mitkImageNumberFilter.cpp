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

/*
 * Last reviewed: 2008/08 by maleike and jochen
 */

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

bool mitk::ImageNumberFilter::PositionSort( const Slice elem1, const Slice elem2 )
{
  /* sort by distance of image origin from world origin */
  if ( elem1.origin*elem1.normal < elem2.origin*elem2.normal ) // projection of origin on inter-slice-direction
  {
    return true;
  }
  else if ( elem1.origin*elem1.normal > elem2.origin*elem2.normal ) // projection of origin on inter-slice-direction
  {
    return false;
  }
  else // if neccessary base the decision on the image number
  {
    return elem1.imageNumber < elem2.imageNumber;
  }
}

mitk::ImageNumberFilter::ImageNumberFilter()
:m_GroupByAcquisitionNumber(false)
{
}

mitk::ImageNumberFilter::~ImageNumberFilter()
{
}
    
void mitk::ImageNumberFilter::SetGroupByAcquisitionNumber(bool on)
{
  m_GroupByAcquisitionNumber = on;
}

bool mitk::ImageNumberFilter::GetGroupByAcquisitionNumber()
{
  return m_GroupByAcquisitionNumber;
}

// the "main"-function
void mitk::ImageNumberFilter::Update()
{
  std::cout << "Update" << std::endl;
  m_Output.clear();
  m_GroupList.clear();
  m_ImageInstanceUIDs.clear();
  m_Abort = false;

  if( m_PicDescriptorList.size() > 0 && m_SeriesOID != "" )
  {
    ProgressBar::GetInstance()->AddStepsToDo( 5 );
    
    // Group slices (mitkIpPicDescriptors) of equal size and orientation.
    SortPicsToGroup();
    ProgressBar::GetInstance()->Progress();

    // Sort slices in groups by slice position in space
    SortSlicesBySlicePosition();
    ProgressBar::GetInstance()->Progress();

    // Split groups into sub-groups with uniform slice distances.
    SeparateBySpacing();
    ProgressBar::GetInstance()->Progress();

    // Ensure an equal number of slices per position in space. 
    SeparateByTime();
    ProgressBar::GetInstance()->Progress();

    // Create an mitk::Image for each group
    GenerateImages();
    ProgressBar::GetInstance()->Progress();
  }
  else 
  {
    std::cout << "ImageNumberFilter: WARNING: No SeriesOID or PicDescriptorList set." << std::endl;
  }
}

void mitk::ImageNumberFilter::SortPicsToGroup()
{
  std::cout << "SortPicsToGroup... " << std::flush;
  for( std::list< mitkIpPicDescriptor* >::iterator currentPic = m_PicDescriptorList.begin(); 
       currentPic != m_PicDescriptorList.end(); 
       ++currentPic )
  {
    if( m_Abort ) 
    {
      std::cout << "ABORT" << std::endl;
      return;
    }

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

    //new slice
    Slice newSlice;
    newSlice.pic = (*currentPic);
    vtk2itk( isg->o, newSlice.origin );
    //set normal
    Vector3D rightVector, downVector;
    vtk2itk( isg->u, rightVector );
    vtk2itk( isg->v, downVector );
    newSlice.normal[0] = Round( ( ( rightVector[1]*downVector[2] ) - ( rightVector[2]*downVector[1] ) ), 2);
    newSlice.normal[1] = Round( ( ( rightVector[2]*downVector[0] ) - ( rightVector[0]*downVector[2] ) ), 2);
    newSlice.normal[2] = Round( ( ( rightVector[0]*downVector[1] ) - ( rightVector[1]*downVector[0] ) ), 2);
    //set imageNumber
    mitkIpPicTSV_t* imagenumberTag = mitkIpPicQueryTag( newSlice.pic, (char*)tagIMAGE_NUMBER );
    if( imagenumberTag && imagenumberTag->type == mitkIpPicInt )
      newSlice.imageNumber = *( (int*)(imagenumberTag->value) );
    else
    {
      mitkIpPicTSV_t *tsv;
      void* data = NULL;
      ipUInt4_t len = 0;
      tsv = mitkIpPicQueryTag( newSlice.pic, (char*)"SOURCE HEADER" );
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
    {
      size_t tagLen = seriesDescriptionTag->n[0];
      char* stringValue = (char*)malloc(tagLen + 1);
      memcpy(stringValue,seriesDescriptionTag->value,tagLen);
      stringValue[tagLen]= '\0';
      std::string s( (const char*)seriesDescriptionTag->value, tagLen );
      currentSeriesDescription = s;
    }     
    else
    {
      mitkIpPicTSV_t *tsv;
      void* data = NULL;
      ipUInt4_t len = 0;
      tsv = mitkIpPicQueryTag( (*currentPic), (char*)"SOURCE HEADER" );
      if( tsv && dicomFindElement( (unsigned char*) tsv->value, 0x0008, 0x103e, &data, &len ) )
      {
        if( data != NULL )
        { 
          currentSeriesDescription = (char*)data;
        }
      }
      else
      {
        currentSeriesDescription = "no Description";
      }
    }
    //dimension
    int currentDimension = (*currentPic)->dim;

    if( currentDimension < 2 || currentDimension > 4 )
    {
      std::cout << "mitk::ImageNumberFilter " 
                << __FILE__ << " l. " << __LINE__ 
                << ": Wrong dimension (" << currentDimension << ") in ipPicDescriptor. Image ignored." << std::endl;
      free( isg );
      continue;
    }

    //normal (multiplied by width * height)
    Vector3D currentNormalWithImageSize;
    rightVector = rightVector * newSlice.pic->n[0];
    downVector = downVector * newSlice.pic->n[1];
    currentNormalWithImageSize[0] = Round( ( ( rightVector[1]*downVector[2] ) - ( rightVector[2]*downVector[1] ) ), 2);
    currentNormalWithImageSize[1] = Round( ( ( rightVector[2]*downVector[0] ) - ( rightVector[0]*downVector[2] ) ), 2);
    currentNormalWithImageSize[2] = Round( ( ( rightVector[0]*downVector[1] ) - ( rightVector[1]*downVector[0] ) ), 2);

    bool foundMatch = false;
    unsigned int curCount = 0;

    if( currentDimension != 4 )
    {
      // searching for equal output
      for (curCount = 0; curCount < m_GroupList.size(); ++curCount)
      {
        //checking referenceUID, seriesDescription, pixelSize and NormaleWithSize
        if(    m_GroupList[ curCount ].referenceUID        == isg->forUID 
            && (m_GroupList[ curCount ].acq == isg->acq || ! m_GroupByAcquisitionNumber)
            /* Following line was deactivated, because this would not create time series
             * for image series where slices are describes as
             *
             *  myspecial_sequence_T=3.2s
             *  myspecial_sequence_T=3.5s
             *  myspecial_sequence_T=3.8s
             *  etc.
             */
            /* && m_GroupList[ curCount ].seriesDescription   == currentSeriesDescription */
            && m_GroupList[ curCount ].pixelSize           == currentPixelSize 
            && Equal( m_GroupList[ curCount ].normalWithImageSize, currentNormalWithImageSize) )
        {
          if(      m_GroupList[ curCount ].dimension == 2       // if we are sorting slices, everything is fine
              || (    m_GroupList[ curCount ].dimension == 3    // else the origin and z extent is also checked
                   && m_GroupList[ curCount ].includedSlices.front().origin == newSlice.origin 
                   && m_GroupList[ curCount ].includedSlices.front().pic->n[2] == (*currentPic)->n[2] 
                 ) 
            )
            m_GroupList[ curCount ].includedSlices.push_back( newSlice ); // similar (equal) group found, inserting current slice or volume
            foundMatch = true;
            break;
        }
      }
    }

    // if nothing is found (or ipPicDescriptor has dimension 4), a new group is created
    if (!foundMatch)
    {
      Group newGroup;
      newGroup.includedSlices.push_back( newSlice );
      newGroup.referenceUID = isg->forUID;
      newGroup.acq = isg->acq;
      newGroup.seriesDescription = currentSeriesDescription;
      newGroup.dimension = currentDimension;
      newGroup.pixelSize = currentPixelSize;
      newGroup.normalWithImageSize = currentNormalWithImageSize;

      std::cout << "Created new group: " << std::endl;
      std::cout << "   refUID " << newGroup.referenceUID << std::endl;
      std::cout << "   desc.  " << newGroup.seriesDescription << std::endl;
      std::cout << "   dim    " << newGroup.dimension << std::endl;
      std::cout << "   pixelSZ" << newGroup.pixelSize << std::endl;
      std::cout << "   normal " << newGroup.normalWithImageSize << std::endl;

      m_GroupList.push_back( newGroup );
    }
   
    free( isg );
  }
      
  std::cout << "sorted all pictures into " << m_GroupList.size() << " groups of similar slices." << std::endl;
}

void mitk::ImageNumberFilter::SortSlicesBySlicePosition()
{
  std::cout << "SortSlicesByImageNumber... " << std::flush;
  for( std::vector< Group >::iterator iter = m_GroupList.begin(); 
       iter != m_GroupList.end(); 
       ++iter )
  {
    std::sort( iter->includedSlices.begin(), 
               iter->includedSlices.end(), 
               PositionSort );  // "comparison operator", defined at beginning of this file
  }
  std::cout << "done" << std::endl;
}

void mitk::ImageNumberFilter::SeparateBySpacing()
{
  std::cout << "SeparateBySpacing " << std::flush;
  ProgressBar::GetInstance()->AddStepsToDo( m_GroupList.size() );
  for( unsigned int n = 0; n < m_GroupList.size(); n++)
  {
    if( m_Abort )
    {
      ProgressBar::GetInstance()->Progress( m_GroupList.size() - n );
      std::cout << " aborted." << std::endl;
      return;
    }

    std::vector< Slice >& slicesOfCurrentGroup = m_GroupList[n].includedSlices;
    
    if ( m_GroupList[n].dimension != 2 )    
    {
      ProgressBar::GetInstance()->Progress();
      continue;
    }
    if ( slicesOfCurrentGroup.size() <= 1 ) // only for groups of slices
    {
      ProgressBar::GetInstance()->Progress();
      continue;
    }

    std::vector< Slice >::iterator sliceIter;
    std::vector< Slice >::iterator firstSlice = slicesOfCurrentGroup.begin();
    double lastDistance = -1.0; // this distance cannot be found between two slices
    for ( sliceIter  = slicesOfCurrentGroup.begin();
          sliceIter != slicesOfCurrentGroup.end();
          ++sliceIter )
    {
      if ( Equal(firstSlice->origin, sliceIter->origin ) ) continue;
        
      // check if the distance between this slice set (slices at the same point in space) and 
      // the last slice set is the same as before
      double currentDistance = Round( (sliceIter->origin - firstSlice->origin).GetNorm(), 2 );
      if ( Equal(currentDistance, lastDistance) )
      {
        // everything is fine, still in the same distance pattern
        firstSlice = sliceIter; // remember correct slice for next distance calculation
        continue;
      }
      else if ( lastDistance < 0.0 )
      {
        // first distance ever calculated
        lastDistance = currentDistance;
        firstSlice = sliceIter;
      }
      else
      {
        std::cout << "Splitting group '" << m_GroupList[n].seriesDescription 
                  << "' into two because slice distance is not consistent" << std::endl;
        std::cout << "   slice distance before was: " << lastDistance << std::endl;
        std::cout << "   slice distance is now: " << currentDistance << std::endl;

        // split group into two separate groups
        Group newGroup;
        newGroup.referenceUID = m_GroupList[n].referenceUID;
        newGroup.seriesDescription = m_GroupList[n].seriesDescription;
        newGroup.dimension = m_GroupList[n].dimension;
        newGroup.pixelSize = m_GroupList[n].pixelSize;
        newGroup.includedSlices.assign( sliceIter, slicesOfCurrentGroup.end() ); // new group contains the non-fitting remaining slices
        sliceIter = slicesOfCurrentGroup.erase( sliceIter, slicesOfCurrentGroup.end() );
        
        ProgressBar::GetInstance()->AddStepsToDo(1);
        m_GroupList.push_back( newGroup );
        std::cout << "." << std::flush;
        break; // continue outer for-groups loop
      }
    }

    ProgressBar::GetInstance()->Progress(); // one group processed
  }
  
  std::cout << " done" << std::endl;
}

void mitk::ImageNumberFilter::SeparateByTime()
{
  std::cout << "SeparateByTime: time steps in groups: " << std::flush; 
  ProgressBar::GetInstance()->AddStepsToDo( m_GroupList.size() );
  for( unsigned int n = 0; n < m_GroupList.size(); n++)
  {
    if( m_Abort )
    {
      ProgressBar::GetInstance()->Progress( m_GroupList.size() - n );
      std::cout << " aborted." << std::endl;
      return;
    }
    
    std::vector< Slice >& slicesOfCurrentGroup = m_GroupList[n].includedSlices;
   
    if ( m_GroupList[n].dimension != 2 )    
    {
      ProgressBar::GetInstance()->Progress();
      continue;
    }
    if ( slicesOfCurrentGroup.size() <= 1 ) // only for groups of slices
    {
      ProgressBar::GetInstance()->Progress();
      continue;
    }

    unsigned int minTimeSteps = std::numeric_limits<unsigned int>::max();

    Vector3D lastOrigin;
    lastOrigin.Fill( std::numeric_limits<mitk::ScalarType>::max() );

    unsigned int timeStepsAtThisPosition = 1;
    unsigned int differentTimeStepCount = 0;
    for ( std::vector< Slice >::iterator sliceIter = slicesOfCurrentGroup.begin();
          sliceIter != slicesOfCurrentGroup.end();
          ++sliceIter )
    {
      if( Equal ( lastOrigin, sliceIter->origin ) ) // while slices share a position in space, count their number
      {
        ++timeStepsAtThisPosition;
      }
      else
      {
        if ( timeStepsAtThisPosition < minTimeSteps ) // remember the minimum number of slices that share a position
        {
          ++differentTimeStepCount;
          minTimeSteps = timeStepsAtThisPosition;
        }
        lastOrigin = sliceIter->origin; // we guess this is a new time step from here on
        timeStepsAtThisPosition = 1;
      }
    }

    std::cout << minTimeSteps << " " << std::flush;

    // Now ensure that for each position in space we have an equal number of slices that
    // share this position. The maximum number of allowed slices here is minTimeSteps.
    // The following loop will go through all slices and sort everything beyond minTimeSteps
    // into a new group (waste?).
    if( differentTimeStepCount > 1 ) // one will happen regularly due to the comparison with max()
    {
      Group newGroup;
      newGroup.referenceUID = m_GroupList[n].referenceUID;
      newGroup.seriesDescription = m_GroupList[n].seriesDescription;
      newGroup.dimension = m_GroupList[n].dimension;
      newGroup.pixelSize = m_GroupList[n].pixelSize;

      unsigned int currentTimeStep = 0;
      lastOrigin = slicesOfCurrentGroup.front().origin;
      std::vector< Slice >::iterator delIter = slicesOfCurrentGroup.begin();
      ++delIter; 
      while( delIter != slicesOfCurrentGroup.end() )
      {
        if( Equal( delIter->origin, lastOrigin ) ) // still at the same position in space?
        {
          if( ++currentTimeStep >= minTimeSteps )  // beyond the allowed limit?
          {
            newGroup.includedSlices.push_back( (*delIter) ); // then sort slice into new group
            delIter = slicesOfCurrentGroup.erase( delIter );
          }
          else
          {
            ++delIter;
          }
        }
        else // new position in space, but the number of slices seen before was ok
        {
          currentTimeStep = 0;
          lastOrigin = delIter->origin;
          ++delIter;
        }
      }
      
      m_GroupList.push_back( newGroup ); // add the new group to the list of groups 
                                         // (and automatically process it within the outer loop)

      ProgressBar::GetInstance()->AddStepsToDo(1);
    }

    ProgressBar::GetInstance()->Progress();
  }

  std::cout << " done" << std::endl;
}

void mitk::ImageNumberFilter::GenerateImages()
{
  std::cout << "GenerateImages: " << std::flush;
  ProgressBar::GetInstance()->AddStepsToDo( m_GroupList.size() );
  for( unsigned int n = 0; n < m_GroupList.size(); n++)
  {
    if( m_Abort )
    {
      ProgressBar::GetInstance()->Progress( m_GroupList.size() - n );
      std::cout << " aborted." << std::endl;
      return;
    }

    std::vector< Slice >& slicesOfCurrentGroup = m_GroupList[n].includedSlices;
    
    // get number of time steps and slice count per time step for this group
    unsigned int timeSteps = 0;
    std::vector< Slice >::iterator sliceIter = slicesOfCurrentGroup.begin();
    while(    sliceIter != slicesOfCurrentGroup.end() 
           && Equal( slicesOfCurrentGroup.front().origin, sliceIter->origin ) )
    {
      ++timeSteps;
      ++sliceIter;
    }
    assert (timeSteps > 0); // otherwise logic error in some earlier function
    unsigned int slicesPerTimeStep = slicesOfCurrentGroup.size() / timeSteps;

    //get image spacing
    Vector3D spacing;
    ipPicDescriptor* chiliPic = reinterpret_cast<ipPicDescriptor*>(slicesOfCurrentGroup.front().pic);
    interSliceGeometry_t* isg = ( interSliceGeometry_t* ) malloc ( sizeof( interSliceGeometry_t ) );
    pFetchSliceGeometryFromPic( chiliPic, isg );
    vtk2itk( isg->ps, spacing );
    if(    Equal(spacing[0], 0.0) 
        && Equal(spacing[1], 0.0) 
        && Equal(spacing[2], 0.0) 
      ) 
    {
      spacing.Fill(1.0); // if no good spacing, then use default spacing 1,1,1
    }
    free( isg );

    // we calculate the spacing ourselves (don't trust the slices)
    std::vector< Slice >::iterator firstSlice = slicesOfCurrentGroup.begin();
    for ( sliceIter  = slicesOfCurrentGroup.begin();
          sliceIter != slicesOfCurrentGroup.end();
          ++sliceIter )
    {
      if ( Equal(firstSlice->origin, sliceIter->origin ) ) continue;

      spacing[2] = (sliceIter->origin - firstSlice->origin).GetNorm();
      break;
     }
        
    // build image volume from a list of pic descriptors
    std::sort( slicesOfCurrentGroup.begin(), slicesOfCurrentGroup.end(), PositionSort );
    
    std::list<mitkIpPicDescriptor*> picList;
    for( std::vector< Slice >::iterator sliceIter = slicesOfCurrentGroup.begin(); 
        sliceIter != slicesOfCurrentGroup.end(); 
        ++sliceIter )
    {
      picList.push_back( sliceIter->pic );
    }

    // method of the super class
    GenerateData( picList, slicesPerTimeStep, timeSteps, spacing, m_GroupList[n].seriesDescription );
    std::cout << "." << std::flush;
    ProgressBar::GetInstance()->Progress(1);
  }
  std::cout << " done" << std::endl;
}

