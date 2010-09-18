#ifndef MITKERODETOOL_H
#define MITKERODETOOL_H

#include "mitkMorphologicTool.h"
#include "MitkExtExports.h"

//itk
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryCrossStructuringElement.h>
#include <itkBinaryErodeImageFilter.h>

#include "mitkErodeTool.xpm"

namespace mitk {

  class MitkExt_EXPORT ErodeTool : public MorphologicTool {

    typedef itk::Image< unsigned char, 3 > SegmentationType;
    typedef itk::BinaryBallStructuringElement< SegmentationType::PixelType, 3 > BallType;
    typedef itk::BinaryCrossStructuringElement< SegmentationType::PixelType, 3 > CrossType;
    typedef itk::BinaryErodeImageFilter< SegmentationType, SegmentationType, BallType > BallErodeFilterType;
    typedef itk::BinaryErodeImageFilter< SegmentationType, SegmentationType, CrossType > CrossErodeFilterType;

  public:
    mitkClassMacro(ErodeTool, MorphologicTool);
    itkNewMacro(ErodeTool);

    virtual const char* GetName() const;
    virtual const char** GetXPM() const;

  protected:

    ErodeTool();
    virtual ~ErodeTool();

    mitk::Image::Pointer ApplyFilter(mitk::Image::Pointer);

    BallType m_Ball;
    BallErodeFilterType::Pointer m_BallErodeFilter;

  };//class


}//namespace

#endif