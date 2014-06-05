/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkConnectomicsNetworkCreator.h"

#include <sstream>
#include <vector>

#include "mitkConnectomicsConstantsManager.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageStatisticsHolder.h"
#include "mitkImageCast.h"

#include "itkImageRegionIteratorWithIndex.h"

// VTK
#include <vtkPolyData.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>

mitk::ConnectomicsNetworkCreator::ConnectomicsNetworkCreator()
: m_FiberBundle()
, m_Segmentation()
, m_ConNetwork( mitk::ConnectomicsNetwork::New() )
, idCounter(0)
, m_LabelToVertexMap()
, m_LabelToNodePropertyMap()
, allowLoops( false )
, m_UseCoMCoordinates( false )
, m_LabelsToCoordinatesMap()
, m_MappingStrategy( EndElementPositionAvoidingWhiteMatter )
, m_EndPointSearchRadius( 10.0 )
, m_ZeroLabelInvalid( true )
, m_AbortConnection( false )
{
}

mitk::ConnectomicsNetworkCreator::ConnectomicsNetworkCreator( mitk::Image::Pointer segmentation, mitk::FiberBundleX::Pointer fiberBundle )
: m_FiberBundle(fiberBundle)
, m_Segmentation(segmentation)
, m_ConNetwork( mitk::ConnectomicsNetwork::New() )
, idCounter(0)
, m_LabelToVertexMap()
, m_LabelToNodePropertyMap()
, allowLoops( false )
, m_LabelsToCoordinatesMap()
, m_MappingStrategy( EndElementPositionAvoidingWhiteMatter )
, m_EndPointSearchRadius( 10.0 )
, m_ZeroLabelInvalid( true )
, m_AbortConnection( false )
{
  mitk::CastToItkImage( segmentation, m_SegmentationItk );
}

mitk::ConnectomicsNetworkCreator::~ConnectomicsNetworkCreator()
{
}

void mitk::ConnectomicsNetworkCreator::SetFiberBundle(mitk::FiberBundleX::Pointer fiberBundle)
{
  m_FiberBundle = fiberBundle;
}

void mitk::ConnectomicsNetworkCreator::SetSegmentation(mitk::Image::Pointer segmentation)
{
  m_Segmentation = segmentation;
  mitk::CastToItkImage( segmentation, m_SegmentationItk );
}

itk::Point<float, 3> mitk::ConnectomicsNetworkCreator::GetItkPoint(double point[3])
{
  itk::Point<float, 3> itkPoint;
  itkPoint[0] = point[0];
  itkPoint[1] = point[1];
  itkPoint[2] = point[2];
  return itkPoint;
}

void mitk::ConnectomicsNetworkCreator::CreateNetworkFromFibersAndSegmentation()
{

  //empty graph
  m_ConNetwork = mitk::ConnectomicsNetwork::New();
  m_LabelToVertexMap.clear();
  m_LabelToNodePropertyMap.clear();
  idCounter = 0;

  vtkSmartPointer<vtkPolyData> fiberPolyData = m_FiberBundle->GetFiberPolyData();
  vtkSmartPointer<vtkCellArray> vLines = fiberPolyData->GetLines();
  vLines->InitTraversal();

  int numFibers = m_FiberBundle->GetNumFibers();
  for( int fiberID( 0 ); fiberID < numFibers; fiberID++ )
  {
    vtkIdType   numPointsInCell(0);
    vtkIdType*  pointsInCell(NULL);
    vLines->GetNextCell ( numPointsInCell, pointsInCell );

    TractType::Pointer singleTract = TractType::New();
    for( int pointInCellID( 0 ); pointInCellID < numPointsInCell ; pointInCellID++)
    {
      // push back point
      PointType point = GetItkPoint( fiberPolyData->GetPoint( pointsInCell[ pointInCellID ] ) );
      singleTract->InsertElement( singleTract->Size(), point );
    }

    if ( singleTract && ( singleTract->Size() > 0 ) )
    {
      AddConnectionToNetwork(
        ReturnAssociatedVertexPairForLabelPair(
        ReturnLabelForFiberTract( singleTract, m_MappingStrategy )
        )
        );
      m_AbortConnection = false;
    }
  }

  // Prune unconnected nodes
  //m_ConNetwork->PruneUnconnectedSingleNodes();

  // provide network with geometry
  m_ConNetwork->SetGeometry( dynamic_cast<mitk::BaseGeometry*>(m_Segmentation->GetGeometry()->Clone().GetPointer()) );
  m_ConNetwork->UpdateBounds();
  m_ConNetwork->SetIsModified( true );

  MBI_INFO << mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_INFO_NETWORK_CREATED;
}

