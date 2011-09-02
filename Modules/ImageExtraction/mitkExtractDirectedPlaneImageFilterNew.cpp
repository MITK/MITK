/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-12-02 14:54:03 +0100 (Do, 2 Dec 2010) $
Version:   $Revision: 21147 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkExtractDirectedPlaneImageFilterNew.h"
#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"
#include "itkImageRegionIterator.h"

#include <mitkImageAccessByItk.h>

mitk::ExtractDirectedPlaneImageFilterNew::ExtractDirectedPlaneImageFilterNew()
:m_CurrentWorldGeometry2D(NULL),
m_ActualInputTimestep(-1)
{
}

mitk::ExtractDirectedPlaneImageFilterNew::~ExtractDirectedPlaneImageFilterNew()
{
}

void mitk::ExtractDirectedPlaneImageFilterNew::GenerateData(){

    mitk::Image::ConstPointer inputImage = ImageToImageFilter::GetInput(0);

    if ( !inputImage )
    {
        MITK_ERROR << "mitk::ExtractDirectedPlaneImageFilterNew: No input available. Please set the input!" << std::endl;
        itkExceptionMacro("mitk::ExtractDirectedPlaneImageFilterNew: No input available. Please set the input!");
        return;
    }

    m_ImageGeometry = inputImage->GetGeometry();

    //If no timestep is set, the lowest given will be selected
    const mitk::TimeSlicedGeometry* inputTimeGeometry = this->GetInput()->GetTimeSlicedGeometry();
    if ( m_ActualInputTimestep == -1)
    {
        ScalarType time = m_CurrentWorldGeometry2D->GetTimeBounds()[0];
        if ( time > ScalarTypeNumericTraits::NonpositiveMin() )
        {
            m_ActualInputTimestep = inputTimeGeometry->MSToTimeStep( time );
        }
    }

    if ( inputImage->GetDimension() > 4 || inputImage->GetDimension() < 2)
    {
        MITK_ERROR << "mitk::ExtractDirectedPlaneImageFilterNew:GenerateData works only with 3D and 3D+t images, sorry." << std::endl;
        itkExceptionMacro("mitk::ExtractDirectedPlaneImageFilterNew works only with 3D and 3D+t images, sorry.");
        return;
    }
    else if ( inputImage->GetDimension() == 4 )
    {
        mitk::ImageTimeSelector::Pointer timeselector = mitk::ImageTimeSelector::New();
        timeselector->SetInput( inputImage );
        timeselector->SetTimeNr( m_ActualInputTimestep );
        timeselector->UpdateLargestPossibleRegion();
        inputImage = timeselector->GetOutput();
    }
    else if ( inputImage->GetDimension() == 2)
    {
        mitk::Image::Pointer resultImage = ImageToImageFilter::GetOutput();
        resultImage = const_cast<mitk::Image*>( inputImage.GetPointer() );
        ImageToImageFilter::SetNthOutput( 0, resultImage);
        return;
    }

    if ( !m_CurrentWorldGeometry2D )
    {
        MITK_ERROR<< "mitk::ExtractDirectedPlaneImageFilterNew::GenerateData has no CurrentWorldGeometry2D set" << std::endl;
        return;
    }

    AccessFixedDimensionByItk( inputImage, ItkSliceExtraction, 3 );

}//Generate Data


void mitk::ExtractDirectedPlaneImageFilterNew::GenerateOutputInformation () 
{
    Superclass::GenerateOutputInformation();
}



