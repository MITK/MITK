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




int VolumeSize = 128;




/*================ #BEGIN test main ================*/
int mitkOverwriteSliceFilterTest(int, char* [])
{

  MITK_TEST_BEGIN("mitkOverwriteSliceFilterTest")




    typedef itk::Image<unsigned short, 3> ImageType;

    typedef itk::ImageRegionConstIterator< ImageType > ImageIterator;

    ImageType::Pointer image = ImageType::New();

    ImageType::IndexType start;
    start[0] = start[1] = start[2] = 0;

    ImageType::SizeType size;
    size[0] = size[1] = size[2] = VolumeSize;

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



    mitk::Image::Pointer referenceImage;
    CastToMitkImage(image, referenceImage);
    mitk::Image::Pointer workingImage;
    CastToMitkImage(image, workingImage);

    typedef mitk::ImagePixelReadAccessor< unsigned short, 3 > ReadAccessorType;
    ReadAccessorType refImgReadAccessor( referenceImage );
    ReadAccessorType workingImgReadAccessor( workingImage );


  /* ============= setup plane ============*/
  int sliceindex = 55;//rand() % 32;
  bool isFrontside = true;
  bool isRotated = false;

  mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
  plane->InitializeStandardPlane(workingImage->GetGeometry(), mitk::PlaneGeometry::Axial, sliceindex, isFrontside, isRotated);
  mitk::Point3D origin = plane->GetOrigin();
  mitk::Vector3D normal;
  normal = plane->GetNormal();
  normal.Normalize();
  origin += normal * 0.5;//pixelspacing is 1, so half the spacing is 0.5
  plane->SetOrigin(origin);



  /* ============= extract slice ============*/
  vtkSmartPointer<mitkVtkImageOverwrite> resliceIdx = vtkSmartPointer<mitkVtkImageOverwrite>::New();
  mitk::ExtractSliceFilter::Pointer slicer = mitk::ExtractSliceFilter::New(resliceIdx);
  slicer->SetInput(workingImage);
  slicer->SetWorldGeometry(plane);
  slicer->SetVtkOutputRequest(true);
  slicer->Modified();
  slicer->Update();

  vtkSmartPointer<vtkImageData> slice = vtkSmartPointer<vtkImageData>::New();
  slice = slicer->GetVtkOutput();



  /* ============= overwrite slice ============*/
  resliceIdx->SetOverwriteMode(true);
  resliceIdx->Modified();
  slicer->Modified();
  slicer->Update();//implicit overwrite



  /* ============= check ref == working ============*/
  bool areSame = true;
  itk::Index<3> id;
  id[0] = id[1] = id[2] = 0;
  for (int x = 0; x < VolumeSize; ++x){
    id[0]  = x;
    for (int y = 0; y < VolumeSize; ++y){
      id[1] = y;
      for (int z = 0; z < VolumeSize; ++z){
        id[2] = z;
        areSame = refImgReadAccessor.GetPixelByIndex( id ) == workingImgReadAccessor.GetPixelByIndex( id );
        if(!areSame)
          goto stop;
      }
    }
  }
stop:
  MITK_TEST_CONDITION(areSame,"test overwrite unmodified slice");



  /* ============= edit slice ============*/
  int idX = std::abs(VolumeSize-59);
  int idY = std::abs(VolumeSize-23);
  int idZ = 0;
  int component = 0;
  double val = 33.0;

  slice->SetScalarComponentFromDouble(idX,idY,idZ,component,val);



  /* ============= overwrite slice ============*/

  vtkSmartPointer<mitkVtkImageOverwrite> resliceIdx2 = vtkSmartPointer<mitkVtkImageOverwrite>::New();
  resliceIdx2->SetOverwriteMode(true);
  resliceIdx2->SetInputSlice(slice);
  mitk::ExtractSliceFilter::Pointer slicer2 = mitk::ExtractSliceFilter::New(resliceIdx2);
  slicer2->SetInput(workingImage);
  slicer2->SetWorldGeometry(plane);
  slicer2->SetVtkOutputRequest(true);
  slicer2->Modified();
  slicer2->Update();




  /* ============= check ============*/
  areSame = true;

  int xx,yy,zz;

  for ( xx = 0; xx < VolumeSize; ++xx){
    id[0]  = xx;
    for ( yy = 0; yy < VolumeSize; ++yy){
      id[1] = yy;
      for ( zz = 0; zz < VolumeSize; ++zz){
        id[2] = zz;
        areSame = refImgReadAccessor.GetPixelByIndex( id ) == workingImgReadAccessor.GetPixelByIndex( id );
        if(!areSame)
          goto stop2;
      }
    }
  }
stop2:
  //MITK_INFO << "index: [" << x << ", " << y << ", " << z << "]";
  MITK_TEST_CONDITION(xx==idX && yy==idY && zz==sliceindex,"test overwrite modified slice");


  MITK_TEST_END()
}
