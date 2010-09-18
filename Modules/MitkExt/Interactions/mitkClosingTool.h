#ifndef MITKClOSINGTOOL_H
#define MITKClOSINGTOOL_H

#include "mitkMorphologicTool.h"
#include "MitkExtExports.h"

//itk
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryCrossStructuringElement.h>
#include <V:\windows\source\ITK3160\Code\Review\itkBinaryMorphologicalClosingImageFilter.h>

#include "mitkClosingTool.xpm"

namespace mitk {

  class MitkExt_EXPORT ClosingTool : public MorphologicTool {

    typedef itk::Image< unsigned char, 3 > SegmentationType;
    typedef itk::BinaryBallStructuringElement< SegmentationType::PixelType, 3 > BallType;
    typedef itk::BinaryCrossStructuringElement< SegmentationType::PixelType, 3 > CrossType;
    typedef itk::BinaryMorphologicalClosingImageFilter< SegmentationType, SegmentationType, BallType > BallClosingFilterType;
    //typedef itk::BinaryCloseImageFilter< SegmentationType, SegmentationType, CrossType > CrossCloseFilterType;

  public:
    mitkClassMacro(ClosingTool, MorphologicTool);
    itkNewMacro(ClosingTool);

    virtual const char* GetName() const;
    virtual const char** GetXPM() const;

  protected:

    ClosingTool();
    virtual ~ClosingTool();

    mitk::Image::Pointer ApplyFilter(mitk::Image::Pointer image);

    BallType m_Ball;
    BallClosingFilterType::Pointer m_BallClosingFilter;

  };//class


}//namespace

#endif