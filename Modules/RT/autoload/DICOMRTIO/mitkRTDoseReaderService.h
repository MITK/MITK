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
     * \brief Reader service for DICOM files of modality RTDOSE.
     *
     * Reads PixelData from a DicomRT dose file, scales with the grid
     * scaling factor to produce Gray values, and stores the result as
     * an mitk::Image with a vtkColorTransferFunction.
     */
    class MITKDICOMRTIO_EXPORT RTDoseReaderService : public mitk::AbstractFileReader
    {
        public:
            /** \brief Copy constructor. */
            RTDoseReaderService(const RTDoseReaderService& other);

            /** \brief Default constructor. Registers reader for the RTDOSE MIME type. */
            RTDoseReaderService();

            ~RTDoseReaderService() override;

            using AbstractFileReader::Read;

    protected:
      /**
       * \brief Read a DICOM dataset from an RTDOSE file.
       *
       * Reads the PixelData from the DicomRT dose file and scales
       * them with a factor for getting Gray values instead of pixel values.
       * The Gray values are stored in an mitk::Image with a vtkColorTransferFunction.
       * Relative values are used for coloring the image. The relative values are
       * relative to a PrescriptionDose defined in the RT-Plan. If there is no
       * RT-Plan file, PrescriptionDose is set to 80% of the maximum dose.
       *
       * \return A vector of loaded BaseData objects (dose images).
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
