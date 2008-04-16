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

#include "mitkSpacingSetFilter.h"

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

// constructor
mitk::SpacingSetFilter::SpacingSetFilter()
{
}

// destructor
mitk::SpacingSetFilter::~SpacingSetFilter()
{
}

// the "main"-function
void mitk::SpacingSetFilter::Update()
{
  m_Output.clear();
  groupList.clear();
  m_ImageInstanceUIDs.clear();
  if( m_PicDescriptorList.size() > 0 && m_SeriesOID != "" )
  {
    ProgressBar::GetInstance()->AddStepsToDo( 7 );
    SortPicsToGroup();
    ProgressBar::GetInstance()->Progress();
    SortSlicesByLocation();
    ProgressBar::GetInstance()->Progress();
    //ShowAllGroupsWithSlices();
    CreatePossibleCombinations();
    ProgressBar::GetInstance()->Progress();
    SortPossibleCombinations();
    ProgressBar::GetInstance()->Progress();
    //ShowAllPossibleCombinations();
    //ShowAllSlicesWithUsedSpacings();
    SearchForMinimumCombination();
    ProgressBar::GetInstance()->Progress();
    //ShowAllResultCombinations();
    CheckForTimeSlicedCombinations();
    ProgressBar::GetInstance()->Progress();
    GenerateImages();
    ProgressBar::GetInstance()->Progress();
  }
  else std::cout<<"SpacingSetFilter-WARNING: No SeriesOID or PicDescriptorList set."<<std::endl;
}

void mitk::SpacingSetFilter::SortPicsToGroup()
{
  for( std::list< ipPicDescriptor* >::iterator currentPic = m_PicDescriptorList.begin(); currentPic != m_PicDescriptorList.end(); currentPic ++ )
  {
    //check intersliceGeomtry
    interSliceGeometry_t* isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
    if( !pFetchSliceGeometryFromPic( (*currentPic), isg ) )
    {
      //PicDescriptor without a geometry not able to sort in a volume
      std::cout<<"SpacingSetFilter-WARNING: Found image without SliceGeometry. Image ignored."<<std::endl;
      free( isg );
      continue;
    }

    //new Slice
    Slice newSlice;
    newSlice.sliceUsedWithSpacing.clear();
    //set PicDescriptor
    newSlice.currentPic = (*currentPic);
    //set Origin
    vtk2itk( isg->o, newSlice.origin );
    //set normale
    Vector3D rightVector, downVector;
    vtk2itk( isg->u, rightVector );
    vtk2itk( isg->v, downVector );
    newSlice.normal[0] = Round( ( ( rightVector[1]*downVector[2] ) - ( rightVector[2]*downVector[1] ) ), 2 );
    newSlice.normal[1] = Round( ( ( rightVector[2]*downVector[0] ) - ( rightVector[0]*downVector[2] ) ), 2);
    newSlice.normal[2] = Round( ( ( rightVector[0]*downVector[1] ) - ( rightVector[1]*downVector[0] ) ), 2);
    //set imageNumber
    ipPicTSV_t* imagenumberTag = ipPicQueryTag( newSlice.currentPic, (char*)tagIMAGE_NUMBER );
    if( imagenumberTag && imagenumberTag->type == ipPicInt )
      newSlice.imageNumber = *( (int*)(imagenumberTag->value) );
    else
    {
      ipPicTSV_t *tsv;
      void* data = NULL;
      ipUInt4_t len = 0;
      tsv = ipPicQueryTag( newSlice.currentPic, (char*)"SOURCE HEADER" );
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
    //dimension
    int currentDimension = (*currentPic)->dim;

    if( currentDimension < 2 || currentDimension > 4 )
    {
      std::cout<<"SpacingSetFilter-WARNING: Wrong PicDescriptor-Dimension. Image ignored."<<std::endl;
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
      maxCount = groupList.size();
    }

    // searching for equal output
    while( curCount < maxCount && !foundMatch )
    {
      //checking referenceUID, seriesDescription, pixelSize and NormaleWithSize
      if( groupList[ curCount ].referenceUID == isg->forUID && groupList[ curCount ].seriesDescription == currentSeriesDescription && groupList[ curCount ].pixelSize == currentPixelSize && groupList[ curCount ].normalWithImageSize == currentNormalWithImageSize )
      {
        if( groupList[ curCount ].dimension == 2 || ( groupList[ curCount ].dimension == 3 && groupList[ curCount ].includedSlices.front().origin == newSlice.origin && groupList[ curCount ].includedSlices.front().currentPic->n[2] == (*currentPic)->n[2] ) )
          foundMatch = true;
        else
          curCount++;
      }
      else
        curCount++;
    }

    //if a matching output found, add slice or create a new group
    if( foundMatch )
      groupList[ curCount ].includedSlices.push_back( newSlice );
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
      newGroup.possibleCombinations.clear();
      groupList.push_back( newGroup );
    }
  free( isg );
  }
}