void mitk::ConnectomicsNetworkCreator::AddConnectionToNetwork(ConnectionType newConnection)
{
  if( m_AbortConnection )
  {
    MITK_DEBUG << "Connection aborted";
    return;
  }

  VertexType vertexA = newConnection.first;
  VertexType vertexB = newConnection.second;

  // check for loops (if they are not allowed)
  if( allowLoops || !( vertexA == vertexB ) )
  {
    // If the connection already exists, increment weight, else create connection
    if ( m_ConNetwork->EdgeExists( vertexA, vertexB ) )
    {
      m_ConNetwork->IncreaseEdgeWeight( vertexA, vertexB );
    }
    else
    {
      m_ConNetwork->AddEdge( vertexA, vertexB );
    }
  }
}


mitk::ConnectomicsNetworkCreator::VertexType mitk::ConnectomicsNetworkCreator::ReturnAssociatedVertexForLabel( ImageLabelType label )
{
  if( m_ZeroLabelInvalid && ( label == 0 ) )
  {
    m_AbortConnection = true;
    return ULONG_MAX;
  }

  // if label is not known, create entry
  if( ! ( m_LabelToVertexMap.count( label ) > 0 ) )
  {
    VertexType newVertex = m_ConNetwork->AddVertex( idCounter );
    idCounter++;
    SupplyVertexWithInformation(label, newVertex);
    m_LabelToVertexMap.insert( std::pair< ImageLabelType, VertexType >( label, newVertex ) );
  }

  //return associated vertex
  return m_LabelToVertexMap.find( label )->second;
}

mitk::ConnectomicsNetworkCreator::ConnectionType mitk::ConnectomicsNetworkCreator::ReturnAssociatedVertexPairForLabelPair( ImageLabelPairType labelpair )
{
  //hand both labels through to the single label function
  ConnectionType connection( ReturnAssociatedVertexForLabel(labelpair.first), ReturnAssociatedVertexForLabel(labelpair.second) );

  return connection;
}

mitk::ConnectomicsNetworkCreator::ImageLabelPairType mitk::ConnectomicsNetworkCreator::ReturnLabelForFiberTract( TractType::Pointer singleTract, mitk::ConnectomicsNetworkCreator::MappingStrategy strategy)
{
  switch( strategy )
  {
  case EndElementPosition:
    {
      return EndElementPositionLabel( singleTract );
    }
  case JustEndPointVerticesNoLabel:
    {
      return JustEndPointVerticesNoLabelTest( singleTract );
    }
  case EndElementPositionAvoidingWhiteMatter:
    {
      return EndElementPositionLabelAvoidingWhiteMatter( singleTract );
    }
  case PrecomputeAndDistance:
    {
      return PrecomputeVertexLocationsBySegmentation( singleTract );
    }
  }

  // To remove warnings, this code should never be reached
  MBI_ERROR << mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_INVALID_MAPPING;
  ImageLabelPairType nullPair( 0,0 );
  return nullPair;
}

