
#ifndef __itkMrtrixPeakImageConverter_cpp
#define __itkMrtrixPeakImageConverter_cpp

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "itkMrtrixPeakImageConverter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <itkContinuousIndex.h>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>


namespace itk {

template< class PixelType >
MrtrixPeakImageConverter< PixelType >::MrtrixPeakImageConverter():
    m_NormalizationMethod(NO_NORM)
{

}

template< class PixelType >
void MrtrixPeakImageConverter< PixelType >
::GenerateData()
{
    // output vector field
    vtkSmartPointer<vtkCellArray> m_VtkCellArray = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints>    m_VtkPoints = vtkSmartPointer<vtkPoints>::New();

    Vector<float, 4> spacing4 = m_InputImage->GetSpacing();
    Point<float, 4> origin4 = m_InputImage->GetOrigin();
    Matrix<double, 4, 4> direction4 = m_InputImage->GetDirection();
    ImageRegion<4> imageRegion4 = m_InputImage->GetLargestPossibleRegion();

    Vector<float, 3> spacing3;
    Point<float, 3> origin3;
    Matrix<double, 3, 3> direction3;
    ImageRegion<3> imageRegion3;

    spacing3[0] = spacing4[0]; spacing3[1] = spacing4[1]; spacing3[2] = spacing4[2];
    origin3[0] = origin4[0]; origin3[1] = origin4[1]; origin3[2] = origin4[2];
    for (int r=0; r<3; r++)
        for (int c=0; c<3; c++)
            direction3[r][c] = direction4[r][c];
    imageRegion3.SetSize(0, imageRegion4.GetSize()[0]);
    imageRegion3.SetSize(1, imageRegion4.GetSize()[1]);
    imageRegion3.SetSize(2, imageRegion4.GetSize()[2]);

    double minSpacing = spacing3[0];
    if (spacing3[1]<minSpacing)
        minSpacing = spacing3[1];
    if (spacing3[2]<minSpacing)
        minSpacing = spacing3[2];

    m_DirectionImageContainer = DirectionImageContainerType::New();

    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;

    int x = m_InputImage->GetLargestPossibleRegion().GetSize(0);
    int y = m_InputImage->GetLargestPossibleRegion().GetSize(1);
    int z = m_InputImage->GetLargestPossibleRegion().GetSize(2);
    int numDirs = m_InputImage->GetLargestPossibleRegion().GetSize(3)/3;

    for (int i=0; i<numDirs; i++)
    {
        ItkDirectionImageType::Pointer directionImage = ItkDirectionImageType::New();
        directionImage->SetSpacing( spacing3 );
        directionImage->SetOrigin( origin3 );
        directionImage->SetDirection( direction3 );
        directionImage->SetRegions( imageRegion3 );
        directionImage->Allocate();
        Vector< PixelType, 3 > nullVec; nullVec.Fill(0.0);
        directionImage->FillBuffer(nullVec);

        for (int a=0; a<x; a++)
            for (int b=0; b<y; b++)
                for (int c=0; c<z; c++)
                {
                    // generate vector field
                    typename InputImageType::IndexType index;
                    index.SetElement(0,a);
                    index.SetElement(1,b);
                    index.SetElement(2,c);
                    vnl_vector<double> dirVec; dirVec.set_size(4);
                    for (int k=0; k<3; k++)
                    {
                        index.SetElement(3,k+i*3);
                        dirVec[k] = m_InputImage->GetPixel(index);
                    }
                    dirVec[3] = 0;

                    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
                    itk::ContinuousIndex<double, 4> center;
                    center[0] = index[0];
                    center[1] = index[1];
                    center[2] = index[2];
                    center[3] = 0;
                    itk::Point<double, 4> worldCenter;
                    m_InputImage->TransformContinuousIndexToPhysicalPoint( center, worldCenter );

                    switch (m_NormalizationMethod)
                    {
                    case NO_NORM:
                        break;
                    case SINGLE_VEC_NORM:
                        dirVec.normalize();
                        break;
                    }
                    dirVec.normalize();
                    dirVec = m_InputImage->GetDirection()*dirVec;

                    itk::Point<double> worldStart;
                    worldStart[0] = worldCenter[0]-dirVec[0]/2 * minSpacing;
                    worldStart[1] = worldCenter[1]-dirVec[1]/2 * minSpacing;
                    worldStart[2] = worldCenter[2]-dirVec[2]/2 * minSpacing;
                    vtkIdType id = m_VtkPoints->InsertNextPoint(worldStart.GetDataPointer());
                    container->GetPointIds()->InsertNextId(id);
                    itk::Point<double> worldEnd;
                    worldEnd[0] = worldCenter[0]+dirVec[0]/2 * minSpacing;
                    worldEnd[1] = worldCenter[1]+dirVec[1]/2 * minSpacing;
                    worldEnd[2] = worldCenter[2]+dirVec[2]/2 * minSpacing;
                    id = m_VtkPoints->InsertNextPoint(worldEnd.GetDataPointer());
                    container->GetPointIds()->InsertNextId(id);
                    m_VtkCellArray->InsertNextCell(container);

                    // generate direction image
                    typename ItkDirectionImageType::IndexType index2;
                    index2[0] = a; index2[1] = b; index2[2] = c;


                    Vector< PixelType, 3 > pixel;
                    pixel.SetElement(0, dirVec[0]);
                    pixel.SetElement(1, dirVec[1]);
                    pixel.SetElement(2, dirVec[2]);
                    directionImage->SetPixel(index2, pixel);
                }
        m_DirectionImageContainer->InsertElement(m_DirectionImageContainer->Size(), directionImage);
    }

    vtkSmartPointer<vtkPolyData> directionsPolyData = vtkSmartPointer<vtkPolyData>::New();
    directionsPolyData->SetPoints(m_VtkPoints);
    directionsPolyData->SetLines(m_VtkCellArray);
    m_OutputFiberBundle = mitk::FiberBundleX::New(directionsPolyData);
}

}

#endif // __itkMrtrixPeakImageConverter_cpp
