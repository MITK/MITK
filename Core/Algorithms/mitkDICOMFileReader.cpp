#include "mitkDICOMFileReader.h"
#include <itkImageFileReader.h>
#ifdef MITK_DICOM_ENABLED
extern "C"
{
#include "ipDicom.h"
}
#endif

void mitk::DICOMFileReader::GenerateOutputInformation()
{
    mitk::Image::Pointer output = this->GetOutput();

    if ((output->IsInitialized()) && (this->GetMTime() <= m_ReadHeaderTime.GetMTime()))
        return;

    itkDebugMacro(<<"Reading file for GenerateOutputInformation()" << m_FileName);

    // Check to see if we can read the file given the name or prefix
    //
    if ( m_FileName == "" )
    {
        throw itk::ImageFileReaderException(__FILE__, __LINE__, "One of FileName or FilePrefix must be non-empty");
    }

    if( m_FileName != "")
    {
		//**************
//		std::string outputfile_name;
		ipUInt4_t header_size;
		ipUInt4_t image_size,slices;
		ipUInt1_t *header = NULL;
		unsigned char *image = NULL;
		ipBool_t color;
        ipBool_t encrypt;
        void * data;
		ipUInt4_t  len;
		outputfile_name = m_FileName;
		dicomGetHeaderAndImage( const_cast<char *>(m_FileName.c_str()),
                          &header, &header_size,
                          &image, &image_size );

        if( !image )
        exit(-1);
		pic = _dicomToPic( header, header_size,
                     image, image_size, color );
		pic->dim--;
		
	/*{
		//ipInt4_t slices;

		ipInt4FromLE( data, &slices );
		memcpy( &slices, data, 4 );
		std::cout<<"Image Dimensioin"<<pic->dim;
		pic->n[pic->dim] = pic->n[pic->dim-1];
		pic->n[pic->dim-1] = slices;
        //pic->dim++;
	}*/
		//outputfile_name = "out3DDicom.pic";
		//ipPicPut( const_cast<char *>(outputfile_name.c_str()),pic );

		
		/////////////

      //ipPicDescriptor* pic_header=ipPicGetHeader(const_cast<char *>(outputfile_name.c_str()), NULL);

      //pic_header=ipPicGetTags(const_cast<char *>(outputfile_name.c_str()), pic_header);

        if( pic == NULL)
        {
            itk::ImageFileReaderException e(__FILE__, __LINE__);
            itk::OStringStream msg;
            msg << " Could not read file "
                << outputfile_name.c_str();
            e.SetDescription(msg.str().c_str());
            throw e;
            return;
        }

        output->Initialize(pic);
    }

    m_ReadHeaderTime.Modified();
}

//##ModelId=3E187255016C
void mitk::DICOMFileReader::GenerateData()
{
    mitk::Image::Pointer output = this->GetOutput();

    // Check to see if we can read the file given the name or prefix
    //
    if ( m_FileName == "" && m_FilePrefix == "" )
    {
        throw itk::ImageFileReaderException(__FILE__, __LINE__, "One of FileName or FilePrefix must be non-empty");
    }

    if( m_FileName != "")
    {
        
    //left to right handed conversion
		if(pic->dim==3)
        {
            ipPicDescriptor* slice=ipPicCopyHeader(pic, NULL);
            slice->dim=2;
            int size=_ipPicSize(slice);
            slice->data=malloc(size);
            unsigned char *p_first=(unsigned char *)pic->data;
            unsigned char *p_last=(unsigned char *)pic->data;
            p_last+=size*(pic->n[2]-1);

            int i, smid=pic->n[2]/2;
            for(i=0; i<smid;++i,p_last-=size, p_first+=size)
            {
                memcpy(slice->data, p_last, size);
                memcpy(p_last, p_first, size);
                memcpy(p_first, slice->data, size);
            }
            ipPicFree(slice);
        }
        output->SetPicVolume(pic);
    }
}

//##ModelId=3E1874D202D0
mitk::DICOMFileReader::DICOMFileReader()
    : m_FileName(""), m_FilePrefix(""), m_FilePattern("")
{
}

//##ModelId=3E1874E50179
mitk::DICOMFileReader::~DICOMFileReader()
{
}