mitk::ConnectomicsNetworkCreator::ImageLabelPairType mitk::ConnectomicsNetworkCreator::EndElementPositionLabel( TractType::Pointer singleTract )
{
  ImageLabelPairType labelpair;

  {// Note: .fib image tracts are safed using index coordinates
    mitk::Point3D firstElementFiberCoord, lastElementFiberCoord;
    mitk::Point3D firstElementSegCoord, lastElementSegCoord;
    itk::Index<3> firstElementSegIndex, lastElementSegIndex;

    if( singleTract->front().Size() != 3 )
    {
      MBI_ERROR << mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_INVALID_DIMENSION_NEED_3;
    }
    for( unsigned int index = 0; index < singleTract->front().Size(); index++ )
    {
      firstElementFiberCoord.SetElement( index, singleTract->front().GetElement( index ) );
      lastElementFiberCoord.SetElement( index, singleTract->back().GetElement( index ) );
    }

    // convert from fiber index coordinates to segmentation index coordinates
    FiberToSegmentationCoords( firstElementFiberCoord, firstElementSegCoord );
    FiberToSegmentationCoords( lastElementFiberCoord, lastElementSegCoord );

    for( int index = 0; index < 3; index++ )
    {
      firstElementSegIndex.SetElement( index, firstElementSegCoord.GetElement( index ) );
      lastElementSegIndex.SetElement( index, lastElementSegCoord.GetElement( index ) );
    }

    int firstLabel = m_SegmentationItk->GetPixel(firstElementSegIndex);
    int lastLabel = m_SegmentationItk->GetPixel(lastElementSegIndex );

    labelpair.first = firstLabel;
    labelpair.second = lastLabel;

    // Add property to property map
    CreateNewNode( firstLabel, firstElementSegIndex, m_UseCoMCoordinates );
    CreateNewNode( lastLabel, lastElementSegIndex, m_UseCoMCoordinates );
  }

  return labelpair;
}

mitk::ConnectomicsNetworkCreator::ImageLabelPairType mitk::ConnectomicsNetworkCreator::PrecomputeVertexLocationsBySegmentation( TractType::Pointer /*singleTract*/ )
{
  ImageLabelPairType labelpair;

  return labelpair;
}

