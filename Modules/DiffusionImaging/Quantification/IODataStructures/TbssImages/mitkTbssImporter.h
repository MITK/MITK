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

#ifndef __mitkTbssImporter_h
#define __mitkTbssImporter_h

#include "mitkCommon.h"
#include "mitkFileReader.h"

#include "itkImage.h"
#include "itkVectorImage.h"

#include "mitkTbssImage.h"
#include "MitkQuantificationExports.h"


namespace mitk
{

/**
  * \brief Converts FSL TBSS data (4D skeleton projection images) to a NRRD image with meta data.
  *
  * The TBSS pipeline of FSL produces a 4D image containing the 3D skeleton projections of all individuals.
  * This class converts the FSL Nifty image to NRRD and adds information about the type of measurement and the study groups.
  */

  class MITKQUANTIFICATION_EXPORT TbssImporter : public itk::Object {

  public:

    // type of the 3d vector image containing the skeletonized images
    typedef itk::VectorImage<float,3> DataImageType;

    mitkClassMacroItkParent( TbssImporter, itk::Object )
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** \brief Converts the FSL Nifty to NRRD and adds the meta data */
    mitk::TbssImage::Pointer Import();

    template <typename TPixel>
    void Import(const mitk::PixelType pixelType, mitk::TbssImage::Pointer tbssImg);

    /** \brief Group info is set by providing a vector with pairs of group name and number*/
    void SetGroupInfo(std::vector< std::pair<std::string, int> > groups)
    {
      m_Groups = groups;
    }

    /** \brief Used to indicate the type of measurement */
    void SetMeasurementInfo(std::string s)
    {
      m_MeasurementInfo = s;
    }

    /** \brief Sets the FSL import volume */
    void SetImportVolume(mitk::Image::Pointer inputVolume)
    {
      m_InputVolume = inputVolume;
    }

  protected:

    TbssImporter(){}

    ~TbssImporter() override{}

    DataImageType::Pointer m_Data;

    std::vector< std::pair<std::string, int> > m_Groups;

    std::string m_MeasurementInfo;

    mitk::Image::Pointer m_InputVolume;

  };
}
#endif // __mitkTbssImporter_h
