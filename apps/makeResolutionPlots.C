void makeResolutionPlots(TString input_file_dir, unsigned int parametrization_level, const int verboseLevel = 0) {
  std::cout << "Running resolution plotter macro....\n";

  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------

  gSystem->Load("libLmdFit");

  // create an instance of PndLmdResultPlotter the plotting helper class
  PndLmdResultPlotter plotter;
  // A small helper class that helps to construct lmddata objects
  PndLmdDataFacade data_facade;

  // ================================ BEGIN CONFIG ================================ //
  // PndLmdResultPlotter sets default pad margins etc that should be fine for most cases
  // you can fine tune it and overwrite the default values 
  gStyle->SetPadTopMargin(0.06);
  //gStyle->SetPadBottomMargin(0.12);
  gStyle->SetPadLeftMargin(0.13);
  gStyle->SetPadRightMargin(0.02);

  gStyle->SetTitleX(0.3);
  //gStyle->SetTitleY();
  //gStyle->SetTitleW();
  //gStyle->SetTitleH();

  // overwrite the default theta plot range if possible (if its larger than the max
  // plot range then it has no effect)
  plotter.setThetaPlotRange(-6.0, 6.0);

  // The plotter has more options for text positioning and tex sizes for which you can
  // overwrite the default values here
  plotter.setTextLeftPos(0.25);
  plotter.setTextTopPos(0.92);
  //plotter.setTextSpacing(0.08);
  plotter.setTextSize(0.06);
  plotter.setLabelSize(0.06);

  //plotter.setLabelOffsetX(0.007);
  //plotter.setLabelOffsetY(0.007);
  //plotter.setTitleOffsetX(1.0);
  plotter.setTitleOffsetY(1.13);
  // ================================= END CONFIG ================================= //
  

  // =============================== BEGIN PLOTTING =============================== //

  char input_filename[50];
  sprintf(input_filename, "/resolution_params_%u.root", parametrization_level);
  TString input_filename_url = input_file_dir + input_filename;
  TFile *infile = new TFile(input_filename_url, "READ");


  switch (parametrization_level) {
    case 0:
      // read in data from a root file
    	std::map<LumiFit::LmdDimensionOptions, std::vector<PndLmdResolution*> > res_map = data_facade.getFittedResolutionsFromPath(infile);
      // first lets create a booky of all resolutions with the fitted resolutions
      plotter.makeResolutionBooky(res_map, "resolution");
      break;
    case 1:
      plotter.makeResolutionSummaryPlots(infile);
      break;
    default:
      std::cout << "Error: The requested parametrization level " << parametrization_level
        << " does not exist. The highest level is 3." << std::endl;

      break;
  }
    
  // ================================ END PLOTTING ================================ //
}