mitk::ConnectomicsNetworkCreator::ImageLabelPairType mitk::ConnectomicsNetworkCreator::EndElementPositionLabelAvoidingWhiteMatter( TractType::Pointer singleTract )
{
  ImageLabelPairType labelpair;

  {// Note: .fib image tracts are safed using index coordinates
    mitk::Point3D firstElementFiberCoord, lastElementFiberCoord;
    mitk::Point3D firstElementSegCoord, lastElementSegCoord;
    itk::Index<3> firstElementSegIndex, lastElementSegIndex;

    if( singleTract->front().Size() != 3 )
    {
      MBI_ERROR << mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_INVALID_DIMENSION_NEED_3;
    }
    for( unsigned int index = 0; index < singleTract->front().Size(); index++ )
    {
      firstElementFiberCoord.SetElement( index, singleTract->front().GetElement( index ) );
      lastElementFiberCoord.SetElement( index, singleTract->back().GetElement( index ) );
    }

    // convert from fiber index coordinates to segmentation index coordinates
    FiberToSegmentationCoords( firstElementFiberCoord, firstElementSegCoord );
    FiberToSegmentationCoords( lastElementFiberCoord, lastElementSegCoord );

    for( int index = 0; index < 3; index++ )
    {
      firstElementSegIndex.SetElement( index, firstElementSegCoord.GetElement( index ) );
      lastElementSegIndex.SetElement( index, lastElementSegCoord.GetElement( index ) );
    }

    int firstLabel = m_SegmentationItk->GetPixel(firstElementSegIndex);
    int lastLabel = m_SegmentationItk->GetPixel(lastElementSegIndex );

    // Check whether the labels belong to the white matter (which means, that the fibers ended early)
    bool extendFront(false), extendEnd(false), retractFront(false), retractEnd(false);
    extendFront = !IsNonWhiteMatterLabel( firstLabel );
    extendEnd = !IsNonWhiteMatterLabel( lastLabel );
    retractFront = IsBackgroundLabel( firstLabel );
    retractEnd = IsBackgroundLabel( lastLabel );

    //if( extendFront || extendEnd )
    //{
    //MBI_INFO << "Before Start: " << firstLabel << " at " << firstElementSegIndex[ 0 ] << " " << firstElementSegIndex[ 1 ] << " " << firstElementSegIndex[ 2 ] << " End: " << lastLabel << " at " << lastElementSegIndex[ 0 ] << " " << lastElementSegIndex[ 1 ] << " " << lastElementSegIndex[ 2 ];
    //}
    if ( extendFront )
    {
      std::vector< int > indexVectorOfPointsToUse;

      //Use first two points for direction
      indexVectorOfPointsToUse.push_back( 1 );
      indexVectorOfPointsToUse.push_back( 0 );

      // label and coordinate temp storage
      int tempLabel( firstLabel );
      itk::Index<3> tempIndex = firstElementSegIndex;

      LinearExtensionUntilGreyMatter( indexVectorOfPointsToUse, singleTract, tempLabel, tempIndex );

      firstLabel = tempLabel;
      firstElementSegIndex = tempIndex;

    }

    if ( extendEnd )
    {
      std::vector< int > indexVectorOfPointsToUse;

      //Use last two points for direction
      indexVectorOfPointsToUse.push_back( singleTract->Size() - 2 );
      indexVectorOfPointsToUse.push_back( singleTract->Size() - 1 );

      // label and coordinate temp storage
      int tempLabel( lastLabel );
      itk::Index<3> tempIndex = lastElementSegIndex;

      LinearExtensionUntilGreyMatter( indexVectorOfPointsToUse, singleTract, tempLabel, tempIndex );

      lastLabel = tempLabel;
      lastElementSegIndex = tempIndex;
    }
        if ( retractFront )
    {
      // label and coordinate temp storage
      int tempLabel( firstLabel );
      itk::Index<3> tempIndex = firstElementSegIndex;

      RetractionUntilBrainMatter( true, singleTract, tempLabel, tempIndex );

      firstLabel = tempLabel;
      firstElementSegIndex = tempIndex;

    }

    if ( retractEnd )
    {
      // label and coordinate temp storage
      int tempLabel( lastLabel );
      itk::Index<3> tempIndex = lastElementSegIndex;

      RetractionUntilBrainMatter( false, singleTract, tempLabel, tempIndex );

      lastLabel = tempLabel;
      lastElementSegIndex = tempIndex;
    }
    //if( extendFront || extendEnd )
    //{
    //    MBI_INFO << "After Start: " << firstLabel << " at " << firstElementSegIndex[ 0 ] << " " << firstElementSegIndex[ 1 ] << " " << firstElementSegIndex[ 2 ] << " End: " << lastLabel << " at " << lastElementSegIndex[ 0 ] << " " << lastElementSegIndex[ 1 ] << " " << lastElementSegIndex[ 2 ];
    //}

    labelpair.first = firstLabel;
    labelpair.second = lastLabel;

    // Add property to property map
    CreateNewNode( firstLabel, firstElementSegIndex, m_UseCoMCoordinates );
    CreateNewNode( lastLabel, lastElementSegIndex, m_UseCoMCoordinates );
  }

  return labelpair;
}

mitk::ConnectomicsNetworkCreator::ImageLabelPairType mitk::ConnectomicsNetworkCreator::JustEndPointVerticesNoLabelTest( TractType::Pointer singleTract )
{
  ImageLabelPairType labelpair;

   {// Note: .fib image tracts are safed using index coordinates
    mitk::Point3D firstElementFiberCoord, lastElementFiberCoord;
    mitk::Point3D firstElementSegCoord, lastElementSegCoord;
    itk::Index<3> firstElementSegIndex, lastElementSegIndex;

    if( singleTract->front().Size() != 3 )
    {
      MBI_ERROR << mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_INVALID_DIMENSION_NEED_3;
    }
    for( unsigned int index = 0; index < singleTract->front().Size(); index++ )
    {
      firstElementFiberCoord.SetElement( index, singleTract->front().GetElement( index ) );
      lastElementFiberCoord.SetElement( index, singleTract->back().GetElement( index ) );
    }

    // convert from fiber index coordinates to segmentation index coordinates
    FiberToSegmentationCoords( firstElementFiberCoord, firstElementSegCoord );
    FiberToSegmentationCoords( lastElementFiberCoord, lastElementSegCoord );

    for( int index = 0; index < 3; index++ )
    {
      firstElementSegIndex.SetElement( index, firstElementSegCoord.GetElement( index ) );
      lastElementSegIndex.SetElement( index, lastElementSegCoord.GetElement( index ) );
    }

    int firstLabel = 1 * firstElementSegIndex[ 0 ] + 1000 * firstElementSegIndex[ 1 ] + 1000000 * firstElementSegIndex[ 2 ];
    int lastLabel = 1 * firstElementSegIndex[ 0 ] + 1000 * firstElementSegIndex[ 1 ] + 1000000 * firstElementSegIndex[ 2 ];

    labelpair.first = firstLabel;
    labelpair.second = lastLabel;

    // Add property to property map
    CreateNewNode( firstLabel, firstElementSegIndex, m_UseCoMCoordinates );
    CreateNewNode( lastLabel, lastElementSegIndex, m_UseCoMCoordinates );
  }

  return labelpair;
}

