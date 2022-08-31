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
#include "PWGCF/Core/AnalysisConfigurableCuts.h"
#include "PWGCF/Core/TrackSelectionFilterAndAnalysis.h"
#include "Framework/runDataProcessing.h"

using namespace o2;
using namespace o2::framework;
using namespace o2::soa;
using namespace o2::framework::expressions;
using namespace o2::analysis;

namespace o2::analysis::dptdptskimming
{
PWGCF::TrackSelectionFilterAndAnalysis* fTrackFilter = nullptr;
}

using namespace dptdptskimming; 

struct DptDptSkim {

#include "dptdptfilterconf.h"

  void init(InitContext const&)
  {
    using namespace dptdptskimming;

    LOGF(info, "DptDptSkimTask::init()");

    /* track filtering configuration */
    PWGCF::TrackSelectionConfigurable trksel(trackfilter.ttype, trackfilter.nclstpc, trackfilter.nxrtpc, trackfilter.nclsits, trackfilter.chi2clustpc,
                                             trackfilter.chi2clusits, trackfilter.xrofctpc, trackfilter.dcaxy, trackfilter.dcaz);
    fTrackFilter = new PWGCF::TrackSelectionFilterAndAnalysis(trksel, PWGCF::SelectionFilterAndAnalysis::kFilter);
  }
  
  void process(aod::Collision const& collision, aod::Tracks const& ftracks) 
  {
  }
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  WorkflowSpec workflow{
    adaptAnalysisTask<DptDptSkim>(cfgc)};
  return workflow;
}

