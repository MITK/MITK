#ifndef STEPPER_H_HEADER_INCLUDED_C1E77191
#define STEPPER_H_HEADER_INCLUDED_C1E77191

#include "mitkCommon.h"

namespace mitk {

//##ModelId=3DF8BF9A005D
//##Documentation
//## @brief Helper class to step through a list
//## A helper class to step through a list. Does not contain the list, just the
//## position in the list (between 0 and GetSteps()). Provides methods like
//## First (go to the first element), Next (go to the next one), etc.
class Stepper
{
  public:
    //##ModelId=3DF8B9DD0141
    unsigned int GetPos();

    //##ModelId=3DF8BA0002D7
    unsigned int GetSteps();

    //##ModelId=3DF8B9B20019
    void SetPos(unsigned int pos);

    //##ModelId=3DF8BA8301FC
    unsigned int GetStepSize();

    //##ModelId=3DF8B9410142
    void Previous();

    //##ModelId=3DF8B92F01DF
    void Last();

    //##ModelId=3DF8BA9301EC
    void SetStepSize(unsigned int stepSize);

    //##ModelId=3DF8B91502F8
    void First();

    //##ModelId=3DF8B92703A4
    void Next();

    //##ModelId=3DF8F6B40153
    void SetSteps(unsigned int steps);
  protected:
    //##ModelId=3DD524BD00DC
    unsigned int m_Pos;
    //##ModelId=3DF8F73C0076
    unsigned int m_Steps;
    //##ModelId=3DF8F74101AE
    unsigned int m_StepSize;

  private:



};

} // namespace mitk



#endif /* STEPPER_H_HEADER_INCLUDED_C1E77191 */
