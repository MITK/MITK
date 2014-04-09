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

#ifndef mitkWatershedTool_h_Included
#define mitkWatershedTool_h_Included

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkAutoSegmentationTool.h"
#include <itkImage.h>

namespace us {
class ModuleResource;
}

namespace mitk
{

class Image;

/**
  \brief Simple watershed segmentation tool.

  \ingroup Interaction
  \ingroup ToolManagerEtAl

  Wraps ITK Watershed Filter into tool concept of MITK. For more information look into ITK documentation.

  \warning Only to be instantiated by mitk::ToolManager.

  $Darth Vader$
*/
class MitkSegmentation_EXPORT WatershedTool : public AutoSegmentationTool
{
  public:

    mitkClassMacro(WatershedTool, AutoSegmentationTool);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void SetThreshold(double t) {
      m_Threshold = t;
    }

    void SetLevel(double l) {
      m_Level = l;
    }

    /** \brief Grabs the tool reference data and creates an ITK pipeline consisting of a GradientMagnitude
      * image filter followed by a Watershed image filter. The output of the filter pipeline is then added
      * to the data storage. */
    void DoIt();

    /** \brief Creates and runs an ITK filter pipeline consisting of the filters: GradientMagnitude-, Watershed- and CastImageFilter.
      *
      * \param originalImage The input image, which is delivered by the AccessByItk macro.
      * \param segmentation A pointer to the output image, which will point to the pipeline output after execution.
      */
    template <typename TPixel, unsigned int VImageDimension>
    void ITKWatershed( itk::Image<TPixel, VImageDimension>* originalImage, itk::SmartPointer<mitk::Image>& segmentation );

    const char** GetXPM() const;
    const char* GetName() const;
    us::ModuleResource GetIconResource() const;

  protected:

    WatershedTool(); // purposely hidden
    virtual ~WatershedTool();

    virtual void Activated();
    virtual void Deactivated();

    /** \brief Threshold parameter of the ITK Watershed Image Filter. See ITK Documentation for more information. */
    double m_Threshold;
    /** \brief Threshold parameter of the ITK Watershed Image Filter. See ITK Documentation for more information. */
    double m_Level;
};

} // namespace

#endif
