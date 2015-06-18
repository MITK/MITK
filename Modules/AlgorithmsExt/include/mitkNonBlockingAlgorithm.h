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

#ifndef MITK_NON_BLOCKING_ALGORITHM_H_INCLUDED_DFARdfWN1tr
#define MITK_NON_BLOCKING_ALGORITHM_H_INCLUDED_DFARdfWN1tr

#include <itkObjectFactory.h>
#include "MitkAlgorithmsExtExports.h"
#include <itkMacro.h>
#include <itkMultiThreader.h>
#include <itkFastMutexLock.h>
#include <itkImage.h>

#include "mitkCommon.h"
#include "mitkPropertyList.h"
#include "mitkProperties.h"
#include "mitkSmartPointerProperty.h"
#include "mitkDataStorage.h"
#include "mitkWeakPointer.h"

#include "mitkImage.h"
#include "mitkSurface.h"

#include <string>
#include <stdexcept>

/// from itkNewMacro(), additionally calls Initialize(), because this couldn't be done from the constructor of NonBlockingAlgorithm
/// (you can't call virtual functions from the constructor of the superclass)
#define mitkAlgorithmNewMacro( classname ) \
static Pointer New(void) { \
  classname* rawPtr = new classname(); \
  Pointer smartPtr = rawPtr; \
  rawPtr->UnRegister(); \
  rawPtr->Initialize(); \
  return smartPtr; \
}\
virtual ::itk::LightObject::Pointer CreateAnother(void) const override\
{\
  Pointer smartPtr = classname::New();\
  ::itk::LightObject::Pointer lightPtr = smartPtr.GetPointer();\
  smartPtr->Initialize(this);\
  return lightPtr;\
}

namespace mitk
{

/*!
    Invokes ResultsAvailable with each new result

    <b>done</b> centralize use of itk::MultiThreader in this class
    @todo do the property-handling in this class
    @todo process "incoming" events in this class
    @todo sollen segmentierungs-dinger von mitk::ImageSource erben? Ivo fragen, wie das mit AllocateOutputs, etc. gehen soll
          eine ImageSourceAlgorithm koennte dann die noetigen Methoden wie GenerateData(), GetOutput() ueberschreiben, so
          dass von dort aus die Methoden von NonBlockingAlgorithm aufgerufen werden.
          Erben v.a. um die Output-Sachen zu uebernehmen, die Anpassungen das einfuehren einer Zwischenklasse, um die Interaces zu verheiraten.
*/
class MITKALGORITHMSEXT_EXPORT NonBlockingAlgorithm : public itk::Object
{
  public:

    // for threading
    class MITKALGORITHMSEXT_EXPORT ThreadParameters
    {
      public:
        itk::SmartPointer<NonBlockingAlgorithm> m_Algorithm;
    };


    mitkClassMacroItkParent( NonBlockingAlgorithm, itk::Object )

    void SetDataStorage(DataStorage& storage);
    DataStorage* GetDataStorage();

// parameter setting

    /// For any kind of normal types
    template <typename T>
    void SetParameter(const char* parameter, const T& value)
    {
      //MITK_INFO << "SetParameter(" << parameter << ") " << typeid(T).name() << std::endl;
      //m_ParameterListMutex->Lock();
      m_Parameters->SetProperty(parameter, GenericProperty<T>::New(value) );
      //m_ParameterListMutex->Unlock();
    }

    /// For any kind of smart pointers
    template <typename T>
    void SetPointerParameter(const char* parameter, const itk::SmartPointer<T>& value)
    {
      //MITK_INFO << this << "->SetParameter smartpointer(" << parameter << ") " << typeid(itk::SmartPointer<T>).name() << std::endl;
      m_ParameterListMutex->Lock();
      m_Parameters->SetProperty(parameter, SmartPointerProperty::New(value.GetPointer()) );
      m_ParameterListMutex->Unlock();
    }
     //virtual void SetParameter( const char*, mitk::BaseProperty* ); // for "number of iterations", ...
                            // create some property observing to inform algorithm object about changes
                           // perhaps some TriggerParameter(string) macro that creates an observer for changes in a specific property like "2ndPoint" for LineAlgorithms

    /// For any kind of BaseData, like Image, Surface, etc. Will be stored inside some SmartPointerProperty
    void SetPointerParameter(const char* parameter, BaseData* value);