void mitk::ConnectomicsNetworkCreator::SupplyVertexWithInformation( ImageLabelType& label, VertexType& vertex )
{ // supply a vertex with the additional information belonging to the label

  // TODO: Implement additional information acquisition

  m_ConNetwork->SetLabel( vertex, m_LabelToNodePropertyMap.find( label )->second.label );
  m_ConNetwork->SetCoordinates( vertex, m_LabelToNodePropertyMap.find( label )->second.coordinates );

}

std::string mitk::ConnectomicsNetworkCreator::LabelToString( ImageLabelType& label )
{
  int tempInt = (int) label;
  std::stringstream ss;//create a stringstream
  std::string tempString;
  ss << tempInt;//add number to the stream
  tempString = ss.str();
  return tempString;//return a string with the contents of the stream
}

mitk::ConnectomicsNetwork::Pointer mitk::ConnectomicsNetworkCreator::GetNetwork()
{
  return m_ConNetwork;
}

void mitk::ConnectomicsNetworkCreator::FiberToSegmentationCoords( mitk::Point3D& fiberCoord, mitk::Point3D& segCoord )
{
  mitk::Point3D tempPoint;

  // convert from fiber index coordinates to segmentation index coordinates
  m_FiberBundle->GetGeometry()->IndexToWorld( fiberCoord, tempPoint );
  m_Segmentation->GetGeometry()->WorldToIndex( tempPoint, segCoord );
}

void mitk::ConnectomicsNetworkCreator::SegmentationToFiberCoords( mitk::Point3D& segCoord, mitk::Point3D& fiberCoord )
{
  mitk::Point3D tempPoint;

  // convert from fiber index coordinates to segmentation index coordinates
  m_Segmentation->GetGeometry()->IndexToWorld( segCoord, tempPoint );
  m_FiberBundle->GetGeometry()->WorldToIndex( tempPoint, fiberCoord );
}

bool mitk::ConnectomicsNetworkCreator::IsNonWhiteMatterLabel( int labelInQuestion )
{
  bool isWhite( false );

  isWhite = (
    ( labelInQuestion == freesurfer_Left_Cerebral_White_Matter )   ||
    ( labelInQuestion == freesurfer_Left_Cerebellum_White_Matter ) ||
    ( labelInQuestion == freesurfer_Right_Cerebral_White_Matter )  ||
    ( labelInQuestion == freesurfer_Right_Cerebellum_White_Matter )||
    ( labelInQuestion == freesurfer_Left_Cerebellum_Cortex )       ||
    ( labelInQuestion == freesurfer_Right_Cerebellum_Cortex )      ||
    ( labelInQuestion == freesurfer_Brain_Stem )
    );

  return !isWhite;
}

bool mitk::ConnectomicsNetworkCreator::IsBackgroundLabel( int labelInQuestion )
{
  bool isBackground( false );

  isBackground = ( labelInQuestion == 0 );

  return isBackground;
}