/*
* The desired slice is extracted by filling the image`s corresponding pixel values in an empty 2 dimensional itk::Image
* Therefor the itk image`s extent in pixel (in each direction) is doubled and its spacing (also in each direction) is divided by two
* (similar to the shannon theorem).
*/
template<typename TPixel, unsigned int VImageDimension>
void mitk::ExtractDirectedPlaneImageFilterNew::ItkSliceExtraction (itk::Image<TPixel, VImageDimension>* inputImage)
{
    typedef itk::Image<TPixel, VImageDimension> InputImageType;
    typedef itk::Image<TPixel, VImageDimension-1> SliceImageType;

    typedef itk::ImageRegionConstIterator< SliceImageType > SliceIterator;

    //Creating an itk::Image that represents the sampled slice
    typename SliceImageType::Pointer resultSlice = SliceImageType::New();

    typename SliceImageType::IndexType start;

    start[0] = 0;
    start[1] = 0;

    Point3D origin = m_CurrentWorldGeometry2D->GetOrigin();
    Vector3D right = m_CurrentWorldGeometry2D->GetAxisVector(0);
    Vector3D bottom = m_CurrentWorldGeometry2D->GetAxisVector(1);

    //Calculation the sample-spacing, i.e the half of the smallest spacing existing in the original image
    Vector3D newPixelSpacing = m_ImageGeometry->GetSpacing();
    float minSpacing = newPixelSpacing[0];
    for (unsigned int i = 1; i < newPixelSpacing.Size(); i++)
    {
        if (newPixelSpacing[i] < minSpacing )
        {
            minSpacing = newPixelSpacing[i];
        }
    }

    newPixelSpacing[0] = 0.5*minSpacing;
    newPixelSpacing[1] = 0.5*minSpacing;
    newPixelSpacing[2] = 0.5*minSpacing;

    float pixelSpacing[2];
    pixelSpacing[0] =  newPixelSpacing[0];
    pixelSpacing[1] =  newPixelSpacing[1];

    //Calculating the size of the sampled slice
    typename SliceImageType::SizeType size;
    Vector2D extentInMM;
    extentInMM[0] = m_CurrentWorldGeometry2D->GetExtentInMM(0);
    extentInMM[1] = m_CurrentWorldGeometry2D->GetExtentInMM(1);

    //The maximum extent is the lenght of the diagonal of the considered plane
    double maxExtent = sqrt(extentInMM[0]*extentInMM[0]+extentInMM[1]*extentInMM[1]);
    unsigned int xTranlation = (maxExtent-extentInMM[0]);
    unsigned int yTranlation = (maxExtent-extentInMM[1]);
    size[0] = (maxExtent+xTranlation)/newPixelSpacing[0];
    size[1] = (maxExtent+yTranlation)/newPixelSpacing[1];

    //Creating an ImageRegion Object
    typename SliceImageType::RegionType region;

    region.SetSize( size );
    region.SetIndex( start );

    //Defining the image`s extent and origin by passing the region to it and allocating memory for it
    resultSlice->SetRegions( region );
    resultSlice->SetSpacing( pixelSpacing );
    resultSlice->Allocate();

    /*
    * Here we create an new geometry so that the transformations are calculated correctly (our resulting slice has a different bounding box and spacing)
    * The original current worldgeometry must be cloned because we have to keep the directions of the axis vector which represents the rotation
    */
    right.Normalize();
    bottom.Normalize();
    //Here we translate the origin to adapt the new geometry to the previous calculated extent
    origin[0] -= xTranlation*right[0]+yTranlation*bottom[0];
    origin[1] -= xTranlation*right[1]+yTranlation*bottom[1];
    origin[2] -= xTranlation*right[2]+yTranlation*bottom[2];

    //Putting it together for the new geometry
    mitk::Geometry3D::Pointer newSliceGeometryTest = dynamic_cast<Geometry3D*>(m_CurrentWorldGeometry2D->Clone().GetPointer());
    newSliceGeometryTest->ChangeImageGeometryConsideringOriginOffset(true);

    //Workaround because of BUG (#6505)
    newSliceGeometryTest->GetIndexToWorldTransform()->SetMatrix(m_CurrentWorldGeometry2D->GetIndexToWorldTransform()->GetMatrix());
    //Workaround end

    newSliceGeometryTest->SetOrigin(origin);
    ScalarType bounds[6]={0, size[0], 0, size[1], 0, 1};
    newSliceGeometryTest->SetBounds(bounds);
    newSliceGeometryTest->SetSpacing(newPixelSpacing);
    newSliceGeometryTest->Modified();

    //Workaround because of BUG (#6505)
    itk::MatrixOffsetTransformBase<mitk::ScalarType,3,3>::MatrixType tempTransform = newSliceGeometryTest->GetIndexToWorldTransform()->GetMatrix();
    //Workaround end

    /* 
    * Now we iterate over the recently created slice.
    * For each slice - pixel we check whether there is an according
    * pixel in the input - image which can be set in the slice.
    * In this way a slice is sampled out of the input - image regrading to the given PlaneGeometry
    */
    Point3D currentSliceIndexPointIn2D;
    Point3D currentImageWorldPointIn3D;
    typename InputImageType::IndexType inputIndex;

    SliceIterator sliceIterator ( resultSlice, resultSlice->GetLargestPossibleRegion() );
    sliceIterator.GoToBegin();

    while ( !sliceIterator.IsAtEnd() )
    {
        /*
        * Here we add 0.5 to to assure that the indices are correctly transformed.
        * (Because of the 0.5er Bug)
        */
        currentSliceIndexPointIn2D[0] = sliceIterator.GetIndex()[0]+0.5;
        currentSliceIndexPointIn2D[1] = sliceIterator.GetIndex()[1]+0.5;
        currentSliceIndexPointIn2D[2] = 0;

        newSliceGeometryTest->IndexToWorld( currentSliceIndexPointIn2D, currentImageWorldPointIn3D );

        m_ImageGeometry->WorldToIndex( currentImageWorldPointIn3D, inputIndex);

        if ( m_ImageGeometry->IsIndexInside( inputIndex ))
        {

            resultSlice->SetPixel( sliceIterator.GetIndex(), inputImage->GetPixel(inputIndex) );

        }
        else
        {
            resultSlice->SetPixel( sliceIterator.GetIndex(), 0);
        }

        ++sliceIterator;
    }

    Image::Pointer resultImage = ImageToImageFilter::GetOutput();
    GrabItkImageMemory(resultSlice, resultImage, NULL, false);
    resultImage->SetClonedGeometry(newSliceGeometryTest);
    //Workaround because of BUG (#6505)
    resultImage->GetGeometry()->GetIndexToWorldTransform()->SetMatrix(tempTransform);
    //Workaround end
}


