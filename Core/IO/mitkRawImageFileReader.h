/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#ifndef RawImageFileReader_H_HEADER_INCLUDED
#define RawImageFileReader_H_HEADER_INCLUDED
#include "itkVector.h"
#include "mitkCommon.h"
#include "mitkFileReader.h"
#include "mitkImageSource.h"

namespace mitk 
{

//##Documentation
//## @brief Reader to read raw image files
/**  The user must set the dimensionality, the dimensions and the pixel type. If they are incorrect, the image will not be opened or the visualization will be incorrect. */
//## @ingroup IO
class MITK_CORE_EXPORT RawImageFileReader : public ImageSource, public FileReader
{
public:
    mitkClassMacro(RawImageFileReader, FileReader);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    itkSetMacro(FileName, std::string);
    itkSetStringMacro(FileName);
    itkGetMacro(FileName, std::string);
    itkGetStringMacro(FileName);

    itkSetMacro(FilePrefix, std::string);
    itkSetStringMacro(FilePrefix);
    itkGetMacro(FilePrefix, std::string);
    itkGetStringMacro(FilePrefix);

    itkSetMacro(FilePattern, std::string);
    itkSetStringMacro(FilePattern);
    itkGetMacro(FilePattern, std::string);
    itkGetStringMacro(FilePattern);
    
    /** Supported pixel types. */
    typedef enum {UCHAR,SCHAR,USHORT,SSHORT, UINT, SINT, FLOAT, DOUBLE} IOPixelType;
    itkSetMacro(PixelType, IOPixelType);
    
    /** Endianity of bits. */
    typedef enum {LITTLE, BIG} EndianityType;
    itkSetMacro(Endianity, EndianityType);
    
    itkSetMacro(Dimensionality, int);
    itkGetMacro(Dimensionality, int);
    
    /** Image dimensions must be set one by one, starting from dimension 0. */
    void SetDimensions(unsigned int i, unsigned int dim);
     
    unsigned int GetDimensions(unsigned int i) const;

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

protected:

    RawImageFileReader();

    ~RawImageFileReader();

    virtual void GenerateData();
    
    template < typename TPixel, unsigned int VImageDimensions > void TypedGenerateData();

    /** Name of file to be read.*/
    std::string m_FileName;

    /** File prefix.  */
    std::string m_FilePrefix;

    /** File pattern. */
    std::string m_FilePattern;
    
    /** Pixel type of image to be read. Must be of type IOPixelType. */
    IOPixelType m_PixelType;

    /** Dimensionality of file to be read. Can be 2 or 3. */    
    int m_Dimensionality;
    
    /** Endianity. Must be set to LITTLE or BIG. Default is BIG. */
    EndianityType m_Endianity;
    
    /** Vector containing dimensions of image to be read. */
    itk::Vector<int, 3> m_Dimensions; 

};

} // namespace mitk

#endif /* RawImageFileReader_H_HEADER_INCLUDED */


