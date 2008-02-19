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

#include <stdio.h>

#include "mitkSpacingSetFilter.h"

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
#include "mitkChiliMacros.h"

// function to round
double mitk::SpacingSetFilter::Round(double number, unsigned int decimalPlaces)
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
mitk::SpacingSetFilter::SpacingSetFilter()
{
}

// destructor
mitk::SpacingSetFilter::~SpacingSetFilter()
{
}

// set-function
void mitk::SpacingSetFilter::SetInput( std::list< ipPicDescriptor* > inputPicDescriptorList, std::string inputSeriesOID )
{
  m_SeriesOID = inputSeriesOID;
  m_PicDescriptorList = inputPicDescriptorList;
}

// get-function
std::vector< mitk::DataTreeNode::Pointer > mitk::SpacingSetFilter::GetOutput()
{
  return m_Output;
}

// the "main"-function
void mitk::SpacingSetFilter::Update()
{
  m_Output.clear();
  groupList.clear();
  m_ImageInstanceUIDs.clear();
  if( m_PicDescriptorList.size() > 0 && m_SeriesOID != "" )
  {
    SortSlicesToGroup();
    SortGroupsByLocation();
    //ShowAllGroupsWithSlices();
    CreatePossibleCombinations();
    SortPossibleCombinations();
    //ShowAllPossibleCombinations();
    //ShowAllSlicesWithUsedSpacings();
    SearchForMinimumCombination();
    //ShowAllResultCombinations();
    CheckForTimeSlicedCombinations();
    GenerateNodes();
  }
  else std::cout<<"SpacingSetFilter-WARNING: No SeriesOID or PicDescriptorList set."<<std::endl;
}

void mitk::SpacingSetFilter::SortSlicesToGroup()
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  for( std::list< ipPicDescriptor* >::iterator currentPic = m_PicDescriptorList.begin(); currentPic != m_PicDescriptorList.end(); currentPic ++ )
  {
    //check intersliceGeomtry
    interSliceGeometry_t* isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
    if( !pFetchSliceGeometryFromPic( (*currentPic), isg ) )
    {
      //PicDescriptor without a geometry not able to sort in a volume
      std::cout<<"SpacingSetFilter-WARNING: Found image without SliceGeometry. Image ignored."<<std::endl;
      delete isg;
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
      delete isg;
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
  delete isg;
  }
#endif
}

void mitk::SpacingSetFilter::SortGroupsByLocation()
{
  for( std::vector< Group >::iterator iter = groupList.begin(); iter != groupList.end(); iter++ )
  {
    std::sort( (*iter).includedSlices.begin(), (*iter).includedSlices.end(), LocationSort );
  }
}

bool mitk::SpacingSetFilter::LocationSort( const Slice elem1, const Slice elem2 )
{
  if( Equal( elem1.origin, elem2.origin ) )
    return elem1.imageNumber < elem2.imageNumber;
  else
    return elem1.origin*elem1.normal < elem2.origin*elem2.normal; // projection of origin on inter-slice-direction
}

void mitk::SpacingSetFilter::CreatePossibleCombinations()
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  //every group
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
  #endif
}

void mitk::SpacingSetFilter::CalculateSpacings( std::vector< Slice >::iterator basis, Group* currentGroup )
{
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
}

void mitk::SpacingSetFilter::searchFollowingSlices( std::vector< Slice >::iterator basis, double spacing, int imageNumberSpacing, Group* currentGroup )
{
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
    std::sort( groupList[n].possibleCombinations.begin(), groupList[n].possibleCombinations.end(), CombinationSort );
  }
}

bool mitk::SpacingSetFilter::CombinationSort( const std::set< Slice* > elem1, const std::set< Slice* > elem2 )
{
  return elem1.size() > elem2.size();
}

void mitk::SpacingSetFilter::SearchForMinimumCombination()
{
#ifdef CHILI_PLUGIN_VERSION_CODE
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
  }
#endif
}