///**TEST** May ba a little bit more efficient but doesn`t already work/
//right.Normalize();
//bottom.Normalize();
//Point3D currentImagePointIn3D = origin /*+ bottom*newPixelSpacing*/;
//unsigned int columns ( 0 );
/**ENDE**/


/****TEST***/

//SliceImageType::IndexType index = sliceIterator.GetIndex();

//if ( columns == (extentInPixel[0]) )
//{
//If we are at the end of a row, then we have to go to the beginning of the next row 
//currentImagePointIn3D = origin;
//currentImagePointIn3D += newPixelSpacing[1]*bottom*index[1];
//columns = 0;
//m_ImageGeometry->WorldToIndex(currentImagePointIn3D, inputIndex);
//}
//else
//{
////
//if ( columns != 0 )
//{
//currentImagePointIn3D += newPixelSpacing[0]*right; 
//}
//m_ImageGeometry->WorldToIndex(currentImagePointIn3D, inputIndex);
//}

//if ( m_ImageGeometry->IsIndexInside( inputIndex ))
//{
//resultSlice->SetPixel( sliceIterator.GetIndex(), inputImage->GetPixel(inputIndex) );
//}
//else if (currentImagePointIn3D == origin)
//{
//Point3D temp;
//temp[0] = bottom[0]*newPixelSpacing[0]*0.5;
//temp[1] = bottom[1]*newPixelSpacing[1]*0.5;
//temp[2] = bottom[2]*newPixelSpacing[2]*0.5;
//origin[0] += temp[0];
//origin[1] += temp[1];
//origin[2] += temp[2];
//currentImagePointIn3D = origin;
//m_ImageGeometry->WorldToIndex(currentImagePointIn3D, inputIndex);
//if ( m_ImageGeometry->IsIndexInside( inputIndex ))
//{
//resultSlice->SetPixel( sliceIterator.GetIndex(), inputImage->GetPixel(inputIndex) );
//}
//}

/****TEST ENDE****/
