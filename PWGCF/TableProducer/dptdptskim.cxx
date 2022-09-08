// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "Framework/AnalysisTask.h"
#include "Framework/AnalysisDataModel.h"
#include "Framework/ASoAHelpers.h"
#include "Common/DataModel/TrackSelectionTables.h"
#include "PWGCF/Core/AnalysisConfigurableCuts.h"
#include "PWGCF/Core/TrackSelectionFilterAndAnalysis.h"
#include "PWGCF/Core/EventSelectionFilterAndAnalysis.h"
#include "Framework/runDataProcessing.h"

using namespace o2;
using namespace o2::framework;
using namespace o2::soa;
using namespace o2::framework::expressions;
using namespace o2::analysis;

namespace o2::analysis::dptdptskimming
{
PWGCF::TrackSelectionFilterAndAnalysis* fTrackFilter = nullptr;
PWGCF::EventSelectionFilterAndAnalysis* fEventFilter = nullptr;
}

using namespace dptdptskimming; 

struct DptDptSkim {

#include "dptdptskimconf.h"

  int nReportedTracks;

  void init(InitContext const&)
  {
    using namespace dptdptskimming;

    LOGF(info, "DptDptSkimTask::init()");

    /* collision filtering configuration */
    PWGCF::EventSelectionConfigurable eventsel("", "", eventfilter.zvtxsel, "");
    fEventFilter = new PWGCF::EventSelectionFilterAndAnalysis(eventsel, PWGCF::SelectionFilterAndAnalysis::kFilter);

    /* track filtering configuration */
    PWGCF::TrackSelectionConfigurable trksel(trackfilter.ttype, trackfilter.nclstpc, trackfilter.nxrtpc, trackfilter.nclsits, trackfilter.chi2clustpc,
                                             trackfilter.chi2clusits, trackfilter.xrofctpc, trackfilter.dcaxy, trackfilter.dcaz, trackfilter.ptrange, trackfilter.etarange);
    fTrackFilter = new PWGCF::TrackSelectionFilterAndAnalysis(trksel, PWGCF::SelectionFilterAndAnalysis::kFilter);
    nReportedTracks = 0;
  }

  void process(aod::Collision const& collision, soa::Join<aod::FullTracks, aod::TracksDCA> const& tracks)
  {
    LOGF(info, "Got a new collision with zvtx %.2f", collision.posZ());

    auto colmask = fEventFilter->Filter(collision);
    LOGF(info, "Got mask 0x%lx", colmask);

    if (colmask != 0UL) {
      for (auto const& track : tracks) {
        auto trkmask = fTrackFilter->Filter(track);
        if ((trkmask & 0xFFFFF9FF) != 0UL and nReportedTracks < 1000) {
          LOGF(info, "  Got track mask 0x%lx, TPC clusters %d, Chi2 per TPC cluster %f, pT %f, eta %f, track type %d",
               trkmask, track.tpcNClsFound(), track.tpcChi2NCl(), track.pt(), track.eta(), track.trackType());
          nReportedTracks++;
        }
      }
    }
  }
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  WorkflowSpec workflow{
    adaptAnalysisTask<DptDptSkim>(cfgc)};
  return workflow;
}

