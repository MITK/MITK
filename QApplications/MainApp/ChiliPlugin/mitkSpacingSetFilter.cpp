/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
    ipPicTSV_t* imagenumberTag = ipPicQueryTag( newSlice.currentPic, tagIMAGE_NUMBER );
    if( imagenumberTag && imagenumberTag->type == ipPicInt )
      newSlice.imageNumber = *( (int*)(imagenumberTag->value) );
    else
    {
      ipPicTSV_t *tsv;
      void* data = NULL;
      ipUInt4_t len = 0;
      tsv = ipPicQueryTag( newSlice.currentPic, "SOURCE HEADER" );
      if( tsv )
      {
        if( dicomFindElement( (unsigned char*) tsv->value, 0x0020, 0x0013, &data, &len ) );
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
    ipPicTSV_t* seriesDescriptionTag = ipPicQueryTag( (*currentPic), tagSERIES_DESCRIPTION );
    if( seriesDescriptionTag )
      currentSeriesDescription = static_cast<char*>( seriesDescriptionTag->value );
    else
    {
      ipPicTSV_t *tsv;
      void* data = NULL;
      ipUInt4_t len = 0;
      tsv = ipPicQueryTag( (*currentPic), "SOURCE HEADER" );
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
    //TODO "double" Vector bei itk vector mit template
    Vector3D currentNormalWithImageSize;
    rightVector = rightVector * newSlice.currentPic->n[0];
    downVector = downVector * newSlice.currentPic->n[1];
    //TODO bei 3D Normale mit n[2]
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
//TODO handle timesliced volumes without or the same imagenumbers
//TODO try to detect timesliced 2D data, current they get shown as single 2D
//TODO while schleifen in funktionen aufteilen
  //every group
  for( unsigned int x = 0; x < groupList.size(); x++)
  {
    //with more than one slice
    if( groupList[x].includedSlices.size() > 1 )
    {
      //this two iterator specify the spacing
      std::vector< Slice >::iterator firstIter = groupList[x].includedSlices.begin();
      std::vector< Slice >::iterator secondIter = firstIter;
      //this have to work until the first iter is at the end of the included slices
      bool firstIterAtEnd = false;
      while( !firstIterAtEnd )
      {
        double masterSpacing;
        Vector3D tempDistance;
        bool originTimeSliced = false;

        //check if current iter is timeslices or not, therefor we have to check bevor and after the current slice
        std::vector< Slice >::iterator firstIterTimeSlicedOne = firstIter;
        std::vector< Slice >::iterator firstIterTimeSlicedTwo = firstIter;
        firstIterTimeSlicedOne++;
        //if the current iter is the first one, only the following iter decide if its timesliced or not
        if( firstIterTimeSlicedTwo != groupList[x].includedSlices.begin() )
          firstIterTimeSlicedTwo--;
        else
          firstIterTimeSlicedTwo++;
        //check if one of them is equal, the origin is timesliced
        if( ( Equal( firstIter->origin, firstIterTimeSlicedOne->origin ) && firstIterTimeSlicedOne != groupList[x].includedSlices.end() ) || ( Equal( firstIter->origin, firstIterTimeSlicedTwo->origin ) ) )
          originTimeSliced = true;

        //search the first spacing
        while( Equal( firstIter->origin, secondIter->origin ) && secondIter != groupList[x].includedSlices.end() )
          secondIter++;
        if( Equal( firstIter->origin, secondIter->origin ) || secondIter == groupList[x].includedSlices.end() )
          firstIterAtEnd = true;  //no spacing found
        else
        {
          tempDistance = secondIter->origin - firstIter->origin;
          masterSpacing = Round( tempDistance.GetNorm(), 2 );
          //we increase the second iter and so the spacing; so we create and count all possible combinations
          bool secondIterAtEnd = false;
          //this have to work until the second iter is at the end of the included slices
          while( !secondIterAtEnd )
          {
            //if the current slice always used with the current spacing, we dont have to create a new combination
            //example: ( 2,4,6,8,10 ) so we dont need to create (4,6,8,10) or (6,8,10) ...
            if( find( (*firstIter).sliceUsedWithSpacing.begin(), (*firstIter).sliceUsedWithSpacing.end(), masterSpacing ) == (*firstIter).sliceUsedWithSpacing.end() )
            {

/*
              //we have not to check the first slice only; for timesliced volumes with same imagenumbers in timeslice we have to check the second one too
              //example: ( 8,8,10,12,14 ), the same imagenumbers have the same location (timesliced); the first eight would create the set ( 8,10,12,14 ) and the second eight (which have no sliceUsedWithSpacing because no one use this slice) would create the same set
              if( find( (*secondIter).sliceUsedWithSpacing.begin(), (*secondIter).sliceUsedWithSpacing.end(), masterSpacing ) == (*secondIter).sliceUsedWithSpacing.end() )
              {
*/

              //create a new combination and add the first two iterators
              std::set< Slice* > newCombination;
              newCombination.clear();

              newCombination.insert( &(*firstIter) );
              newCombination.insert( &(*secondIter) );
              //add the current spacing to the included slice, so that we dont create such combinations like the example show
              (*secondIter).sliceUsedWithSpacing.insert( masterSpacing );

              //we have the spacing between the first to slices, now we want to count the same following spacing
              std::vector< Slice >::iterator searchSpacingFirst = secondIter;
              std::vector< Slice >::iterator searchSpacingSecond = secondIter;
              double referenceSpacing;
              bool searchIterSecondAtEnd = false;
              //not only the timeslice of the origin is important, we have to know if the slice bevor was timesliced too
              bool sliceBevorWasTimeSliced =false;

              //this have to work until the second search iter is at the end of the included slices --> no following spacing
              while( !searchIterSecondAtEnd )
              {
                //search the first spacing between the followingSpacingIterator
                while( Equal( searchSpacingFirst->origin, searchSpacingSecond->origin ) && searchSpacingSecond != groupList[x].includedSlices.end() )
                  searchSpacingSecond++;
                //there is no spacing or we are at the end of the included slices
                if( Equal( searchSpacingFirst->origin, searchSpacingSecond->origin ) || searchSpacingSecond == groupList[x].includedSlices.end() )
                  searchIterSecondAtEnd = true;
                else
                {
                  //we found a spacing, now we have to check if the found spacing equal with the known spacing
                  tempDistance = searchSpacingSecond->origin - searchSpacingFirst->origin;
                  referenceSpacing = Round( tempDistance.GetNorm(), 2 );
                  //increase the spacing until its equal or higher
                  while( referenceSpacing < masterSpacing && searchSpacingSecond != groupList[x].includedSlices.end() )
                  {
                    searchSpacingSecond++;
                    tempDistance = searchSpacingSecond->origin - searchSpacingFirst->origin;
                    referenceSpacing = Round( tempDistance.GetNorm(), 2 );
                  }
                  //no equal spacing found
                  if( referenceSpacing != masterSpacing || searchSpacingSecond == groupList[x].includedSlices.end() )
                    searchIterSecondAtEnd = true;
                  else
                  {
                    //check if current slice is timesliced
                    bool currentTimeSliced = false;
                    std::vector< Slice >::iterator curIterTimeSlicedOne = searchSpacingSecond;
                    std::vector< Slice >::iterator curIterTimeSlicedTwo = searchSpacingSecond;
                    curIterTimeSlicedOne++;
                    curIterTimeSlicedTwo--;
                    if( ( Equal( searchSpacingSecond->origin, curIterTimeSlicedOne->origin ) && curIterTimeSlicedOne != groupList[x].includedSlices.end() ) || ( Equal( searchSpacingSecond->origin, firstIterTimeSlicedTwo->origin ) && firstIterTimeSlicedTwo != groupList[x].includedSlices.begin()-- ) )
                      currentTimeSliced = true;

                    //if the current slice or the origin is timesliced
                    if( originTimeSliced || currentTimeSliced )
                    {
                      //set that we know it for the next slice
                      if( currentTimeSliced) sliceBevorWasTimeSliced = true;
                      else sliceBevorWasTimeSliced = false;

                      std::vector< Slice >::iterator timeSliceReference = searchSpacingSecond;
                      //if we found timesliced data, we use the image number to sort them
                      //so we take the image-number-spacing from the first slices and compare them with the current slices
                      while( searchSpacingSecond->imageNumber - searchSpacingFirst->imageNumber != secondIter->imageNumber - firstIter->imageNumber && Equal( searchSpacingSecond->origin, timeSliceReference->origin ) )

//  && find( (*searchSpacingSecond).sliceUsedWithSpacing.begin(), (*searchSpacingSecond).sliceUsedWithSpacing.end(), masterSpacing ) != (*searchSpacingSecond).sliceUsedWithSpacing.end()

                      {
                        searchSpacingSecond++;
                      }
                      //if the image-number-spacing is equal, we add the slice to the combination
                      if( searchSpacingSecond->imageNumber - searchSpacingFirst->imageNumber == secondIter->imageNumber - firstIter->imageNumber && Equal( searchSpacingSecond->origin, timeSliceReference->origin ) )
                      {

// && find( (*searchSpacingSecond).sliceUsedWithSpacing.begin(), (*searchSpacingSecond).sliceUsedWithSpacing.end(), referenceSpacing ) == (*searchSpacingSecond).sliceUsedWithSpacing.end()

                        newCombination.insert( &(*searchSpacingSecond) );
                        (*searchSpacingSecond).sliceUsedWithSpacing.insert( referenceSpacing );
                        searchSpacingFirst = searchSpacingSecond;
                      }
                      else
                      //else no matching slice found
                        searchIterSecondAtEnd = true;
                    }
                    else
                    {
                      //if the origin or the current slice are not timesliced but the slice bevor, we have to attend the image number yet
                      //example ( 50, 49, 48, 47, 46 ) and ( 7, 6, 5, 4, 3 ) the slice with the number 7 have the same position like 46 and the following spacing is the same
                      //so the result without this filter would be ( 50, 49, 48, 47, 46, 6, 5, 4, 3 )
                      //if the origin is not timesliced, but the current slice, the imagenumber next to the last one get searched
                      //if there is no imagenumber, the volume ends
                      if( sliceBevorWasTimeSliced && searchSpacingSecond->imageNumber - searchSpacingFirst->imageNumber != secondIter->imageNumber - firstIter->imageNumber )

//|| find( (*searchSpacingSecond).sliceUsedWithSpacing.begin(), (*searchSpacingSecond).sliceUsedWithSpacing.end(), referenceSpacing ) != (*searchSpacingSecond).sliceUsedWithSpacing.end() )

                      {
                        searchIterSecondAtEnd = true;
                      }
                      else
                      //if nothing is timesliced, we can add the found slice to the combination
                      {
                        newCombination.insert( &(*searchSpacingSecond) );
                        (*searchSpacingSecond).sliceUsedWithSpacing.insert( referenceSpacing );
                        searchSpacingFirst = searchSpacingSecond;
                      }
                    }
                  }
                }
              }
              //definition: a volume have three or more slices
              if( newCombination.size() > 2 )
                groupList[x].possibleCombinations.push_back( newCombination );
            }
            //all following spacings counted, now we increase the second iter, create a new spacing and searching for the following one again
            secondIter++;
             //until we reached the end of the included slices
             if( secondIter == groupList[x].includedSlices.end() )
              secondIterAtEnd = true;
            else
            {
              tempDistance = secondIter->origin - firstIter->origin;
              masterSpacing =  Round( tempDistance.GetNorm(), 2 );
            }
          }
        }
        //now we searched all possible spacings between the first and second iter until the second one reached the end of the included slices, so we increase the first one and searching from there for all possible spacings
        firstIter++;
        std::vector< Slice >::iterator firstIterNext = firstIter;
        //until we reached the end - 1 of the included slices
        if( firstIterNext == groupList[x].includedSlices.end() )
          firstIterAtEnd = true;
        secondIter = firstIter;
      }
    }
    //we add no two-slice-volumes and one-slice-volumes to the possibleCombination, but its possible that a volume and a single slice create the result, so the single slices get added too
    for( std::vector< Slice >::iterator iter = groupList[x].includedSlices.begin(); iter != groupList[x].includedSlices.end(); iter++ )
    {
      std::set< Slice* > singleCombination;
      singleCombination.clear();
      singleCombination.insert( &(*iter) );
      groupList[x].possibleCombinations.push_back( singleCombination );
    }
  }
#endif
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
      std::vector< std::set< Slice* > >::iterator iterEnd = groupList[n].possibleCombinations.end();
      iterEnd--;
      //here the final result get saved
      groupList[n].resultCombinations.clear();

      //this variables used by RekCombinationSearch()
      iterGroupEnd = groupList[n].possibleCombinations.end();  // -> the end to search and combine
      totalCombinationCount = groupList[n].includedSlices.size();  // -> the count of all included slices (total set)
      groupResultCombinations.clear();  // -> all results with the minimum count of used combinations; changed while RekCombinationSearch(); at the end the results get saved to groupList[n].resultCombinations

      int count = 1;
      while( iterBegin != iterEnd )
      {
        //the possible combinations sorted, from combinations with lots of elements to combinations with less elements
        //if the mulitplication from the current minimal-needed-combination-count with the current combination-element-count is lower then the count of all needed slices, there is no possibility to create a result wich include all slices and we can break up
        if( groupResultCombinations.size() != 0 && ( ( ( (*iterBegin).size() ) * ( groupResultCombinations.front().size() ) ) < totalCombinationCount ) )
          break;

        //the current combination
        std::vector< std::set< Slice* > > resultCombination;
        resultCombination.clear();
        resultCombination.push_back( (*iterBegin) );

        //if the count is equal, the set's included the same elements and we have a result
        if( totalCombinationCount == (*iterBegin).size() )
        {
          if( groupResultCombinations.size() != 0 && groupResultCombinations.front().size() > 1 )
            groupResultCombinations.clear();
          if( groupResultCombinations.size() == 0 || groupResultCombinations.front().size() == 1 )
            groupResultCombinations.push_back( resultCombination );
        }
        else
        //else we have to search, combine, check, ...
          RekCombinationSearch( iterBegin, (*iterBegin), resultCombination );
        iterBegin++;
      }
      //the result get copied
      groupList[n].resultCombinations = groupResultCombinations;
    }
  }
#endif
}