void mitk::SpacingSetFilter::RekCombinationSearch( std::vector< std::set< Slice* > >::iterator mitkHideIfNoVersionCode(iterBegin), unsigned int mitkHideIfNoVersionCode(remainingCombinations), std::set< Slice* > mitkHideIfNoVersionCode(currentCombination), std::vector< std::set< Slice* > > mitkHideIfNoVersionCode(resultCombinations) )
{
#ifdef CHILI_PLUGIN_VERSION_CODE
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
#endif
}

void mitk::SpacingSetFilter::CheckForTimeSlicedCombinations()
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  //for every group
  for( unsigned int n = 0; n < groupList.size(); n++)
  {
    std::vector< std::vector < std::set< Slice* > > > newResult;
    newResult.clear();

    //test if more than one result exist
    if( groupList[n].resultCombinations.front().size() > 1 )
    {
      std::vector < std::set< Slice* > > singleCombination;
      singleCombination.clear();
      std::vector < std::set< Slice* > >::iterator last = groupList[n].resultCombinations.front().end();
      last--;
      //use all results
      for( std::vector < std::set< Slice* > >::iterator root = groupList[n].resultCombinations.front().begin(); root != last; root ++ )
      {
        if( (*root).size() > 0 )
        {
          bool addCombination = false;
          std::vector < std::set< Slice* > >::iterator next = root;
          next++;
          //test with following results
          for( std::vector < std::set< Slice* > >::iterator walk = next; walk != groupList[n].resultCombinations.front().end(); walk ++ )
          {
            //timesliced combinations have the same size
            if( (*root).size() == (*walk).size() )
            {
              std::set< Slice* >::iterator combinationOne = (*root).begin();
              std::set< Slice* >::iterator combinationTwo = (*walk).begin();
              unsigned x;
              //check the single positions
              for( x = 0; x < (*root).size(); x++ )
              {
                if( !Equal( (*combinationOne)->origin, (*combinationTwo)->origin ) )
                  break;
                combinationOne++;
                combinationTwo++;
              }
              //found timesliced combinations
              if( x == (*root).size() )
              {
                std::set< Slice* > tempUnion;
                tempUnion.clear();
                if( addCombination )
                {
                  set_union( (*walk).begin(), (*walk).end(), singleCombination.back().begin(), singleCombination.back().end(), inserter( tempUnion, tempUnion.begin() ) );

                  singleCombination.pop_back();
                  singleCombination.push_back( tempUnion );
                }
                else
                {
                  set_union( (*root).begin(), (*root).end(), (*walk).begin(), (*walk).end(), inserter( tempUnion, tempUnion.begin() ) );
                  singleCombination.push_back( tempUnion );
                }
                addCombination = true;
                (*walk).clear();
              }
            }
          }
          if( !addCombination )
            singleCombination.push_back( *root );
        }
      }
      if( (*last).size() > 0 )
        singleCombination.push_back( *last );
      newResult.push_back( singleCombination );
    }
    else
      newResult.push_back( groupList[n].resultCombinations.front() );

    groupList[n].resultCombinations = newResult;
  }
#endif
}

