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
///
/// \brief Joined tables can be used as argument to the process function.
/// \author
/// \since

#include "Framework/runDataProcessing.h"
#include "Framework/AnalysisTask.h"
#include "Framework/AnalysisDataModel.h"
#include "Framework/ASoAHelpers.h"
#include "Framework/HistogramRegistry.h"
#include "Framework/RunningWorkflowInfo.h"
#include "CommonConstants/MathConstants.h"
#include "Common/DataModel/EventSelection.h"
#include "Common/DataModel/TrackSelectionTables.h"
#include "Common/DataModel/Centrality.h"
#include "PWGCF/Core/CorrelationContainer.h"
#include "PWGCF/Core/PairCuts.h"

namespace o2::aod
{
namespace hash
{
DECLARE_SOA_COLUMN(Bin, bin, int);
} // namespace hash
DECLARE_SOA_TABLE(Hashes, "AOD", "HASH", hash::Bin);

using Hash = Hashes::iterator;
} // namespace o2::aod

using namespace o2;
using namespace o2::framework;
using namespace o2::framework::expressions;

static constexpr float cfgPairCutDefaults[1][5] = {{-1, -1, -1, -1, -1}};

struct firstcorrelations {
  Configurable<float> cfgZVtxCut = {"zvtxcut", 7.0, "Vertex z cut. Default 7 cm"};
  Configurable<float> cfgPtCutMin = {"minpt", 0.2, "Minimum accepted track pT. Default 0.2 GeV"};
  Configurable<float> cfgPtCutMax = {"maxpt", 5.0, "Maximum accepted track pT. Default 5.0 GeV"};
  Configurable<float> cfgEtaCut = {"etacut", 0.8, "Eta cut. Default 0.8"};

  Configurable<LabeledArray<float>> cfgPairCut{"cfgPairCut", {cfgPairCutDefaults[0], 5, {"Photon", "K0", "Lambda", "Phi", "Rho"}}, "Pair cuts on various particles"};

  ConfigurableAxis axisVertex{"axisVertex", {7, -7, 7}, "vertex axis for histograms"};
  ConfigurableAxis axisDeltaPhi{"axisDeltaPhi", {72, -constants::math::PIHalf, constants::math::PIHalf * 3}, "delta phi axis for histograms"};
  ConfigurableAxis axisDeltaEta{"axisDeltaEta", {40, -2, 2}, "delta eta axis for histograms"};
  ConfigurableAxis axisPtTrigger{"axisPtTrigger", {VARIABLE_WIDTH, 0.5, 1.0, 1.5, 2.0, 3.0, 4.0, 6.0, 10.0}, "pt trigger axis for histograms"};
  ConfigurableAxis axisPtAssoc{"axisPtAssoc", {VARIABLE_WIDTH, 0.5, 1.0, 1.5, 2.0, 3.0, 4.0, 6.0}, "pt associated axis for histograms"};
  ConfigurableAxis axisMultiplicity{"axisMultiplicity", {VARIABLE_WIDTH, 0, 5, 10, 20, 30, 40, 50, 100.1}, "multiplicity / centrality axis for histograms"};
  ConfigurableAxis axisVertexEfficiency{"axisVertexEfficiency", {10, -10, 10}, "vertex axis for efficiency histograms"};
  ConfigurableAxis axisEtaEfficiency{"axisEtaEfficiency", {20, -1.0, 1.0}, "eta axis for efficiency histograms"};
  ConfigurableAxis axisPtEfficiency{"axisPtEfficiency", {VARIABLE_WIDTH, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.25, 1.5, 1.75, 2.0, 2.25, 2.5, 2.75, 3.0, 3.25, 3.5, 3.75, 4.0, 4.5, 5.0, 6.0, 7.0, 8.0}, "pt axis for efficiency histograms"};

  OutputObj<CorrelationContainer> same{"sameEvent"};
  OutputObj<CorrelationContainer> mixed{"mixedEvent"};

  HistogramRegistry registry{"registry"};
  PairCuts mPairCuts;
  bool doPairCuts = false;