void mitk::SpacingSetFilter::SortSlicesByLocation()
{
  for( std::vector< Group >::iterator iter = groupList.begin(); iter != groupList.end(); iter++ )
  {
    std::sort( (*iter).includedSlices.begin(), (*iter).includedSlices.end(), PositionSort );
  }
}

bool mitk::SpacingSetFilter::PositionSort( const Slice elem1, const Slice elem2 )
{
  if( Equal( elem1.origin, elem2.origin ) )
    return elem1.imageNumber < elem2.imageNumber;
  else
    return elem1.origin*elem1.normal < elem2.origin*elem2.normal; // projection of origin on inter-slice-direction
}

void mitk::SpacingSetFilter::CreatePossibleCombinations()
{
  //every group
  ProgressBar::GetInstance()->AddStepsToDo( groupList.size() );
  for( unsigned int x = 0; x < groupList.size(); x++ )
  {
    if( groupList[x].includedSlices.size() == 1 )
    {
      m_Set.clear();
      m_Set.insert( &(*groupList[x].includedSlices.begin() ) );
      groupList[x].possibleCombinations.push_back( m_Set );
    }
    else
    {
      if( groupList[x].includedSlices.size() > 1 )
      {
        std::vector< Slice >::iterator lastIter = groupList[x].includedSlices.end();
        lastIter--;
        for( std::vector< Slice >::iterator walkIter = groupList[x].includedSlices.begin(); walkIter != lastIter; walkIter++ )
        {
          //add the current slice as single possibleCombination
          m_Set.clear();
          m_Set.insert( &(*walkIter) );
          groupList[x].possibleCombinations.push_back( m_Set );

          CalculateSpacings( walkIter, &groupList[x] );
        }
        //add the last slice
        m_Set.clear();
        m_Set.insert( &(*lastIter) );
        groupList[x].possibleCombinations.push_back( m_Set );
      }
    }
  }
  ProgressBar::GetInstance()->Progress();
}

