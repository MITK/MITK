/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNonBlockingAlgorithm_h
#define mitkNonBlockingAlgorithm_h

#include <MitkAlgorithmsExtExports.h>
#include <itkImage.h>
#include <itkMacro.h>
#include <itkObjectFactory.h>

#include <mitkCommon.h>
#include <mitkDataStorage.h>
#include <mitkProperties.h>
#include <mitkPropertyList.h>
#include <mitkSmartPointerProperty.h>
#include <mitkWeakPointer.h>

#include <mitkImage.h>
#include <mitkSurface.h>

#include <mutex>
#include <stdexcept>
#include <string>

/// from itkNewMacro(), additionally calls Initialize(), because this couldn't be done from the constructor of
/// NonBlockingAlgorithm
/// (you can't call virtual functions from the constructor of the superclass)
#define mitkAlgorithmNewMacro(classname)                                                                               \
  \
static Pointer                                                                                                         \
    New(void)                                                                                                          \
  {                                                                                                                    \
    classname *rawPtr = new classname();                                                                               \
    Pointer smartPtr = rawPtr;                                                                                         \
    rawPtr->UnRegister();                                                                                              \
    rawPtr->Initialize();                                                                                              \
    return smartPtr;                                                                                                   \
  \
}                                                                                                                 \
  \
virtual::itk::LightObject::Pointer                                                                                     \
    CreateAnother(void) const override                                                                                 \
  \
{                                                                                                                 \
    Pointer smartPtr = classname::New();                                                                               \
    ::itk::LightObject::Pointer lightPtr = smartPtr.GetPointer();                                                      \
    smartPtr->Initialize(this);                                                                                        \
    return lightPtr;                                                                                                   \
  \
}

namespace mitk
{
  /**
   * \brief Base class for algorithms that can run in a background thread.
   *
   * NonBlockingAlgorithm provides a framework for running computations
   * asynchronously. Subclasses implement ThreadedUpdateFunction() with their
   * algorithm logic. The algorithm can be started via StartAlgorithm() (non-blocking)
   * or StartBlockingAlgorithm() (blocking). Upon completion, a ResultAvailable or
   * ProcessingError event is emitted via the GUI thread.
   *
   * Parameters are stored in a PropertyList and can be set via SetParameter(),
   * SetPointerParameter(), or SetItkImageAsMITKImagePointerParameter(). Trigger
   * parameters can be defined so that modifying them automatically restarts the
   * algorithm.
   *
   * \note Use the mitkAlgorithmNewMacro() macro for subclass instantiation, as
   * Initialize() cannot be called from the base constructor (virtual dispatch).
   *
   * \sa NonBlockingAlgorithmEvent
   * \sa ResultAvailable
   * \sa ProcessingError
   * \sa SegmentationSink
   */
  class MITKALGORITHMSEXT_EXPORT NonBlockingAlgorithm : public itk::Object
  {
  public:
    mitkClassMacroItkParent(NonBlockingAlgorithm, itk::Object);

    /**
     * \brief Set the DataStorage for this algorithm.
     * \param[in] storage Reference to the DataStorage to use.
     */
    void SetDataStorage(DataStorage &storage);

    /**
     * \brief Get the DataStorage associated with this algorithm.
     * \return Pointer to the DataStorage, or nullptr if expired.
     */
    DataStorage *GetDataStorage();

    // parameter setting

    /**
     * \brief Set a parameter of any normal (value) type.
     * \tparam T The parameter value type.
     * \param[in] parameter The parameter name.
     * \param[in] value The parameter value.
     */
    template <typename T>
    void SetParameter(const char *parameter, const T &value)
    {
      // MITK_INFO << "SetParameter(" << parameter << ") " << typeid(T).name() << std::endl;
      // m_ParameterListMutex->Lock();
      m_Parameters->SetProperty(parameter, GenericProperty<T>::New(value));
      // m_ParameterListMutex->Unlock();
    }