  void init(InitContext&)
  {
    LOGF(info, "Starting init");
    registry.add("yields", "centrality vs pT vs eta", {HistType::kTH3F, {{100, 0, 100, "centrality"}, {40, 0, 20, "p_{T}"}, {100, -2, 2, "#eta"}}});
    registry.add("etaphi", "centrality vs eta vs phi", {HistType::kTH3F, {{100, 0, 100, "centrality"}, {100, -2, 2, "#eta"}, {200, 0, 2 * M_PI, "#varphi"}}});

    const int maxMixBin = axisMultiplicity->size() * axisVertex->size();
    registry.add("eventcount", "bin", {HistType::kTH1F, {{maxMixBin + 2, -2.5, -0.5 + maxMixBin, "bin"}}});

    mPairCuts.SetHistogramRegistry(&registry);

    LOGF(info, "Middle init");
    if (cfgPairCut->get("Photon") > 0 || cfgPairCut->get("K0") > 0 || cfgPairCut->get("Lambda") > 0 || cfgPairCut->get("Phi") > 0 || cfgPairCut->get("Rho") > 0) {
      mPairCuts.SetPairCut(PairCuts::Photon, cfgPairCut->get("Photon"));
      mPairCuts.SetPairCut(PairCuts::K0, cfgPairCut->get("K0"));
      mPairCuts.SetPairCut(PairCuts::Lambda, cfgPairCut->get("Lambda"));
      mPairCuts.SetPairCut(PairCuts::Phi, cfgPairCut->get("Phi"));
      mPairCuts.SetPairCut(PairCuts::Rho, cfgPairCut->get("Rho"));
      doPairCuts = true;
    }
    std::vector<AxisSpec> axisList = {{axisDeltaEta, "#Delta#eta"},
                                      {axisPtAssoc, "p_{T} (GeV/c)"},
                                      {axisPtTrigger, "p_{T} (GeV/c)"},
                                      {axisMultiplicity, "multiplicity / centrality"},
                                      {axisDeltaPhi, "#Delta#varphi (rad)"},
                                      {axisVertex, "z-vtx (cm)"},
                                      {axisEtaEfficiency, "#eta"},
                                      {axisPtEfficiency, "p_{T} (GeV/c)"},
                                      {axisVertexEfficiency, "z-vtx (cm)"}};
    same.setObject(new CorrelationContainer("sameEvent", "sameEvent", axisList));
    mixed.setObject(new CorrelationContainer("mixedEvent", "mixedEvent", axisList));
    LOGF(info, "Finishing init");
  }

  template <typename TCollision, typename TTracks>
  void fillQA(TCollision collision, float centrality, TTracks tracks)
  {
    for (auto& track1 : tracks) {
      registry.fill(HIST("yields"), centrality, track1.pt(), track1.eta());
      registry.fill(HIST("etaphi"), centrality, track1.eta(), track1.phi());
    }
  }

  template <typename TTarget, typename TCollision>
  bool fillCollision(TTarget target, TCollision collision, float centrality)
  {
    target->fillEvent(centrality, CorrelationContainer::kCFStepAll);

    if (!collision.alias()[kINT7] || !collision.sel7()) {
      return false;
    }

    target->fillEvent(centrality, CorrelationContainer::kCFStepReconstructed);

    return true;
  }

  template <typename TTarget, typename TTracks>
  void fillCorrelations(TTarget target, TTracks tracks1, TTracks tracks2, float centrality, float posZ)
  {
    for (auto& track1 : tracks1) {
      target->getTriggerHist()->Fill(CorrelationContainer::kCFStepReconstructed, track1.pt(), centrality, posZ, 1.0);

      for (auto& track2 : tracks2) {
        if (track1 == track2) {
          continue;
        }
        if (doPairCuts && mPairCuts.conversionCuts(track1, track2)) {
          continue;
        }
        float deltaPhi = track1.phi() - track2.phi();
        if (deltaPhi > 1.5f * PI) {
          deltaPhi -= TwoPI;
        }
        if (deltaPhi < -PIHalf) {
          deltaPhi += TwoPI;
        }

        target->getPairHist()->Fill(CorrelationContainer::kCFStepReconstructed,
                                    track1.eta() - track2.eta(), track2.pt(), track1.pt(), centrality, deltaPhi, posZ,
                                    1.0);
      }
    }
  }

  Filter collisionZVtxFilter = nabs(aod::collision::posZ) < cfgZVtxCut;

  Filter trackFilter = (nabs(aod::track::eta) < cfgEtaCut) && (aod::track::pt > cfgPtCutMin) && (aod::track::pt < cfgPtCutMax) &&
                       ((aod::track::isGlobalTrack == (uint8_t) true) || (aod::track::isGlobalTrackSDD == (uint8_t) true));