void mitk::SpacingSetFilter::CalculateSpacings( std::vector< Slice >::iterator basis, Group* currentGroup )
{
  ProgressBar::GetInstance()->AddStepsToDo( 1 );
  std::vector< Slice >::iterator referenceIter = basis;

  for( std::vector< Slice >::iterator walkIter = basis; walkIter != currentGroup->includedSlices.end(); walkIter++ )
  {
    if( !Equal( walkIter->origin, referenceIter->origin ) )
    {
      //calculate spacing and image-number-spacing
      Vector3D tempDistance = walkIter->origin - basis->origin;
      double spacing = Round( tempDistance.GetNorm(), 2 );
      int imageNumberSpacing = walkIter->imageNumber - basis->imageNumber;

      bool search = true;
      if( EqualImageNumbers( basis ) )
      {
        if( find( basis->sliceUsedWithSpacing.begin(), basis->sliceUsedWithSpacing.end(), spacing ) != basis->sliceUsedWithSpacing.end() )
          search = false;
      }

      if( EqualImageNumbers( walkIter ) )
      {
        if( find( walkIter->sliceUsedWithSpacing.begin(), walkIter->sliceUsedWithSpacing.end(), spacing ) != walkIter->sliceUsedWithSpacing.end() )
          search = false;
      }

      if( search )
      {
        m_Set.clear();
        m_Set.insert( &(*basis) );
        m_Set.insert( &(*walkIter) );
        walkIter->sliceUsedWithSpacing.insert( spacing );
        searchFollowingSlices( walkIter, spacing, imageNumberSpacing, currentGroup );

        if( m_Set.size() > 2 )
        {
          if( EqualImageNumbers( basis ) )
            referenceIter = walkIter;
          currentGroup->possibleCombinations.push_back( m_Set );
        }
      }
    }
  }
  ProgressBar::GetInstance()->Progress();
}

void mitk::SpacingSetFilter::searchFollowingSlices( std::vector< Slice >::iterator basis, double spacing, int imageNumberSpacing, Group* currentGroup )
{
  ProgressBar::GetInstance()->AddStepsToDo( 1 );
  std::vector< Slice >::iterator referenceIter = basis;
  bool walkON = true;
  while( walkON )
  {
    walkON = false;
    for( std::vector< Slice >::iterator walkIter = referenceIter; walkIter != currentGroup->includedSlices.end(); walkIter++ )
    {
      //calculate spacing and image-number-spacing
      Vector3D tempDistance = walkIter->origin - referenceIter->origin;
      double currentSpacing = Round( tempDistance.GetNorm(), 2 );
      int currentImageNumberSpacing = walkIter->imageNumber - referenceIter->imageNumber;

      if( currentSpacing == spacing && currentImageNumberSpacing == imageNumberSpacing )
      {
        if( find( walkIter->sliceUsedWithSpacing.begin(), walkIter->sliceUsedWithSpacing.end(), spacing ) == walkIter->sliceUsedWithSpacing.end() )
        {
          m_Set.insert( &(*walkIter) );
          walkIter->sliceUsedWithSpacing.insert( spacing );
          referenceIter = walkIter;
          walkON = true;
          break;
        }
      }
      else
        if( currentSpacing > spacing )
          break;
    }
  }
  ProgressBar::GetInstance()->Progress();
}

bool mitk::SpacingSetFilter::EqualImageNumbers( std::vector< Slice >::iterator testIter )
{
  bool result = false;
  std::vector< Slice >::iterator imageNumberIter = testIter;
  imageNumberIter--;
  while( Equal( imageNumberIter->origin, testIter->origin ) )
  {
    if( imageNumberIter->imageNumber == testIter->imageNumber )
    {
      result = true;
      break;
    }
    imageNumberIter--;
  }

  if( !result )
  {
    imageNumberIter = testIter;
    imageNumberIter++;
    while( Equal( imageNumberIter->origin, testIter->origin ) )
    {
      if( imageNumberIter->imageNumber == testIter->imageNumber )
      {
        result = true;
        break;
      }
      imageNumberIter++;
    }
  }

  return result;
}

void mitk::SpacingSetFilter::SortPossibleCombinations()
{
  //its very important to sort the combinations
  //they get sorted by there number of elements
  //look at the begining of "RekCombinationSearch()", there the sort get used to break up unnecessary recursive steps
  for( unsigned int n = 0; n< groupList.size(); n++ )
  {
    std::sort( groupList[n].possibleCombinations.begin(), groupList[n].possibleCombinations.end(), CombinationCountSort );
  }
}

bool mitk::SpacingSetFilter::CombinationCountSort( const std::set< Slice* > elem1, const std::set< Slice* > elem2 )
{
  return elem1.size() > elem2.size();
}