    /**
     * \brief Set a parameter that holds a smart pointer.
     * \tparam T The type pointed to by the smart pointer.
     * \param[in] parameter The parameter name.
     * \param[in] value The smart pointer value.
     */
    template <typename T>
    void SetPointerParameter(const char *parameter, const itk::SmartPointer<T> &value)
    {
      // MITK_INFO << this << "->SetParameter smartpointer(" << parameter << ") " << typeid(itk::SmartPointer<T>).name()
      // << std::endl;
      m_ParameterListMutex.lock();
      m_Parameters->SetProperty(parameter, SmartPointerProperty::New(value.GetPointer()));
      m_ParameterListMutex.unlock();
    }
    // virtual void SetParameter( const char*, mitk::BaseProperty* ); // for "number of iterations", ...
    // create some property observing to inform algorithm object about changes
    // perhaps some TriggerParameter(string) macro that creates an observer for changes in a specific property like
    // "2ndPoint" for LineAlgorithms

    /**
     * \brief Set a parameter that holds a BaseData pointer (Image, Surface, etc.).
     *
     * The value is stored internally as a SmartPointerProperty.
     *
     * \param[in] parameter The parameter name.
     * \param[in] value The BaseData pointer.
     */
    void SetPointerParameter(const char *parameter, BaseData *value);

    /**
     * \brief Set an ITK image as a MITK Image parameter (raw pointer version).
     *
     * The ITK image is imported into a mitk::Image and stored as a pointer parameter.
     *
     * \tparam TPixel The pixel type of the ITK image.
     * \tparam VImageDimension The dimensionality of the ITK image.
     * \param[in] parameter The parameter name.
     * \param[in] itkImage The ITK image to store.
     */
    template <typename TPixel, unsigned int VImageDimension>
    void SetItkImageAsMITKImagePointerParameter(const char *parameter, itk::Image<TPixel, VImageDimension> *itkImage)
    {
      // MITK_INFO << "SetParameter ITK image(" << parameter << ") " << typeid(itk::Image<TPixel,
      // VImageDimension>).name() << std::endl;
      // create an MITK image for that
      mitk::Image::Pointer mitkImage = mitk::Image::New();
      mitkImage = ImportItkImage(itkImage);
      SetPointerParameter(parameter, mitkImage);
    }

    /**
     * \brief Set an ITK image as a MITK Image parameter (smart pointer version).
     *
     * The ITK image is imported into a mitk::Image and stored as a pointer parameter.
     *
     * \tparam TPixel The pixel type of the ITK image.
     * \tparam VImageDimension The dimensionality of the ITK image.
     * \param[in] parameter The parameter name.
     * \param[in] itkImage Smart pointer to the ITK image to store.
     */
    template <typename TPixel, unsigned int VImageDimension>
    void SetItkImageAsMITKImagePointerParameter(const char *parameter,
                                                const itk::SmartPointer<itk::Image<TPixel, VImageDimension>> &itkImage)
    {
      // MITK_INFO << "SetParameter ITK image(" << parameter << ") " << typeid(itk::SmartPointer<itk::Image<TPixel,
      // VImageDimension> >).name() << std::endl;
      // create an MITK image for that
      mitk::Image::Pointer mitkImage = mitk::Image::New();
      mitkImage = ImportItkImage(itkImage);
      SetPointerParameter(parameter, mitkImage);
    }

    /**
     * \brief Get a parameter of any normal (value) type.
     * \tparam T The parameter value type.
     * \param[in] parameter The parameter name.
     * \param[out] value The retrieved parameter value.
     * \throw std::invalid_argument if the parameter does not exist.
     */
    template <typename T>
    void GetParameter(const char *parameter, T &value) const
    {
      // MITK_INFO << "GetParameter normal(" << parameter << ") " << typeid(T).name() << std::endl;
      // m_ParameterListMutex->Lock();
      BaseProperty *p = m_Parameters->GetProperty(parameter);
      GenericProperty<T> *gp = dynamic_cast<GenericProperty<T> *>(p);
      if (gp)
      {
        value = gp->GetValue();
        // m_ParameterListMutex->Unlock();
        return;
      }
      // m_ParameterListMutex->Unlock();

      std::string error("There is no parameter \"");
      error += parameter;
      error += '"';
      throw std::invalid_argument(error);
    }

