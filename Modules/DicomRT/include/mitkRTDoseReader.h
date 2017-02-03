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


#ifndef mitkDicomRTReader_h
#define mitkDicomRTReader_h

#include <mitkAbstractFileReader.h>

#include <mitkImage.h>
#include <mitkIDICOMTagsOfInterest.h>
#include <MitkDicomRTExports.h>

namespace mitk
{
    /**
    * \brief RTDoseReader reads DICOM files of modality RTDOSE.
    */
    class MITKDICOMRT_EXPORT RTDoseReader : public mitk::AbstractFileReader
    {

        public:
            RTDoseReader(const RTDoseReader& other);
            RTDoseReader();

            /**
                * @brief Reads a dicom dataset from a RTDOSE file
                * @param filename The path with the dose file
                * @return Returns a mitkDataNode::Pointer in which a mitk::Image is stored
                *
                * The method reads the PixelData from the DicomRT dose file and scales
                * them with a factor for getting Gray-values instead of pixel-values.
                * The Gray-values are stored in a mitkImage with a vtkColorTransferFunc.
                * Relative values are used for coloring the image. The relative values are
                * relative to a PrescriptionDose defined in the RT-Plan. If there is no
                * RT-Plan file PrescriptionDose is set to 80% of the maximum dose.
                */
            //mitk::DataNode::Pointer LoadRTDose(const char* filename);

            virtual ~RTDoseReader();

            using AbstractFileReader::Read;
            virtual std::vector<itk::SmartPointer<BaseData> > Read() override;

    private:
        RTDoseReader* Clone() const override;
        /**
        * \brief Scales an image with a factor
        *
        * \param gridscale the factor to scale with
        */
        template<typename TPixel, unsigned int VImageDimension>
        void MultiplyGridScaling(itk::Image< TPixel, VImageDimension>* image, float gridscale);

        mitk::IDICOMTagsOfInterest* GetDicomTagsOfInterestService();

        mitk::Image::Pointer scaledDoseImage;
        us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };

}

#endif