void mitk::SpacingSetFilter::SearchForMinimumCombination()
{
  ProgressBar::GetInstance()->AddStepsToDo( groupList.size() );
  for( unsigned int n = 0; n < groupList.size(); n++ )
  {
    //a not timesliced 2D-image have only one possibleCombination, so we use this as resultCombination
    if( groupList[n].possibleCombinations.size() == 1 )
    {
      std::vector< std::set< Slice* > > resultCombination;
      resultCombination.clear();
      resultCombination.push_back( groupList[n].possibleCombinations.front() );
      groupList[n].resultCombinations.clear();
      groupList[n].resultCombinations.push_back( resultCombination );
    }
    else
    //if more possible combinations included, we searching for the smallest number of combinations which represented the whole
    {
      std::vector< std::set< Slice* > >::iterator iterBegin = groupList[n].possibleCombinations.begin();
      int remainingCombinations = groupList[n].possibleCombinations.size();
      std::vector< std::set< Slice* > >::iterator iterEnd = groupList[n].possibleCombinations.end();
      iterEnd--;
      //here the final result get saved
      groupList[n].resultCombinations.clear();

      //this variables used by RekCombinationSearch()
      m_IterGroupEnd = groupList[n].possibleCombinations.end();  // -> the end to search and combine
      m_TotalCombinationCount = groupList[n].includedSlices.size();  // -> the count of all included slices (total set)
      m_GroupResultCombinations.clear();  // -> all results with the minimum count of used combinations; changed while RekCombinationSearch(); at the end the results get saved to groupList[n].resultCombinations

      while( iterBegin != iterEnd )
      {
        //the possible combinations sorted, from combinations with lots of elements to combinations with less elements
        //if the mulitplication from the current minimal-needed-combination-count with the current combination-element-count is lower then the count of all needed slices, there is no possibility to create a result wich include all slices and we can break up
        if( m_GroupResultCombinations.size() != 0 && ( ( ( (*iterBegin).size() ) * ( m_GroupResultCombinations.front().size() ) ) < m_TotalCombinationCount ) )
          break;

        //the current combination
        std::vector< std::set< Slice* > > resultCombination;
        resultCombination.clear();
        resultCombination.push_back( (*iterBegin) );

        //if the count is equal, the set's included the same elements and we have a result
        if( m_TotalCombinationCount == (*iterBegin).size() )
        {
          if( m_GroupResultCombinations.size() != 0 && m_GroupResultCombinations.front().size() > 1 )
            m_GroupResultCombinations.clear();
          if( m_GroupResultCombinations.size() == 0 || m_GroupResultCombinations.front().size() == 1 )
            m_GroupResultCombinations.push_back( resultCombination );
        }
        else
        //else we have to search, combine, check, ...
          RekCombinationSearch( iterBegin, remainingCombinations, (*iterBegin), resultCombination );
        iterBegin++;
        remainingCombinations--;
      }
      //the result get copied
      groupList[n].resultCombinations = m_GroupResultCombinations;
    }
    ProgressBar::GetInstance()->Progress();
  }
}

