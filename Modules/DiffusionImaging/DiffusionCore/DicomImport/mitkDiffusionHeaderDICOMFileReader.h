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

#ifndef MITKDIFFUSIONHEADERFILEREADER_H
#define MITKDIFFUSIONHEADERFILEREADER_H

#include <MitkDiffusionCoreExports.h>

#include <itkLightObject.h>
#include "mitkDiffusionImage.h"

#include "gdcmScanner.h"
#include "gdcmReader.h"

namespace mitk
{

/**
 * @brief The DiffusionImageHeaderInformation struct
 */
struct DiffusionImageDICOMHeaderInformation
{
  DiffusionImageDICOMHeaderInformation()
    : b_value(0),
      baseline(false),
      isotropic(false)
  {
    g_vector.fill(0);
  }

  void Print()
  {
    MITK_INFO << " DiffusionImageHeaderInformation : \n"
              << "    : b value   : " << b_value << "\n"
              << "    : gradient  : " << g_vector << "\n"
              << "    : isotropic : " << isotropic << "\n --- \n";
  }

  unsigned int b_value;
  vnl_vector_fixed< double, 3> g_vector;
  bool baseline;
  bool isotropic;
};

struct DiffusionImageMosaicDICOMHeaderInformation
    : public DiffusionImageDICOMHeaderInformation
{
  unsigned long n_images;
  bool slicenormalup;
};



/**
 * @class DiffusionHeaderDICOMFileReader
 *
 * @brief Abstract class for all vendor specific diffusion file header reader
 *
 * To provide a diffusion header reader for a new vendor, reimplement the \sa ReadDiffusionHeader method.
 */
class MitkDiffusionCore_EXPORT DiffusionHeaderDICOMFileReader
    : public itk::LightObject
{
public:

  typedef std::vector< DiffusionImageDICOMHeaderInformation > DICOMHeaderListType;

  mitkClassMacro( DiffusionHeaderDICOMFileReader, itk::LightObject )
  itkSimpleNewMacro( Self )

  /**
   * @brief IsDiffusionHeader Parse the given dicom file and collect the special diffusion image information
   * @return
   */
  virtual bool ReadDiffusionHeader( std::string ){ return false; }

  DICOMHeaderListType GetHeaderInformation();

protected:
  DiffusionHeaderDICOMFileReader();

  virtual ~DiffusionHeaderDICOMFileReader();

  DICOMHeaderListType m_HeaderInformationList;
};

/**
 * @brief Retrieve the value of a gdcm tag to the given string
 *
 * @param tag the gdcm::Tag to be search for
 * @param dataset a gdcm::DataSet to look into
 * @param target a string to store the value of the given tag if found
 * @param verbose make some output
 *
 * @return true if a string was found, false otherwise
 */
bool RevealBinaryTag(const gdcm::Tag tag, const gdcm::DataSet& dataset, std::string& target);

bool RevealBinaryTagC(const gdcm::Tag tag, const gdcm::DataSet& dataset, char* target_array );


} // end namespace mitk

#endif // MITKDIFFUSIONHEADERFILEREADER_H
