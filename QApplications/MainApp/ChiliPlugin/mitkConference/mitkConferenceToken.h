#ifndef MITK_CONFERENC_TOKEN
#define MITK_CONFERENC_TOKEN

#include <itkObject.h>

namespace mitk
{
/*
typedef enum
{
  succes=1,
  trylater= 2,
  networkerror= 30,
  networktimeout= 31,
}state;
*/

class ConferenceToken : public itk::Object
{
  public:
    ConferenceToken();
    ~ConferenceToken();
    bool HaveToken();
    bool GetToken();

  private:
    //Use only one global token
    static bool m_Token;
    static int m_Ref;
};

}
#endif