void mitk::ConnectomicsNetworkCreator::LinearExtensionUntilGreyMatter(
  std::vector<int> & indexVectorOfPointsToUse,
  TractType::Pointer singleTract,
  int & label,
  itk::Index<3> & mitkIndex )
{
  if( indexVectorOfPointsToUse.size() > singleTract->Size() )
  {
    MBI_WARN << mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_MORE_POINTS_THAN_PRESENT;
    return;
  }

  if( indexVectorOfPointsToUse.size() < 2 )
  {
    MBI_WARN << mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_ESTIMATING_LESS_THAN_2;
    return;
  }

  for( unsigned int index( 0 ); index < indexVectorOfPointsToUse.size(); index++ )
  {
    if( indexVectorOfPointsToUse[ index ] < 0 )
    {
      MBI_WARN << mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_ESTIMATING_BEYOND_START;
      return;
    }
    if( (unsigned int)indexVectorOfPointsToUse[ index ] > singleTract->Size() )
    {
      MBI_WARN << mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_ESTIMATING_BEYOND_END;
      return;
    }
  }

  mitk::Point3D startPoint, endPoint;
  std::vector< double > differenceVector;
  differenceVector.resize( singleTract->front().Size() );

  {
    // which points to use, currently only last two //TODO correct using all points
    int endPointIndex = indexVectorOfPointsToUse.size() - 1;
    int startPointIndex = indexVectorOfPointsToUse.size() - 2;

    // convert to segmentation coords
    mitk::Point3D startFiber, endFiber;
    for( unsigned int index = 0; index < singleTract->front().Size(); index++ )
    {
      endFiber.SetElement( index, singleTract->GetElement( indexVectorOfPointsToUse[ endPointIndex ] ).GetElement( index ) );
      startFiber.SetElement( index, singleTract->GetElement( indexVectorOfPointsToUse[ startPointIndex ] ).GetElement( index ) );
    }

    FiberToSegmentationCoords( endFiber, endPoint );
    FiberToSegmentationCoords( startFiber, startPoint );

    // calculate straight line

    for( unsigned int index = 0; index < singleTract->front().Size(); index++ )
    {
      differenceVector[ index ] = endPoint.GetElement( index ) - startPoint.GetElement( index );
    }

    // normalizing direction vector

    double length( 0.0 );
    double sum( 0.0 );

    for( unsigned int index = 0; index < differenceVector.size() ; index++ )
    {
      sum = sum + differenceVector[ index ] * differenceVector[ index ];
    }

    length = std::sqrt( sum );

    for( unsigned int index = 0; index < differenceVector.size() ; index++ )
    {
      differenceVector[ index ] = differenceVector[ index ] / length;
    }

    // follow line until first non white matter label
    itk::Index<3> tempIndex;
    int tempLabel( label );

    bool keepOn( true );

    itk::ImageRegion<3> itkRegion = m_SegmentationItk->GetLargestPossibleRegion();

    for( int parameter( 0 ) ; keepOn ; parameter++ )
    {
      if( parameter > 1000 )
      {
        MBI_WARN << mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_DID_NOT_FIND_WHITE;
        break;
      }

      for( int index( 0 ); index < 3; index++ )
      {
        tempIndex.SetElement( index, endPoint.GetElement( index ) + parameter * differenceVector[ index ] );
      }

      if( itkRegion.IsInside( tempIndex ) )
      {
        tempLabel = m_SegmentationItk->GetPixel( tempIndex );
      }
      else
      {
        tempLabel = -1;
      }

      if( IsNonWhiteMatterLabel( tempLabel ) )
      {
        if( tempLabel < 1 )
        {
          keepOn = false;
          //MBI_WARN << mitk::ConnectomicsConstantsManager::CONNECTOMICS_WARNING_NOT_EXTEND_TO_WHITE;
        }
        else
        {
          label = tempLabel;
          mitkIndex = tempIndex;
          keepOn = false;
        }
      }
    }

  }
}

