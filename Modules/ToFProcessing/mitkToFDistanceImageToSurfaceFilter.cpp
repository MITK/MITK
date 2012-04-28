/*=========================================================================
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkToFDistanceImageToSurfaceFilter.h>
#include <mitkInstantiateAccessFunctions.h>
#include <mitkSurface.h>

#include <itkImage.h>

#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkPolyDataNormals.h>
#include <vtkCleanPolyData.h>
#include <vtkSmartPointer.h>

#include <math.h>

mitk::ToFDistanceImageToSurfaceFilter::ToFDistanceImageToSurfaceFilter() :
    m_IplScalarImage(NULL), m_CameraIntrinsics(), m_TextureImageWidth(0), m_TextureImageHeight(0), m_InterPixelDistance(), m_TextureIndex(0)
{
    //intialize these variables just once and not in every run of the filter!
    m_InterPixelDistance.Fill(0.045);
    m_CameraIntrinsics = mitk::CameraIntrinsics::New();
    m_CameraIntrinsics->SetFocalLength(295.78960196187319,296.1255427948447);
    m_CameraIntrinsics->SetPrincipalPoint(113.29063841714108,97.243216122015184);
    m_CameraIntrinsics->SetDistorsionCoeffs(-0.36874385358645773f,-0.14339503290129013,0.0033210108720361795,-0.004277703352074105);
    m_Mesh = vtkSmartPointer<vtkPolyData>::New();
    m_Points = vtkSmartPointer<vtkPoints>::New();
    m_Points->SetDataTypeToDouble();
    m_Polys = vtkSmartPointer<vtkCellArray>::New();
}

mitk::ToFDistanceImageToSurfaceFilter::~ToFDistanceImageToSurfaceFilter()
{
}

void mitk::ToFDistanceImageToSurfaceFilter::SetInput( Image* distanceImage, mitk::CameraIntrinsics::Pointer cameraIntrinsics )
{
    this->SetCameraIntrinsics(cameraIntrinsics);
    this->SetInput(0,distanceImage);
}

void mitk::ToFDistanceImageToSurfaceFilter::SetInput( unsigned int idx,  Image* distanceImage, mitk::CameraIntrinsics::Pointer cameraIntrinsics )
{
    this->SetCameraIntrinsics(cameraIntrinsics);
    this->SetInput(idx,distanceImage);
}

void mitk::ToFDistanceImageToSurfaceFilter::SetInput(  mitk::Image* distanceImage )
{
    this->SetInput(0,distanceImage);
}

//TODO: braucht man diese Methode?
void mitk::ToFDistanceImageToSurfaceFilter::SetInput( unsigned int idx,  mitk::Image* distanceImage )
{
    if ((distanceImage == NULL) && (idx == this->GetNumberOfInputs() - 1)) // if the last input is set to NULL, reduce the number of inputs by one
        this->SetNumberOfInputs(this->GetNumberOfInputs() - 1);
    else
        this->ProcessObject::SetNthInput(idx, distanceImage);   // Process object is not const-correct so the const_cast is required here

    this->CreateOutputsForAllInputs();
}

mitk::Image* mitk::ToFDistanceImageToSurfaceFilter::GetInput()
{
    return this->GetInput(0);
}

mitk::Image* mitk::ToFDistanceImageToSurfaceFilter::GetInput( unsigned int idx )
{
    if (this->GetNumberOfInputs() < 1)
        return NULL; //TODO: geeignete exception werfen
    return static_cast< mitk::Image*>(this->ProcessObject::GetInput(idx));
}

void mitk::ToFDistanceImageToSurfaceFilter::GenerateData()
{
    mitk::Surface::Pointer output = this->GetOutput();
    assert(output);
    mitk::Image::Pointer input = this->GetInput();
    assert(input);
    int xDimension = input->GetDimension(0);
    int yDimension = input->GetDimension(1);
    unsigned int size = xDimension*yDimension; //size of the image-array
    std::vector<bool> isPointValid;
    isPointValid.resize(size);
    vtkSmartPointer<vtkFloatArray> scalarArray = vtkSmartPointer<vtkFloatArray>::New();
    vtkSmartPointer<vtkFloatArray> textureCoords = vtkSmartPointer<vtkFloatArray>::New();
    textureCoords->SetNumberOfComponents(2);

    float textureScaleCorrection1 = 0.0;
    float textureScaleCorrection2 = 0.0;
    if (this->m_TextureImageHeight > 0.0 && this->m_TextureImageWidth > 0.0)
    {
        textureScaleCorrection1 = float(this->m_TextureImageHeight) / float(this->m_TextureImageWidth);
        textureScaleCorrection2 = ((float(this->m_TextureImageWidth) - float(this->m_TextureImageHeight))/2) / float(this->m_TextureImageWidth);
    }

    float* scalarFloatData = NULL;

    if (this->m_IplScalarImage) // if scalar image is defined use it for texturing
    {
        scalarFloatData = (float*)this->m_IplScalarImage->imageData;
    }
    else if (this->GetInput(m_TextureIndex)) // otherwise use intensity image (input(2))
    {
        scalarFloatData = (float*)this->GetInput(m_TextureIndex)->GetData();
    }

    float* inputFloatData = (float*)(input->GetData());
    //calculate world coordinates. *0.5 is faster then /2.
    mitk::ToFProcessingCommon::ToFScalarType focalLength = (m_CameraIntrinsics->GetFocalLengthX()*m_InterPixelDistance[0]+m_CameraIntrinsics->GetFocalLengthY()*m_InterPixelDistance[1])*0.5;
    mitk::ToFProcessingCommon::ToFPoint2D principalPoint;
    principalPoint[0] = m_CameraIntrinsics->GetPrincipalPointX();
    principalPoint[1] = m_CameraIntrinsics->GetPrincipalPointY();

    //array to save the distance values corresponding to 1 quad
    mitk::ToFProcessingCommon::ToFScalarType lastDistances[4] = {0.0f, 0.0f, 0.0f, 0.0f};


    for (int j=0; j<yDimension; j++)
    {
        for (int i=0; i<xDimension; i++)
        {
            // distance value
            unsigned int pixelID = i+j*xDimension;

            mitk::ToFProcessingCommon::ToFScalarType distance = inputFloatData[pixelID];

            mitk::ToFProcessingCommon::ToFPoint3D cartesianCoordinates =
                    mitk::ToFProcessingCommon::IndexToCartesianCoordinates(i,j,distance,focalLength,m_InterPixelDistance,principalPoint);

            //Test if the distance is (almost) 0.0 or smaller, because we want to exclude 0 and negative values.
            if (distance<=mitk::eps)
            {
                isPointValid[pixelID] = false;
            }
            else
            {
                isPointValid[pixelID] = true;

                //expensive call, because of memory allocation of VTK
                m_Points->InsertPoint(pixelID, cartesianCoordinates.GetDataPointer());



                if((i >= 1) && (j >= 1))
                {
                    lastDistances[2] = lastDistances[3]; //(i-1,j)
                    lastDistances[3] = cartesianCoordinates[2]; // = (i,j)
                    //save as much arithmetic as possible
                    vtkIdType xy = pixelID;
                    vtkIdType x_1y = pixelID-1;
                    vtkIdType xy_1 = i+(j-1)*xDimension;
                    vtkIdType x_1y_1 = xy_1-1;

                    //Delaunay-like triangulation.
                    //The edge is placed such that the difference between opposing points is minimized.
                    if (isPointValid[xy]&&isPointValid[x_1y]&&isPointValid[x_1y_1]&&isPointValid[xy_1])
                    {
                        //Check which two corner-points have the smallest difference
                        lastDistances[0] = inputFloatData[x_1y_1];
                        lastDistances[1] = inputFloatData[xy_1];
                        if( fabs(lastDistances[3] - lastDistances[0]) < fabs(lastDistances[1] - lastDistances[2]) )
                        {
                            //Edge: left down to top right
                            //expensive call, because of memory allocation of VTK
                            m_Polys->InsertNextCell(3);
                            m_Polys->InsertCellPoint(xy);
                            m_Polys->InsertCellPoint(xy_1);
                            m_Polys->InsertCellPoint(x_1y_1);
                            m_Polys->InsertNextCell(3);
                            m_Polys->InsertCellPoint(xy);
                            m_Polys->InsertCellPoint(x_1y);
                            m_Polys->InsertCellPoint(x_1y_1);
                        }
                        else
                        {
                            //Edge: top to down right
                            //expensive call, because of memory allocation of VTK
                            m_Polys->InsertNextCell(3);
                            m_Polys->InsertCellPoint(xy_1);
                            m_Polys->InsertCellPoint(x_1y);
                            m_Polys->InsertCellPoint(xy);
                            m_Polys->InsertNextCell(3);
                            m_Polys->InsertCellPoint(xy_1);
                            m_Polys->InsertCellPoint(x_1y_1);
                            m_Polys->InsertCellPoint(x_1y);
                        }
                    }
                }
                else if(i == 0)
                {
                    lastDistances[3] = cartesianCoordinates[2]; // = (i,j)
                }

                if (scalarFloatData)
                {
                    scalarArray->InsertTuple1(pixelID, scalarFloatData[pixelID]);
                }
                if (this->m_TextureImageHeight > 0.0 && this->m_TextureImageWidth > 0.0)
                {

                    float xNorm = (((float)i)/xDimension)*textureScaleCorrection1 + textureScaleCorrection2 ; // correct video texture scale 640 * 480!!
                    float yNorm = 1.0 - ((float)j)/yDimension; //flip y-axis
                    textureCoords->InsertTuple2(pixelID, xNorm, yNorm);
                }
            }
        }
    }

    m_Mesh->SetPoints(m_Points);
    m_Mesh->SetPolys(m_Polys);
    if (scalarArray->GetNumberOfTuples()>0)
    {
        m_Mesh->GetPointData()->SetScalars(scalarArray);
        if (this->m_TextureImageHeight > 0.0 && this->m_TextureImageWidth > 0.0)
        {
            m_Mesh->GetPointData()->SetTCoords(textureCoords);
        }
    }
    output->SetVtkPolyData(m_Mesh);
}

void mitk::ToFDistanceImageToSurfaceFilter::CreateOutputsForAllInputs()
{
    this->SetNumberOfOutputs(this->GetNumberOfInputs());  // create outputs for all inputs
    for (unsigned int idx = 0; idx < this->GetNumberOfOutputs(); ++idx)
        if (this->GetOutput(idx) == NULL)
        {
            DataObjectPointer newOutput = this->MakeOutput(idx);
            this->SetNthOutput(idx, newOutput);
        }
    this->Modified();
}

void mitk::ToFDistanceImageToSurfaceFilter::GenerateOutputInformation()
{
    this->GetOutput();
    itkDebugMacro(<<"GenerateOutputInformation()");
}

void mitk::ToFDistanceImageToSurfaceFilter::SetScalarImage(IplImage* iplScalarImage)
{
    this->m_IplScalarImage = iplScalarImage;
    this->Modified();
}

IplImage* mitk::ToFDistanceImageToSurfaceFilter::GetScalarImage()
{
    return this->m_IplScalarImage;
}

void mitk::ToFDistanceImageToSurfaceFilter::SetTextureImageWidth(int width)
{
    this->m_TextureImageWidth = width;
}

void mitk::ToFDistanceImageToSurfaceFilter::SetTextureImageHeight(int height)
{
    this->m_TextureImageHeight = height;
}
