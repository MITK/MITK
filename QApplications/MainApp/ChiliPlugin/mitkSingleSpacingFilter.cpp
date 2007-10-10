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

// the "main"-function
void mitk::SingleSpacingFilter::Update()
{
  m_Output.clear();
  groupList.clear();

  if( m_PicDescriptorList.size() > 0 && m_SeriesOID != "" )
  {
    SortSlicesToGroup();
    SortGroupsByLocation();
    //ShowAllGroupsWithSlices();
    CreateSpacings();
    //ShowAllFoundSpacings();
    GenerateNodes();
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
      delete isg;
      continue;
    }

    //new Slice
    Slice newSlice;
    //set PicDescriptor
    newSlice.currentPic = (*currentPic);
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

    //dimension
    int currentDimension = (*currentPic)->dim;
    if( currentDimension < 2 || currentDimension > 4 )
    {
      std::cout<<"SingleSpacingFilter-WARNING: Wrong PicDescriptor-Dimension. Image ignored."<<std::endl;
      delete isg;
      continue;
    }

    //search and set the slice to group, therefore some attributes have to generated

    //SliceOrigin
    Vector3D sliceOrigin;
    vtk2itk( isg->o, sliceOrigin );
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
    //normale with ImageSize
    Vector3D rightVector, downVector;
    vtk2itk( isg->u, rightVector );
    vtk2itk( isg->v, downVector );
    rightVector = rightVector * newSlice.currentPic->n[0];
    downVector = downVector * newSlice.currentPic->n[1];
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
      maxCount = groupList.size();
    }

    // searching for equal output
    while( curCount < maxCount && !foundMatch )
    {
      //checking referenceUID, seriesDescription, pixelSize and NormaleWithSize
      if( groupList[ curCount ].referenceUID == isg->forUID && groupList[ curCount ].seriesDescription == currentSeriesDescription && groupList[ curCount ].pixelSize == currentPixelSize && groupList[ curCount ].normalWithImageSize == currentNormalWithImageSize )
      {
        if( groupList[ curCount ].dimension == 2 || ( groupList[ curCount ].dimension == 3 && groupList[ curCount ].includedPositions.front().origin == sliceOrigin && groupList[ curCount ].includedPositions.front().includedSlices.front().currentPic->n[2] == (*currentPic)->n[2] ) )
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
      int maxPosCount = groupList[ curCount ].includedPositions.size();
      while( curPosCount < maxPosCount && !foundPositionMatch )
      {
        if( groupList[ curCount ].includedPositions[curPosCount].origin == sliceOrigin )
          foundPositionMatch = true;
        else
          curPosCount++;
      }
      //PositionAtSpace always exist
      if( foundPositionMatch )
        groupList[ curCount ].includedPositions[curPosCount].includedSlices.push_back( newSlice );
      else
      //dont exist yet
      {
        PositionAtSpace newPosition;
        newPosition.origin = sliceOrigin;
        Vector3D rightVector, downVector;
        vtk2itk( isg->u, rightVector );
        vtk2itk( isg->v, downVector );
        newPosition.normal[0] = Round( ( ( rightVector[1]*downVector[2] ) - ( rightVector[2]*downVector[1] ) ), 2 );
        newPosition.normal[1] = Round( ( ( rightVector[2]*downVector[0] ) - ( rightVector[0]*downVector[2] ) ), 2);
        newPosition.normal[2] = Round( ( ( rightVector[0]*downVector[1] ) - ( rightVector[1]*downVector[0] ) ), 2);
        newPosition.includedSlices.push_back( newSlice );
        groupList[ curCount ].includedPositions.push_back( newPosition );
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
      newGroup.foundSpacings.clear();

      PositionAtSpace newPosition;
      newPosition.origin = sliceOrigin;
      Vector3D rightVector, downVector;
      vtk2itk( isg->u, rightVector );
      vtk2itk( isg->v, downVector );
      newPosition.normal[0] = Round( ( ( rightVector[1]*downVector[2] ) - ( rightVector[2]*downVector[1] ) ), 2 );
      newPosition.normal[1] = Round( ( ( rightVector[2]*downVector[0] ) - ( rightVector[0]*downVector[2] ) ), 2);
      newPosition.normal[2] = Round( ( ( rightVector[0]*downVector[1] ) - ( rightVector[1]*downVector[0] ) ), 2);
      newPosition.includedSlices.push_back( newSlice );
      newGroup.includedPositions.push_back( newPosition );

       groupList.push_back( newGroup );
    }
  delete isg;
  }
#endif
}