void mitk::ConnectomicsNetworkCreator::RetractionUntilBrainMatter( bool retractFront, TractType::Pointer singleTract,
                                                                  int & label, itk::Index<3> & mitkIndex )
{
  int retractionStartIndex( singleTract->Size() - 1 );
  int retractionStepIndexSize( -1 );
  int retractionTerminationIndex( 0 );

  if( retractFront )
  {
    retractionStartIndex = 0;
    retractionStepIndexSize = 1;
    retractionTerminationIndex = singleTract->Size() - 1;
  }

  int currentRetractionIndex = retractionStartIndex;

  bool keepRetracting( true );

  mitk::Point3D currentPoint, nextPoint;
  std::vector< double > differenceVector;
  differenceVector.resize( singleTract->front().Size() );

  while( keepRetracting && ( currentRetractionIndex != retractionTerminationIndex ) )
  {
    // convert to segmentation coords
    mitk::Point3D currentPointFiberCoord, nextPointFiberCoord;
    for( unsigned int index = 0; index < singleTract->front().Size(); index++ )
    {
      currentPointFiberCoord.SetElement( index, singleTract->GetElement( currentRetractionIndex ).GetElement( index ) );
      nextPointFiberCoord.SetElement( index, singleTract->GetElement( currentRetractionIndex + retractionStepIndexSize ).GetElement( index ) );
    }

    FiberToSegmentationCoords( currentPointFiberCoord, currentPoint );
    FiberToSegmentationCoords( nextPointFiberCoord, nextPoint );

    // calculate straight line

    for( unsigned int index = 0; index < singleTract->front().Size(); index++ )
    {
      differenceVector[ index ] = nextPoint.GetElement( index ) - currentPoint.GetElement( index );
    }

    // calculate length of direction vector

    double length( 0.0 );
    double sum( 0.0 );

    for( unsigned int index = 0; index < differenceVector.size() ; index++ )
    {
      sum = sum + differenceVector[ index ] * differenceVector[ index ];
    }

    length = std::sqrt( sum );

    // retract
    itk::Index<3> tempIndex;
    int tempLabel( label );

    for( int parameter( 0 ) ; parameter < length ; parameter++ )
    {

      for( int index( 0 ); index < 3; index++ )
      {
        tempIndex.SetElement( index,
          currentPoint.GetElement( index ) + ( 1.0 + parameter ) / ( 1.0 + length ) * differenceVector[ index ] );
      }

      tempLabel = m_SegmentationItk->GetPixel( tempIndex );

      if( !IsBackgroundLabel( tempLabel ) )
      {
        // check whether result is within the search space
        {
          mitk::Point3D endPoint, foundPointSegmentation, foundPointFiber;
          for( unsigned int index = 0; index < singleTract->front().Size(); index++ )
          {
            // this is in fiber (world) coordinates
            endPoint.SetElement( index, singleTract->GetElement( retractionStartIndex ).GetElement( index ) );
          }

          for( int index( 0 ); index < 3; index++ )
          {
            foundPointSegmentation.SetElement( index,
              currentPoint.GetElement( index ) + ( 1.0 + parameter ) / ( 1.0 + length ) * differenceVector[ index ] );
          }

          SegmentationToFiberCoords( foundPointSegmentation, foundPointFiber );

          std::vector< double > finalDistance;
          finalDistance.resize( singleTract->front().Size() );
          for( unsigned int index = 0; index < singleTract->front().Size(); index++ )
          {
            finalDistance[ index ] = foundPointFiber.GetElement( index ) - endPoint.GetElement( index );
          }

          // calculate length of direction vector

          double finalLength( 0.0 );
          double finalSum( 0.0 );

          for( unsigned int index = 0; index < finalDistance.size() ; index++ )
          {
            finalSum = finalSum + finalDistance[ index ] * finalDistance[ index ];
          }
          finalLength = std::sqrt( finalSum );

          if( finalLength > m_EndPointSearchRadius )
          {
            // the found point was not within the search space
            return;
          }
        }

        label = tempLabel;
        mitkIndex = tempIndex;
        return;
      }
      // hit next point without finding brain matter
      currentRetractionIndex = currentRetractionIndex + retractionStepIndexSize;
      if( ( currentRetractionIndex < 1 ) || ( (unsigned int)currentRetractionIndex > ( singleTract->Size() - 2 ) ) )
      {
        keepRetracting = false;
      }
    }
  }
}

