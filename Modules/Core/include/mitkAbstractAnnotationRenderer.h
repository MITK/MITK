/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAbstractAnnotationRenderer_h
#define mitkAbstractAnnotationRenderer_h

#include <mitkAnnotation.h>
#include <mitkServiceInterface.h>
#include <usGetModuleContext.h>
#include <usServiceTracker.h>
#include <MitkCoreExports.h>
#include <mitkCommon.h>

namespace mitk
{
  class BaseRenderer;

  /**
   * \brief Base class for Annotation layout managers.
   *
   * An AbstractAnnotationRenderer can be implemented to control a set of Annotation
   * objects by means of position and size within a specific BaseRenderer. It tracks
   * Annotation micro services and manages their lifecycle.
   *
   * \sa Annotation
   * \sa BaseRenderer
   */
  class MITKCORE_EXPORT AbstractAnnotationRenderer : public us::ServiceTracker<Annotation>
  {
  public:
    typedef us::ServiceTracker<Annotation> Superclass;

    /**
     * \brief Construct an annotation renderer for a specific renderer and annotation renderer ID.
     *
     * \param[in] rendererID The ID of the BaseRenderer this annotation renderer is associated with.
     * \param[in] arID The unique ID for this annotation renderer.
     */
    AbstractAnnotationRenderer(const std::string &rendererID, const std::string &arID);

    /** \brief Virtual destructor. */
    ~AbstractAnnotationRenderer() override;

    /**
     * \brief Get the unique ID of this annotation renderer.
     * \return The annotation renderer ID.
     */
    const std::string GetID() const;

    /**
     * \brief Get the ID of the associated BaseRenderer.
     * \return The renderer ID string.
     */
    const std::string GetRendererID() const;

    /** \brief Notify that the current BaseRenderer has changed. */
    void CurrentBaseRendererChanged();

    /** \brief Called when the render window is modified. Override to react to changes. */
    virtual void OnRenderWindowModified() {}

    /** \brief Remove all tracked Annotation objects from the associated renderer. */
    void RemoveAllAnnotation();

    /** \brief Update all tracked Annotation objects. */
    void Update();

    static const std::string US_INTERFACE_NAME;    ///< Micro service interface name.
    static const std::string US_PROPKEY_ID;         ///< Service property key for the annotation renderer ID.
    static const std::string US_PROPKEY_RENDERER_ID; ///< Service property key for the renderer ID.

  protected:
    /**
     * \brief Get the BaseRenderer associated with this annotation renderer.
     * \return Pointer to the current BaseRenderer.
     */
    BaseRenderer *GetCurrentBaseRenderer();

  private:
    /** \brief copy constructor (disabled) */
    AbstractAnnotationRenderer(const AbstractAnnotationRenderer &);

    /** \brief assignment operator (disabled) */
    AbstractAnnotationRenderer &operator=(const AbstractAnnotationRenderer &);

    TrackedType AddingService(const ServiceReferenceType &reference) override;

    void ModifiedService(const ServiceReferenceType & /*reference*/, TrackedType tracked) override;

    void RemovedService(const ServiceReferenceType & /*reference*/, TrackedType tracked) override;

    /** \brief Called when the set of tracked annotations changes. Override to react. */
    virtual void OnAnnotationRenderersChanged() {}

    const std::string m_RendererID;
    const std::string m_ID;
  };

} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::AbstractAnnotationRenderer, "org.mitk.services.AbstractAnnotationRenderer")

#endif