void mitk::SingleSpacingFilter::SortGroupsByLocation()
{
  //sort the PositionAtSpace by there location
  for( std::vector< Group >::iterator iter = groupList.begin(); iter != groupList.end(); iter++ )
  {
    std::sort( (*iter).includedPositions.begin(), (*iter).includedPositions.end(), LocationSort );
  }
}

bool mitk::SingleSpacingFilter::LocationSort( const PositionAtSpace elem1, const PositionAtSpace elem2 )
{
  //helpfunction to sort
  return elem1.origin*elem1.normal < elem2.origin*elem2.normal; // projection of origin on inter-slice-direction
}

void mitk::SingleSpacingFilter::CreateSpacings()
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  for( unsigned int n = 0; n < groupList.size(); n++)
  {
    //get spacing
    interSliceGeometry_t* isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
    pFetchSliceGeometryFromPic( groupList[n].includedPositions.front().includedSlices.front().currentPic, isg );

    Vector3D currentSpacing;
    vtk2itk( isg->ps, currentSpacing );
    delete isg;
    if( currentSpacing[0] == 0 && currentSpacing[1] == 0 && currentSpacing[2] == 0 )
      currentSpacing.Fill(1.0);
    for (unsigned int i = 0; i < 3; ++i)
      currentSpacing[i] = Round( currentSpacing[i], 2 );

    std::vector< PositionAtSpace >::iterator root = groupList[n].includedPositions.begin();
    std::vector< PositionAtSpace >::iterator rootEnd = groupList[n].includedPositions.end();
    rootEnd--;
    std::vector< PositionAtSpace >::iterator walk;

    while( root != rootEnd )
    {
      walk = root;
      walk++;
      while( walk != groupList[n].includedPositions.end() )
      {
        //create Spacing[2]
        Vector3D tempSpacing = (*walk).origin - (*root).origin;
        currentSpacing[2] = tempSpacing.GetNorm();
        currentSpacing[2] = Round( currentSpacing[2], 2 );

        //search if spacing already exist
        bool foundMatch = false;
        int curCount = 0;
        int maxCount = groupList[n].foundSpacings.size();
        while ( curCount < maxCount && !foundMatch )
        {
          if( groupList[n].foundSpacings[ curCount ].spacing == currentSpacing )
            foundMatch = true;
          else
            curCount++;
        }

        if( foundMatch )
          //spacing exist, increase count
          groupList[n].foundSpacings[ curCount ].count++;
        else
        {
          //spacing dont exist, create new
          Spacing newSpacing;
          newSpacing.count = 1;
          newSpacing.spacing = currentSpacing;
          groupList[n].foundSpacings.push_back( newSpacing );
        }
        walk++;
      }
      root++;
    }
  }
#endif
}

bool mitk::SingleSpacingFilter::SpacingCountSort( const Spacing elem1, const Spacing elem2 )
{
  //helpfunction to sort the found spacings, the higher one get first
  return elem1.count > elem2.count;
}

bool mitk::SingleSpacingFilter::ImageNumberSort( const Slice elem1, const Slice elem2 )
{
  //helpfunction to sort the slices in the PositionAtSpace, the smaler imagenumber get first
  return elem1.imageNumber < elem2.imageNumber;
}

