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
#include "PWGCF/Core/AnalysisConfigurableCuts.h"
#include "PWGCF/Core/TrackSelectionFilterAndAnalysis.h"
#include "PWGCF/Core/EventSelectionFilterAndAnalysis.h"
#include "Framework/runDataProcessing.h"

/* TODO: this will go to its proper header at some point */
#ifndef O2_ANALYSIS_TWOPSKIMMED_H
#define O2_ANALYSIS_TWOPSKIMMED_H

#include "Framework/ASoA.h"
#include "Framework/AnalysisDataModel.h"

namespace o2
{
namespace aod
{
/* we have to change from int to bool when bool columns work properly */
namespace twopskim
{
DECLARE_SOA_COLUMN(TwoPSkimmedCollisionCentMult, centmult, float); //! The centrality/multiplicity pecentile
DECLARE_SOA_COLUMN(TwoPSkimmedCollisionFlags, selflags, uint64_t); //! The skimming flags for collision selection
} // namespace twopskim
DECLARE_SOA_TABLE(TwoPSkimmedCollisions, "AOD", "TWOPSKMDCOLL", //! Accepted reconstructed collisions/events filtered table
                  o2::soa::Index<>,
                  collision::PosZ,
                  twopskim::TwoPSkimmedCollisionCentMult,
                  twopskim::TwoPSkimmedCollisionFlags);
using TwoPSkimmedCollision = TwoPSkimmedCollisions::iterator;
DECLARE_SOA_TABLE(TwoPSkimmedGenCollisions, "AOD", "TWOPSKMDGENCOLL", //! Accepted generated collisions/events filtered table
                  o2::soa::Index<>,
                  mccollision::PosZ,
                  twopskim::TwoPSkimmedCollisionCentMult,
                  twopskim::TwoPSkimmedCollisionFlags);
using TwoPSkimmedGenCollision = TwoPSkimmedGenCollisions::iterator;
namespace twopskim
{
DECLARE_SOA_INDEX_COLUMN(TwoPSkimmedCollision, event);           //! Reconstructed collision/event
DECLARE_SOA_INDEX_COLUMN(TwoPSkimmedGenCollision, mcevent);      //! Generated collision/event
DECLARE_SOA_COLUMN(TwoPSkimmedTrackFlags, trackflags, uint64_t); //! The skimming flags for track selection
DECLARE_SOA_COLUMN(sPt, spt, float);                             //! The track charge signed transverse momentum
DECLARE_SOA_DYNAMIC_COLUMN(Pt, pt,                               //! The track transverse momentum
                           [](float signedpt) -> float {
                             return abs(signedpt);
                           });
DECLARE_SOA_COLUMN(Eta, eta, float);        //! The track pseudorapidity
DECLARE_SOA_COLUMN(Phi, phi, float);        //! The track azimuthal angle
DECLARE_SOA_COLUMN(Charge, charge, int8_t); //! Charge: in units of electric charge
DECLARE_SOA_DYNAMIC_COLUMN(Sign, sign,      //! The track transverse momentum
                           [](float signedpt) -> short {
                             return (signedpt >= 0) ? 1 : -1;
                           });
} // namespace twopskim
DECLARE_SOA_TABLE(TwoPSkimmedTracks, "AOD", "TWOPSKMDTRKS", //! The reconstructed tracks filtered table
                  twopskim::TwoPSkimmedCollisionId,
                  twopskim::TwoPSkimmedTrackFlags,
                  twopskim::sPt,
                  twopskim::Eta,
                  twopskim::Phi,
                  twopskim::Pt<twopskim::sPt>,
                  twopskim::Sign<twopskim::sPt>);
DECLARE_SOA_TABLE(TwoPSkimmedParticles, "AOD", "TWOPSKMDPARTS", //! The generated particles filtered table
                  twopskim::TwoPSkimmedGenCollisionId,
                  twopskim::TwoPSkimmedTrackFlags,
                  twopskim::sPt,
                  twopskim::Eta,
                  twopskim::Phi,
                  twopskim::Pt<twopskim::sPt>,
                  twopskim::Sign<twopskim::sPt>);
} // namespace aod
} // namespace o2

