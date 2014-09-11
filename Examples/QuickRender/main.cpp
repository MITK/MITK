
#include "QmlMitkFourRenderWindowWidget.h"

#include <QGuiApplication>
#include <QtQuick>

#include "mitkStandaloneDataStorage.h"
#include "mitkDataNodeFactory.h"
#include "mitkGlobalInteraction.h"
#include "mitkImage.h"

#include "QmlMitkBigRenderLock.h"

mitk::DataStorage::Pointer FillDataStorage(int argc, char **argv)
{
  if (argc < 2)
  {
    fprintf(stderr, "Usage:   %s [filename1] [filename2] ...\n", "");
    exit(EXIT_FAILURE);
  }

  // Create a DataStorage
  mitk::DataStorage::Pointer storage = mitk::StandaloneDataStorage::New().GetPointer();

  //*************************************************************************
  // Part II: Create some data by reading files
  //*************************************************************************
  int i;
  for (i = 1; i < argc; ++i)
  {
    // For testing
    if (strcmp(argv[i], "-testing") == 0)
      continue;

    // Create a DataNodeFactory to read a data format supported
    // by the DataNodeFactory (many image formats, surface formats, etc.)
    mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
    const char * filename = argv[i];
    try
    {
      nodeReader->SetFileName(filename);
      nodeReader->Update();

      // Since the DataNodeFactory directly creates a node,
      // use the datastorage to add the read node
      mitk::DataNode::Pointer node = nodeReader->GetOutput();
      storage->Add(node);

      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
      if (image.IsNotNull())
      {
        // Set the property "volumerendering" to the Boolean value "true"
        node->SetProperty("volumerendering", mitk::BoolProperty::New(false));
        node->SetProperty("name", mitk::StringProperty::New("testimage"));
        node->SetProperty("layer", mitk::IntProperty::New(1));
      }
    } catch (...)
    {
      fprintf(stderr, "Could not open file %s \n\n", filename);
      return 0;
    }
  }

  return storage;
}

#include "QmlMitkRenderingManagerFactory.h"
void MitkStaticInitialization()
{
  // TODO THIS IS BAD! Move to module activator
  static QmlMitkRenderingManagerFactory sanglier;

  mitk::GlobalInteraction::GetInstance()->Initialize("global"); // unbelievable.. still necessary..
}

void SetupRenderWindowItems( QQuickItem* container, mitk::DataStorage::Pointer dataStorage )
{
  QmlMitkFourRenderWindowWidget* mitkMultiWidget = container->findChild<QmlMitkFourRenderWindowWidget*>("mitkMultiWidget");
  if (mitkMultiWidget)
  {
    mitkMultiWidget->SetDataStorage( dataStorage );
  }
  else
  {
    MITK_ERROR << "No QmlMitkFourRenderWindowWidget item found during QuickRender application initialization. Uh oh..";
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    MitkStaticInitialization();
    mitk::DataStorage::Pointer dataStorage = FillDataStorage(argc,argv);

    // TODO: where to put this? Module activator?? There must be a more Qt-style place..
    qmlRegisterType<QmlMitkRenderWindowItem>("QmlMitk", 1, 0, "QmlMitkRenderWindowItem");
    qmlRegisterType<QmlMitkFourRenderWindowWidget>("QmlMitk", 1, 0, "QmlMitkFourRenderWindowWidget");
    QQuickView view;

    view.setSource(QUrl("qrc:///MITK/Examples/QuickRender/QuickRender.qml"));
    view.setResizeMode( QQuickView::SizeRootObjectToView );


    QQuickItem* root = view.rootObject();
    SetupRenderWindowItems( root, dataStorage );

    view.show();

    QmlMitkBigRenderLock giantRenderLock;
    app.installEventFilter(&giantRenderLock);
    return app.exec();
}
