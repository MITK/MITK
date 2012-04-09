
#include <QObject>

#include <ctkPluginActivator.h>
#include <ctkPluginException.h>
#include <service/cm/ctkConfigurationAdmin.h>
#include <service/cm/ctkConfiguration.h>
#include <service/event/ctkEventAdmin.h>

class MyApplicationPlugin : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

  MyApplicationPlugin();
  ~MyApplicationPlugin();

  //! [0]
  void start(ctkPluginContext* context)
  {
    // Get a service reference for the Config Admin service
    ctkServiceReference cmRef = context->getServiceReference<ctkConfigurationAdmin>();
    ctkConfigurationAdmin* configAdmin = 0;
    if (cmRef)
    {
      configAdmin = context->getService<ctkConfigurationAdmin>(cmRef);
    }

    // Use the CTK Configuration Admin service to configure the BlueBerry help system.
    // This assumes that the plug-in providing the Config Admin implementation is
    // already active.
    if (configAdmin)
    {
      // Get a ctkConfiguration object for the PID "org.blueberry.services.help"
      // (or create an unbound instance if it does not exist yet).
      ctkConfigurationPtr conf = configAdmin->getConfiguration("org.blueberry.services.help", QString());

      // Configure the help system using a custom home page
      ctkDictionary helpProps;
      helpProps.insert("homePage", "qthelp://org.company.plugin/bundle/index.html");
      conf->update(helpProps);

      // Unget the service
      context->ungetService(cmRef);
    }
    else
    {
      // Warn that the Config Admin service is unavailable
    }
  }
  //! [0]

  void stop(ctkPluginContext *context);

  //! [1]
  void requestHelp(ctkPluginContext* context)
  {
    if (context == 0)
    {
      // Warn that the plugin context is zero
      return;
    }

    // Check if the org.blueberry.ui.qt.help plug-in is installed and started
    QList<QSharedPointer<ctkPlugin> > plugins = context->getPlugins();
    foreach(QSharedPointer<ctkPlugin> p, plugins)
    {
      if (p->getSymbolicName() == "org.blueberry.ui.qt.help" &&
          p->getState() != ctkPlugin::ACTIVE)
      {
        // The plug-in is in RESOLVED state but is not started yet.
        // Try to activate the plug-in explicitly, so that it can react
        // to events send via the CTK Event Admin.
        try
        {
          p->start(ctkPlugin::START_TRANSIENT);
        }
        catch (const ctkPluginException& pe)
        {
          // Warn that activating the org.blueberry.ui.qt.help plug-in failed
          return;
        }
      }
    }

    ctkServiceReference eventAdminRef = context->getServiceReference<ctkEventAdmin>();
    ctkEventAdmin* eventAdmin = 0;
    if (eventAdminRef)
    {
      eventAdmin = context->getService<ctkEventAdmin>(eventAdminRef);
    }

    if (eventAdmin == 0)
    {
      // Warn that the ctkEventAdmin service was not found
    }
    else
    {
      // Create the event and send it asynchronuously
      ctkEvent ev("org/blueberry/ui/help/CONTEXTHELP_REQUESTED");
      eventAdmin->postEvent(ev);
    }
  }
  //! [1]

private:

};

int main(int argc, char* argv[])
{
  return 0;
}
