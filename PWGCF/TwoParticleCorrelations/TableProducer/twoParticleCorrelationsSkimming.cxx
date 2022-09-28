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
#include "Common/DataModel/Centrality.h"
#include "Common/DataModel/PIDResponse.h"
#include "PWGCF/TwoParticleCorrelations/DataModel/TwoParticleCorrelationsSkimmed.h"
#include "PWGCF/TwoParticleCorrelations/Core/SkimmingConfigurableCuts.h"
#include "PWGCF/TwoParticleCorrelations/Core/EventSelectionFilterAndAnalysis.h"
#include "PWGCF/TwoParticleCorrelations/Core/TrackSelectionFilterAndAnalysis.h"
#include "PWGCF/TwoParticleCorrelations/Core/PIDSelectionFilterAndAnalysis.h"
#include "Framework/runDataProcessing.h"

using namespace o2;
using namespace o2::framework;
using namespace o2::soa;
using namespace o2::framework::expressions;
using namespace o2::analysis;

namespace o2::analysis::cfskim
{
#define LOGTRACKCOLLISIONS info
#define LOGTRACKTRACKS info

using pidTables = soa::Join<aod::pidTPCEl, aod::pidTPCMu, aod::pidTPCPi, aod::pidTPCKa, aod::pidTPCPr,
                            aod::pidTOFEl, aod::pidTOFMu, aod::pidTOFPi, aod::pidTOFKa, aod::pidTOFPr>;

PWGCF::TrackSelectionFilterAndAnalysis* fTrackFilter = nullptr;
PWGCF::EventSelectionFilterAndAnalysis* fEventFilter = nullptr;
PWGCF::PIDSelectionFilterAndAnalysis* fPIDFilter = nullptr;
} // namespace o2::analysis::cfskim

using namespace cfskim;

void setEventCutsLabels(std::shared_ptr<TH1> h)
{
  using namespace aod::run2;

  /* labels taken from O2/Framework DataTypes.h */
  const char* evcutslabel[kTRDHEE + 1] = {
    "kINELgtZERO",
    "kPileupInMultBins",
    "kConsistencySPDandTrackVertices",
    "kTrackletsVsClusters",
    "kNonZeroNContribs",
    "kIncompleteDAQ",
    "kPileUpMV",
    "kTPCPileUp",
    "kTimeRangeCut",
    "kEMCALEDCut",
    "kAliEventCutsAccepted",
    "kIsPileupFromSPD",
    "kIsV0PFPileup",
    "kIsTPCHVdip",
    "kIsTPCLaserWarmUp",
    "kTRDHCO",
    "kTRDHJT",
    "kTRDHSE",
    "kTRDHQU",
    "kTRDHEE"};

  for (int bit = kINELgtZERO; bit <= kTRDHEE; ++bit) {
    h->GetXaxis()->SetBinLabel(bit + 1, evcutslabel[bit]);
  }
}

void reportEventCuts(std::shared_ptr<TH1> h, uint32_t eventcuts)
{
  using namespace aod::run2;
  auto entries = h->GetEntries();
  for (int bit = kINELgtZERO; bit <= kTRDHEE; ++bit) {
    if (TESTBIT(eventcuts, bit)) {
      h->Fill(bit + 0.5);
    }
  }
  h->SetEntries(entries + 1);
}

struct TwoParticleCorrelationsSkimming {
  /* skimmed data tables */
  Produces<aod::CFCollisions> skimmedcollision;
  Produces<aod::CFTracks> skimmedtrack;
  Produces<aod::CFTrackPIDs> skimmtrackpid;
  Produces<aod::CFMCCollisions> skimmedgencollision;
  Produces<aod::CFMCParticles> skimmedparticles;

#include "skimmingconf.h"

  int nReportedTracks;
  HistogramRegistry historeg;

  void init(InitContext const&)
  {
    using namespace cfskim;

    LOGF(info, "DptDptSkimTask::init()");

    /* collision filtering configuration */
    PWGCF::EventSelectionConfigurable eventsel(eventfilter.centmultsel, "", eventfilter.zvtxsel, "");
    fEventFilter = new PWGCF::EventSelectionFilterAndAnalysis(eventsel, PWGCF::SelectionFilterAndAnalysis::kFilter);

    /* track filtering configuration */
    PWGCF::TrackSelectionConfigurable trksel(trackfilter.ttype, trackfilter.nclstpc, trackfilter.nxrtpc, trackfilter.nclsits, trackfilter.chi2clustpc,
                                             trackfilter.chi2clusits, trackfilter.xrofctpc, trackfilter.dcaxy, trackfilter.dcaz, trackfilter.ptrange, trackfilter.etarange);
    fTrackFilter = new PWGCF::TrackSelectionFilterAndAnalysis(trksel, PWGCF::SelectionFilterAndAnalysis::kFilter);
    PWGCF::PIDSelectionConfigurable pidsel(pidfilter.pidtpcfilter.tpcel, pidfilter.pidtpcfilter.tpcmu, pidfilter.pidtpcfilter.tpcpi, pidfilter.pidtpcfilter.tpcka, pidfilter.pidtpcfilter.tpcpr,
                                           pidfilter.pidtoffilter.tpcel, pidfilter.pidtoffilter.tpcmu, pidfilter.pidtoffilter.tpcpi, pidfilter.pidtoffilter.tpcka, pidfilter.pidtoffilter.tpcpr);
    fPIDFilter = new PWGCF::PIDSelectionFilterAndAnalysis(pidsel, PWGCF::SelectionFilterAndAnalysis::kFilter);
    nReportedTracks = 0;

    /* TODO: upload the cuts signatures to the CCDB */
    LOGF(info, "Collision skimming signature: %s", fEventFilter->getCutStringSignature().Data());
    LOGF(info, "Track skimming signature: %s", fTrackFilter->getCutStringSignature().Data());
    LOGF(info, "PID skimming signature: %s", fPIDFilter->getCutStringSignature().Data());

    historeg.add("EventCuts", "EventCuts", {HistType::kTH1F, {{aod::run2::kTRDHEE + 1, 0, aod::run2::kTRDHEE + 1}}});
    setEventCutsLabels(historeg.get<TH1>(HIST("EventCuts")));
  }

