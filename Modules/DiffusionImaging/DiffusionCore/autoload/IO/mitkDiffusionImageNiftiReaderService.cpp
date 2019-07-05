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

#ifndef __mitkDiffusionImageNiftiReaderService_cpp
#define __mitkDiffusionImageNiftiReaderService_cpp

#include "mitkDiffusionImageNiftiReaderService.h"

#include <iostream>
#include <fstream>


// Diffusion properties
#include <mitkGradientDirectionsProperty.h>
#include <mitkBValueMapProperty.h>
#include <mitkMeasurementFrameProperty.h>
#include <mitkProperties.h>

// ITK includes
#include <itkImageRegionIterator.h>
#include <itkMetaDataObject.h>
#include "itksys/SystemTools.hxx"
#include "itkImageFileReader.h"
#include "itkMetaDataObject.h"
#include "itkNiftiImageIO.h"

#include "mitkCustomMimeType.h"
#include "mitkDiffusionCoreIOMimeTypes.h"

#include <mitkITKImageImport.h>
#include <mitkImageWriteAccessor.h>
#include <mitkImageDataItem.h>
#include <mitkLocaleSwitch.h>
#include "mitkIOUtil.h"
#include <mitkDiffusionFunctionCollection.h>
#include <vtkNIFTIImageReader.h>
#include <vtkNIFTIImageHeader.h>
#include <vtkImageIterator.h>

