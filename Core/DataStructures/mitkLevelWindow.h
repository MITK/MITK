#ifndef LEVELWINDOW_H_HEADER_INCLUDED_C1F4F02C
#define LEVELWINDOW_H_HEADER_INCLUDED_C1F4F02C

#include "mitkCommon.h"

namespace mitk {

//##ModelId=3E0B12240067
class LevelWindow : public itk::Object
{
  public:
	/** Standard class typedefs. */
    //##ModelId=3E33EEF40183
	typedef LevelWindow              Self;
    //##ModelId=3E33EEF401AB
    typedef itk::Object              Superclass;
    //##ModelId=3E33EEF401C9
	typedef itk::SmartPointer<Self>  Pointer;
    //##ModelId=3E33EEF401F1
	typedef itk::SmartPointer<const Self>  ConstPointer;

	/** Method for creation through the object factory. */
	itkNewMacro(Self);

	/** Run-time type information (and related methods). */
    itkTypeMacro(itk::Object,LevelWindow);

    //##ModelId=3E0B12640203
    float GetLevel();

    //##ModelId=3E0B12960165
    float GetWindow();

    //##ModelId=3E0B130203B9
    float GetMin();

    //##ModelId=3E0B130A0049
    float GetMax();

    //##ModelId=3E0B130E037A
    void SetLevel(float level);

    //##ModelId=3E0B131C0168
    void SetWindow(float window);

    //##ModelId=3E0B132303A3
    void SetLevelWindow(float level, float window);

    //##ModelId=3E0B13320187
    void SetMin(float min);

    //##ModelId=3E0B1339006F
    void SetMax(float max);

    //##ModelId=3E0B133D0292
    void SetMinMax(float min, float max);

  protected:
    //##ModelId=3E345B580260
    LevelWindow();

    //##ModelId=3E0B122C0393
    //##ModelId=3E345B58029C
    virtual ~LevelWindow();

    float m_Min;

    //##ModelId=3E0B123D0167
    float m_Max;

    //##ModelId=3E19538C0312
	float m_RangeMin;

    //##ModelId=3E19538C0326
	float m_RangeMax;

};

} // namespace mitk



#endif /* LEVELWINDOW_H_HEADER_INCLUDED_C1F4F02C */
