@PLUGIN_COPYRIGHT@

#ifndef @UBUNDLE_NAMESPACE@_@UACTIVATOR_CLASS@_H_INCLUDED
#define @UBUNDLE_NAMESPACE@_@UACTIVATOR_CLASS@_H_INCLUDED

#include <@ACTIVATOR_BASE_CLASS_H@>

@BEGIN_NAMESPACE@

/*!
 * \ingroup @NormalizedPluginID@_internal
 *
 * \brief @ACTIVATOR_CLASS@
 *
 * Document your class here.
 *
 * \sa @ACTIVATOR_BASE_CLASS@
 */
class @ACTIVATOR_CLASS@ : public @ACTIVATOR_BASE_CLASS@
{

  public:

    void Start(berry::IBundleContext::Pointer context);
    void Stop(berry::IBundleContext::Pointer context);

};


@END_NAMESPACE@

#endif // @UBUNDLE_NAMESPACE@_@UACTIVATOR_CLASS@_H_INCLUDED