    /// For any kind of ITK images (C pointers)
    template <typename TPixel, unsigned int VImageDimension>
    void SetItkImageAsMITKImagePointerParameter(const char* parameter, itk::Image<TPixel, VImageDimension>* itkImage)
    {
      //MITK_INFO << "SetParameter ITK image(" << parameter << ") " << typeid(itk::Image<TPixel, VImageDimension>).name() << std::endl;
      // create an MITK image for that
      mitk::Image::Pointer mitkImage = mitk::Image::New();
      mitkImage = ImportItkImage( itkImage );
      SetPointerParameter( parameter, mitkImage );
    }

    /// For any kind of ITK images (smartpointers)
    template <typename TPixel, unsigned int VImageDimension>
    void SetItkImageAsMITKImagePointerParameter(const char* parameter, const itk::SmartPointer<itk::Image<TPixel, VImageDimension> >& itkImage)
    {
      //MITK_INFO << "SetParameter ITK image(" << parameter << ") " << typeid(itk::SmartPointer<itk::Image<TPixel, VImageDimension> >).name() << std::endl;
      // create an MITK image for that
      mitk::Image::Pointer mitkImage = mitk::Image::New();
      mitkImage = ImportItkImage( itkImage );
      SetPointerParameter( parameter, mitkImage );
    }

// parameter getting

    template <typename T>
    void GetParameter(const char* parameter, T& value) const
    {
      //MITK_INFO << "GetParameter normal(" << parameter << ") " << typeid(T).name() << std::endl;
      //m_ParameterListMutex->Lock();
      BaseProperty* p = m_Parameters->GetProperty(parameter);
      GenericProperty<T>* gp = dynamic_cast< GenericProperty<T>* >( p );
      if ( gp )
      {
        value = gp->GetValue();
        //m_ParameterListMutex->Unlock();
        return;
      }
      //m_ParameterListMutex->Unlock();

      std::string error("There is no parameter \"");
      error += parameter;
      error += '"';
      throw std::invalid_argument( error );
    }

    template <typename T>
    void GetPointerParameter(const char* parameter, itk::SmartPointer<T>& value) const
    {
      //MITK_INFO << this << "->GetParameter smartpointer(" << parameter << ") " << typeid(itk::SmartPointer<T>).name() << std::endl;
      //m_ParameterListMutex->Lock();
      BaseProperty* p = m_Parameters->GetProperty(parameter);
      if (p)
      {
        SmartPointerProperty* spp = dynamic_cast< SmartPointerProperty* >( p );
        if ( spp )
        {
          T* t = dynamic_cast<T*>( spp->GetSmartPointer().GetPointer() );
          value = t;
          //m_ParameterListMutex->Unlock();
          return;
        }
      }
      //m_ParameterListMutex->Unlock();

      std::string error("There is no parameter \"");
      error += parameter;
      error += '"';
      throw std::invalid_argument( error );
    }

// start/stop functions

    virtual void Reset();

    void StartAlgorithm(); // for those who want to trigger calculations on their own
                          // --> need for an OPTION: manual/automatic starting
    void StartBlockingAlgorithm(); // for those who want to trigger calculations on their own
    void StopAlgorithm();

    void TriggerParameterModified(const itk::EventObject&);

    void ThreadedUpdateSuccessful(const itk::EventObject&);
    void ThreadedUpdateFailed(const itk::EventObject&);

  protected:

    NonBlockingAlgorithm();  // use smart pointers
    virtual ~NonBlockingAlgorithm();

    void DefineTriggerParameter(const char*);
    void UnDefineTriggerParameter(const char*);

    virtual void Initialize(const NonBlockingAlgorithm* other = nullptr);
    virtual bool ReadyToRun();

    virtual bool ThreadedUpdateFunction(); // will be called from a thread after calling StartAlgorithm
    virtual void ThreadedUpdateSuccessful(); // will be called after the ThreadedUpdateFunction() returned
    virtual void ThreadedUpdateFailed(); // will when ThreadedUpdateFunction() returns false

    PropertyList::Pointer m_Parameters;

    WeakPointer<DataStorage> m_DataStorage;

  private:

    static ITK_THREAD_RETURN_TYPE StaticNonBlockingAlgorithmThread(void* param);

    typedef std::map< std::string, unsigned long > MapTypeStringUInt;

    MapTypeStringUInt m_TriggerPropertyConnections;

    itk::FastMutexLock::Pointer m_ParameterListMutex;

    int m_ThreadID;
    int m_UpdateRequests;
    ThreadParameters m_ThreadParameters;
    itk::MultiThreader::Pointer m_MultiThreader;

    bool m_KillRequest;

};

} // namespace

#include "mitkNonBlockingAlgorithmEvents.h"

#endif
