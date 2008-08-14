#define SPACE_NAVIGATOR_MAIN_SOURCE

#include "mitkSpaceNavigatorDriver.h"
#include "mitkTDMouseEventThrower.h"

SpaceNavigatorDriver* SpaceNavigatorDriver::GetInstance()
{
  //create the singleton instance of the driver, so it can listen for events
  static SpaceNavigatorDriver instance;
  return &instance;
}

SpaceNavigatorDriver::SpaceNavigatorDriver()
{
    LRESULT result=0;

    HRESULT hr=::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED );
    if (!SUCCEEDED(hr))
    {
      cout << "Fehler" << endl;

      // stop creation of window
      result = -1;
    }
    else
    {
      HRESULT hr = InitializeCOM();
      if (!SUCCEEDED(hr))
      {
         // stop creation of window
         result = -1;
      }
      else
        cout << "alles bestens \n";
    }
}

SpaceNavigatorDriver::~SpaceNavigatorDriver()
{
     UninitializeCOM();
}

inline HRESULT SpaceNavigatorDriver::HookEvent1(CComPtr<ISimpleDevice> device)
{
  // Get the interfaces to the sensor and the keyboard;
  return device->get_Sensor(&m_pISensor);
}

inline HRESULT SpaceNavigatorDriver::HookEvent2()
{
  return __hook(&_ISensorEvents::SensorInput, m_pISensor, 
    &SpaceNavigatorDriver::OnSensorInput);
}

inline HRESULT SpaceNavigatorDriver::HookEvent3(CComPtr<ISimpleDevice> device)
{
  return device->get_Keyboard(&m_pIKeyboard);
}

inline HRESULT SpaceNavigatorDriver::HookEvent4()
{
  return __hook(&_IKeyboardEvents::KeyDown, m_pIKeyboard, 
    &SpaceNavigatorDriver::OnKeyDown);
}

inline HRESULT SpaceNavigatorDriver::HookEvent5()
{
  return __hook(&_IKeyboardEvents::KeyUp, m_pIKeyboard, 
    &SpaceNavigatorDriver::OnKeyUp);
}



HRESULT SpaceNavigatorDriver::InitializeCOM()
{
   HRESULT hr;
   CComPtr<IUnknown> _3DxDevice;

   // Create the device object
   hr = _3DxDevice.CoCreateInstance(__uuidof(Device));
   if (SUCCEEDED(hr))
   {

      CComPtr<ISimpleDevice> _3DxSimpleDevice;

      hr = _3DxDevice.QueryInterface(&_3DxSimpleDevice);
      if (SUCCEEDED(hr))
      {
         hr = __hook(&_ISimpleDeviceEvents::DeviceChange, _3DxSimpleDevice, 
                        &SpaceNavigatorDriver::OnDeviceChange);

         //workaround for .net 2003 Bug Nr. 829749
         hr = this->HookEvent1(_3DxSimpleDevice);
         hr = this->HookEvent2();
         hr = this->HookEvent3(_3DxSimpleDevice);
         hr = this->HookEvent4();
         hr = this->HookEvent5();

         // Connect to the driver
         _3DxSimpleDevice->Connect();

         cout << "Connected to COM-object for space navigation..." << endl;
      }
   }
   else
      cout << "CoCreateInstance failed";

   return hr;
}

HRESULT SpaceNavigatorDriver::OnDeviceChange(long reserved )
{
   HRESULT result = S_OK;
   //cout << "SpaceNavigatorDriver::OnDeviceChange(reserved =0" << reserved << endl;

   HRESULT hr;
   ISimpleDevicePtr _p3DxDevice;
   hr = m_pIKeyboard->get_Device((IDispatch**)&_p3DxDevice);
   if (SUCCEEDED(hr))
   {
      long device;
      _p3DxDevice->get_Type(&device);
      //cout << "Attached device=" << device;
      _p3DxDevice.Release();

      long keys, programmableKeys;
      m_pIKeyboard->get_Keys(&keys);
      m_pIKeyboard->get_ProgrammableKeys(&programmableKeys);
      //cout << "Number of Keys=%d\tProgrammable keys=%d\n" << keys << " / " << programmableKeys;
     
      for (long i=1; i<=keys; i++)
      {
         BSTR bstrLabel;
         BSTR bstrName;
         m_pIKeyboard->GetKeyLabel(i, &bstrLabel);
         m_pIKeyboard->GetKeyName(i, &bstrName);
         CString strLabel(bstrLabel);
         CString strName(bstrName);

         //cout << "Key Label=%s\tKey Name=%s\n" << 
           //(const TCHAR *)strLabel << " / " <<  (const TCHAR *)strName;

      }
      mitk::TDMouseEventThrower::GetInstance()->DeviceChange (device, keys, programmableKeys);
   }
   return result;
}

