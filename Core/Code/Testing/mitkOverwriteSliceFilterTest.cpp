/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include <mitkTestingMacros.h>

#include <mitkOverwriteSliceFilter.h>
#include <mitkExtractSliceFilter.h>
#include <mitkVtkImageMapReslice.h>
#include <mitkVtkImageIdxReslice.h>
#include <mitkImageCast.h>
#include <mitkGeometry3D.h>

#include <itkImage.h>
#include <itkImageRegionIterator.h>

#include <vtkImageData.h>
#include <vtkSmartPointer.h>



int VolumeSize = 128;



static void OverwriteByIndexShiftTest(mitk::Image* workingImage, mitk::Image* refImg)
{
	/* ============= setup plane ============*/
	int sliceindex = 55;//rand() % 32;
	bool isFrontside = true; 
	bool isRotated = false;

	mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
	plane->InitializeStandardPlane(workingImage->GetGeometry(), mitk::PlaneGeometry::Transversal, sliceindex, isFrontside, isRotated);
	mitk::Point3D origin = plane->GetOrigin();
	mitk::Vector3D normal;
	normal = plane->GetNormal();		
	normal.Normalize();		
	origin += normal * 0.5;//pixelspacing is 1, so half the spacing is 0.5		
	plane->SetOrigin(origin);


	
	/* ============= extract slice ============*/
	mitk::ExtractSliceFilter::Pointer slicer = mitk::ExtractSliceFilter::New();
	slicer->SetInput(workingImage);
	slicer->SetWorldGeometry(plane);
	slicer->SetVtkOutputRequest(true);
	slicer->Modified();
	slicer->Update();

	vtkSmartPointer<vtkImageData> slice = vtkSmartPointer<vtkImageData>::New();
	slice = slicer->GetVtkOutput();




  /* ============= extract map ============*/
	vtkSmartPointer<mitkVtkImageMapReslice> resliceMap = vtkSmartPointer<mitkVtkImageMapReslice>::New();
	mitk::ExtractSliceFilter::Pointer slicerMap = mitk::ExtractSliceFilter::New(resliceMap);
	slicerMap->SetInput(workingImage);
	slicerMap->SetWorldGeometry(plane);
	slicerMap->SetVtkOutputRequest(true);
	slicerMap->Modified();
	slicerMap->Update();

	vtkSmartPointer<vtkImageData> map = vtkSmartPointer<vtkImageData>::New();
	map = slicerMap->GetVtkOutput();
	MITK_TEST_CONDITION_REQUIRED( map->GetScalarSize() == sizeof(int), "min size of scalar");

	/* ============= overwrite slice ============*/
	mitk::OverwriteSliceFilter::Pointer overwriter = mitk::OverwriteSliceFilter::New();
	overwriter->SetInput(workingImage);
	overwriter->SetInputMap(map);
	overwriter->SetInputSlice(slice);
	overwriter->Modified();
	overwriter->Update();


	MITK_INFO<<slice->GetScalarType();
	MITK_INFO<<map->GetScalarType();


	/* ============= check ref == working ============*/
	bool areSame = true;
	mitk::Index3D id;
	id[0] = id[1] = id[2] = 0;
	for (int x = 0; x < VolumeSize; ++x){
		id[0]  = x;
		for (int y = 0; y < VolumeSize; ++y){
			id[1] = y;
			for (int z = 0; z < VolumeSize; ++z){
				id[2] = z;					
				areSame = refImg->GetPixelValueByIndex(id) == workingImage->GetPixelValueByIndex(id);
				if(!areSame)
					goto stop;
			}
		}
	}
stop:
	MITK_TEST_CONDITION(areSame,"comparing images");



	/* ============= edit slice ============*/
	int idX = VolumeSize-20;
	int idY = VolumeSize-66;
	int idZ = 0;
	int component = 0;
	double val = 33.0;

	slice->SetScalarComponentFromDouble(idX,idY,idZ,component,val);



	/* ============= overwrite slice ============*/
	overwriter->Modified();
	overwriter->Update();


	/* ============= check ============*/
	areSame = true;

	int x,y,z;

	for ( x = 0; x < VolumeSize; ++x){
		id[0]  = x;
		for ( y = 0; y < VolumeSize; ++y){
			id[1] = y;
			for ( z = 0; z < VolumeSize; ++z){
				id[2] = z;					
				areSame = refImg->GetPixelValueByIndex(id) == workingImage->GetPixelValueByIndex(id);
				if(!areSame)
					goto stop2;
			}
		}
	}
stop2:
	MITK_INFO << "index: [" << x << ", " << y << ", " << z << "]"; 
	MITK_TEST_CONDITION(x==idX && y==idY && z==sliceindex,"overwrited the right index");

}



