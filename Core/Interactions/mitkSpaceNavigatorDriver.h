
#ifndef SpaceNavigatorDriver__h__
#define SpaceNavigatorDriver__h__

#ifdef SPACE_NAVIGATOR_MAIN_SOURCE

#define _ATL_ATTRIBUTES 1
#define _WIN32_DCOM

#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atltypes.h>
#include <atlctl.h>
#include <atlhost.h>

#import "progid:TDxInput.Device" embedded_idl no_namespace

using namespace ATL;

#include <mitkVector.h>
#include <iostream>

using std::cout;
using std::endl;

[module(name="XYZName")];
[ event_receiver(com) ]
class SpaceNavigatorDriver
{
public:
  static SpaceNavigatorDriver* GetInstance();

private:

  SpaceNavigatorDriver();
	~SpaceNavigatorDriver();

  //methods for workaround of .net 2003 compiler Artikel-ID	:	829749
  //http://support.microsoft.com/kb/829749/de
  inline HRESULT HookEvent1(CComPtr<ISimpleDevice> device);
  inline HRESULT HookEvent2();
  inline HRESULT HookEvent3(CComPtr<ISimpleDevice> device);
  inline HRESULT HookEvent4();
  inline HRESULT HookEvent5();

#if 1
   CComPtr<ISensor> m_pISensor;
   CComPtr<IKeyboard> m_pIKeyboard;
#else
   ISensorPtr m_pISensor;
   IKeyboardPtr m_pIKeyboard;
#endif

public:

// COM Event handlers
   HRESULT InitializeCOM();
   HRESULT OnDeviceChange (long reserved);
   HRESULT OnKeyDown (int keyCode);
   HRESULT OnKeyUp (int keyCode);
   HRESULT OnSensorInput(void);
   HRESULT UninitializeCOM();
};

#else // SPACE_NAVIGATOR_MAIN_SOURCE

class SpaceNavigatorDriver
{
public:
  static SpaceNavigatorDriver* GetInstance();
};

#endif // SPACE_NAVIGATOR_MAIN_SOURCE

#endif // SpaceNavigatorDriver__h__
