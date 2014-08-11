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

#include "mitkOverwriteDirectedPlaneImageFilter.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkSegmentationInterpolationController.h"
#include "mitkApplyDiffImageOperation.h"
#include "mitkOperationEvent.h"
#include "mitkInteractionConst.h"
#include "mitkUndoController.h"
#include "mitkDiffImageApplier.h"
#include "mitkImageTimeSelector.h"

#include <itkImageSliceIteratorWithIndex.h>
#include <itkImageRegionIterator.h>

mitk::OverwriteDirectedPlaneImageFilter::OverwriteDirectedPlaneImageFilter()
:m_PlaneGeometry(0),
m_ImageGeometry3D(0),
m_TimeStep(0),
m_Dimension0(0),
m_Dimension1(1),
m_CreateUndoInformation(false)
{
  MITK_WARN << "Class is deprecated! Use mitkVtkImageOverwrite instead.";
}

mitk::OverwriteDirectedPlaneImageFilter::~OverwriteDirectedPlaneImageFilter()
{
}

void mitk::OverwriteDirectedPlaneImageFilter::GenerateData()
{
    //
    // this is the place to implement the major part of undo functionality (bug #491)
    // here we have to create undo/do operations
    //
    // WHO is the operation actor? This object may not be destroyed ever (design of undo stack)!
    // -> some singleton method of this filter?
    //
    // neccessary additional objects:
    //  - something that executes the operations
    //  - the operation class (must hold a binary diff or something)
    //  - observer commands to know when the image is deleted (no further action then, perhaps even remove the operations from the undo stack)
    //
    //Image::ConstPointer input = ImageToImageFilter::GetInput(0);
    Image::Pointer input3D = ImageToImageFilter::GetInput(0);

    //Image::ConstPointer slice = m_SliceImage;

    if ( input3D.IsNull() || m_SliceImage.IsNull() ) return;

    if ( input3D->GetDimension() == 4 )
    {
        ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
        timeSelector->SetInput( input3D );
        timeSelector->SetTimeNr( m_TimeStep );
        timeSelector->UpdateLargestPossibleRegion();
        input3D = timeSelector->GetOutput();
    }

    m_ImageGeometry3D = input3D->GetGeometry();
    /*
    if ( m_SliceDifferenceImage.IsNull() ||
    m_SliceDifferenceImage->GetDimension(0) != m_SliceImage->GetDimension(0) ||
    m_SliceDifferenceImage->GetDimension(1) != m_SliceImage->GetDimension(1) )
    {
    m_SliceDifferenceImage = mitk::Image::New();
    mitk::PixelType pixelType( typeid(short signed int) );
    m_SliceDifferenceImage->Initialize( pixelType, 2, m_SliceImage->GetDimensions() );
    }
    */
    //MITK_INFO << "Overwriting slice " << m_SliceIndex << " in dimension " << m_SliceDimension << " at time step " << m_TimeStep << std::endl;
    // this will do a long long if/else to find out both pixel types
    /*AccessFixedDimensionByItk( input3D, ItkImageSwitch, 3 );*/
    AccessFixedDimensionByItk( input3D, ItkSliceOverwriting, 3 );

    //SegmentationInterpolationController* interpolator = SegmentationInterpolationController::InterpolatorForImage( input3D );
    //if (interpolator)
    //{
    //  interpolator->BlockModified(true);
    //  //interpolator->SetChangedSlice( m_SliceDifferenceImage, m_SliceDimension, m_SliceIndex, m_TimeStep );
    //}
    /*
    if ( m_CreateUndoInformation )
    {
    // create do/undo operations (we don't execute the doOp here, because it has already been executed during calculation of the diff image
    ApplyDiffImageOperation* doOp = new ApplyDiffImageOperation( OpTEST, const_cast<Image*>(input.GetPointer()), m_SliceDifferenceImage, m_TimeStep, m_SliceDimension, m_SliceIndex );
    ApplyDiffImageOperation* undoOp = new ApplyDiffImageOperation( OpTEST, const_cast<Image*>(input.GetPointer()), m_SliceDifferenceImage, m_TimeStep, m_SliceDimension, m_SliceIndex );
    undoOp->SetFactor( -1.0 );
    OperationEvent* undoStackItem = new OperationEvent( DiffImageApplier::GetInstanceForUndo(), doOp, undoOp, this->EventDescription(m_SliceDimension, m_SliceIndex, m_TimeStep) );
    UndoController::GetCurrentUndoModel()->SetOperationEvent( undoStackItem );
    }
    */
    // this image is modified (good to know for the renderer)
    input3D->Modified();

    /*if (interpolator)
    {
    interpolator->BlockModified(false);
    }*/
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::OverwriteDirectedPlaneImageFilter::ItkSliceOverwriting( itk::Image<TPixel,VImageDimension>* input3D )
{
    typedef itk::Image<TPixel, VImageDimension-1> SliceImageType;
    typedef itk::Image<TPixel, VImageDimension> VolumeImageType;

    typedef itk::ImageSliceIteratorWithIndex< VolumeImageType > OutputSliceIteratorType;
    typedef itk::ImageRegionConstIterator< SliceImageType >     SliceIteratorType;

    typename SliceImageType::Pointer sliceImage = SliceImageType::New();
    CastToItkImage(m_SliceImage,sliceImage);

    SliceIteratorType sliceIterator( sliceImage, sliceImage->GetLargestPossibleRegion() );

    sliceIterator.GoToBegin();

    Point3D currentPointIn2D;
    Point3D worldPointIn3D;

    //Here we just iterate over the slice which must be written into the 3D volumen and set the corresponding pixel in our 3D volume
    while ( !sliceIterator.IsAtEnd() )
    {
        currentPointIn2D[0] = sliceIterator.GetIndex()[0]+0.5;
        currentPointIn2D[1] = sliceIterator.GetIndex()[1]+0.5;
        currentPointIn2D[2] = 0;

        m_PlaneGeometry->IndexToWorld( currentPointIn2D, worldPointIn3D );

        typename itk::Image<TPixel,VImageDimension>::IndexType outputIndex;
        m_ImageGeometry3D->WorldToIndex( worldPointIn3D, outputIndex );

        // Only access ITK image if it's inside
        if ( m_ImageGeometry3D->IsIndexInside( outputIndex ) )
        {
            input3D->SetPixel( outputIndex, (TPixel)sliceIterator.Get() );
        }


        ++sliceIterator;
    }
}

/****TEST***/
//Maybe a bit more efficient but doesn`t already work. See also ExtractCirectedPlaneImageFilter

//typename itk::Image<TPixel2,VImageDimension2>::IndexType outputIndex;

//if ( columns == extent[0] )
//{
////If we are at the end of a row, then we have to go to the beginning of the next row
//currentImagePointIn3D = origin;
//currentImagePointIn3D += spacing[1]*bottom*currentPointIn2D[1];
//columns = 0;
//m_ImageGeometry3D->WorldToIndex(currentImagePointIn3D, outputIndex);
//}
//else
//{
//if ( columns != 0 )
//{
//currentImagePointIn3D += spacing[0]*right;
//}
//m_ImageGeometry3D->WorldToIndex(currentImagePointIn3D, outputIndex);
//}

//if ( m_ImageGeometry3D->IsIndexInside( outputIndex ))
//{
//outputImage->SetPixel( outputIndex, (TPixel2)inputIterator.Get() );
//}
//else if (currentImagePointIn3D == origin)
//{
//Point3D temp;
//temp[0] = bottom[0]*spacing[0]*0.5;
//temp[1] = bottom[1]*spacing[1]*0.5;
//temp[2] = bottom[2]*spacing[2]*0.5;
//origin[0] += temp[0];
//origin[1] += temp[1];
//origin[2] += temp[2];
//currentImagePointIn3D = origin;
//m_ImageGeometry3D->WorldToIndex(currentImagePointIn3D, outputIndex);
//if ( m_ImageGeometry3D->IsIndexInside( outputIndex ))
//{
//outputImage->SetPixel( outputIndex, (TPixel2)inputIterator.Get() );
//}
//}
//columns++;

/****TEST ENDE****/

//*
//    // Offset the world coordinate by one pixel to compensate for
//    // index/world origin differences.
//    Point3D offsetIndex;
//    offsetIndex.Fill( 1 );
//    Point3D offsetWorld;
//    offsetWorld.Fill( 0 );
//    m_PlaneGeometry->IndexToWorld( offsetIndex, offsetWorld );
//    // remove origin shift
//    const Point3D origin = m_PlaneGeometry->GetOrigin();
//    offsetWorld[0] -= origin[0];
//    offsetWorld[1] -= origin[1];
//    offsetWorld[2] -= origin[2];
//    // offset world coordinate
//    worldPointIn3D[ 0 ] += offsetWorld[0];
//    worldPointIn3D[ 1 ] += offsetWorld[1];
//    worldPointIn3D[ 2 ] += offsetWorld[2];
//*/

// basically copied from mitk/Core/Algorithms/mitkImageAccessByItk.h
/*#define myMITKOverwriteDirectedPlaneImageFilterAccessByItk(mitkImage, itkImageTypeFunction, pixeltype, dimension, itkimage2)            \
//  if ( typeId == typeid(pixeltype) )                                                    \
//{                                                                                        \
//    typedef itk::Image<pixeltype, dimension> ImageType;                                   \
//    typedef mitk::ImageToItk<ImageType> ImageToItkType;                                    \
//    itk::SmartPointer<ImageToItkType> imagetoitk = ImageToItkType::New();                 \
//    imagetoitk->SetInput(mitkImage);                                                     \
//    imagetoitk->Update();                                                               \
//    itkImageTypeFunction(imagetoitk->GetOutput(), itkimage2);                          \
//}
//
//#define myMITKOverwriteDirectedPlaneImageFilterAccessAllTypesByItk(mitkImage, itkImageTypeFunction,       dimension, itkimage2)    \
//{                                                                                                                           \
//    myMITKOverwriteDirectedPlaneImageFilterAccessByItk(mitkImage, itkImageTypeFunction, double,         dimension, itkimage2) else   \
//    myMITKOverwriteDirectedPlaneImageFilterAccessByItk(mitkImage, itkImageTypeFunction, float,          dimension, itkimage2) else    \
//    myMITKOverwriteDirectedPlaneImageFilterAccessByItk(mitkImage, itkImageTypeFunction, int,            dimension, itkimage2) else     \
//    myMITKOverwriteDirectedPlaneImageFilterAccessByItk(mitkImage, itkImageTypeFunction, unsigned int,   dimension, itkimage2) else      \
//    myMITKOverwriteDirectedPlaneImageFilterAccessByItk(mitkImage, itkImageTypeFunction, short,          dimension, itkimage2) else     \
//    myMITKOverwriteDirectedPlaneImageFilterAccessByItk(mitkImage, itkImageTypeFunction, unsigned short, dimension, itkimage2) else    \
//    myMITKOverwriteDirectedPlaneImageFilterAccessByItk(mitkImage, itkImageTypeFunction, char,           dimension, itkimage2) else   \
//    myMITKOverwriteDirectedPlaneImageFilterAccessByItk(mitkImage, itkImageTypeFunction, unsigned char,  dimension, itkimage2)       \
//}*/
//
//
//template<typename TPixel, unsigned int VImageDimension>
//void mitk::OverwriteDirectedPlaneImageFilter::ItkImageSwitch( itk::Image<TPixel,VImageDimension>* itkImage )
//{
//  const std::type_info& typeId=*(m_SliceImage->GetPixelType().GetTypeId());
//
//  myMITKOverwriteDirectedPlaneImageFilterAccessAllTypesByItk( m_SliceImage, ItkImageProcessing, 2, itkImage );
//}

//template<typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
//void mitk::OverwriteDirectedPlaneImageFilter::ItkImageProcessing( itk::Image<TPixel1,VImageDimension1>* inputImage, itk::Image<TPixel2,VImageDimension2>* outputImage )
//{
//  typedef itk::Image<TPixel1, VImageDimension1> SliceImageType;
//  typedef itk::Image<short signed int, VImageDimension1> DiffImageType;
//  typedef itk::Image<TPixel2, VImageDimension2> VolumeImageType;
//
//  typedef itk::ImageSliceIteratorWithIndex< VolumeImageType > OutputSliceIteratorType;
//  typedef itk::ImageRegionConstIterator< SliceImageType >     InputSliceIteratorType;
//  //typedef itk::ImageRegionIterator< DiffImageType >     DiffSliceIteratorType;
//
//  InputSliceIteratorType inputIterator( inputImage, inputImage->GetLargestPossibleRegion() );
//
//  //typename DiffImageType::Pointer diffImage;
//  //CastToItkImage( m_SliceDifferenceImage, diffImage );
//  //DiffSliceIteratorType diffIterator( diffImage, diffImage->GetLargestPossibleRegion() );
//
//  inputIterator.GoToBegin();
//  //diffIterator.GoToBegin();
//
//  //TEST
//  Point3D origin = m_PlaneGeometry->GetOrigin();
//  Vector3D right = m_PlaneGeometry->GetAxisVector(0);
//  Vector3D bottom = m_PlaneGeometry->GetAxisVector(1);
//  right.Normalize();
//  bottom.Normalize();
//
//  Vector2D spacing = inputImage->GetSpacing();
//
//  Vector2D extentInMM;
//  extentInMM[0] = m_PlaneGeometry->GetExtentInMM(0);
//  extentInMM[1] = m_PlaneGeometry->GetExtentInMM(1);
//
//  Vector2D extent;
//  extent[0] = m_PlaneGeometry->GetExtent(0);
//  extent[1] = m_PlaneGeometry->GetExtent(1);
//  //TEST ENDE
//
//  Point3D currentPointIn2D, worldPointIn3D;
//  TPixel2 outputPixel = 0;
//
//  int debugCounter( 0 );
//
//  std::ofstream geometryFile;
//  geometryFile.precision(30);
//  geometryFile.open("C:/Users/fetzer/Desktop/TEST/geometryFileOv.txt");
//
//  geometryFile<<"Offset: [ "<<m_PlaneGeometry->GetIndexToWorldTransform()->GetOffset()[0]<<", "<<m_PlaneGeometry->GetIndexToWorldTransform()->GetOffset()[1]<<", "<<m_PlaneGeometry->GetIndexToWorldTransform()->GetOffset()[2]<<" ]"<<std::endl;
//  geometryFile<<"Transform: "<<m_PlaneGeometry->GetIndexToWorldTransform()->GetMatrix()<<std::endl;
//
//  //std::ofstream overriderFile;
//  //overriderFile.open("C:/Users/fetzer/Desktop/TEST/overridePoints.txt");
//
//  //std::ofstream overriderFileIndex;
//  //overriderFileIndex.open("C:/Users/fetzer/Desktop/TEST/overrideIndexPoints.txt");
//
//
//  //TEST
//  Point3D currentImagePointIn3D = origin;
//  unsigned int columns ( 0 );
//  //TEST ENDE
//
//  while ( !inputIterator.IsAtEnd() )
//  {
//    // Input world point
//    currentPointIn2D[0] = inputIterator.GetIndex()[0]+0.5;
//    currentPointIn2D[1] = inputIterator.GetIndex()[1]+0.5;
//    currentPointIn2D[2] = 0;
//
//m_PlaneGeometry->IndexToWorld( currentPointIn2D, worldPointIn3D );
//
//typename itk::Image<TPixel2,VImageDimension2>::IndexType outputIndex;
//    m_ImageGeometry3D->WorldToIndex( worldPointIn3D, outputIndex );
//
//    // Only access ITK image if it's inside
//    if ( m_ImageGeometry3D->IsIndexInside( outputIndex ) )
//    {
//      //outputPixel = outputImage->GetPixel( outputIndex );
//      outputImage->SetPixel( outputIndex, (TPixel2)inputIterator.Get() );
//      /*if( inputIterator.Get() == mitk::paint::addPixelValue )
//      {
//        outputImage->SetPixel( outputIndex, (TPixel2)( 1 ) );
//      }
//      else if( inputIterator.Get() == mitk::paint::subPixelValue )
//      {
//        outputImage->SetPixel( outputIndex, (TPixel2)( 0 ) );
//      }*/
//    }
//
///****TEST***/
//
////typename itk::Image<TPixel2,VImageDimension2>::IndexType outputIndex;
//
////if ( columns == extent[0] )
////{
//////If we are at the end of a row, then we have to go to the beginning of the next row
////currentImagePointIn3D = origin;
////currentImagePointIn3D += spacing[1]*bottom*currentPointIn2D[1];
//columns = 0;
////m_ImageGeometry3D->WorldToIndex(currentImagePointIn3D, outputIndex);
////}
////else
////{
////if ( columns != 0 )
////{
////currentImagePointIn3D += spacing[0]*right;
////}
////m_ImageGeometry3D->WorldToIndex(currentImagePointIn3D, outputIndex);
////}
//
////if ( m_ImageGeometry3D->IsIndexInside( outputIndex ))
////{
////outputImage->SetPixel( outputIndex, (TPixel2)inputIterator.Get() );
////}
////else if (currentImagePointIn3D == origin)
////{
////Point3D temp;
////temp[0] = bottom[0]*spacing[0]*0.5;
////temp[1] = bottom[1]*spacing[1]*0.5;
////temp[2] = bottom[2]*spacing[2]*0.5;
////origin[0] += temp[0];
////origin[1] += temp[1];
////origin[2] += temp[2];
////currentImagePointIn3D = origin;
////m_ImageGeometry3D->WorldToIndex(currentImagePointIn3D, outputIndex);
////if ( m_ImageGeometry3D->IsIndexInside( outputIndex ))
////{
////outputImage->SetPixel( outputIndex, (TPixel2)inputIterator.Get() );
////}
////}
////columns++;
//
///****TEST ENDE****/
//
////*
////    // Offset the world coordinate by one pixel to compensate for
////    // index/world origin differences.
////    Point3D offsetIndex;
////    offsetIndex.Fill( 1 );
////    Point3D offsetWorld;
////    offsetWorld.Fill( 0 );
////    m_PlaneGeometry->IndexToWorld( offsetIndex, offsetWorld );
////    // remove origin shift
////    const Point3D origin = m_PlaneGeometry->GetOrigin();
////    offsetWorld[0] -= origin[0];
////    offsetWorld[1] -= origin[1];
////    offsetWorld[2] -= origin[2];
////    // offset world coordinate
////    worldPointIn3D[ 0 ] += offsetWorld[0];
////    worldPointIn3D[ 1 ] += offsetWorld[1];
////    worldPointIn3D[ 2 ] += offsetWorld[2];
////*/
//    // Output index
//
//
////For the purpose of debug
////if( debugCounter%100 == 0)
//////{
////Point3D contIndex;
////m_ImageGeometry3D->WorldToIndex(worldPointIn3D,contIndex);
////overriderFile.precision(10);
////overriderFile<<"2D-Index: [ "<<currentPointIn2D[0]<<", "<<currentPointIn2D[1]<<" ] "<<"WorldIndex: [ "<<worldPointIn3D[0]<<", "<<worldPointIn3D[1]<<", "<<worldPointIn3D[2]<<" ]"<<" ContIndex: [ "<<contIndex[0]<<", "<<contIndex[1]<<", "<<contIndex[2]<<" ]"<<" Index: [ "<<outputIndex[0]<<", "<<outputIndex[1]<<", "<<outputIndex[2]<<" ]"<<std::endl;
////overriderFile<<"[ "<<worldPointIn3D[0]<<", "<<worldPointIn3D[1]<<", "<<worldPointIn3D[2]<<" ]"<<std::endl;
////overriderFileIndex<<"2D-Index: [ "<<currentPointIn2D[0]<<", "<<currentPointIn2D[1]<<" ] "<<"3D-Index: [ "<<outputIndex[0]<<", "<<outputIndex[1]<<", "<<outputIndex[2]<<" ]"<<std::endl;
////}
//
//    // Set difference image
//    //diffIterator.Set( static_cast<short signed int>(inputIterator.Get() - outputPixel ) ); // oh oh, not good for bigger values
//    ++inputIterator;
////++debugCounter;
//    //++diffIterator;
//  }
//  /*overriderFile.close();
//  overriderFileIndex.close();*/
//  geometryFile.close();
//
///*
//  typename VolumeImageType::RegionType            sliceInVolumeRegion;
//
//  sliceInVolumeRegion = outputImage->GetLargestPossibleRegion();
//  sliceInVolumeRegion.SetSize( m_SliceDimension, 1 );             // just one slice
//  sliceInVolumeRegion.SetIndex( m_SliceDimension, m_SliceIndex ); // exactly this slice, please
//
//  OutputSliceIteratorType outputIterator( outputImage, sliceInVolumeRegion );
//  outputIterator.SetFirstDirection(m_Dimension0);
//  outputIterator.SetSecondDirection(m_Dimension1);
//
//  // iterate over output slice (and over input slice simultaneously)
//  outputIterator.GoToBegin();
//  while ( !outputIterator.IsAtEnd() )
//  {
//    while ( !outputIterator.IsAtEndOfSlice() )
//    {
//      while ( !outputIterator.IsAtEndOfLine() )
//      {
//        diffIterator.Set( static_cast<short signed int>(inputIterator.Get() - outputIterator.Get()) ); // oh oh, not good for bigger values
//        outputIterator.Set( (TPixel2) inputIterator.Get() );
//        ++outputIterator;
//        ++inputIterator;
//        ++diffIterator;
//      }
//      outputIterator.NextLine();
//    }
//    outputIterator.NextSlice();
//  }
//  */
//}
/*
std::string mitk::OverwriteDirectedPlaneImageFilter::EventDescription( unsigned int sliceDimension, unsigned int sliceIndex, unsigned int timeStep )
{
std::stringstream s;

s << "Changed slice (";

switch (sliceDimension)
{
default:
case 2:
s << "T";
break;
case 1:
s << "C";
break;
case 0:
s << "S";
break;
}

s << " " << sliceIndex << " " << timeStep << ")";

return s.str();
}
*/

