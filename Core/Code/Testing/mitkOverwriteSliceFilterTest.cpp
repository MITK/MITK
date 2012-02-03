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
#include <mitkImageCast.h>
#include <mitkGeometry3D.h>

#include <itkImage.h>
#include <itkImageRegionIterator.h>

#include <vtkImageData.h>
#include <vtkSmartPointer.h>

/*================ #BEGIN test main ================*/
int mitkOverwriteSliceFilterTest(int argc, char* argv[])
{

	MITK_TEST_BEGIN("mitkOverwriteSliceFilterTest")


		int VolumeSize = 10;

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
		
		mitk::Image::Pointer imageInMitk;
		CastToMitkImage(image, imageInMitk);



		int sliceindex = 6;//rand() % 32;
		bool isFrontside = true; 
		bool isRotated = false;
		
		mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
		plane->InitializeStandardPlane(imageInMitk->GetGeometry(), mitk::PlaneGeometry::Transversal, sliceindex, isFrontside, isRotated);
		mitk::Point3D origin = plane->GetOrigin();
		mitk::Vector3D normal;
		normal = plane->GetNormal();		
		normal.Normalize();		
		origin += normal * 0.5;//pixelspacing is 1, so half the spacing is 0.5		
		plane->SetOrigin(origin);




		mitk::ExtractSliceFilter::Pointer slicer = mitk::ExtractSliceFilter::New();
		slicer->SetInput(imageInMitk);
		slicer->SetWorldGeometry(plane);
		slicer->SetVtkOutputRequest(true);
		slicer->Modified();
		slicer->Update();

		vtkSmartPointer<vtkImageData> slice = vtkSmartPointer<vtkImageData>::New();
		slice = slicer->GetVtkOutput();



		vtkSmartPointer<mitkVtkImageMapReslice> resliceMap = vtkSmartPointer<mitkVtkImageMapReslice>::New();
		mitk::ExtractSliceFilter::Pointer slicerMap = mitk::ExtractSliceFilter::New(resliceMap);
		slicerMap->SetInput(imageInMitk);
		slicerMap->SetWorldGeometry(plane);
		slicerMap->SetVtkOutputRequest(true);
		slicerMap->Modified();
		slicerMap->Update();

		vtkSmartPointer<vtkImageData> map = vtkSmartPointer<vtkImageData>::New();
		map = slicerMap->GetVtkOutput();



		mitk::OverwriteSliceFilter::Pointer overwriter = mitk::OverwriteSliceFilter::New();
		overwriter->SetInput(imageInMitk);
		overwriter->SetInputSlice(slice);
		overwriter->SetInputMap(map);
		overwriter->Modified();
		overwriter->Update();

		mitk::Image::Pointer overwritedImage = overwriter->GetOutput();

		bool areSame = true;
		mitk::Index3D id;
		id[0] = id[1] = id[2] = 0;
		for (int x = 0; x < VolumeSize; ++x){
			id[0]  = x;
			for (int y = 0; y < VolumeSize; ++y){
				id[1] = y;
				for (int z = 0; z < VolumeSize; ++z){
					id[2] = z;
					areSame = imageInMitk->GetPixelValueByIndex(id) == overwritedImage->GetPixelValueByIndex(id);
					if(!areSame)
						goto stop;
				}
			}
		}
stop:
		MITK_TEST_CONDITION(areSame,"comparing images");



		slice->SetScalarComponentFromDouble(2,3,4,0,0.0);
		overwriter->SetInput(overwritedImage);
		overwriter->SetInputSlice(slice);
		overwriter->Modified();
		overwriter->Update();

		mitk::Image::Pointer overwr = overwriter->GetOutput();

		areSame = true;
		
		int x,y,z;
		
		for ( x = 0; x < VolumeSize; ++x){
			id[0]  = x;
			for ( y = 0; y < VolumeSize; ++y){
				id[1] = y;
				for ( z = 0; z < VolumeSize; ++z){
					id[2] = z;
					MITK_INFO << imageInMitk->GetPixelValueByIndex(id);
					MITK_INFO << overwr->GetPixelValueByIndex(id);
					areSame = imageInMitk->GetPixelValueByIndex(id) == overwr->GetPixelValueByIndex(id);
					if(!areSame)
						goto stop2;
				}
			}
		}
stop2:
		MITK_INFO << x << " " << y << " " << z; 
		MITK_TEST_CONDITION(x==2 && y==3 && z==4,"overwrited the right index");


	MITK_TEST_END()
}