#endif // O2_ANALYSIS_TWOPSKIMMED_H

using namespace o2;
using namespace o2::framework;
using namespace o2::soa;
using namespace o2::framework::expressions;
using namespace o2::analysis;

namespace o2::analysis::twopskim
{
#define LOGTRACKCOLLISIONS debug
#define LOGTRACKTRACKS debug

PWGCF::TrackSelectionFilterAndAnalysis* fTrackFilter = nullptr;
PWGCF::EventSelectionFilterAndAnalysis* fEventFilter = nullptr;
}

using namespace twopskim;

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

struct DptDptSkim {
  /* skimmed data tables */
  Produces<aod::TwoPSkimmedCollisions> skimmedcollision;
  Produces<aod::TwoPSkimmedTracks> skimmedtrack;
  Produces<aod::TwoPSkimmedGenCollisions> skimmedgencollision;
  Produces<aod::TwoPSkimmedParticles> skimmedparticles;

#include "dptdptskimconf.h"

  int nReportedTracks;
  HistogramRegistry historeg;

  void init(InitContext const&)
  {
    using namespace twopskim;

    LOGF(info, "DptDptSkimTask::init()");

    /* collision filtering configuration */
    PWGCF::EventSelectionConfigurable eventsel(eventfilter.centmultsel, "", eventfilter.zvtxsel, "");
    fEventFilter = new PWGCF::EventSelectionFilterAndAnalysis(eventsel, PWGCF::SelectionFilterAndAnalysis::kFilter);

    /* track filtering configuration */
    PWGCF::TrackSelectionConfigurable trksel(trackfilter.ttype, trackfilter.nclstpc, trackfilter.nxrtpc, trackfilter.nclsits, trackfilter.chi2clustpc,
                                             trackfilter.chi2clusits, trackfilter.xrofctpc, trackfilter.dcaxy, trackfilter.dcaz, trackfilter.ptrange, trackfilter.etarange);
    fTrackFilter = new PWGCF::TrackSelectionFilterAndAnalysis(trksel, PWGCF::SelectionFilterAndAnalysis::kFilter);
    nReportedTracks = 0;

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

  void processRun2(soa::Join<aod::Collisions, aod::CentRun2V0Ms, aod::CentRun2CL0s, aod::CentRun2CL1s>::iterator const& collision, soa::Join<aod::BCs, aod::Run2BCInfos> const&, soa::Join<aod::FullTracks, aod::TracksDCA> const& tracks)
  {
    /* for the time being this will apply only to Run 1+2 converted data */
    LOGF(LOGTRACKCOLLISIONS, "Got a new collision with zvtx %.2f and V0M %.2f, CL0 %.2f, CL1 %.2f", collision.posZ(), collision.centRun2V0M(), collision.centRun2CL0(), collision.centRun2CL1());

    auto colmask = filterRun2Collision(collision, collision.bc_as<soa::Join<aod::BCs, aod::Run2BCInfos>>());
    LOGF(LOGTRACKCOLLISIONS, "Got mask 0x%lx", colmask);

    if (colmask != 0UL) {
      skimmedcollision(collision.posZ(), 50.0, colmask);
      for (auto const& track : tracks) {
        auto trkmask = fTrackFilter->Filter(track);
        if (trkmask != 0UL) {
          skimmedtrack(skimmedcollision.lastIndex(), trkmask, track.pt() * track.sign(), track.eta(), track.phi());
        }
        if ((trkmask & 0xFFFFF9FF) != 0UL and nReportedTracks < 1000) {
          LOGF(LOGTRACKTRACKS, "  Got track mask 0x%lx, TPC clusters %d, Chi2 per TPC cluster %f, pT %f, eta %f, track type %d",
               trkmask, track.tpcNClsFound(), track.tpcChi2NCl(), track.pt(), track.eta(), track.trackType());
          nReportedTracks++;
        }
      }
    }
  }
  PROCESS_SWITCH(DptDptSkim, processRun2, "Process on Run 1 or Run 2 data", true);
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  WorkflowSpec workflow{
    adaptAnalysisTask<DptDptSkim>(cfgc)};
  return workflow;
}