HRESULT SpaceNavigatorDriver::OnKeyDown(int keyCode )
{
   HRESULT result = S_OK;
   mitk::TDMouseEventThrower::GetInstance()->KeyDown (keyCode);
   return result;
}

HRESULT SpaceNavigatorDriver::OnKeyUp(int keyCode )
{
   HRESULT result = S_OK;
   mitk::TDMouseEventThrower::GetInstance()->KeyUp (keyCode);
   return result;
}

#define ScaleRotation		1024.0
#define ScaleTranslation	512.0
#define Sensitivity        1.0

HRESULT SpaceNavigatorDriver::OnSensorInput(void)
{
   HRESULT result = S_OK;
   static DWORD s_dwLastDraw = 0;

   //cout << "CCubeWindow::OnSensorInput()";
   try {
      CComPtr<IAngleAxis> pRotation;
      HRESULT hr = m_pISensor->get_Rotation(&pRotation);

      double angle;
      pRotation->get_Angle(&angle);

      CComPtr<IVector3D> pTranslation;
      hr = m_pISensor->get_Translation(&pTranslation);
   
      double length;
      pTranslation->get_Length(&length);

      if (angle > 0. || length > 0.)
      {
        mitk::Vector3D translation;
        mitk::Vector3D rotation;
        double val;

        double timeFactor = 1.;

        DWORD dwNow = ::GetTickCount();
        if (s_dwLastDraw)
        {
          double period;
          m_pISensor->get_Period(&period);
          timeFactor = (double)(dwNow-s_dwLastDraw)/period;
        }
        s_dwLastDraw = dwNow;

        length *= timeFactor;
        length /= ScaleTranslation*Sensitivity;
        
        pTranslation->put_Length(length);

        pTranslation->get_X(&val);
        translation[0] = (float)val;
        pTranslation->get_Y(&val);
        translation[1] = (float)val;
        pTranslation->get_Z(&val);
        translation[2] = (float)val;
        
        pRotation->get_X(&val);
        rotation[0] = (float)val;
        pRotation->get_Y(&val);
        rotation[1] = (float)val;
        pRotation->get_Z(&val);
        rotation[2] = (float)val;

        angle *= timeFactor;
        angle /= ScaleRotation*Sensitivity;

        mitk::TDMouseEventThrower::GetInstance()->SensorInput( translation, rotation, angle);
  //cout << translation[0] << "\t" << translation[1] << "\t" << translation[2] << endl;
      }
      else
         s_dwLastDraw = 0;

      pRotation.Release();
      pTranslation.Release();
   }
   catch (...)
   {
      cout << "Some sort of exception handling" << endl;
   }

   return result;
}

HRESULT SpaceNavigatorDriver::UninitializeCOM()
{
   HRESULT hr;
   CComPtr<IUnknown> _3DxDevice;

   // Create the device object
   hr = _3DxDevice.CoCreateInstance(__uuidof(Device));
   if (SUCCEEDED(hr))
   {
      CComPtr<ISimpleDevice> _3DxSimpleDevice;
      _3DxDevice.QueryInterface(&_3DxSimpleDevice);
      _3DxSimpleDevice->Disconnect();
   }


   if (m_pISensor)
   {
      // unhook (unadvise) the sensor event sink
      __unhook(&_ISensorEvents::SensorInput, m_pISensor, 
                     &SpaceNavigatorDriver::OnSensorInput);

      m_pISensor.Release();
   }

   if (m_pIKeyboard)
   {
      __unhook(&_IKeyboardEvents::KeyDown, m_pIKeyboard, 
                     &SpaceNavigatorDriver::OnKeyDown);

      m_pIKeyboard.Release();
   }

   return hr;
}