void mitk::SpacingSetFilter::RekCombinationSearch( std::vector< std::set< Slice* > >::iterator iterBegin, std::set< Slice* > currentCombination, std::vector< std::set< Slice* > > resultCombination )
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  //to found a combination which represent the whole, the single combinations get combined
  //the intersection created and if they empty, the two combinations combined to one
  //if the intersection not empty the two combinations included the same elements
  //the new combination get checked with the next combination and combined until they represented the whole

  iterBegin++;
  while ( iterBegin != iterGroupEnd )
  {
    //if we have found a result, we should not combine more combinationens than the result included
    if( groupResultCombinations.size() != 0 && resultCombination.size() >= groupResultCombinations.front().size() )
      return;

    //the current result can integrated two more combinations bevor the size is equal to the number of combinations from the found result
    //the current combination count have 5 elements less than the total combination
    //the current found combinations have only one element, they are sorted, so they dont get more
    //two more integrations with combinations with one element can not fill the lack of 5 elements, so we can break up
    if( groupResultCombinations.size() != 0 && ( ( ( groupResultCombinations.front().size() - resultCombination.size() ) * iterBegin->size() ) + currentCombination.size() ) < totalCombinationCount )
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
      //resultCombination contain the single combinations of the current tempUnion
      resultCombination.push_back( (*iterBegin) );

      //to check if the tempUnion contain the whole slices, we use the size; the union is a std::set which not contain equal elements
      if( totalCombinationCount == tempUnion.size() )
      {
        if( groupResultCombinations.size() != 0 && groupResultCombinations.front().size() > resultCombination.size() )
          groupResultCombinations.clear();
        if( groupResultCombinations.size() == 0 || groupResultCombinations.front().size() == resultCombination.size() )
          groupResultCombinations.push_back( resultCombination );
      }
      else
      //if the current union not contain all elements we have to continue the search
        RekCombinationSearch( iterBegin, tempUnion, resultCombination );
      //for the recursive degression we have to delete the add combination from the possible result
      resultCombination.pop_back();
    }
    iterBegin++;
  }
