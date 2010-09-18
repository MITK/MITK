#ifndef MITKERODETOOL_H
#define MITKERODETOOL_H

#include "mitkMorphologicTool.h"
#include "MitkExtExports.h"

//itk
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryCrossStructuringElement.h>
#include <itkBinaryDilateImageFilter.h>

#include "mitkDilateTool.xpm"

namespace mitk {

  class MitkExt_EXPORT DilateTool : public MorphologicTool {

    typedef itk::Image< unsigned char, 3 > SegmentationType;
    typedef itk::BinaryBallStructuringElement< SegmentationType::PixelType, 3 > BallType;
    typedef itk::BinaryCrossStructuringElement< SegmentationType::PixelType, 3 > CrossType;
    typedef itk::BinaryDilateImageFilter< SegmentationType, SegmentationType, BallType > BallDilateFilterType;
    typedef itk::BinaryDilateImageFilter< SegmentationType, SegmentationType, CrossType > CrossDilateFilterType;

  public:
    mitkClassMacro(DilateTool, MorphologicTool);
    itkNewMacro(DilateTool);

    virtual const char* GetName() const;
    virtual const char** GetXPM() const;

  protected:

    DilateTool();
    virtual ~DilateTool();

    mitk::Image::Pointer ApplyFilter(mitk::Image::Pointer image);

    BallType m_Ball;
    BallDilateFilterType::Pointer m_BallDilateFilter;

  };//class


}//namespace

#endif