#ifndef MITKOPENINGTOOL_H
#define MITKOPENINGTOOL_H

#include "mitkMorphologicTool.h"
#include "MitkExtExports.h"

//itk
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryCrossStructuringElement.h>
#include <V:\windows\source\ITK3160\Code\Review\itkBinaryMorphologicalOpeningImageFilter.h>

#include "mitkOpeningTool.xpm"

namespace mitk {

  class MitkExt_EXPORT OpeningTool : public MorphologicTool {

    typedef itk::Image< unsigned char, 3 > SegmentationType;
    typedef itk::BinaryBallStructuringElement< SegmentationType::PixelType, 3 > BallType;
    typedef itk::BinaryCrossStructuringElement< SegmentationType::PixelType, 3 > CrossType;
    typedef itk::BinaryMorphologicalOpeningImageFilter< SegmentationType, SegmentationType, BallType > BallOpeningFilterType;
    //typedef itk::BinaryOpenImageFilter< SegmentationType, SegmentationType, CrossType > CrossOpenFilterType;

  public:
    mitkClassMacro(OpeningTool, MorphologicTool);
    itkNewMacro(OpeningTool);

    virtual const char* GetName() const;
    virtual const char** GetXPM() const;

  protected:

    OpeningTool();
    virtual ~OpeningTool();

    mitk::Image::Pointer ApplyFilter(mitk::Image::Pointer image);

    BallType m_Ball;
    BallOpeningFilterType::Pointer m_BallOpeningFilter;

  };//class


}//namespace

#endif