void mitk::SingleSpacingFilter::GenerateNodes()
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  for( unsigned int n = 0; n < groupList.size(); n++)
  {
    //sort spacings by size
    if( groupList[n].foundSpacings.size() != 0 )
      std::sort( groupList[n].foundSpacings.begin(), groupList[n].foundSpacings.end(), SpacingCountSort );
    //sort PositionAtSpace by imageNumber
    for( std::vector< PositionAtSpace >::iterator iter = groupList[n].includedPositions.begin(); iter != groupList[n].includedPositions.end(); iter++ )
      std::sort( (*iter).includedSlices.begin(), (*iter).includedSlices.end(), ImageNumberSort );

    std::vector< PositionAtSpace > copyToWork = groupList[n].includedPositions;
    std::vector< PositionAtSpace > result;

    while( !copyToWork.empty() )
    {
      result.clear();
      unsigned int min_t = 0;

      if( groupList[n].dimension > 2 )
      {
        if( groupList[n].dimension == 3 )
          min_t = copyToWork.front().includedSlices.size();
        else
          min_t = copyToWork.front().includedSlices.front().currentPic->n[3];
        result = copyToWork;
        copyToWork.clear();
      }
      else
      {
        if( copyToWork.size() == 1 )
        {
          min_t = copyToWork.front().includedSlices.size();
          result.push_back( copyToWork.front() );
          copyToWork.clear();
        }
        else
        {
          //now use first spacing to create an output
          std::vector< PositionAtSpace >::iterator iterOne = copyToWork.begin();
          std::vector< PositionAtSpace >::iterator iterTwo = iterOne;

          bool stop = false;
          Vector3D tempDistance;
          double referenceSpacing;

          while( !stop )
          {
            tempDistance = (*iterTwo).origin - (*iterOne).origin;
            referenceSpacing = Round( tempDistance.GetNorm(), 2 );
            while( referenceSpacing < groupList[n].foundSpacings.front().spacing[2] && iterTwo != copyToWork.end() )
            {
              iterTwo++;
              tempDistance = (*iterTwo).origin - (*iterOne).origin;
              referenceSpacing = Round( tempDistance.GetNorm(), 2 );
            }

            if( referenceSpacing == groupList[n].foundSpacings.front().spacing[2] && iterTwo != copyToWork.end() )
            {
              result.push_back( (*iterOne) );
              if( min_t == 0 || (*iterOne).includedSlices.size() < min_t )
                min_t = (*iterOne).includedSlices.size();
              copyToWork.erase( iterOne );
              iterTwo--;
              iterOne = iterTwo;
            }
            else
            {
              stop = true;
              result.push_back( (*iterOne) );
              copyToWork.erase( iterOne );
              if( min_t == 0 || (*iterOne).includedSlices.size() < min_t )
                min_t = (*iterOne).includedSlices.size();
            }
          }
        }
      }

      //create mitk::Image
      Vector3D rightVector, downVector, spacing;
      Point3D origin;
      ipPicDescriptor* header;
      Image::Pointer resultImage;

      header = ipPicCopyHeader( result.front().includedSlices.front().currentPic, NULL );

      if( result.size() == 1 && groupList[n].dimension == 2 )
      {
        //2D
        if( min_t == 1 )
        {
          header->dim = 2;
          header->n[2] = 0;
          header->n[3] = 0;
        }
        //2D + t
        else
        {
          header->dim = 4;
          header->n[2] = 1;
          header->n[3] = min_t;
        }
      }
      else
      {
        //3D
        if( groupList[n].dimension == 2 )
          header->n[2] = result.size();
        else
          header->n[2] = result.front().includedSlices.front().currentPic->n[2];

        if( min_t == 1 )
        {
          header->dim = 3;
          header->n[3] = 0;
        }
        //3D + t
        else
        {
          header->dim = 4;
          header->n[3] = min_t;
        }
      }

      resultImage = Image::New();
      resultImage->Initialize( header );

      interSliceGeometry_t* isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
      pFetchSliceGeometryFromPic( result.front().includedSlices.front().currentPic, isg );

      //rightVector, downVector, origin
      vtk2itk( isg->u, rightVector );
      vtk2itk( isg->v, downVector );
      vtk2itk( isg->o, origin );

      // its possible that a 2D-Image have no right- or down-Vector, but its not possible to initialize a [0,0,0] vector
      if( rightVector[0] == 0 && rightVector[1] == 0 && rightVector[2] == 0 )
        rightVector[0] = 1;
      if( downVector[0] == 0 && downVector[1] == 0 && downVector[2] == 0 )
        downVector[2] = -1;

      // set the timeBounds
      ScalarType timeBounds[] = {0.0, 1.0};
      // set the planeGeomtry
      PlaneGeometry::Pointer planegeometry = PlaneGeometry::New();

      if( result.size() > 1 && groupList[n].dimension == 2 )
      {
        planegeometry->InitializeStandardPlane( resultImage->GetDimension(0), resultImage->GetDimension(1), rightVector, downVector, &groupList[n].foundSpacings.front().spacing );
      }
      else
      {
        Vector3D spacing;
        vtk2itk( isg->ps, spacing );
        if( spacing[0] == 0 && spacing[1] == 0 && spacing[2] == 0 )
          spacing.Fill(1.0);
        for (unsigned int i = 0; i < 3; ++i)
          spacing[i] = Round( spacing[i], 2 );
        planegeometry->InitializeStandardPlane( resultImage->GetDimension(0), resultImage->GetDimension(1), rightVector, downVector, &spacing );
      }

      delete isg;

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
      for( unsigned int x = 0; x < result.size(); x++ )
      {
        for( unsigned int t = 0; t < min_t; t++ )
        {
          if( groupList[n].dimension == 3 )
            resultImage->SetPicVolume( result[x].includedSlices[t].currentPic, t );
          else
            resultImage->SetPicSlice( result[x].includedSlices[t].currentPic, x, t );
        }
      }

      if( resultImage->IsInitialized() && resultImage.IsNotNull() )
      {
        DataTreeNode::Pointer node = mitk::DataTreeNode::New();
        node->SetData( resultImage );
        DataTreeNodeFactory::SetDefaultImageProperties( node );

        if( groupList[n].seriesDescription == "" )
          groupList[n].seriesDescription = "no SeriesDescription";
        node->SetProperty( "name", new StringProperty( groupList[n].seriesDescription ) );
        node->SetProperty( "NumberOfSlices", new IntProperty( result.size() ) );
        node->SetProperty( "NumberOfTimeSlices", new IntProperty( min_t ) );
        if( m_SeriesOID != "" )
          node->SetProperty( "SeriesOID", new StringProperty( m_SeriesOID ) );

        mitk::PropertyList::Pointer tempPropertyList = CreatePropertyListFromPicTags( result.front().includedSlices.front().currentPic );
        for( mitk::PropertyList::PropertyMap::const_iterator iter = tempPropertyList->GetMap()->begin(); iter != tempPropertyList->GetMap()->end(); iter++ )
        {
          node->SetProperty( iter->first.c_str(), iter->second.first );
        }

        m_Output.push_back( node );
      }
    }
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

void mitk::SingleSpacingFilter::ShowAllGroupsWithSlices()
{
  for( unsigned int n = 0; n < groupList.size(); n++)
  {
    std::cout << "-----------" << std::endl;
    std::cout << "SingleSpacingFilter-Output" << n << std::endl;
    std::cout << "ReferenceUID:" << groupList[n].referenceUID << std::endl;
    std::cout << "SeriesDescription:" << groupList[n].seriesDescription << std::endl;
    std::cout << "PixelSize:" << groupList[n].pixelSize << std::endl;
    std::cout << "Normal with image size:" << groupList[n].normalWithImageSize << std::endl;
    std::cout << "-----------" << std::endl;

    for( std::vector< PositionAtSpace >::iterator it = groupList[n].includedPositions.begin(); it != groupList[n].includedPositions.end(); it++ )
    {
      std::cout<<"PositionAtSpace with Origin: "<<(*it).origin<<" and Normal: "<<(*it).normal<<std::endl;
      for( std::vector< Slice >::iterator iter = (*it).includedSlices.begin(); iter != (*it).includedSlices.end(); iter++ )
      {
        std::cout<<"include Slice "<<(*iter).imageNumber<<std::endl;
      }
    }
  }
}

void mitk::SingleSpacingFilter::ShowAllFoundSpacings()
{
  for( unsigned int n = 0; n < groupList.size(); n++)
  {
    std::cout << "-----------" << std::endl;
    std::cout << "SingleSpacingFilter-Output" << n << std::endl;
    std::cout << "-----------" << std::endl;

    for( std::vector< Spacing >::iterator it = groupList[n].foundSpacings.begin(); it != groupList[n].foundSpacings.end(); it++ )
    {
      std::cout<<"Spacing: "<<(*it).spacing<<" Count: "<<(*it).count<<std::endl;
    }
  }
}