    /**
     * \brief Get a parameter that holds a smart pointer.
     * \tparam T The type pointed to by the smart pointer.
     * \param[in] parameter The parameter name.
     * \param[out] value The retrieved smart pointer value.
     * \throw std::invalid_argument if the parameter does not exist.
     */
    template <typename T>
    void GetPointerParameter(const char *parameter, itk::SmartPointer<T> &value) const
    {
      // MITK_INFO << this << "->GetParameter smartpointer(" << parameter << ") " << typeid(itk::SmartPointer<T>).name()
      // << std::endl;
      // m_ParameterListMutex->Lock();
      BaseProperty *p = m_Parameters->GetProperty(parameter);
      if (p)
      {
        SmartPointerProperty *spp = dynamic_cast<SmartPointerProperty *>(p);
        if (spp)
        {
          T *t = dynamic_cast<T *>(spp->GetSmartPointer().GetPointer());
          value = t;
          // m_ParameterListMutex->Unlock();
          return;
        }
      }
      // m_ParameterListMutex->Unlock();

      std::string error("There is no parameter \"");
      error += parameter;
      error += '"';
      throw std::invalid_argument(error);
    }

    /**
     * \brief Reset the algorithm to its initial state.
     *
     * Calls Initialize() to re-create default parameters.
     */
    virtual void Reset();

    /**
     * \brief Start the algorithm asynchronously in a background thread.
     *
     * The algorithm will only start if ReadyToRun() returns true and no kill
     * request is pending. If the thread is already running, additional update
     * requests are queued.
     */
    void StartAlgorithm();

    /**
     * \brief Start the algorithm and block until it completes.
     *
     * Equivalent to calling StartAlgorithm() followed by StopAlgorithm().
     */
    void StartBlockingAlgorithm();

    /**
     * \brief Wait for the running algorithm thread to finish.
     */
    void StopAlgorithm();

    /**
     * \brief Callback invoked when a trigger parameter is modified.
     *
     * Automatically restarts the algorithm.
     *
     * \param[in] event The modification event (unused).
     */
    void TriggerParameterModified(const itk::EventObject & event);

    /**
     * \brief GUI-thread callback when the threaded update succeeds.
     * \param[in] event The event object (unused).
     */
    void ThreadedUpdateSuccessful(const itk::EventObject & event);

    /**
     * \brief GUI-thread callback when the threaded update fails.
     * \param[in] event The event object (unused).
     */
    void ThreadedUpdateFailed(const itk::EventObject & event);

  protected:
    NonBlockingAlgorithm(); // use smart pointers
    ~NonBlockingAlgorithm() override;

    void DefineTriggerParameter(const char *);
    void UnDefineTriggerParameter(const char *);

    virtual void Initialize(const NonBlockingAlgorithm *other = nullptr);
    virtual bool ReadyToRun();

    virtual bool ThreadedUpdateFunction();   // will be called from a thread after calling StartAlgorithm
    virtual void ThreadedUpdateSuccessful(); // will be called after the ThreadedUpdateFunction() returned
    virtual void ThreadedUpdateFailed();     // will when ThreadedUpdateFunction() returns false

    PropertyList::Pointer m_Parameters;

    WeakPointer<DataStorage> m_DataStorage;

  private:
    static void StaticNonBlockingAlgorithmThread(NonBlockingAlgorithm* algorithm);

    typedef std::map<std::string, unsigned long> MapTypeStringUInt;

    MapTypeStringUInt m_TriggerPropertyConnections;

    std::mutex m_ParameterListMutex;

    int m_UpdateRequests;
    std::thread m_Thread;

    bool m_KillRequest;
  };

} // namespace

#include <mitkNonBlockingAlgorithmEvents.h>

#endif
