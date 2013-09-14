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

#include "mitkNrrdTensorImageReader.h"

#include "itkImageFileReader.h"
#include "itkImageRegionIterator.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include <itkNiftiImageIO.h>

#include "mitkITKImageImport.h"
#include "mitkImageDataItem.h"

namespace mitk
{

void NrrdTensorImageReader
::GenerateData()
{
    if ( m_FileName == "")
    {
        throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, the filename is empty!");
    }
    else
    {
        try
        {
            const std::string& locale = "C";
            const std::string& currLocale = setlocale( LC_ALL, NULL );

            if ( locale.compare(currLocale)!=0 )
            {
                try
                {
                    setlocale(LC_ALL, locale.c_str());
                }
                catch(...)
                {
                    MITK_INFO << "Could not set locale " << locale;
                }
            }

            try
            {
                MITK_INFO << "Trying to load dti as nifti ...";
                std::string fname3 = "temp_dti.nii";
                itksys::SystemTools::CopyAFile(m_FileName.c_str(), fname3.c_str());

                typedef itk::Image<float,4> ImageType;
                itk::NiftiImageIO::Pointer io = itk::NiftiImageIO::New();
                typedef itk::ImageFileReader<ImageType> FileReaderType;
                FileReaderType::Pointer reader = FileReaderType::New();
                reader->SetImageIO(io);
                reader->SetFileName(fname3);
                reader->Update();
                itksys::SystemTools::RemoveFile(fname3.c_str());

                ImageType::Pointer img = reader->GetOutput();
                itk::Size<4> size = img->GetLargestPossibleRegion().GetSize();

                itk::ImageRegion< 3 > region;
                region.SetSize(0,size[0]);
                region.SetSize(1,size[1]);
                region.SetSize(2,size[2]);
                itk::Vector<double,3> spacing;
                spacing[0] = img->GetSpacing()[0];
                spacing[1] = img->GetSpacing()[1];
                spacing[2] = img->GetSpacing()[2];
                itk::Point<double,3> origin;
                origin[0] = img->GetOrigin()[0];
                origin[1] = img->GetOrigin()[1];
                origin[2] = img->GetOrigin()[2];
                itk::Matrix<double,3,3> direction;
                direction[0][0] = img->GetDirection()[0][0];
                direction[1][0] = img->GetDirection()[1][0];
                direction[2][0] = img->GetDirection()[2][0];
                direction[0][1] = img->GetDirection()[0][1];
                direction[1][1] = img->GetDirection()[1][1];
                direction[2][1] = img->GetDirection()[2][1];
                direction[0][2] = img->GetDirection()[0][2];
                direction[1][2] = img->GetDirection()[1][2];
                direction[2][2] = img->GetDirection()[2][2];

                typedef itk::Image<itk::DiffusionTensor3D<float>,3> VecImgType;
                typedef VecImgType::PixelType FixPixType;
                VecImgType::Pointer vecImg = VecImgType::New();
                vecImg->SetSpacing( spacing );
                vecImg->SetOrigin( origin );
                vecImg->SetDirection( direction );
                vecImg->SetRegions( region );
                vecImg->Allocate();

                itk::ImageRegionIterator<VecImgType> ot (vecImg, vecImg->GetLargestPossibleRegion() );
                ot = ot.Begin();

                while (!ot.IsAtEnd())
                {
                    itk::DiffusionTensor3D<float> tensor;
                    ImageType::IndexType idx;
                    idx[0] = ot.GetIndex()[0]; idx[1] = ot.GetIndex()[1]; idx[2] = ot.GetIndex()[2];

                    if (size[3]==6)
                    {
                        for (int te=0; te<size[3]; te++)
                        {
                            idx[3] = te;
                            tensor.SetElement(te, img->GetPixel(idx));
                        }
                    }
                    else if (size[3]==9)
                    {
                        idx[3] = 0;
                        tensor.SetElement(0, img->GetPixel(idx));
                        idx[3] = 1;
                        tensor.SetElement(1, img->GetPixel(idx));
                        idx[3] = 2;
                        tensor.SetElement(2, img->GetPixel(idx));
                        idx[3] = 4;
                        tensor.SetElement(3, img->GetPixel(idx));
                        idx[3] = 5;
                        tensor.SetElement(4, img->GetPixel(idx));
                        idx[3] = 8;
                        tensor.SetElement(5, img->GetPixel(idx));
                    }
                    else
                        throw itk::ImageFileReaderException(__FILE__, __LINE__, "Unknown number of komponents for DTI file. Should be 6 or 9!");

                    FixPixType fixVec(tensor);
                    ot.Set(fixVec);
                    ++ot;
                }
                this->GetOutput()->InitializeByItk(vecImg.GetPointer());
                this->GetOutput()->SetVolume(vecImg->GetBufferPointer());
            }
            catch(...)
            {
                MITK_INFO << "Trying to load dti as nrrd ...";

                typedef itk::VectorImage<float,3> ImageType;
                itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
                typedef itk::ImageFileReader<ImageType> FileReaderType;
                FileReaderType::Pointer reader = FileReaderType::New();
                reader->SetImageIO(io);
                reader->SetFileName(this->m_FileName);
                reader->Update();
                ImageType::Pointer img = reader->GetOutput();

                typedef itk::Image<itk::DiffusionTensor3D<float>,3> VecImgType;
                VecImgType::Pointer vecImg = VecImgType::New();
                vecImg->SetSpacing( img->GetSpacing() );   // Set the image spacing
                vecImg->SetOrigin( img->GetOrigin() );     // Set the image origin
                vecImg->SetDirection( img->GetDirection() );  // Set the image direction
                vecImg->SetRegions( img->GetLargestPossibleRegion());
                vecImg->Allocate();

                itk::ImageRegionIterator<VecImgType> ot (vecImg, vecImg->GetLargestPossibleRegion() );
                ot = ot.Begin();

                itk::ImageRegionIterator<ImageType> it (img, img->GetLargestPossibleRegion() );
                it = it.Begin();

                typedef ImageType::PixelType  VarPixType;
                typedef VecImgType::PixelType FixPixType;
                int numComponents = img->GetNumberOfComponentsPerPixel();

                itk::MetaDataDictionary imgMetaDictionary = img->GetMetaDataDictionary();
                std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
                std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
                std::string metaString;

                bool readFrame = false;
                double xx, xy, xz, yx, yy, yz, zx, zy, zz;
                MeasurementFrameType measFrame;
                measFrame.SetIdentity();
                MeasurementFrameType measFrameTransp;
                measFrameTransp.SetIdentity();

                for (; itKey != imgMetaKeys.end(); itKey ++)
                {
                    itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
                    if (itKey->find("measurement frame") != std::string::npos)
                    {
                        sscanf(metaString.c_str(), " ( %lf , %lf , %lf ) ( %lf , %lf , %lf ) ( %lf , %lf , %lf ) \n", &xx, &xy, &xz, &yx, &yy, &yz, &zx, &zy, &zz);

                        if (xx>10e-10 || xy>10e-10 || xz>10e-10 ||
                                yx>10e-10 || yy>10e-10 || yz>10e-10 ||
                                zx>10e-10 || zy>10e-10 || zz>10e-10 )
                        {
                            readFrame = true;

                            measFrame(0,0) = xx;
                            measFrame(0,1) = xy;
                            measFrame(0,2) = xz;
                            measFrame(1,0) = yx;
                            measFrame(1,1) = yy;
                            measFrame(1,2) = yz;
                            measFrame(2,0) = zx;
                            measFrame(2,1) = zy;
                            measFrame(2,2) = zz;

                            measFrameTransp = measFrame.GetTranspose();
                        }
                    }
                }

                if (numComponents==6)
                {
                    while (!it.IsAtEnd())
                    {
                        // T'=RTR'
                        VarPixType vec = it.Get();
                        FixPixType fixVec(vec.GetDataPointer());

                        if(readFrame)
                        {
                            itk::DiffusionTensor3D<float> tensor;
                            tensor.SetElement(0, vec.GetElement(0));
                            tensor.SetElement(1, vec.GetElement(1));
                            tensor.SetElement(2, vec.GetElement(2));
                            tensor.SetElement(3, vec.GetElement(3));
                            tensor.SetElement(4, vec.GetElement(4));
                            tensor.SetElement(5, vec.GetElement(5));

                            tensor = ConvertMatrixTypeToFixedArrayType(tensor.PreMultiply(measFrame));
                            tensor = ConvertMatrixTypeToFixedArrayType(tensor.PostMultiply(measFrameTransp));
                            fixVec = tensor;
                        }

                        ot.Set(fixVec);
                        ++ot;
                        ++it;
                    }
                }
                else if(numComponents==9)
                {
                    while (!it.IsAtEnd())
                    {
                        VarPixType vec = it.Get();
                        itk::DiffusionTensor3D<float> tensor;
                        tensor.SetElement(0, vec.GetElement(0));
                        tensor.SetElement(1, vec.GetElement(1));
                        tensor.SetElement(2, vec.GetElement(2));
                        tensor.SetElement(3, vec.GetElement(4));
                        tensor.SetElement(4, vec.GetElement(5));
                        tensor.SetElement(5, vec.GetElement(8));

                        if(readFrame)
                        {
                            tensor = ConvertMatrixTypeToFixedArrayType(tensor.PreMultiply(measFrame));
                            tensor = ConvertMatrixTypeToFixedArrayType(tensor.PostMultiply(measFrameTransp));
                        }

                        FixPixType fixVec(tensor);
                        ot.Set(fixVec);
                        ++ot;
                        ++it;
                    }
                }
                else if (numComponents==1)
                {
                    typedef itk::Image<float,4> ImageType;
                    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
                    typedef itk::ImageFileReader<ImageType> FileReaderType;
                    FileReaderType::Pointer reader = FileReaderType::New();
                    reader->SetImageIO(io);
                    reader->SetFileName(this->m_FileName);
                    reader->Update();
                    ImageType::Pointer img = reader->GetOutput();

                    itk::Size<4> size = img->GetLargestPossibleRegion().GetSize();

                    MITK_INFO << size;

                    while (!ot.IsAtEnd())
                    {
                        itk::DiffusionTensor3D<float> tensor;
                        ImageType::IndexType idx;
                        idx[0] = ot.GetIndex()[0]; idx[1] = ot.GetIndex()[1]; idx[2] = ot.GetIndex()[2];

                        if (size[3]==6)
                        {
                            for (int te=0; te<size[3]; te++)
                            {
                                idx[3] = te;
                                tensor.SetElement(te, img->GetPixel(idx));
                            }

                            //                        idx[3] = 0;
                            //                        tensor.SetElement(0, img->GetPixel(idx));
                            //                        idx[3] = 1;
                            //                        tensor.SetElement(1, img->GetPixel(idx));
                            //                        idx[3] = 3;
                            //                        tensor.SetElement(2, img->GetPixel(idx));
                            //                        idx[3] = 2;
                            //                        tensor.SetElement(3, img->GetPixel(idx));
                            //                        idx[3] = 4;
                            //                        tensor.SetElement(4, img->GetPixel(idx));
                            //                        idx[3] = 5;
                            //                        tensor.SetElement(5, img->GetPixel(idx));
                        }
                        else if (size[3]==9)
                        {
                            idx[3] = 0;
                            tensor.SetElement(0, img->GetPixel(idx));
                            idx[3] = 1;
                            tensor.SetElement(1, img->GetPixel(idx));
                            idx[3] = 2;
                            tensor.SetElement(2, img->GetPixel(idx));
                            idx[3] = 4;
                            tensor.SetElement(3, img->GetPixel(idx));
                            idx[3] = 5;
                            tensor.SetElement(4, img->GetPixel(idx));
                            idx[3] = 8;
                            tensor.SetElement(5, img->GetPixel(idx));
                        }
                        else
                            throw itk::ImageFileReaderException(__FILE__, __LINE__, "Unknown number of komponents for DTI file. Should be 6 or 9!");

                        if(readFrame)
                        {
                            tensor = ConvertMatrixTypeToFixedArrayType(tensor.PreMultiply(measFrame));
                            tensor = ConvertMatrixTypeToFixedArrayType(tensor.PostMultiply(measFrameTransp));
                        }
                        FixPixType fixVec(tensor);
                        ot.Set(fixVec);
                        ++ot;
                    }
                }
                else
                {
                    throw itk::ImageFileReaderException(__FILE__, __LINE__, "Image has wrong number of pixel components!");
                }

                this->GetOutput()->InitializeByItk(vecImg.GetPointer());
                this->GetOutput()->SetVolume(vecImg->GetBufferPointer());

            }

            try
            {
                setlocale(LC_ALL, currLocale.c_str());
            }
            catch(...)
            {
                MITK_INFO << "Could not reset locale " << currLocale;
            }

        }
        catch(std::exception& e)
        {
            throw itk::ImageFileReaderException(__FILE__, __LINE__, e.what());
        }
        catch(...)
        {
            throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, an error occurred while reading the requested DTI file!");
        }
    }
}

void NrrdTensorImageReader::GenerateOutputInformation()
{

}



const char* NrrdTensorImageReader
::GetFileName() const
{
    return m_FileName.c_str();
}


void NrrdTensorImageReader
::SetFileName(const char* aFileName)
{
    m_FileName = aFileName;
}


const char* NrrdTensorImageReader
::GetFilePrefix() const
{
    return m_FilePrefix.c_str();
}


void NrrdTensorImageReader
::SetFilePrefix(const char* aFilePrefix)
{
    m_FilePrefix = aFilePrefix;
}


const char* NrrdTensorImageReader
::GetFilePattern() const
{
    return m_FilePattern.c_str();
}


void NrrdTensorImageReader
::SetFilePattern(const char* aFilePattern)
{
    m_FilePattern = aFilePattern;
}


bool NrrdTensorImageReader
::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/)
{
    // First check the extension
    if(  filename == "" )
    {
        return false;
    }
    std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
    ext = itksys::SystemTools::LowerCase(ext);

    if (ext == ".hdti" || ext == ".dti")
    {
        return true;
    }

    return false;
}

itk::DiffusionTensor3D<float> NrrdTensorImageReader
::ConvertMatrixTypeToFixedArrayType(const itk::DiffusionTensor3D<float>::Superclass::MatrixType & matrix)
{
    /*       | 0  1  2  |
      *       | X  3  4  |
      *       | X  X  5  |
      */
    itk::DiffusionTensor3D<float> arr;
    arr.SetElement(0,matrix(0,0));
    arr.SetElement(1,matrix(0,1));
    arr.SetElement(2,matrix(0,2));
    arr.SetElement(3,matrix(1,3));
    arr.SetElement(4,matrix(1,4));
    arr.SetElement(5,matrix(2,5));
    return arr;
}

} //namespace MITK
