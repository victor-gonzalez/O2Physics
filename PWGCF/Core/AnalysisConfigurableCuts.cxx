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

#include <regex>
#include <TObjArray.h>

#include "Framework/AnalysisDataModel.h"
#include "Framework/ASoAHelpers.h"
#include "Common/DataModel/TrackSelectionTables.h"
#include "PWGCF/Core/AnalysisConfigurableCuts.h"

using namespace o2;
using namespace o2::framework;
using namespace o2::soa;
using namespace o2::framework::expressions;
using namespace o2::analysis::PWGCF;

ClassImp(SimpleInclusiveCut);

SimpleInclusiveCut::SimpleInclusiveCut() : TNamed(),
                                           mX(0),
                                           mY(0.0)
{
  //
  // default constructor
  //
}

SimpleInclusiveCut::SimpleInclusiveCut(const char* name, int _x, float _y) : TNamed(name, name),
                                                                             mX(_x),
                                                                             mY(_y)
{
  //
  // explicit constructor
  //
}

SimpleInclusiveCut& SimpleInclusiveCut::operator=(const SimpleInclusiveCut& sic)
{
  //
  // assignment operator
  //
  if (this != &sic) {
    TNamed::operator=(sic);
    mX = sic.mX;
    mY = sic.mY;
  }
  return (*this);
}

/// \brief These are the implemented basic bricks
/// If more are implemented the list must be expanded and the
/// corresponding brick construction implemented
// template <typename TValueToFilter>
// const char* CutBrick<TValueToFilter>::mgImplementedbricks[] = {"lim", "th", "rg", "xrg", "mrg", "cwv"};

templateClassImp(CutBrick);
templateClassImp(CutBrickLimit);
templateClassImp(CutBrickThreshold);
templateClassImp(CutBrickRange);
templateClassImp(CutBrickExtToRange);
templateClassImp(CutBrickSelectorMultipleRanges);
templateClassImp(o2::analysis::CutWithVariations);

/// Default constructor
SpecialCutBrick::SpecialCutBrick()
  : TNamed(),
    mState(kPASSIVE),
    mMode(kUNSELECTED)
{
}

/// Named constructor
/// \param name The name of the brick
SpecialCutBrick::SpecialCutBrick(const char* name, const char* title)
  : TNamed(name, title),
    mState(kPASSIVE),
    mMode(kUNSELECTED)
{
}

ClassImp(SpecialCutBrick);

/// \brief Constructor from regular expression
TrackSelectionBrick::TrackSelectionBrick(const TString& regex) : SpecialCutBrick(regex, regex)
{
  if (regex.EqualTo("FB1LHC2010")) {
    constructFB1LHC2010();
  } else if (regex.EqualTo("FB1")) {
    constructFB1LHC2011();
  } else if (regex.EqualTo("FB32LHC2010")) {
    constructFB32LHC2010();
  } else if (regex.EqualTo("FB32")) {
    constructFB32LHC2011();
  } else if (regex.EqualTo("FB64LHC2010")) {
    constructFB64LHC2010();
  } else if (regex.EqualTo("FB64")) {
    constructFB64LHC2011();
  } else {
    ::Fatal("TrackSelectionBrick::TrackSelectionBrick", "Wrong RE: %s, trying to construct an unknown track type selector", regex.Data());
  }
}

// Default TPC only track selection according to LHC2010
void TrackSelectionBrick::constructFB1LHC2010()
{
  SetTrackType(o2::aod::track::Run2Track);
  SetRequireGoldenChi2(true);
  SetMinNClustersTPC(50);
  SetMaxChi2PerClusterTPC(4.f);
  SetMaxDcaXY(2.4f);
  SetMaxDcaZ(3.2f);
  /* TODO: 2D DCA cut */
}

// Default track selection requiring one hit in the SPD DCAxy according to LHC2010
void TrackSelectionBrick::constructFB32LHC2010()
{
  SetTrackType(o2::aod::track::Run2Track);
  SetRequireITSRefit(true);
  SetRequireTPCRefit(true);
  SetRequireGoldenChi2(true);
  SetMinNCrossedRowsTPC(70);
  SetMinNCrossedRowsOverFindableClustersTPC(0.8f);
  SetMaxChi2PerClusterTPC(4.f);
  SetRequireHitsInITSLayers(1, {0, 1}); // one hit in any SPD layer
  SetMaxChi2PerClusterITS(36.f);
  SetMaxDcaXYPtDep([](float pt) { return 0.0182f + 0.0350f / pow(pt, 1.01f); });
  SetMaxDcaZ(2.f);
}

// Default track selection requiring no hit in the SPD and one in the innermost DCAxy according to LHC2010
// SDD -> complementary tracks to global selection
void TrackSelectionBrick::constructFB64LHC2010()
{
  constructFB32LHC2010();
  ResetITSRequirements();
  SetRequireNoHitsInITSLayers({0, 1}); // no hit in SPD layers
  SetRequireHitsInITSLayers(1, {2});   // one hit in first SDD layer
}

// Default TPC only track selection according to LHC2011
void TrackSelectionBrick::constructFB1LHC2011()
{
  /* the same as for LHC2010 */
  constructFB1LHC2010();
}

// Default track selection requiring one hit in the SPD DCAxy according to LHC2011
void TrackSelectionBrick::constructFB32LHC2011()
{
  SetTrackType(o2::aod::track::Run2Track);
  SetRequireITSRefit(true);
  SetRequireTPCRefit(true);
  SetRequireGoldenChi2(true);
  SetMinNCrossedRowsTPC(70);
  SetMinNCrossedRowsOverFindableClustersTPC(0.8f);
  SetMaxChi2PerClusterTPC(4.f);
  SetRequireHitsInITSLayers(1, {0, 1}); // one hit in any SPD layer
  SetMaxChi2PerClusterITS(36.f);
  SetMaxDcaXYPtDep([](float pt) { return 0.0105f + 0.0350f / pow(pt, 1.1f); });
  SetMaxDcaZ(2.f);
}

// Default track selection requiring no hit in the SPD and one in the innermost DCAxy according to LHC2011
// SDD -> complementary tracks to global selection
void TrackSelectionBrick::constructFB64LHC2011()
{
  constructFB32LHC2011();
  ResetITSRequirements();
  SetRequireNoHitsInITSLayers({0, 1}); // no hit in SPD layers
  SetRequireHitsInITSLayers(1, {2});   // one hit in first SDD layer
}

bool TrackSelectionBrick::FulfillsITSHitRequirements(uint8_t itsClusterMap)
{
  constexpr uint8_t bit = 1;
  for (auto& itsRequirement : mRequiredITSHits) {
    auto hits = std::count_if(itsRequirement.second.begin(), itsRequirement.second.end(), [&](auto&& requiredLayer) { return itsClusterMap & (bit << requiredLayer); });
    if ((itsRequirement.first == -1) && (hits > 0)) {
      return false; // no hits were required in specified layers
    } else if (hits < itsRequirement.first) {
      return false; // not enough hits found in specified layers
    }
  }
  return true;
}

const std::string TrackSelectionBrick::mCutNames[static_cast<int>(TrackSelectionBrick::TrackCuts::kNCuts)] =
  {
    "TrackType",
    "TPCNCls",
    "TPCCrossedRowsOverNCls",
    "TPCRefit",
    "ITSNCls",
    "ITSChi2NDF",
    "ITSRefit",
    "ITSHits",
    "GoldenChi2",
    "DCAxy",
    "DCAz"};

ClassImp(TrackSelectionBrick);
