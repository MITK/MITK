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
#include "SegmentationExports.h"
#include "mitkAutoSegmentationTool.h"

namespace mitk
{

class Image;

/**
  \brief Simple watershed filter tool.

  \sa ExtractImageFilter
  \sa OverwriteSliceImageFilter

  \ingroup Interaction
  \ingroup ToolManagerEtAl

  Wraps ITK Watershed Filter into tool concept of MITK. For more information look into ITK documentation.

  \warning Only to be instantiated by mitk::ToolManager.

  $Darth Vader$
*/
class Segmentation_EXPORT WatershedTool : public AutoSegmentationTool
{
  public:

    mitkClassMacro(WatershedTool, AutoSegmentationTool);
    itkNewMacro(WatershedTool);

    void DoIt();

    template <typename TPixel, unsigned int VImageDimension>
    void ITKWatershed( itk::Image<TPixel, VImageDimension>* originalImage, mitk::Image* segmentation, unsigned int timeStep );

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;


  protected:

    WatershedTool(); // purposely hidden
    virtual ~WatershedTool();

    virtual void Activated();
    virtual void Deactivated();
};

} // namespace

#endif