void mitk::SpacingSetFilter::RekCombinationSearch( std::vector< std::set< Slice* > >::iterator mitkHideIfNoVersionCode(iterBegin), unsigned int mitkHideIfNoVersionCode(remainingCombinations), std::set< Slice* > mitkHideIfNoVersionCode(currentCombination), std::vector< std::set< Slice* > > mitkHideIfNoVersionCode(resultCombinations) )
{
  //to found a combination which represent the whole, the single combinations get combined
  //the intersection created and if they empty, the two combinations combined to one
  //if the intersection not empty the two combinations included the same elements
  //the new combination get checked with the next combination and combined until they represented the whole

  //resultCombinations = current used combinations (as single combinations)
  //currentCombination = current used combinations (as one union combination)
  //groupResultCombinations.front() = current minimal known combinations
  //iterBegin = current combination to check
  //remainingCombinations = number of remeaning combinations to check
  //m_TotalCombinationCount = number of

  ProgressBar::GetInstance()->AddStepsToDo( 1 );
  iterBegin++;
  remainingCombinations--;
  while ( iterBegin != m_IterGroupEnd )
  {
    //if we have found a result, we should not combine more combinationens than the result included
    if( m_GroupResultCombinations.size() != 0 && resultCombinations.size() >= m_GroupResultCombinations.front().size() )
      return;

    //the current result can integrated two more combinations bevor the size is equal to the number of combinations from the found result
    //the current combination count have 5 elements less than the total combination
    //the current found combinations have only one element, they are sorted, so they dont get more
    //two more integrations with combinations with one element can not fill the lack of 5 elements, so we can break up
    if( m_GroupResultCombinations.size() != 0 && ( ( ( m_GroupResultCombinations.front().size() - resultCombinations.size() ) * iterBegin->size() ) + currentCombination.size() ) < m_TotalCombinationCount )
      return;

    //the current result integrated 90 combinations from 100
    //the current combination count is five position bevor the last element
    //the current combinations included one element
    //the five positions with five elements can not fill 10 needed element, so we can break up
    if( m_GroupResultCombinations.size() != 0 && ( remainingCombinations * iterBegin->size() ) < ( m_TotalCombinationCount - currentCombination.size() ) )
      return;

    //create intersection
    std::set< Slice* > intersection;
    intersection.clear();
    set_intersection( currentCombination.begin(), currentCombination.end(), (*iterBegin).begin(), (*iterBegin).end(), inserter( intersection, intersection.begin() ) );

    //if the intersection is empty the set's can be unified
    if( intersection.empty() )
    {
      //combine the set's
      std::set< Slice* > tempUnion;
      tempUnion.clear();
      set_union( currentCombination.begin(), currentCombination.end(), (*iterBegin).begin(), (*iterBegin).end(), inserter( tempUnion, tempUnion.begin() ) );
      //we have to save the new combination of the two one and the single used combination to create the single resultimages at the end
      //tempUnion contain the combination of the two one
      //resultCombinations contain the single combinations of the current tempUnion
      resultCombinations.push_back( (*iterBegin) );

      //to check if the tempUnion contain the whole slices, we use the size; the union is a std::set which not contain equal elements
      if( m_TotalCombinationCount == tempUnion.size() )
      {
        if( m_GroupResultCombinations.size() != 0 && m_GroupResultCombinations.front().size() > resultCombinations.size() )
          m_GroupResultCombinations.clear();
        if( m_GroupResultCombinations.size() == 0 || m_GroupResultCombinations.front().size() == resultCombinations.size() )
          m_GroupResultCombinations.push_back( resultCombinations );
      }
      else
      //if the current union not contain all elements we have to continue the search
        RekCombinationSearch( iterBegin, remainingCombinations, tempUnion, resultCombinations );
      //for the recursive degression we have to delete the add combination from the possible result
      resultCombinations.pop_back();
    }
    iterBegin++;
    remainingCombinations--;
  }
  ProgressBar::GetInstance()->Progress();
}

