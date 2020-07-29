/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkRTDoseReaderService_h
#define mitkRTDoseReaderService_h

#include <mitkAbstractFileReader.h>

#include <usServiceRegistration.h>
#include <mitkImage.h>
#include <MitkDICOMRTIOExports.h>

namespace mitk
{
    /**
    * \brief RTDoseReaderService reads DICOM files of modality RTDOSE.
    */
    class MITKDICOMRTIO_EXPORT RTDoseReaderService : public mitk::AbstractFileReader
    {
        public:
            RTDoseReaderService(const RTDoseReaderService& other);
            RTDoseReaderService();

            ~RTDoseReaderService() override;

            using AbstractFileReader::Read;

    protected:
      /**
      * @brief Reads a dicom dataset from a RTDOSE file
      * The method reads the PixelData from the DicomRT dose file and scales
      * them with a factor for getting Gray-values instead of pixel-values.
      * The Gray-values are stored in a mitkImage with a vtkColorTransferFunc.
      * Relative values are used for coloring the image. The relative values are
      * relative to a PrescriptionDose defined in the RT-Plan. If there is no
      * RT-Plan file PrescriptionDose is set to 80% of the maximum dose.
      */
      std::vector<itk::SmartPointer<BaseData>> DoRead() override;

    private:
      RTDoseReaderService* Clone() const override;
        /**
        * \brief Scales an image with a factor
        *
        * \param gridscale the factor to scale with
        */
        template<typename TPixel, unsigned int VImageDimension>
        void MultiplyGridScaling(itk::Image< TPixel, VImageDimension>* image, float gridscale);

        mitk::Image::Pointer scaledDoseImage;
        us::ServiceRegistration<mitk::IFileReader> m_FileReaderServiceReg;
  };

}

#endif
