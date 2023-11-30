#include "TCanvas.h"
#include <string>

void plotTheta(std::string inFile) {
  if (inFile == "") {
    std::cout << "Usage: root -l -q 'plotTheta.C(\"inFile.root\")'"
              << std::endl;
    // return;
    infile = "lmd_acc_data_1of1.root";
  }
}