void mitk::SpacingSetFilter::CheckForTimeSlicedCombinations()
{
  for( unsigned int n = 0; n < groupList.size(); n++)
  {
    if( groupList[n].resultCombinations.front().size() > 1 ) //TODO only one result is handled
    {
      std::vector< std::set< Slice* > >::iterator rootCombinationIter = groupList[n].resultCombinations.front().begin();
      while( rootCombinationIter != groupList[n].resultCombinations.front().end() )
      {
        std::set< Slice* > timeSlicedVolume;
        timeSlicedVolume.clear();
        timeSlicedVolume.insert( (*rootCombinationIter).begin(), (*rootCombinationIter).end() );

        std::vector< std::set< Slice* > >::iterator searchCombinationIter = rootCombinationIter;
        searchCombinationIter++;
        while( searchCombinationIter != groupList[n].resultCombinations.front().end() )
        {
          if( (*rootCombinationIter).size() == (*searchCombinationIter).size() && Equal( (*(*rootCombinationIter).begin())->origin, (*(*searchCombinationIter).begin())->origin ) )  //the combinations have the same size and the same origin
          {
            if( (*rootCombinationIter).size() > 1 )  //3D
            {
              //calculate the spacing at the first combination
              std::set< Slice* >::iterator spacingIter = (*rootCombinationIter).begin();
              spacingIter++;
              Vector3D tempDistance = (*spacingIter)->origin - (*(*rootCombinationIter).begin())->origin;
              double referenceSpacingOne = Round( tempDistance.GetNorm(), 2 );
              int imageNumberSpacingOne = (*spacingIter)->imageNumber - (*(*rootCombinationIter).begin())->imageNumber;

              //calculate the spacing at the second combination
              spacingIter = (*searchCombinationIter).begin();
              spacingIter++;
              tempDistance = (*spacingIter)->origin - (*(*searchCombinationIter).begin())->origin;
              double referenceSpacingTwo = Round( tempDistance.GetNorm(), 2 );
              int imageNumberSpacingTwo = (*spacingIter)->imageNumber - (*(*searchCombinationIter).begin())->imageNumber;

              //equal spacings == timesliced
              if( referenceSpacingOne == referenceSpacingTwo && imageNumberSpacingOne == imageNumberSpacingTwo )
              {
                timeSlicedVolume.insert( (*searchCombinationIter).begin(), (*searchCombinationIter).end() );
                searchCombinationIter = groupList[n].resultCombinations.front().erase( searchCombinationIter );
              }
              else searchCombinationIter++;
            }
            else  //2D
            {
              timeSlicedVolume.insert( (*(*searchCombinationIter).begin()) );
              searchCombinationIter = groupList[n].resultCombinations.front().erase( searchCombinationIter );
            }
          }
          else searchCombinationIter++;
        }

        if( timeSlicedVolume.size() > rootCombinationIter->size() )
        {
          groupList[n].resultCombinations.front().push_back( timeSlicedVolume );
          rootCombinationIter = groupList[n].resultCombinations.front().erase( rootCombinationIter );
        }
        else rootCombinationIter++;
      }
    }
  }
}