void mitk::SpacingSetFilter::GenerateNodes()
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  for( unsigned int n = 0; n < groupList.size(); n++)
  {
    //TODO groupList[n].resultCombinations.size() > 1, thats mean that there are more than one combinations to create the image

    for( std::vector < std::set< Slice* > >::iterator iter = groupList[n].resultCombinations.front().begin(); iter != groupList[n].resultCombinations.front().end(); iter ++ )
    {
      Image::Pointer resultImage = Image::New();
      unsigned int timeSteps;
      unsigned int sliceSteps;
      std::list< std::string > ListOfUIDs;
      ListOfUIDs.clear();

      if( groupList[n].dimension == 4 )
      {
        sliceSteps = (*iter->begin())->currentPic->n[2];
        timeSteps = (*iter->begin())->currentPic->n[3];

        resultImage->Initialize( groupList[n].includedSlices.front().currentPic );
        resultImage->SetPicChannel( groupList[n].includedSlices.front().currentPic );

        //get ImageInstanceUID
        std::string SingleUID;
        ipPicTSV_t* missingImageTagQuery = ipPicQueryTag( groupList[n].includedSlices.front().currentPic, (char*)tagIMAGE_INSTANCE_UID );
        if( missingImageTagQuery )
          SingleUID = static_cast<char*>( missingImageTagQuery->value );
        else
        {
          ipPicTSV_t *dicomHeader = ipPicQueryTag( groupList[n].includedSlices.front().currentPic, (char*)"SOURCE HEADER" );
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
        Vector3D rightVector, downVector, spacing;
        Point3D origin;
        ipPicDescriptor* header = ipPicCopyHeader( (*iter->begin())->currentPic, NULL );
        timeSteps = 0;
        std::set< Slice* >::iterator timeIter = (*iter).begin();
        while( timeIter != (*iter).end() && Equal( (*iter->begin())->origin, (*timeIter)->origin ) )
        {
          timeSteps++;
          timeIter++;
        }
        sliceSteps = iter->size()/timeSteps;

        if( groupList[n].dimension == 3 )
        {
          if( (*iter).size() == 1 )
            header->dim = 3;
          else
            header->dim = 4;
          header->n[2] = (*iter->begin())->currentPic->n[2];
          header->n[3] = iter->size()-1;
        }
        else
        {
          //2D
          if( sliceSteps == 1 )
          {
            if( timeSteps == 1 )
            {
              header->dim = 2;
              header->n[2] = 0;
              header->n[3] = 0;
            }
            else
            {
              //+t
              header->dim = 4;
              header->n[2] = 1;
              header->n[3] = timeSteps;
            }
          }
          else
          {
            //3D
            if( timeSteps == 1 )
            {
              header->dim = 3;
              header->n[2] = sliceSteps;
              header->n[3] = 0;
            }
            else
            {
              //+t
              header->dim = 4;
              header->n[2] = sliceSteps;
              header->n[3] = timeSteps;
            }
          }
        }

        if( groupList[n].dimension == 3 )
          resultImage->Initialize( header, 1, -1, (*iter->begin())->currentPic->n[2] );
        else
          resultImage->Initialize( header, 1, -1, sliceSteps );

        interSliceGeometry_t* isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
        if( !pFetchSliceGeometryFromPic( (*iter->begin())->currentPic, isg ) )
        {
          delete isg;
          return;
        }

        //rightVector, downVector
        vtk2itk( isg->u, rightVector );
        vtk2itk( isg->v, downVector );
        // its possible that a 2D-Image have no right- or down-Vector, but its not possible to initialize a [0,0,0] vector
        if( rightVector[0] == 0 && rightVector[1] == 0 && rightVector[2] == 0 )
          rightVector[0] = 1;
        if( downVector[0] == 0 && downVector[1] == 0 && downVector[2] == 0 )
          downVector[2] = -1;

        //spacing
        vtk2itk( isg->ps, spacing );
        if( ( spacing[0] == 0 && spacing[1] == 0 && spacing[2] == 0 ) || spacing[2] == 0.01 )
          spacing.Fill(1.0);

        //get the most used spacing
        if( (*iter).size() > 1 &&  groupList[n].dimension == 2 )
        {
          std::list<SpacingStruct> SpacingList;
          std::set< Slice* >::iterator iterFirst = (*iter).begin();
          std::set< Slice* >::iterator iterSecond = iterFirst;
          iterSecond++;
          Vector3D tmpSpacing;

          for( ; iterSecond != (*iter).end(); iterSecond++ )
          {
            if( !Equal( (*iterSecond)->origin, (*iterFirst)->origin ) )
            {
              tmpSpacing = (*iterSecond)->origin - (*iterFirst)->origin;
              spacing[2] = tmpSpacing.GetNorm();
              //search for spacing
              std::list<SpacingStruct>::iterator searchIter = SpacingList.begin();
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
              //if not exist, create new entry
              if( searchIter == SpacingList.end() )
              {
                SpacingStruct newElement;
                newElement.spacing = spacing;
                newElement.count = 1;
                SpacingList.push_back( newElement );
              }
              iterFirst = iterSecond;
            }
          }
          //get maximum spacing
          int count = 0;
          for( std::list<SpacingStruct>::iterator searchIter = SpacingList.begin(); searchIter != SpacingList.end(); searchIter++ )
          {
            if( searchIter->count > count )
            {
              spacing = searchIter->spacing;
              count = searchIter->count;
            }
          }
        }
        //origin
        vtk2itk( isg->o, origin );

        // set the timeBounds
        ScalarType timeBounds[] = {0.0, 1.0};
        // set the planeGeomtry
        PlaneGeometry::Pointer planegeometry = PlaneGeometry::New();

        planegeometry->InitializeStandardPlane( resultImage->GetDimension(0), resultImage->GetDimension(1), rightVector, downVector, &spacing );
        planegeometry->SetOrigin( origin );
        planegeometry->SetFrameOfReferenceID( FrameOfReferenceUIDManager::AddFrameOfReferenceUID( groupList[n].referenceUID.c_str() ) );
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
        unsigned int slice = 0;
        unsigned int time = 0;
        for( std::set< Slice* >::iterator it = (*iter).begin(); it != (*iter).end(); it++)
        {
          //get ImageInstanceUID
          std::string SingleUID;
          ipPicTSV_t* missingImageTagQuery = ipPicQueryTag( (*it)->currentPic, (char*)tagIMAGE_INSTANCE_UID );
          if( missingImageTagQuery )
            SingleUID = static_cast<char*>( missingImageTagQuery->value );
          else
          {
            ipPicTSV_t *dicomHeader = ipPicQueryTag( (*it)->currentPic, (char*)"SOURCE HEADER" );
            void* data = NULL;
            ipUInt4_t len = 0;
            if( dicomHeader && dicomFindElement( (unsigned char*) dicomHeader->value, 0x0008, 0x0018, &data, &len ) && data != NULL )
              SingleUID = static_cast<char*>( data );
          }
          ListOfUIDs.push_back( SingleUID );

          //add to mitk::Image
          if( groupList[n].dimension == 3 )
            resultImage->SetPicVolume( (*it)->currentPic, slice );
          else
            resultImage->SetPicSlice( (*it)->currentPic, slice, time );

          if( time < timeSteps-1 )
            time ++;
          else
          {
            time = 0;
            slice ++;
          }
        }
        delete isg;
      }

      if( resultImage->IsInitialized() && resultImage.IsNotNull() )
      {
        DataTreeNode::Pointer node = mitk::DataTreeNode::New();
        node->SetData( resultImage );
        DataTreeNodeFactory::SetDefaultImageProperties( node );

        if( groupList[n].seriesDescription == "" )
          groupList[n].seriesDescription = "no SeriesDescription";
        node->SetProperty( "name", new StringProperty( groupList[n].seriesDescription ) );
        node->SetProperty( "NumberOfSlices", new IntProperty( sliceSteps ) );
        node->SetProperty( "NumberOfTimeSlices", new IntProperty( timeSteps ) );
        if( m_SeriesOID != "" )
          node->SetProperty( "SeriesOID", new StringProperty( m_SeriesOID ) );

        mitk::PropertyList::Pointer tempPropertyList = CreatePropertyListFromPicTags( (*iter->begin())->currentPic );
        for( mitk::PropertyList::PropertyMap::const_iterator iter = tempPropertyList->GetMap()->begin(); iter != tempPropertyList->GetMap()->end(); iter++ )
        {
          node->SetProperty( iter->first.c_str(), iter->second.first );
        }

        m_Output.push_back( node );
        m_ImageInstanceUIDs.push_back( ListOfUIDs );
      }
    }
  }
#endif
}

std::vector< std::list< std::string > > mitk::SpacingSetFilter::GetImageInstanceUIDs()
{
  return m_ImageInstanceUIDs;
}

const mitk::PropertyList::Pointer mitk::SpacingSetFilter::CreatePropertyListFromPicTags( ipPicDescriptor* imageToExtractTagsFrom )
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
          std::cout << "WARNING: Type of PIC-Tag '" << currentTag->type << "'( " << propertyName << " ) not handled in mitkSpacingSetFilter." << std::endl;
        }
        break;
      }
    }
    // proceed to the next tag
    currentTagNode = currentTagNode->next;
  }
  return resultPropertyList;
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