  using aodTracks = soa::Filtered<soa::Join<aod::Tracks, aod::TrackSelection>>;

  void processSame(soa::Filtered<soa::Join<aod::Collisions, aod::EvSels, aod::CentV0Ms>>::iterator const& collision, aodTracks const& tracks)
  {
    const auto centrality = collision.centV0M();

    if (fillCollision(same, collision, centrality) == false) {
      return;
    }
    registry.fill(HIST("eventcount"), -2);
    fillQA(collision, centrality, tracks);
    fillCorrelations(same, tracks, tracks, centrality, collision.posZ());
  }
  PROCESS_SWITCH(firstcorrelations, processSame, "Process same event", true);

  void processMixed(soa::Filtered<soa::Join<aod::Collisions, aod::Hashes, aod::EvSels, aod::CentV0Ms>>& collisions, aodTracks const& tracks)
  {
    collisions.bindExternalIndices(&tracks);
    auto tracksTuple = std::make_tuple(tracks);
    AnalysisDataProcessorBuilder::GroupSlicer slicer(collisions, tracksTuple);

    // Strictly upper categorised collisions, for cfgNoMixedEvents combinations per bin, skipping those in entry -1
    for (auto& [collision1, collision2] : selfCombinations("fBin", 5, -1, collisions, collisions)) {

      LOGF(info, "processMixedAOD: Mixed collisions bin: %d pair: %d (%f), %d (%f)", collision1.bin(), collision1.index(), collision1.posZ(), collision2.index(), collision2.posZ());

      // TODO in principle these should be already checked on hash level, because in this way we don't check collision 2
      // TODO not correct because event-level histograms on collision1 are filled for each pair (important :))
      if (fillCollision(mixed, collision1, collision1.centV0M()) == false) {
        continue;
      }
      registry.fill(HIST("eventcount"), collision1.bin());

      auto it1 = slicer.begin();
      auto it2 = slicer.begin();
      for (auto& slice : slicer) {
        if (slice.groupingElement().index() == collision1.index()) {
          it1 = slice;
          break;
        }
      }
      for (auto& slice : slicer) {
        if (slice.groupingElement().index() == collision2.index()) {
          it2 = slice;
          break;
        }
      }

      auto tracks1 = std::get<aodTracks>(it1.associatedTables());
      tracks1.bindExternalIndices(&collisions);
      auto tracks2 = std::get<aodTracks>(it2.associatedTables());
      tracks2.bindExternalIndices(&collisions);

      auto bc = collision1.bc_as<aod::BCsWithTimestamps>();

      // LOGF(info, "Tracks: %d and %d entries", tracks1.size(), tracks2.size());

      // TODO mixed event weight missing
      fillCorrelations(mixed, tracks1, tracks2, collision1.centV0M(), collision1.posZ());
    }
  }
  PROCESS_SWITCH(firstcorrelations, processMixed, "Process mixed events", true);
};

struct HashTask {
  std::vector<float> vtxBinsEdges{-7.0f, -5.0f, -3.0f, -1.0f, 1.0f, 3.0f, 5.0f, 7.0f};
  std::vector<float> multBinsEdges{0.0f, 5.0f, 10.0f, 20.0f, 30.0f, 40.0f, 50.0, 100.1f};
  Produces<aod::Hashes> hashes;

  // Calculate hash for an element based on 2 properties and their bins.
  int getHash(std::vector<float> const& xBins, std::vector<float> const& yBins, float colX, float colY)
  {
    if (colX < xBins[0] || colY < yBins[0]) {
      return -1;
    }
    for (unsigned int i = 1; i < xBins.size(); i++) {
      if (colX < xBins[i]) {
        for (unsigned int j = 1; j < yBins.size(); j++) {
          if (colY < yBins[j]) {
            return i + j * (xBins.size() + 1);
          }
        }
        return -1;
      }
    }
    return -1;
  }

  void process(soa::Join<aod::Collisions, aod::CentV0Ms> const& collisions)
  {
    for (auto& collision : collisions) {
      int hash = getHash(vtxBinsEdges, multBinsEdges, collision.posZ(), collision.centV0M());
      hashes(hash);
    }
  }
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  return WorkflowSpec{
    adaptAnalysisTask<HashTask>(cfgc),
    adaptAnalysisTask<firstcorrelations>(cfgc),
  };
}
