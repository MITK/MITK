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
#include <mitkTestingMacros.h>

#include <mitkExtractSliceFilter.h>
#include <mitkVtkImageOverwrite.h>
#include <mitkImageCast.h>
#include <mitkGeometry3D.h>
#include <mitkRotationOperation.h>
#include <mitkInteractionConst.h>
#include <mitkImagePixelReadAccessor.h>

#include <itkImage.h>
#include <itkImageRegionIterator.h>

#include <vtkImageData.h>
#include <vtkSmartPointer.h>




int ObliquePlaneTestVolumeSize = 128;




static void OverwriteObliquePlaneTest(mitk::Image* workingImage, mitk::Image* refImg)
{

/*==============TEST WITHOUT MITK CONVERTION=============================*/

    /* ============= setup plane ============*/
  int sliceindex = (int)(ObliquePlaneTestVolumeSize /2);//rand() % 32;
  bool isFrontside = true;
  bool isRotated = false;

  mitk::PlaneGeometry::Pointer obliquePlane = mitk::PlaneGeometry::New();
  obliquePlane->InitializeStandardPlane(workingImage->GetGeometry(), mitk::PlaneGeometry::Axial, sliceindex, isFrontside, isRotated);
  mitk::Point3D origin = obliquePlane->GetOrigin();
  mitk::Vector3D normal;
  normal = obliquePlane->GetNormal();
  normal.Normalize();
  origin += normal * 0.5;//pixelspacing is 1, so half the spacing is 0.5
  obliquePlane->SetOrigin(origin);

  mitk::Vector3D rotationVector = obliquePlane->GetAxisVector(0);
  rotationVector.Normalize();

  float degree = 45.0;

  mitk::RotationOperation* op = new mitk::RotationOperation(mitk::OpROTATE, obliquePlane->GetCenter(), rotationVector, degree);
  obliquePlane->ExecuteOperation(op);
  delete op;


  /* ============= extract slice ============*/
  mitk::ExtractSliceFilter::Pointer slicer = mitk::ExtractSliceFilter::New();
  slicer->SetInput(workingImage);
  slicer->SetWorldGeometry(obliquePlane);
  slicer->SetVtkOutputRequest(true);
  slicer->Modified();
  slicer->Update();

  vtkSmartPointer<vtkImageData> slice = vtkSmartPointer<vtkImageData>::New();
  slice = slicer->GetVtkOutput();



  /* ============= overwrite slice ============*/
  vtkSmartPointer<mitkVtkImageOverwrite> resliceIdx = vtkSmartPointer<mitkVtkImageOverwrite>::New();
  mitk::ExtractSliceFilter::Pointer overwriter = mitk::ExtractSliceFilter::New(resliceIdx);
  resliceIdx->SetOverwriteMode(true);
  resliceIdx->SetInputSlice(slice);
  resliceIdx->Modified();
  overwriter->SetInput(workingImage);
  overwriter->SetWorldGeometry(obliquePlane);
  overwriter->SetVtkOutputRequest(true);
  overwriter->Modified();
  overwriter->Update();

  typedef mitk::ImagePixelReadAccessor< unsigned short, 3 > ReadAccessorType;
  ReadAccessorType refImgReadAccessor( refImg );
  ReadAccessorType workingImgReadAccessor( workingImage );



  /* ============= check ref == working ============*/
  bool areSame = true;
  itk::Index<3> id;
  id[0] = id[1] = id[2] = 0;
  for (int x = 0; x < ObliquePlaneTestVolumeSize ; ++x){
    id[0]  = x;
    for (int y = 0; y < ObliquePlaneTestVolumeSize ; ++y){
      id[1] = y;
      for (int z = 0; z < ObliquePlaneTestVolumeSize ; ++z){
        id[2] = z;
        areSame = refImgReadAccessor.GetPixelByIndex(id) == workingImgReadAccessor.GetPixelByIndex(id);
        if(!areSame)
          goto stop;
      }
    }
  }
stop:
  MITK_TEST_CONDITION(areSame,"comparing images (no mitk convertion) [oblique]");





/*==============TEST WITH MITK CONVERTION=============================*/

    /* ============= extract slice ============*/
  mitk::ExtractSliceFilter::Pointer slicer2 = mitk::ExtractSliceFilter::New();
  slicer2->SetInput(workingImage);
  slicer2->SetWorldGeometry(obliquePlane);
  slicer2->Modified();
  slicer2->Update();


  mitk::Image::Pointer sliceInMitk = slicer2->GetOutput();
  vtkSmartPointer<vtkImageData> slice2 = vtkSmartPointer<vtkImageData>::New();
  slice2 = sliceInMitk->GetVtkImageData();


  /* ============= overwrite slice ============*/
  vtkSmartPointer<mitkVtkImageOverwrite> resliceIdx2 = vtkSmartPointer<mitkVtkImageOverwrite>::New();
  mitk::ExtractSliceFilter::Pointer overwriter2 = mitk::ExtractSliceFilter::New(resliceIdx2);
  resliceIdx2->SetOverwriteMode(true);
  resliceIdx2->SetInputSlice(slice2);
  resliceIdx2->Modified();
  overwriter2->SetInput(workingImage);
  overwriter2->SetWorldGeometry(obliquePlane);
  overwriter2->SetVtkOutputRequest(true);
  overwriter2->Modified();
  overwriter2->Update();



  /* ============= check ref == working ============*/
  areSame = true;
  id[0] = id[1] = id[2] = 0;
  for (int x = 0; x < ObliquePlaneTestVolumeSize ; ++x){
    id[0]  = x;
    for (int y = 0; y < ObliquePlaneTestVolumeSize ; ++y){
      id[1] = y;
      for (int z = 0; z < ObliquePlaneTestVolumeSize ; ++z){
        id[2] = z;
        areSame = refImgReadAccessor.GetPixelByIndex(id) == workingImgReadAccessor.GetPixelByIndex(id);
        if(!areSame)
          goto stop2;
      }
    }
  }
stop2:
  MITK_TEST_CONDITION(areSame,"comparing images (with mitk convertion) [oblique]");


/*==============TEST EDIT WITHOUT MITK CONVERTION=============================*/

  /* ============= edit slice ============*/
  int idX = std::abs(ObliquePlaneTestVolumeSize -59);
  int idY = std::abs(ObliquePlaneTestVolumeSize -23);
  int idZ = 0;
  int component = 0;
  double val = 33.0;

  slice->SetScalarComponentFromDouble(idX,idY,idZ,component,val);

  mitk::Vector3D indx;
  indx[0] = idX; indx[1] = idY; indx[2] = idZ;
  sliceInMitk->GetGeometry()->IndexToWorld(indx, indx);

  /* ============= overwrite slice ============*/

  vtkSmartPointer<mitkVtkImageOverwrite> resliceIdx3 = vtkSmartPointer<mitkVtkImageOverwrite>::New();
  resliceIdx3->SetOverwriteMode(true);
  resliceIdx3->SetInputSlice(slice);
  mitk::ExtractSliceFilter::Pointer overwriter3 = mitk::ExtractSliceFilter::New(resliceIdx3);
  overwriter3->SetInput(workingImage);
  overwriter3->SetWorldGeometry(obliquePlane);
  overwriter3->SetVtkOutputRequest(true);
  overwriter3->Modified();
  overwriter3->Update();




  /* ============= check ============*/
  areSame = true;

  int x,y,z;

  for ( x = 0; x < ObliquePlaneTestVolumeSize ; ++x){
    id[0]  = x;
    for ( y = 0; y < ObliquePlaneTestVolumeSize ; ++y){
      id[1] = y;
      for ( z = 0; z < ObliquePlaneTestVolumeSize ; ++z){
        id[2] = z;
        areSame = refImgReadAccessor.GetPixelByIndex(id) == workingImgReadAccessor.GetPixelByIndex(id);
        if(!areSame)
          goto stop3;
      }
    }
  }
stop3:
  //MITK_INFO << "index: [" << x << ", " << y << ", " << z << "]";
  //MITK_INFO << indx;
  MITK_TEST_CONDITION(x==idX && y==z,"overwrited the right index [oblique]");
}




