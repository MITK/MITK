MITK_CREATE_MODULE(PharmacokineticModelsServices
  DEPENDS
    PUBLIC MitkPharmacokinetics
    PRIVATE MitkCore
  AUTOLOAD_WITH MitkModelFit
)
