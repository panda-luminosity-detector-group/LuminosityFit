#ifndef DATA_TRACKDATA_H_
#define DATA_TRACKDATA_H_

#include <array>

namespace Lmd {
    namespace Data {
        struct Track {
            std::array<double, 3> Position;
            std::array<double, 3> Momentum;
        };

        // It is assumed that events are containing only single tracks.
        struct TrackPairInfo {
            Track RecoLMD;
            Track RecoIP;
            Track MCLMD;
            Track MCIP;

            // using std::optional would be better, but lets keep the c++ standard at 11 for now
            bool IsReconstructedAtLmd = false;
            bool IsReconstructedAtIP = false;
            bool IsSecondary = false;       
            int PDGCode = -2212;
        };
    }
}

#endif