/*================ #BEGIN test main ================*/
int mitkOverwriteSliceFilterObliquePlaneTest(int , char* [])
{

  MITK_TEST_BEGIN("mitkOverwriteSliceFilterObliquePlaneTest")




    typedef itk::Image<unsigned short, 3> ImageType;

    typedef itk::ImageRegionConstIterator< ImageType > ImageIterator;

    ImageType::Pointer image = ImageType::New();

    ImageType::IndexType start;
    start[0] = start[1] = start[2] = 0;

    ImageType::SizeType size;
    size[0] = size[1] = size[2] = ObliquePlaneTestVolumeSize ;

    ImageType::RegionType imgRegion;
    imgRegion.SetSize(size);
    imgRegion.SetIndex(start);

    image->SetRegions(imgRegion);
    image->SetSpacing(1.0);
    image->Allocate();


    ImageIterator imageIterator( image, image->GetLargestPossibleRegion() );
    imageIterator.GoToBegin();


    unsigned short pixelValue = 0;

    //fill the image with distinct values
    while ( !imageIterator.IsAtEnd() )
    {
      image->SetPixel(imageIterator.GetIndex(), pixelValue);
      ++imageIterator;
      ++pixelValue;
    }
    /* end setup itk image */



    mitk::Image::Pointer refImage;
    CastToMitkImage(image, refImage);
    mitk::Image::Pointer workingImg;
    CastToMitkImage(image, workingImg);
    OverwriteObliquePlaneTest(workingImg, refImage);


  MITK_TEST_END()
}