void mitk::ConnectomicsNetworkCreator::CalculateCenterOfMass()
{

  const int dimensions = 3;
  int max = m_Segmentation->GetStatistics()->GetScalarValueMax();
  int min = m_Segmentation->GetStatistics()->GetScalarValueMin();

  int range = max - min +1;

  // each label owns a vector of coordinates
  std::vector< std::vector< std::vector< double> > > coordinatesPerLabelVector;
  coordinatesPerLabelVector.resize( range );

  itk::ImageRegionIteratorWithIndex<ITKImageType> it_itkImage( m_SegmentationItk, m_SegmentationItk->GetLargestPossibleRegion() );

  for( it_itkImage.GoToBegin(); !it_itkImage.IsAtEnd(); ++it_itkImage )
  {
    std::vector< double > coordinates;
    coordinates.resize(dimensions);

    itk::Index< dimensions > index = it_itkImage.GetIndex();

    for( int loop(0); loop < dimensions; loop++)
    {
      coordinates.at( loop ) = index.GetElement( loop );
    }

    // add the coordinates to the corresponding label vector
    coordinatesPerLabelVector.at( it_itkImage.Value() - min ).push_back( coordinates );
  }

  for(int currentIndex(0), currentLabel( min ); currentIndex < range; currentIndex++, currentLabel++ )
  {
    std::vector< double > currentCoordinates;
    currentCoordinates.resize(dimensions);

    int numberOfPoints = coordinatesPerLabelVector.at( currentIndex ).size();

    std::vector< double > sumCoords;
    sumCoords.resize( dimensions );

    for( int loop(0); loop < numberOfPoints; loop++ )
    {
      for( int loopDimension( 0 ); loopDimension < dimensions; loopDimension++ )
      {
        sumCoords.at( loopDimension ) += coordinatesPerLabelVector.at( currentIndex ).at( loop ).at( loopDimension );
      }
    }

    for( int loopDimension( 0 ); loopDimension < dimensions; loopDimension++ )
    {
      currentCoordinates.at( loopDimension ) = sumCoords.at( loopDimension ) / numberOfPoints;
    }

    m_LabelsToCoordinatesMap.insert( std::pair< int, std::vector<double> >( currentLabel, currentCoordinates ) );
  }

  //can now use center of mass coordinates
  m_UseCoMCoordinates = true;
}

std::vector< double >  mitk::ConnectomicsNetworkCreator::GetCenterOfMass( int label )
{
  // if label is not known, warn
  if( ! ( m_LabelsToCoordinatesMap.count( label ) > 0 ) )
  {
    MITK_ERROR << "Label " << label << " not found. Could not return coordinates.";
    std::vector< double > nullVector;
    nullVector.resize(3);
    return nullVector;
  }

  //return associated coordinates
  return m_LabelsToCoordinatesMap.find( label )->second;
}

void mitk::ConnectomicsNetworkCreator::CreateNewNode( int label, itk::Index<3> index, bool useCoM )
{
  // Only create node if it does not exist yet

  if( ! ( m_LabelToNodePropertyMap.count( label ) > 0 ) )
  {
    NetworkNode newNode;

    newNode.coordinates.resize( 3 );
    if( !useCoM )
    {
      for( unsigned int loop = 0; loop < newNode.coordinates.size() ; loop++ )
      {
        newNode.coordinates[ loop ] = index[ loop ] ;
      }
    }
    else
    {
      std::vector<double> labelCoords = GetCenterOfMass( label );
      for( unsigned int loop = 0; loop < newNode.coordinates.size() ; loop++ )
      {
        newNode.coordinates[ loop ] = labelCoords.at( loop ) ;
      }
    }

    newNode.label = LabelToString( label );

    m_LabelToNodePropertyMap.insert( std::pair< ImageLabelType, NetworkNode >( label, newNode ) );
  }
}