static void OverwriteByPointerTest(mitk::Image* workingImage, mitk::Image* refImg)
{

	/* ============= setup plane ============*/
	int sliceindex = 55;//rand() % 32;
	bool isFrontside = true; 
	bool isRotated = false;

	mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
	plane->InitializeStandardPlane(workingImage->GetGeometry(), mitk::PlaneGeometry::Transversal, sliceindex, isFrontside, isRotated);
	mitk::Point3D origin = plane->GetOrigin();
	mitk::Vector3D normal;
	normal = plane->GetNormal();		
	normal.Normalize();		
	origin += normal * 0.5;//pixelspacing is 1, so half the spacing is 0.5		
	plane->SetOrigin(origin);



	/* ============= extract slice ============*/
	vtkSmartPointer<mitkVtkImageIdxReslice> resliceIdx = vtkSmartPointer<mitkVtkImageIdxReslice>::New();
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
	mitk::Index3D id;
	id[0] = id[1] = id[2] = 0;
	for (int x = 0; x < VolumeSize; ++x){
		id[0]  = x;
		for (int y = 0; y < VolumeSize; ++y){
			id[1] = y;
			for (int z = 0; z < VolumeSize; ++z){
				id[2] = z;					
				areSame = refImg->GetPixelValueByIndex(id) == workingImage->GetPixelValueByIndex(id);
				if(!areSame)
					goto stop;
			}
		}
	}
stop:
	MITK_TEST_CONDITION(areSame,"comparing images");



	/* ============= edit slice ============*/
	int idX = VolumeSize-20;
	int idY = VolumeSize-66;
	int idZ = 0;
	int component = 0;
	double val = 33.0;

	slice->SetScalarComponentFromDouble(idX,idY,idZ,component,val);



	/* ============= overwrite slice ============*/

	vtkSmartPointer<mitkVtkImageIdxReslice> resliceIdx2 = vtkSmartPointer<mitkVtkImageIdxReslice>::New();
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

	int x,y,z;

	for ( x = 0; x < VolumeSize; ++x){
		id[0]  = x;
		for ( y = 0; y < VolumeSize; ++y){
			id[1] = y;
			for ( z = 0; z < VolumeSize; ++z){
				id[2] = z;					
				areSame = refImg->GetPixelValueByIndex(id) == workingImage->GetPixelValueByIndex(id);
				if(!areSame)
					goto stop2;
			}
		}
	}
stop2:
	MITK_INFO << "index: [" << x << ", " << y << ", " << z << "]"; 
	MITK_TEST_CONDITION(x==idX && y==idY && z==sliceindex,"overwrited the right index");
}






/*================ #BEGIN test main ================*/
int mitkOverwriteSliceFilterTest(int argc, char* argv[])
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
		


		mitk::Image::Pointer refImage;
		CastToMitkImage(image, refImage);
		mitk::Image::Pointer workingImg;
		CastToMitkImage(image, workingImg);
		OverwriteByPointerTest(workingImg, refImage);


		mitk::Image::Pointer refImage2;
		CastToMitkImage(image, refImage2);
		mitk::Image::Pointer workingImg2;
		CastToMitkImage(image, workingImg2);
		OverwriteByIndexShiftTest(workingImg2, refImage2);



	MITK_TEST_END()
}