void mitk::SpacingSetFilter::GenerateImages()
{
  //generate mitk::Images
  for( unsigned int n = 0; n < groupList.size(); n++)
  {
    for( std::vector < std::set< Slice* > >::iterator combinationIter = groupList[n].resultCombinations.front().begin(); combinationIter != groupList[n].resultCombinations.front().end(); combinationIter ++ )  //TODO only one result is handled
    {
      //get all ipPicDescriptor
      std::list<ipPicDescriptor*> usedPic;
      usedPic.clear();
      for( std::set< Slice* >::iterator picIter = combinationIter->begin(); picIter != combinationIter->end(); picIter++ )
        usedPic.push_back( (*picIter)->currentPic );

      //get time, count
      int timeSteps = 0, sliceSteps = 0;
      std::set< Slice* >::iterator originIter = combinationIter->begin();
      while( originIter != combinationIter->end() && Equal( (*combinationIter->begin())->origin, (*originIter)->origin ) )
      {
        timeSteps++;
        originIter++;
      }
      sliceSteps = combinationIter->size()/timeSteps;

      //get spacing
      Vector3D spacing;
        //initialize
      interSliceGeometry_t* isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
      pFetchSliceGeometryFromPic( (*combinationIter->begin())->currentPic, isg );
      vtk2itk( isg->ps, spacing );
      if( spacing[0] == 0 && spacing[1] == 0 && spacing[2] == 0 )
        spacing.Fill(1.0);
      free( isg );
        //get the not rounded spacing
      std::list<Spacing> SpacingList;
      std::set< Slice* >::iterator walkIter = combinationIter->begin();
      walkIter++;
      for( ; walkIter != combinationIter->end(); walkIter ++)
      {
        std::set<Slice*>::iterator iterB4 = walkIter;
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

      GenerateData( usedPic, sliceSteps, timeSteps, spacing, groupList[n].seriesDescription );
    }
  }
}

void mitk::SpacingSetFilter::ShowAllGroupsWithSlices()
{
  for( unsigned int n = 0; n < groupList.size(); n++)
  {
    //the Head of all groups
    std::cout << "-----------" << std::endl;
    std::cout << "SpacingSetFilter-Output" << n << std::endl;
    std::cout << "ReferenceUID:" << groupList[n].referenceUID << std::endl;
    std::cout << "SeriesDescription:" << groupList[n].seriesDescription << std::endl;
    std::cout << "PixelSize:" << groupList[n].pixelSize << std::endl;
    std::cout << "Normal with image size:" << groupList[n].normalWithImageSize << std::endl;
    std::cout << "-----------" << std::endl;

    //every single descriptor with ImageNumber
    std::cout << "All included PicDescriptors:" << std::endl;
    for( std::vector< Slice >::iterator it = groupList[n].includedSlices.begin(); it != groupList[n].includedSlices.end(); it++ )
    {
      std::cout<<"Image: "<<(*it).imageNumber<<std::endl;
    }
  }
}

void mitk::SpacingSetFilter::ShowAllPossibleCombinations()
{
  for( unsigned int n = 0; n < groupList.size(); n++)
  {
    for( std::vector< std::set< Slice* > >::iterator iter = groupList[n].possibleCombinations.begin(); iter != groupList[n].possibleCombinations.end(); iter++ )
    {
      std::ostringstream stringHelper;
      stringHelper << "( ";
      for( std::set< Slice* >::iterator iterator = (*iter).begin(); iterator != (*iter).end(); iterator++ )
      {
        stringHelper << (*iterator)->imageNumber << " ";
      }
      stringHelper << ")";
      std::cout<< stringHelper.str() <<std::endl;
    }
    std::cout<<"Output"<<n<<": with "<<groupList[n].possibleCombinations.size()<<" combinations"<<std::endl;
  }
}

void mitk::SpacingSetFilter::ShowAllResultCombinations()
{
  for( unsigned int n = 0; n < groupList.size(); n++)
  {
    std::cout<<"Output"<<n<<std::endl;
    for( std::vector< std::vector < std::set< Slice* > > >::iterator it = groupList[n].resultCombinations.begin(); it != groupList[n].resultCombinations.end(); it++)
    {
      std::ostringstream stringHelper;
      stringHelper << "{ ";

      for( std::vector< std::set< Slice* > >::iterator iter = (*it).begin(); iter != (*it).end(); iter++ )
      {
        stringHelper << "( ";
        for( std::set< Slice* >::iterator iterator = (*iter).begin(); iterator != (*iter).end(); iterator++ )
        {
          stringHelper << (*iterator)->imageNumber << " ";
        }
        stringHelper << ") ";
      }
      stringHelper << "}";
      std::cout<< stringHelper.str() <<std::endl;
    }
  }
}

void mitk::SpacingSetFilter::ShowAllSlicesWithUsedSpacings()
{
  for( unsigned int n = 0; n < groupList.size(); n++)
  {
    for( std::vector< Slice >::iterator it = groupList[n].includedSlices.begin(); it != groupList[n].includedSlices.end(); it++ )
    {
      std::ostringstream stringHelper;
      stringHelper << "Image: " << (*it).imageNumber << " Used Spacings:";
      for( std::set< double >::iterator iter = (*it).sliceUsedWithSpacing.begin(); iter != (*it).sliceUsedWithSpacing.end(); iter++ )
        stringHelper << " " << (*iter);
      std::cout<<stringHelper.str()<<std::endl;
    }
  }
}

#endif