namespace mitk
{

DiffusionImageNiftiReaderService::
DiffusionImageNiftiReaderService(const DiffusionImageNiftiReaderService & other)
  : AbstractFileReader(other)
{
}


DiffusionImageNiftiReaderService* DiffusionImageNiftiReaderService::Clone() const
{
  return new DiffusionImageNiftiReaderService(*this);
}


DiffusionImageNiftiReaderService::
~DiffusionImageNiftiReaderService()
{}

DiffusionImageNiftiReaderService::
DiffusionImageNiftiReaderService(CustomMimeType mime_type, std::string mime_type_description ) : mitk::AbstractFileReader( mime_type, mime_type_description )
{
  Options defaultOptions;
  defaultOptions["Apply image rotation to gradients"] = true;
  this->SetDefaultOptions(defaultOptions);

  m_ServiceReg = this->RegisterService();
}

//DiffusionImageNiftiReaderService::
//DiffusionImageNiftiReaderService() : mitk::AbstractFileReader( CustomMimeType( mitk::DiffusionCoreIOMimeTypes::DWI_NIFTI_MIMETYPE() ), mitk::DiffusionCoreIOMimeTypes::DWI_NIFTI_MIMETYPE_DESCRIPTION() )
//{
//  m_ServiceReg = this->RegisterService();
//}

std::vector<itk::SmartPointer<mitk::BaseData> >
DiffusionImageNiftiReaderService::
Read()
{
  std::vector<itk::SmartPointer<mitk::BaseData> > result;

  // Since everything is completely read in GenerateOutputInformation() it is stored
  // in a cache variable. A timestamp is associated.
  // If the timestamp of the cache variable is newer than the MTime, we only need to
  // assign the cache variable to the DataObject.
  // Otherwise, the tree must be read again from the file and OuputInformation must
  // be updated!

  if(m_OutputCache.IsNull()) InternalRead();

  result.push_back(m_OutputCache.GetPointer());
  return result;
}


void DiffusionImageNiftiReaderService::InternalRead()
{
  OutputType::Pointer outputForCache = OutputType::New();
  if ( this->GetInputLocation() == "")
  {
    throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, the filename to be read is empty!");
  }
  else
  {
    try
    {
      mitk::LocaleSwitch localeSwitch("C");

      MITK_INFO << "DiffusionImageNiftiReaderService: reading image information";
      VectorImageType::Pointer itkVectorImage;

      std::string ext = this->GetMimeType()->GetExtension( this->GetInputLocation() );
      ext = itksys::SystemTools::LowerCase( ext );

      if(ext == ".fsl" || ext == ".fslgz")
      {
        // create temporary file with correct ending for nifti-io
        std::string fname3 = "temp_dwi";
        fname3 += ext == ".fsl" ? ".nii" : ".nii.gz";
        itksys::SystemTools::CopyAFile(this->GetInputLocation().c_str(), fname3.c_str());

        // create reader and read file
        typedef itk::Image<DiffusionPixelType,4> ImageType4D;
        itk::NiftiImageIO::Pointer io2 = itk::NiftiImageIO::New();
        typedef itk::ImageFileReader<ImageType4D> FileReaderType;
        FileReaderType::Pointer reader = FileReaderType::New();
        reader->SetFileName(fname3);
        reader->SetImageIO(io2);
        reader->Update();
        ImageType4D::Pointer img4 = reader->GetOutput();

        // delete temporary file
        itksys::SystemTools::RemoveFile(fname3.c_str());

        // convert 4D file to vector image
        itkVectorImage = VectorImageType::New();

        VectorImageType::SpacingType spacing;
        ImageType4D::SpacingType spacing4 = img4->GetSpacing();
        for(int i=0; i<3; i++)
          spacing[i] = spacing4[i];
        itkVectorImage->SetSpacing( spacing );   // Set the image spacing

        VectorImageType::PointType origin;
        ImageType4D::PointType origin4 = img4->GetOrigin();
        for(int i=0; i<3; i++)
          origin[i] = origin4[i];
        itkVectorImage->SetOrigin( origin );     // Set the image origin

        VectorImageType::DirectionType direction;
        ImageType4D::DirectionType direction4 = img4->GetDirection();
        for(int i=0; i<3; i++)
          for(int j=0; j<3; j++)
            direction[i][j] = direction4[i][j];
        itkVectorImage->SetDirection( direction );  // Set the image direction

        VectorImageType::RegionType region;
        ImageType4D::RegionType region4 = img4->GetLargestPossibleRegion();

        VectorImageType::RegionType::SizeType size;
        ImageType4D::RegionType::SizeType size4 = region4.GetSize();

        for(int i=0; i<3; i++)
          size[i] = size4[i];

        VectorImageType::RegionType::IndexType index;
        ImageType4D::RegionType::IndexType index4 = region4.GetIndex();
        for(int i=0; i<3; i++)
          index[i] = index4[i];

        region.SetSize(size);
        region.SetIndex(index);
        itkVectorImage->SetRegions( region );

        itkVectorImage->SetVectorLength(size4[3]);
        itkVectorImage->Allocate();

        itk::ImageRegionIterator<VectorImageType>   it ( itkVectorImage,  itkVectorImage->GetLargestPossibleRegion() );
        typedef VectorImageType::PixelType VecPixType;
        for (it.GoToBegin(); !it.IsAtEnd(); ++it)
        {
          VecPixType vec = it.Get();
          VectorImageType::IndexType currentIndex = it.GetIndex();
          for(int i=0; i<3; i++)
            index4[i] = currentIndex[i];
          for(unsigned int ind=0; ind<vec.Size(); ind++)
          {
            index4[3] = ind;
            vec[ind] = img4->GetPixel(index4);
          }
          it.Set(vec);
        }
      }
      else if(ext == ".nii" || ext == ".nii.gz")
      {
        // create reader and read file
        typedef itk::Image<DiffusionPixelType,4> ImageType4D;
        itk::NiftiImageIO::Pointer io2 = itk::NiftiImageIO::New();

        ImageType4D::Pointer img4 = ImageType4D::New();
        if (io2->CanReadFile(this->GetInputLocation().c_str()))
        {
          typedef itk::ImageFileReader<ImageType4D> FileReaderType;
          FileReaderType::Pointer reader = FileReaderType::New();
          reader->SetFileName( this->GetInputLocation() );
          reader->SetImageIO(io2);
          reader->Update();
          img4 = reader->GetOutput();
        }
        else
        {
          vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
          if (reader->CanReadFile(this->GetInputLocation().c_str()))
          {
            reader->SetFileName(this->GetInputLocation().c_str());
            reader->SetTimeAsVector(true);
            reader->Update();
            auto vtk_image = reader->GetOutput();
            auto header = reader->GetNIFTIHeader();
            auto dim = header->GetDim(0);
            auto matrix = reader->GetQFormMatrix();
            if (matrix == nullptr)
              matrix = reader->GetSFormMatrix();
            itk::Matrix<double, 4, 4> direction; direction.SetIdentity();
            itk::Matrix<double, 4, 4> ras_lps; ras_lps.SetIdentity();
            ras_lps[0][0] = -1;
            ras_lps[1][1] = -1;
            if (matrix != nullptr)
            {
              for (int r=0; r<dim; r++)
                  for (int c=0; c<dim; c++)
                      direction[r][c] = matrix->GetElement(r, c);
            }
            direction = direction*ras_lps;

            itk::Vector< double, 4 > spacing; spacing.Fill(1.0);
            vtk_image->GetSpacing(spacing.GetDataPointer());

            itk::Point< double, 4 > origin; origin.Fill(0.0);
            vtk_image->GetOrigin(origin.GetDataPointer());

            itk::ImageRegion< 4 > region;
            for (int i=0; i<4; ++i)
              if (i<dim)
                region.SetSize(i, header->GetDim(i+1));
              else
                region.SetSize(i, 1);

            img4->SetSpacing( spacing );
            img4->SetOrigin( origin );
            img4->SetDirection( direction );
            img4->SetRegions( region );
            img4->Allocate();
            img4->FillBuffer(0);

            switch (header->GetDim(0)) {
            case 4:
              for (int g=0; g<header->GetDim(4); g++)
                for (int z=0; z<header->GetDim(3); z++)
                  for (int y=0; y<header->GetDim(2); y++)
                    for (int x=0; x<header->GetDim(1); x++)
                    {
                      double val = vtk_image->GetScalarComponentAsDouble(x, y, z, g);

                      ImageType4D::IndexType idx;
                      idx[0] = x;
                      idx[1] = y;
                      idx[2] = z;
                      idx[3] = g;
                      img4->SetPixel(idx, val);
                    }
              break;
            default:
              mitkThrow() << "Image dimension " << header->GetDim(0) << " not supported!";
              break;
            }
          }
        }

        // convert 4D file to vector image
        itkVectorImage = VectorImageType::New();

        VectorImageType::SpacingType spacing;
        ImageType4D::SpacingType spacing4 = img4->GetSpacing();
        for(int i=0; i<3; i++)
          spacing[i] = spacing4[i];
        itkVectorImage->SetSpacing( spacing );   // Set the image spacing

        VectorImageType::PointType origin;
        ImageType4D::PointType origin4 = img4->GetOrigin();
        for(int i=0; i<3; i++)
          origin[i] = origin4[i];
        itkVectorImage->SetOrigin( origin );     // Set the image origin

        VectorImageType::DirectionType direction;
        ImageType4D::DirectionType direction4 = img4->GetDirection();
        for(int i=0; i<3; i++)
          for(int j=0; j<3; j++)
            direction[i][j] = direction4[i][j];
        itkVectorImage->SetDirection( direction );  // Set the image direction

        VectorImageType::RegionType region;
        ImageType4D::RegionType region4 = img4->GetLargestPossibleRegion();

        VectorImageType::RegionType::SizeType size;
        ImageType4D::RegionType::SizeType size4 = region4.GetSize();

        for(int i=0; i<3; i++)
          size[i] = size4[i];

        VectorImageType::RegionType::IndexType index;
        ImageType4D::RegionType::IndexType index4 = region4.GetIndex();
        for(int i=0; i<3; i++)
          index[i] = index4[i];

        region.SetSize(size);
        region.SetIndex(index);
        itkVectorImage->SetRegions( region );

        itkVectorImage->SetVectorLength(size4[3]);
        itkVectorImage->Allocate();

        itk::ImageRegionIterator<VectorImageType>   it ( itkVectorImage,  itkVectorImage->GetLargestPossibleRegion() );
        typedef VectorImageType::PixelType VecPixType;
        for (it.GoToBegin(); !it.IsAtEnd(); ++it)
        {
          VecPixType vec = it.Get();
          VectorImageType::IndexType currentIndex = it.GetIndex();
          for(int i=0; i<3; i++)
            index4[i] = currentIndex[i];
          for(unsigned int ind=0; ind<vec.Size(); ind++)
          {
            index4[3] = ind;
            vec[ind] = img4->GetPixel(index4);
          }
          it.Set(vec);
        }
      }

      // Diffusion Image information START
      GradientDirectionContainerType::Pointer DiffusionVectors = GradientDirectionContainerType::New();
      MeasurementFrameType MeasurementFrame;
      double BValue = -1;
      // Diffusion Image information END

      if(ext == ".fsl" || ext == ".fslgz" || ext == ".nii" || ext == ".nii.gz")
      {
        std::string base_path = itksys::SystemTools::GetFilenamePath(this->GetInputLocation());
        std::string base = this->GetMimeType()->GetFilenameWithoutExtension(this->GetInputLocation());
        if (!base_path.empty())
        {
          base = base_path + "/" + base;
          base_path += "/";
        }

        // check for possible file names
        std::string bvals_file, bvecs_file;
        if (itksys::SystemTools::FileExists(base+".bvals"))
          bvals_file = base+".bvals";
        else if (itksys::SystemTools::FileExists(base+".bval"))
          bvals_file = base+".bval";
        else if (itksys::SystemTools::FileExists(base_path+"bvals"))
          bvals_file = base_path + "bvals";
        else if (itksys::SystemTools::FileExists(base_path+"bval"))
          bvals_file = base_path + "bval";

        if (itksys::SystemTools::FileExists(std::string(base+".bvecs").c_str()))
          bvecs_file = base+".bvecs";
        else if (itksys::SystemTools::FileExists(base+".bvec"))
          bvals_file = base+".bvec";
        else if (itksys::SystemTools::FileExists(base_path+"bvecs"))
          bvecs_file = base_path + "bvecs";
        else if (itksys::SystemTools::FileExists(base_path+"bvec"))
          bvecs_file = base_path + "bvec";

        DiffusionVectors = mitk::gradients::ReadBvalsBvecs(bvals_file, bvecs_file, BValue);
        MeasurementFrame.set_identity();
      }

      outputForCache = mitk::GrabItkImageMemory( itkVectorImage);

      // create BValueMap
      mitk::BValueMapProperty::BValueMap BValueMap = mitk::BValueMapProperty::CreateBValueMap(DiffusionVectors,BValue);
      mitk::DiffusionPropertyHelper::SetOriginalGradientContainer(outputForCache, DiffusionVectors);
      mitk::DiffusionPropertyHelper::SetMeasurementFrame(outputForCache, MeasurementFrame);
      mitk::DiffusionPropertyHelper::SetBValueMap(outputForCache, BValueMap);
      mitk::DiffusionPropertyHelper::SetReferenceBValue(outputForCache, BValue);
      mitk::DiffusionPropertyHelper::SetApplyMatrixToGradients(outputForCache, us::any_cast<bool>(this->GetOptions()["Apply image rotation to gradients"]));
      mitk::DiffusionPropertyHelper::InitializeImage(outputForCache);

      // Since we have already read the tree, we can store it in a cache variable
      // so that it can be assigned to the DataObject in GenerateData();
      m_OutputCache = outputForCache;
      m_CacheTime.Modified();
    }
    catch(std::exception& e)
    {
      MITK_INFO << "Std::Exception while reading file!!";
      MITK_INFO << e.what();
      throw itk::ImageFileReaderException(__FILE__, __LINE__, e.what());
    }
    catch(...)
    {
      MITK_INFO << "Exception while reading file!!";
      throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, an error occurred while reading the requested vessel tree file!");
    }
  }
}

} //namespace MITK

#endif