#endif
}

void mitk::SpacingSetFilter::GenerateNodes()
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  for( unsigned int n = 0; n < groupList.size(); n++)
  {
    //TODO groupList[n].resultCombinations.size() > 1

    if( groupList[n].dimension > 2 )
    {
//TODO erzeugung des mitk::images funktioniert so nicht, da die pics aus chili nicht das tag isg haben, sondern die isg von chili; davon weiß image nichts, deswegen müssen die geomtrien selber initialisiert werden und damit dann das mitk::image
      Image::Pointer resultImage;
      resultImage = Image::New();
      resultImage->Initialize( groupList[n].includedSlices.front().currentPic );
      int count = 0;
      for( std::vector< Slice >::iterator iterator = groupList[n].includedSlices.begin(); iterator != groupList[n].includedSlices.end(); iterator++ )
      {
        resultImage->SetPicVolume( iterator->currentPic ,count );
        count ++;
      }

      if( resultImage->IsInitialized() && resultImage.IsNotNull() )
      {
        DataTreeNode::Pointer node = mitk::DataTreeNode::New();
        node->SetData( resultImage );
        DataTreeNodeFactory::SetDefaultImageProperties( node );

        if( groupList[n].seriesDescription == "" )
          groupList[n].seriesDescription = "no SeriesDescription";
        node->SetProperty( "name", new StringProperty( groupList[n].seriesDescription ) );
        node->SetProperty( "NumberOfSlices", new IntProperty( groupList[n].includedSlices.front().currentPic->n[2] ) );
        if( m_SeriesOID != "" )
          node->SetProperty( "SeriesOID", new StringProperty( m_SeriesOID ) );

        mitk::PropertyList::Pointer tempPropertyList = CreatePropertyListFromPicTags( groupList[n].includedSlices.front().currentPic );
        for( mitk::PropertyList::PropertyMap::const_iterator iter = tempPropertyList->GetMap()->begin(); iter != tempPropertyList->GetMap()->end(); iter++ )
        {
          node->SetProperty( iter->first.c_str(), iter->second.first );
        }
        m_Output.push_back( node );
      }
    }
    else
    {
      //for 2D-slices a real spacing is needed, because if a volume have a spacing 5 and the 2D-slice with 0.01 mitk create a lots of slices wich not used / needed, therefore we have to initialize the 2D-slice with the same spacing like the volume
      Vector3D maxSpacing;
      maxSpacing[2] = 0;
      for( std::vector < std::set< Slice* > >::iterator iter = groupList[n].resultCombinations.front().begin(); iter != groupList[n].resultCombinations.front().end(); iter ++ )
      {
        if( (*iter).size() > 1 )
        {
          std::set< Slice* >::iterator nextIter = iter->begin();
          nextIter++;
          Vector3D tempSpacing = (*nextIter)->origin - (*iter->begin())->origin;
          tempSpacing[2] = tempSpacing.GetNorm();
          tempSpacing[2] = Round( tempSpacing[2], 2 );
          if( tempSpacing[2] > maxSpacing[2] )
            maxSpacing = tempSpacing;
        }
     }

      //now we know the included maxspacing and can create all possible outputs
      for( std::vector < std::set< Slice* > >::iterator iter = groupList[n].resultCombinations.front().begin(); iter != groupList[n].resultCombinations.front().end(); iter ++ )
      {
        //create mitk::Image
        Vector3D rightVector, downVector, spacing;
        Point3D origin;
        ipPicDescriptor* header;
        Image::Pointer resultImage;

        header = ipPicCopyHeader( (*iter->begin())->currentPic, NULL );

        if( (*iter).size() == 1 )
        //2D
        {
          header->dim = 2;
          header->n[2] = 0;
          header->n[3] = 0;
        }
        else
        //3D
        {
          header->dim = 3;
          header->n[2] = iter->size();
          header->n[3] = 0;
        }

        resultImage = Image::New();
        resultImage->Initialize( header, 1, -1, iter->size() );

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
        if( spacing[0] == 0 && spacing[1] == 0 && spacing[2] == 0 )
          spacing.Fill(1.0);
        else
        {
          for (unsigned int i = 0; i < 3; ++i)
            spacing[i] = Round( spacing[i], 2 );
        }

        if( (*iter).size() > 1 )
        {
          std::set< Slice* >::iterator nextIter = iter->begin();
          nextIter++;
          Vector3D tempSpacing = (*nextIter)->origin - (*iter->begin())->origin;
          spacing[2] = tempSpacing.GetNorm();
          spacing[2] = Round( spacing[2], 2 );
        }
        else
        {
          spacing[2] = maxSpacing[2];
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
        int count = 0;
        for( std::set< Slice* >::iterator it = (*iter).begin(); it != (*iter).end(); it++)
        {
          resultImage->SetPicSlice( (*it)->currentPic, count, 0 );
          count ++;
        }

        if( resultImage->IsInitialized() && resultImage.IsNotNull() )
        {
          DataTreeNode::Pointer node = mitk::DataTreeNode::New();
          node->SetData( resultImage );
          DataTreeNodeFactory::SetDefaultImageProperties( node );

          if( groupList[n].seriesDescription == "" )
            groupList[n].seriesDescription = "no SeriesDescription";
          node->SetProperty( "name", new StringProperty( groupList[n].seriesDescription ) );
          node->SetProperty( "NumberOfSlices", new IntProperty( (*iter).size() ) );
          if( m_SeriesOID != "" )
            node->SetProperty( "SeriesOID", new StringProperty( m_SeriesOID ) );

          mitk::PropertyList::Pointer tempPropertyList = CreatePropertyListFromPicTags( (*iter->begin())->currentPic );
          for( mitk::PropertyList::PropertyMap::const_iterator iter = tempPropertyList->GetMap()->begin(); iter != tempPropertyList->GetMap()->end(); iter++ )
          {
            node->SetProperty( iter->first.c_str(), iter->second.first );
          }

          m_Output.push_back( node );
        }
        delete isg;
      }
    }
  }
#endif
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
    std::cout<<"Output"<<n<<": with "<<groupList[n].possibleCombinations.size()<<" combinations"<<std::endl;
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