  template <typename Coll, typename BcInfo>
  uint64_t filterRun2Collision(Coll const& collision, BcInfo const& bcinfo)
  {
    using namespace aod::run2;
    using namespace aod::collision;

    uint32_t eventcuts = bcinfo.eventCuts();
    bool accepted = true;

    // NOT CONFIGURABLE EVENT SELECTION
    /* incomplete data acquisition */
    if (not TESTBIT(eventcuts, kIncompleteDAQ)) {
      accepted = false;
    }
    /* pile-up */
    /* TODO: check if this is also valid for Run 1 data */
    if (not TESTBIT(eventcuts, kPileupInMultBins) or
        not TESTBIT(eventcuts, kTrackletsVsClusters) or
        not TESTBIT(eventcuts, kPileUpMV) or
        not TESTBIT(eventcuts, kTimeRangeCut) or
        not TESTBIT(eventcuts, kTPCPileUp) or
        TESTBIT(eventcuts, kIsPileupFromSPD) or
        TESTBIT(eventcuts, kIsV0PFPileup)) {
      accepted = false;
    }
    /* TPC issues*/
    if (TESTBIT(eventcuts, kIsTPCHVdip) or
        TESTBIT(eventcuts, kIsTPCLaserWarmUp)) {
      accepted = false;
    }
    /* vertex */
    if (not TESTBIT(eventcuts, kNonZeroNContribs) or
        (((collision.flags() & Run2VertexerZ) == Run2VertexerZ) && collision.covZZ() < 0.25)) {
      accepted = false;
    }
    reportEventCuts(historeg.get<TH1>(HIST("EventCuts")), eventcuts);

    // CONFIGURABLE EVENT SELECTION
    if (accepted) {
      return fEventFilter->Filter(collision);
    } else {
      return 0UL;
    }
  }

  void processRun2(soa::Join<aod::Collisions, aod::CentRun2V0Ms, aod::CentRun2CL0s, aod::CentRun2CL1s>::iterator const& collision,
                   soa::Join<aod::BCs, aod::Timestamps, aod::Run2BCInfos> const&, soa::Join<aod::FullTracks, aod::TracksDCA, pidTables> const& tracks)
  {
    /* for the time being this will apply only to Run 1+2 converted data */
    LOGF(LOGTRACKCOLLISIONS, "Got a new collision with zvtx %.2f and V0M %.2f, CL0 %.2f, CL1 %.2f", collision.posZ(), collision.centRun2V0M(), collision.centRun2CL0(), collision.centRun2CL1());

    auto bc = collision.bc_as<soa::Join<aod::BCs, aod::Timestamps, aod::Run2BCInfos>>();
    auto colmask = filterRun2Collision(collision, bc);
    LOGF(LOGTRACKCOLLISIONS, "Got mask 0x%lx", colmask);

    if (colmask != 0UL) {
      skimmedcollision(collision.posZ(), bc.runNumber(), bc.timestamp(), colmask, fEventFilter->GetMultiplicities());
      int nFilteredTracks = 0;
      int nCollisionReportedTracks = 0;
      for (auto const& track : tracks) {
        auto trkmask = fTrackFilter->Filter(track);
        auto pidmask = fPIDFilter->Filter(track);
        if (trkmask != 0UL) {
          skimmedtrack(skimmedcollision.lastIndex(), trkmask, track.pt(), track.eta(), track.phi());
          skimmtrackpid(pidmask);
          nFilteredTracks++;
        }
        if (trkmask != 0UL and nReportedTracks < 1000) {
          if (nCollisionReportedTracks < 20) {
            LOGF(LOGTRACKTRACKS, "  Got track mask 0x%08lx and PID mask 0x%08lx", trkmask, pidmask);
            LOGF(LOGTRACKTRACKS, "    TPC clusters %d, Chi2 per TPC cluster %f, pT %f, eta %f, track type %d",
                 track.tpcNClsFound(), track.tpcChi2NCl(), track.pt(), track.eta(), track.trackType());
            nCollisionReportedTracks++;
            nReportedTracks++;
          }
        }
      }
      LOGF(LOGTRACKCOLLISIONS, ">> Filtered %d tracks", nFilteredTracks);
    }
  }
  PROCESS_SWITCH(TwoParticleCorrelationsSkimming, processRun2, "Process on Run 1 or Run 2 data", true);
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  WorkflowSpec workflow{
    adaptAnalysisTask<TwoParticleCorrelationsSkimming>(cfgc)};
  return workflow;
}
