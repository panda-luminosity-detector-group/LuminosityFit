#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class std::pair<std::string,std::string>+;
#pragma link C++ class std::pair<unsigned int,unsigned int>+;
#pragma link C++ class ModelFitResult+;
#pragma link C++ struct ModelStructs::minimization_parameter+;

#pragma link C++ class EstimatorOptions+;
#pragma link C++ struct DataStructs::DimensionRange+;
#pragma link C++ class LumiFit::LmdDimensionOptions+;
#pragma link C++ class LumiFit::LmdDimensionRange+;
#pragma link C++ class LumiFit::LmdDimension+;
#pragma link C++ enum LumiFit::LmdDimensionType+;

#pragma link C++ class std::set < LumiFit::LmdDimension >+;
#pragma link C++ class std::set < LumiFit::LmdDimensionType >+;

#pragma link C++ class PndLmdLumiFitResult+;
#pragma link C++ class PndLmdFitStorage+;
#pragma link C++ class PndLmdElasticDataBundle+;
#pragma link C++ class PndLmdFitDataBundle+;
#pragma link C++ class PndLmdFitOptions+;
#pragma link C++ class PndLmdAbstractData+;
#pragma link C++ class PndLmdHistogramData+;
#pragma link C++ class PndLmdAngularData+;
#pragma link C++ class PndLmdAcceptance+;

#pragma link C++ class std::vector < PndLmdAngularData >+;
#pragma link C++ class std::vector < PndLmdAcceptance >+;
#pragma link C++ class std::vector < PndLmdHistogramData >+;

#endif
