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
#ifndef ANALYSIS_CONFIGURABLE_CUTS_CLASSES_H
#define ANALYSIS_CONFIGURABLE_CUTS_CLASSES_H

#include <Rtypes.h>
#include <TString.h>
#include <TObject.h>
#include <TNamed.h>
#include <TMath.h>
#include <TList.h>
#include <set>
#include <vector>
#include <regex>
#include <TObjArray.h>

#include "Framework/Logger.h"
#include "Framework/DataTypes.h"

namespace o2
{
namespace analysis
{
namespace PWGCF
{
/// \class EventSelectionCuts
/// \brief Class which implements configurable event selection cuts
///
class EventSelectionCuts
{
 private:
  int mOfflinetrigger = 1;                 ///< offline trigger, default MB = 1
  TString mCentmultestimator = "V0M";      ///< centrality / multiplicity estimation, default V0M
  int mRemovepileupcode = 1;               ///< Procedure for pile-up removal, default V0M vs TPCout tracks = 1
  TString mRemovepileupfn = "-2500+5.0*x"; ///< function for pile-up removal, procedure dependent, defaul V0M vs TPCout tracks for LHC15o HIR
  std::vector<std::vector<float>> mVertexZ{{-7.0, 7.0}, {-3.0, 3.0}, {-10.0, 10.0}};

 public:
  EventSelectionCuts(int ot, const char* ce, int rp, const char* rpf, std::vector<std::vector<float>> vtx) : mOfflinetrigger(ot),
                                                                                                             mCentmultestimator(ce),
                                                                                                             mRemovepileupcode(rp),
                                                                                                             mRemovepileupfn(rpf),
                                                                                                             mVertexZ(vtx)
  {
  }

  EventSelectionCuts() : mOfflinetrigger(0),
                         mCentmultestimator(""),
                         mRemovepileupcode(0),
                         mRemovepileupfn(""),
                         mVertexZ({{}})
  {
  }

 private:
  ClassDefNV(EventSelectionCuts, 1);
};

/// \class DptDptBinningCuts
/// \brief Class which implements configurable acceptance cuts
///
class DptDptBinningCuts
{
 public:
  int mZVtxbins = 28;       ///< the number of z_vtx bins default 28
  float mZVtxmin = -7.0;    ///< the minimum z_vtx value, default -7.0 cm
  float mZVtxmax = 7.0;     ///< the maximum z_vtx value, default 7.0 cm
  int mPTbins = 18;         ///< the number of pT bins, default 18
  float mPTmin = 0.2;       ///< the minimum pT value, default 0.2 GeV
  float mPTmax = 2.0;       ///< the maximum pT value, default 2.0 GeV
  int mEtabins = 16;        ///< the number of eta bins default 16
  float mEtamin = -0.8;     ///< the minimum eta value, default -0.8
  float mEtamax = 0.8;      ///< the maximum eta value, default 0.8
  int mPhibins = 72;        ///< the number of phi bins, default 72
  float mPhibinshift = 0.5; ///< the shift in the azimuthal origen, defoult 0.5, i.e half a bin

 private:
  ClassDefNV(DptDptBinningCuts, 1);
};

/// \brief Simple class for a generic check within a concrete range of a magnitude
class CheckRangeCfg
{
 public:
  bool mDoIt = false;    ///< do the actual check
  float mLowValue = 0.0; ///< range lowest value
  float mUpValue = 0.0;  ///< range upper value
 private:
  ClassDefNV(CheckRangeCfg, 1);
};

/// \brief Simple class for configuring a track selection object
class TrackSelectionCfg
{
 public:
  bool mUseIt = false;          ///< use this track selection configuration
  bool mOnGen = false;          ///< apply it to generator level also
  int mTPCclusters = 0;         ///< minimum number of TPC clusters
  int mTPCxRows = 70;           ///< minimum number of TPC crossed rows
  float mTPCXRoFClusters = 0.8; ///< minimu value of the TPC ratio no of crossed rows over findable clusters
  float mDCAxy = 2.4;           ///< maximum DCA on xy plane
  float mDCAz = 3.2;            ///< maximum DCA on z axis

 private:
  ClassDefNV(TrackSelectionCfg, 1);
};

class SimpleInclusiveCut : public TNamed
{
 public:
  int mX = 1;
  float mY = 2.f;
  SimpleInclusiveCut();
  SimpleInclusiveCut(const char*, int, float);
  SimpleInclusiveCut(const SimpleInclusiveCut&) = default;
  ~SimpleInclusiveCut() override = default;

  SimpleInclusiveCut& operator=(const SimpleInclusiveCut&);

 private:
  ClassDef(SimpleInclusiveCut, 1);
};

/// \class CutBrick
/// \brief Virtual class which implements the base component of the selection cuts
///
template <typename TValueToFilter>
class CutBrick : public TNamed
{
 public:
  CutBrick();
  CutBrick(const char*, const char*);
  CutBrick(const CutBrick&) = delete;
  virtual ~CutBrick() override = default;
  CutBrick& operator=(const CutBrick&) = delete;

 public:
  /// Returns whether the cut brick is active alowing the selection
  /// \return true if the brick is active
  bool IsActive() { return mState == kACTIVE; }
  /// Returns whether the cut is brick is incorporated in the selection chain
  bool IsArmed() { return mMode == kSELECTED; }
  /// Pure virtual function. Filters the passed value
  /// The brick or brick components will change to active if the passed value
  /// fits within the brick or brick components scope
  /// \returns a vector of booleans with true on the component for which the value activated the component brick
  virtual std::vector<bool> Filter(const TValueToFilter&) = 0;
  /// Pure virtual function. Return the length needed to code the brick status
  /// The length is in brick units. The actual length is implementation dependent
  /// \returns Brick length in units of bricks
  virtual int Length() = 0;

  static CutBrick<TValueToFilter>* constructBrick(const char* name, const char* regex, const std::set<std::string>& allowed);

 protected:
  /// Set the status of the cut significative (or not) for the selection chain
  void Arm(bool doit = true) { doit ? mMode = kSELECTED : mMode = kUNSELECTED; }

 protected:
  /// \enum BrickStatus
  /// \brief The status of the brick
  enum BrickStatus {
    kPASSIVE, ///< if the passed value does not comply whith the brick condition
    kACTIVE   ///< if the passed value comply whith the brick condition
  };
  /// \enum BrickMode
  /// \brief The mode of operation of the brick
  enum BrickMode {
    kUNSELECTED, ///< if the status of the brick is not significative
    kSELECTED    ///< if the status of the brick is significative
  };

  BrickStatus mState = kPASSIVE;
  BrickMode mMode = kUNSELECTED;

  ClassDef(CutBrick, 1);
};

/// \class CutBrickLimit
/// \brief Class which implements a limiting cut brick.
/// The brick will be active if the filtered value is below the limit
template <typename TValueToFilter>
class CutBrickLimit : public CutBrick<TValueToFilter>
{
 public:
  CutBrickLimit();
  CutBrickLimit(const char*, const TValueToFilter&);
  CutBrickLimit(const TString&);
  virtual ~CutBrickLimit() override = default;
  CutBrickLimit(const CutBrickLimit&) = delete;
  CutBrickLimit& operator=(const CutBrickLimit&) = delete;

  virtual std::vector<bool> Filter(const TValueToFilter&);
  virtual int Length() { return 1; }

 private:
  void ConstructCutFromString(const TString&);

  TValueToFilter mLimit; ///< the limiting upper value
  ClassDef(CutBrickLimit, 1);
};

/// \class CutBrickThreshold
/// \brief Class which implements a threshold cut brick.
/// The brick will be active if the filtered value is above the threshold
template <typename TValueToFilter>
class CutBrickThreshold : public CutBrick<TValueToFilter>
{
 public:
  CutBrickThreshold();
  CutBrickThreshold(const char*, const TValueToFilter&);
  CutBrickThreshold(const TString&);
  virtual ~CutBrickThreshold() override = default;
  CutBrickThreshold(const CutBrickThreshold&) = delete;
  CutBrickThreshold& operator=(const CutBrickThreshold&) = delete;

  virtual std::vector<bool> Filter(const TValueToFilter&);
  virtual int Length() { return 1; }

 private:
  void ConstructCutFromString(const TString&);

  TValueToFilter mThreshold; ///< the threshold value
  ClassDef(CutBrickThreshold, 1);
};

/// \class CutBrickRange
/// \brief Class which implements a range cut brick.
/// The brick will be active if the filtered value is within the range
template <typename TValueToFilter>
class CutBrickRange : public CutBrick<TValueToFilter>
{
 public:
  CutBrickRange();
  CutBrickRange(const char*, const TValueToFilter&, const TValueToFilter&);
  CutBrickRange(const TString&);
  virtual ~CutBrickRange() override = default;
  CutBrickRange(const CutBrickRange&) = delete;
  CutBrickRange& operator=(const CutBrickRange&) = delete;

  virtual std::vector<bool> Filter(const TValueToFilter&);
  virtual int Length() { return 1; }

 private:
  void ConstructCutFromString(const TString&);

  TValueToFilter mLow;  ///< the lower value of the range
  TValueToFilter mHigh; ///< the upper value of the range
  ClassDef(CutBrickRange, 1);
};

/// \class CutBrickExtToRange
/// \brief Class which implements an external to range cut brick.
/// The brick will be active if the filtered value is outside the range
template <typename TValueToFilter>
class CutBrickExtToRange : public CutBrick<TValueToFilter>
{
 public:
  CutBrickExtToRange();
  CutBrickExtToRange(const char*, const TValueToFilter&, const TValueToFilter&);
  CutBrickExtToRange(const TString&);
  virtual ~CutBrickExtToRange() override = default;
  CutBrickExtToRange(const CutBrickExtToRange&) = delete;
  CutBrickExtToRange& operator=(const CutBrickExtToRange&) = delete;

  virtual std::vector<bool> Filter(const TValueToFilter&);
  virtual int Length() { return 1; }

 private:
  void ConstructCutFromString(const TString&);

  TValueToFilter mLow;  ///< the lower value of the range
  TValueToFilter mHigh; ///< the upper value of the range
  ClassDef(CutBrickExtToRange, 1);
};

/// \class CutBrickSelectorMultipleRanges
/// \brief Class which implements a string as selector an multiple ranges
/// The brick will be active if the filtered value is inside any of the multiple ranges
/// Otherwise it will be passive
/// Each range might be active in its own if the filtered value is within its scope
template <typename TValueToFilter>
class CutBrickSelectorMultipleRanges : public CutBrick<TValueToFilter>
{
 public:
  CutBrickSelectorMultipleRanges();
  CutBrickSelectorMultipleRanges(const char*, const std::vector<TValueToFilter>&);
  CutBrickSelectorMultipleRanges(const TString&);
  virtual ~CutBrickSelectorMultipleRanges() override = default;
  CutBrickSelectorMultipleRanges(const CutBrickSelectorMultipleRanges&) = delete;
  CutBrickSelectorMultipleRanges& operator=(const CutBrickSelectorMultipleRanges&) = delete;

  virtual std::vector<bool> Filter(const TValueToFilter&);
  /// Return the length needed to code the brick status
  /// The length is in brick units. The actual length is implementation dependent
  /// \returns Brick length in units of bricks
  virtual int Length() { return mActive.size(); }

 private:
  void ConstructCutFromString(const TString&);

  std::vector<TValueToFilter> mEdges; ///< the value of the ranges edges (len = nranges + 1)
  std::vector<bool> mActive;          ///< if the associated range is active with the passed value to filter (len = nranges)
  ClassDef(CutBrickSelectorMultipleRanges, 1);
};

/// \class CutWithVariations
/// \brief Class which implements a cut with its default configuration
/// and its potential variations to be used for systematic tests
template <typename TValueToFilter>
class CutWithVariations : public CutBrick<TValueToFilter>
{
 public:
  /// Default constructor
  CutWithVariations();
  CutWithVariations(const char*, const char*, bool);
  CutWithVariations(const TString&);
  virtual ~CutWithVariations() override = default;
  CutWithVariations(const CutWithVariations&) = delete;
  CutWithVariations& operator=(const CutWithVariations&) = delete;

  bool AddDefaultBrick(CutBrick<TValueToFilter>* brick);
  bool AddVariationBrick(CutBrick<TValueToFilter>* brick);
  virtual std::vector<bool> Filter(const TValueToFilter&);
  virtual int Length();

 private:
  void ConstructCutFromString(const TString&);

  bool mAllowSeveralDefaults; ///< true if allows to store several cut default values
  TList mDefaultBricks;       ///< the list with the cut default values bricks
  TList mVariationBricks;     ///< the list with the cut variation values bricks
  ClassDef(CutWithVariations, 1);
};

/// \class SpecialCutBrick
/// \brief Virtual class which implements the base component of the special selection cuts
/// Special selection cuts are needed because the tables access seems cannot be
/// implemented in templated classes in the class dictionary. If this were found to be
/// feasible things will be easier deriving the special cut bricks directly from CutBrick
///
class SpecialCutBrick : public TNamed
{
 public:
  SpecialCutBrick();
  SpecialCutBrick(const char*, const char*);
  SpecialCutBrick(const SpecialCutBrick&) = delete;
  virtual ~SpecialCutBrick() override = default;
  SpecialCutBrick& operator=(const SpecialCutBrick&) = delete;

 public:
  /// Returns whether the cut brick is active alowing the selection
  /// \return true if the brick is active
  bool IsActive() { return mState == kACTIVE; }
  /// Returns whether the cut is brick is incorporated in the selection chain
  bool IsArmed() { return mMode == kSELECTED; }
  /// Pure virtual function. Return the length needed to code the brick status
  /// The length is in brick units. The actual length is implementation dependent
  /// \returns Brick length in units of bricks
  virtual int Length() = 0;

 protected:
  /// Set the status of the cut significative (or not) for the selection chain
  void Arm(bool doit = true) { doit ? mMode = kSELECTED : mMode = kUNSELECTED; }

 protected:
  /// \enum BrickStatus
  /// \brief The status of the brick
  enum BrickStatus {
    kPASSIVE, ///< if the passed value does not comply whith the brick condition
    kACTIVE   ///< if the passed value comply whith the brick condition
  };
  /// \enum BrickMode
  /// \brief The mode of operation of the brick
  enum BrickMode {
    kUNSELECTED, ///< if the status of the brick is not significative
    kSELECTED    ///< if the status of the brick is significative
  };

  BrickStatus mState = kPASSIVE;
  BrickMode mMode = kUNSELECTED;

  ClassDef(SpecialCutBrick, 1);
};

class TrackSelectionBrick : public SpecialCutBrick
{
 public:
  TrackSelectionBrick() = default;
  TrackSelectionBrick(const TString&);

  enum class TrackCuts : int {
    kTrackType = 0,
    kTPCNCls,
    kTPCCrossedRows,
    kTPCCrossedRowsOverNCls,
    kTPCChi2NDF,
    kTPCRefit,
    kITSNCls,
    kITSChi2NDF,
    kITSRefit,
    kITSHits,
    kGoldenChi2,
    kDCAxy,
    kDCAz,
    kNCuts
  };

  static const std::string mCutNames[static_cast<int>(TrackCuts::kNCuts)];

  template <typename TrackToFilter>
  bool Filter(TrackToFilter const& track)
  {
    if ((track.trackType() == mTrackType) &&
        (mCheckNClustersTPC && (track.tpcNClsFound() >= mMinNClustersTPC)) &&
        (mCheckNCrossedRowsTPC && (track.tpcNClsCrossedRows() >= mMinNCrossedRowsTPC)) &&
        (mCheckMinNCrossedRowsOverFindableClustersTPC && (track.tpcCrossedRowsOverFindableCls() >= mMinNCrossedRowsOverFindableClustersTPC)) &&
        (mCheckNClustersITS && (track.itsNCls() >= mMinNClustersITS)) &&
        (mCheckMaxChi2PerClusterITS && (track.itsChi2NCl() <= mMaxChi2PerClusterITS)) &&
        (mCheckMaxChi2PerClusterTPC && (track.tpcChi2NCl() <= mMaxChi2PerClusterTPC)) &&
        ((mRequireITSRefit) ? (track.flags() & o2::aod::track::ITSrefit) : true) &&
        ((mRequireTPCRefit) ? (track.flags() & o2::aod::track::TPCrefit) : true) &&
        ((mRequireGoldenChi2) ? (track.flags() & o2::aod::track::GoldenChi2) : true) &&
        FulfillsITSHitRequirements(track.itsClusterMap()) &&
        (mCheckMaxDcaXY && (abs(track.dcaXY()) <= ((mMaxDcaXYPtDep) ? mMaxDcaXYPtDep(track.pt()) : mMaxDcaXY))) &&
        (mCheckMaxDcaZ && (abs(track.dcaZ()) <= mMaxDcaZ))) {
      this->mState = this->kACTIVE;
      return true;
    } else {
      this->mState = this->kPASSIVE;
      return false;
    }
  }

  int Length() { return 1; }

  void SetTrackType(o2::aod::track::TrackTypeEnum trackType) { mTrackType = trackType; }
  void SetRequireITSRefit(bool requireITSRefit = true) { mRequireITSRefit = requireITSRefit; }
  void SetRequireTPCRefit(bool requireTPCRefit = true) { mRequireTPCRefit = requireTPCRefit; }
  void SetRequireGoldenChi2(bool requireGoldenChi2 = true) { mRequireGoldenChi2 = requireGoldenChi2; }
  void SetMinNClustersTPC(int minNClustersTPC) { mMinNClustersTPC = minNClustersTPC; }
  void SetMinNCrossedRowsTPC(int minNCrossedRowsTPC) { mMinNCrossedRowsTPC = minNCrossedRowsTPC; }
  void SetMinNCrossedRowsOverFindableClustersTPC(float minNCrossedRowsOverFindableClustersTPC) { mMinNCrossedRowsOverFindableClustersTPC = minNCrossedRowsOverFindableClustersTPC; }
  void SetMinNClustersITS(int minNClustersITS) { mMinNClustersITS = minNClustersITS; }
  void SetMaxChi2PerClusterTPC(float maxChi2PerClusterTPC) { mMaxChi2PerClusterTPC = maxChi2PerClusterTPC; }
  void SetMaxChi2PerClusterITS(float maxChi2PerClusterITS) { mMaxChi2PerClusterITS = maxChi2PerClusterITS; }
  void SetMaxDcaXY(float maxDcaXY) { mMaxDcaXY = maxDcaXY; }
  void SetMaxDcaZ(float maxDcaZ) { mMaxDcaZ = maxDcaZ; }

  void SetMaxDcaXYPtDep(std::function<float(float)> ptDepCut) { mMaxDcaXYPtDep = ptDepCut; }
  void SetRequireHitsInITSLayers(int8_t minNRequiredHits, std::set<uint8_t> requiredLayers)
  {
    // layer 0 corresponds to the the innermost ITS layer
    if (minNRequiredHits > requiredLayers.size()) {
      LOGF(FATAL, "More ITS hits required than layers specified.");
    } else {
      mRequiredITSHits.push_back(std::make_pair(minNRequiredHits, requiredLayers));
    }
  }
  void SetRequireNoHitsInITSLayers(std::set<uint8_t> excludedLayers) { mRequiredITSHits.push_back(std::make_pair(-1, excludedLayers)); }
  void ResetITSRequirements() { mRequiredITSHits.clear(); }

  void DisableNClustersTPCCheck(bool disable = true) { mCheckNClustersTPC = not disable; }
  void DisableNCrossedRowsTPCCheck(bool disable = true) { mCheckNCrossedRowsTPC = not disable; }
  void DisableNClustersITSCheck(bool disable = true) { mCheckNClustersITS = not disable; }
  void DisableMaxChi2PerClusterTPCCheck(bool disable = true) { mCheckMaxChi2PerClusterTPC = not disable; }
  void DisableMaxChi2PerClusterITSCheck(bool disable = true) { mCheckMaxChi2PerClusterITS = not disable; }
  void DisableMinNCrossedRowsOverFindableClustersTPCCheck(bool disable = true) { mCheckMinNCrossedRowsOverFindableClustersTPC = not disable; }
  void DisableMaxDcaXYCheck(bool disable = true) { mCheckMaxDcaXY = not disable; }
  void DisableMaxDcaZCheck(bool disable = true) { mCheckMaxDcaZ = not disable; }

 private:
  void constructFB1LHC2010();
  void constructFB1LHC2011();
  void constructFB32LHC2010();
  void constructFB32LHC2011();
  void constructFB64LHC2010();
  void constructFB64LHC2011();

  bool FulfillsITSHitRequirements(uint8_t itsClusterMap);

  o2::aod::track::TrackTypeEnum mTrackType{o2::aod::track::TrackTypeEnum::Track};

  // track quality cuts
  int mMinNClustersTPC{0};                            // min number of TPC clusters
  int mMinNCrossedRowsTPC{0};                         // min number of crossed rows in TPC
  int mMinNClustersITS{0};                            // min number of ITS clusters
  float mMaxChi2PerClusterTPC{1e10f};                 // max tpc fit chi2 per TPC cluster
  float mMaxChi2PerClusterITS{1e10f};                 // max its fit chi2 per ITS cluster
  float mMinNCrossedRowsOverFindableClustersTPC{0.f}; // min ratio crossed rows / findable clusters

  float mMaxDcaXY{1e10f};                       // max dca in xy plane
  float mMaxDcaZ{1e10f};                        // max dca in z direction
  std::function<float(float)> mMaxDcaXYPtDep{}; // max dca in xy plane as function of pT

  bool mRequireITSRefit{false};   // require refit in ITS
  bool mRequireTPCRefit{false};   // require refit in TPC
  bool mRequireGoldenChi2{false}; // require golden chi2 cut (Run 2 only)

  bool mCheckNClustersTPC{true};                           // check the number of TPC clusters
  bool mCheckNCrossedRowsTPC{true};                        // check the number of crossed rows int TPC
  bool mCheckNClustersITS{true};                           // check the number of ITS clusters
  bool mCheckMaxChi2PerClusterTPC{true};                   // check max tpc fit chi2 per TPC cluster
  bool mCheckMaxChi2PerClusterITS{true};                   // check max its fit chi2 per ITS cluster
  bool mCheckMinNCrossedRowsOverFindableClustersTPC{true}; // check min ratio crossed rows / findable clusters
  bool mCheckMaxDcaXY{true};                               // check max dca in xy plane
  bool mCheckMaxDcaZ{true};                                // check max dca in z direction

  // vector of ITS requirements (minNRequiredHits in specific requiredLayers)
  std::vector<std::pair<int8_t, std::set<uint8_t>>> mRequiredITSHits{};

  ClassDef(TrackSelectionBrick, 1);
};

///////////////////////////////////////////////////////////////////////////////////////
template <typename TValueToFilter>
CutBrick<TValueToFilter>* CutBrick<TValueToFilter>::constructBrick(const char* name, const char* regex, const std::set<std::string>& allowed)
{
  CutBrick<TValueToFilter>* thebrick = nullptr;

  bool found = false;
  for (auto bname : allowed) {
    if (TString(regex).BeginsWith(bname)) {
      found = true;
      break;
    }
  }
  if (not found) {
    ::Fatal("CutBrick<TValueToFilter>* CutBrick<TValueToFilter>::constructBrick", "Wrong RE: %s, trying to construct a not allowed basic cut brick", regex);
  }
  TString brickregex = TString(name) + "{" + TString(regex) + "}";

  if (TString(regex).BeginsWith("lim")) {
    thebrick = new CutBrickLimit<TValueToFilter>(brickregex);
  } else if (TString(regex).BeginsWith("th")) {
    thebrick = new CutBrickThreshold<TValueToFilter>(brickregex);
  } else if (TString(regex).BeginsWith("rg")) {
    thebrick = new CutBrickRange<TValueToFilter>(brickregex);
  } else if (TString(regex).BeginsWith("xrg")) {
    thebrick = new CutBrickExtToRange<TValueToFilter>(brickregex);
  } else if (TString(regex).BeginsWith("mrg")) {
    thebrick = new CutBrickSelectorMultipleRanges<TValueToFilter>(brickregex);
  } else if (TString(regex).BeginsWith("cwv")) {
    thebrick = new CutWithVariations<TValueToFilter>(brickregex);
  } else {
    ::Fatal("CutBrick<TValueToFilter>* CutBrick<TValueToFilter>::constructBrick", "Wrong RE: %s, trying to construct an unknown basic cut brick", regex);
  }
  return thebrick;
}

/// Default constructor
template <typename TValueToFilter>
CutBrick<TValueToFilter>::CutBrick()
  : TNamed(),
    mState(kPASSIVE),
    mMode(kUNSELECTED)
{
}

/// Named constructor
/// \param name The name of the brick
template <typename TValueToFilter>
CutBrick<TValueToFilter>::CutBrick(const char* name, const char* title)
  : TNamed(name, title),
    mState(kPASSIVE),
    mMode(kUNSELECTED)
{
}

///////////////////////////////////////////////////////////////////////////////////////
/// Default constructor
template <typename TValueToFilter>
CutBrickLimit<TValueToFilter>::CutBrickLimit()
  : CutBrick<TValueToFilter>(),
    mLimit(TValueToFilter(0))
{
}

/// Named constructor
/// \param name The name of the brick
/// \param value The limit value
template <typename TValueToFilter>
CutBrickLimit<TValueToFilter>::CutBrickLimit(const char* name, const TValueToFilter& value)
  : CutBrick<TValueToFilter>(name, TString::Format("%s{lim{%f}}", name, float(value))),
    mLimit(value)
{
}

/// \brief Cut string constructor
/// \param cutstr The cuts string
template <typename TValueToFilter>
CutBrickLimit<TValueToFilter>::CutBrickLimit(const TString& cutstr)
  : CutBrick<TValueToFilter>(),
    mLimit(TValueToFilter(0))
{
  ConstructCutFromString(cutstr);
}

/// \brief Construct the cut from a cut string
/// \param cutstr The cut string
/// The cut string should have the structure
///    name{lim{val}}
/// If the cut string is correctly parsed the cut is correctly built
/// if not a fatal exception is rised
template <typename TValueToFilter>
void CutBrickLimit<TValueToFilter>::ConstructCutFromString(const TString& cutstr)
{
  std::regex cutregex("^(\\w+)\\{lim\\{((?:-?[\\d]+\\.?[\\d]*)|(?:-?[\\d]*\\.?[\\d]+))}}$", std::regex_constants::ECMAScript | std::regex_constants::icase);
  std::string in(cutstr.Data());
  std::smatch m;

  std::regex_search(in, m, cutregex);
  if (m.empty() or (m.size() < 3)) {
    Fatal("CutBrickLimit<TValueToFilter>::ConstructCutFromString", "Wrong RE: %s, use pT{lim{2.0}} for instance", cutstr.Data());
  } else {
    this->SetName(m[1].str().c_str());
    this->SetTitle(cutstr.Data());
    mLimit = TValueToFilter(std::stod(m[2]));
  }
}

/// \brief Filter the passed value to update the brick status accordingly
/// \param value The value to filter
/// \return true if the value passed the cut false otherwise
template <typename TValueToFilter>
std::vector<bool> CutBrickLimit<TValueToFilter>::Filter(const TValueToFilter& value)
{
  std::vector<bool> res;
  if (value < mLimit) {
    this->mState = this->kACTIVE;
    res.push_back(true);
  } else {
    this->mState = this->kPASSIVE;
    res.push_back(false);
  }
  return res;
}

///////////////////////////////////////////////////////////////////////////////////////
/// Default constructor
template <typename TValueToFilter>
CutBrickThreshold<TValueToFilter>::CutBrickThreshold()
  : CutBrick<TValueToFilter>(),
    mThreshold(0)
{
}

/// Named constructor
/// \param name The name of the brick
/// \param value The threshold value
template <typename TValueToFilter>
CutBrickThreshold<TValueToFilter>::CutBrickThreshold(const char* name, const TValueToFilter& value)
  : CutBrick<TValueToFilter>(name, TString::Format("%s{th{%f}}", name, float(value))),
    mThreshold(value)
{
}

/// \brief Cut string constructor
/// \param cutstr The cuts string
template <typename TValueToFilter>
CutBrickThreshold<TValueToFilter>::CutBrickThreshold(const TString& cutstr)
  : CutBrick<TValueToFilter>(),
    mThreshold(0)
{
  ConstructCutFromString(cutstr);
}

/// \brief Construct the cut from a cut string
/// \param cutstr The cut string
/// The cut string should have the structure
///    name{th{val}}
/// If the cut string is correctly parsed the cut is correctly built
/// if not a fatal exception is rised
template <typename TValueToFilter>
void CutBrickThreshold<TValueToFilter>::ConstructCutFromString(const TString& cutstr)
{
  std::regex cutregex("^(\\w+)\\{th\\{((?:-?[\\d]+\\.?[\\d]*)|(?:-?[\\d]*\\.?[\\d]+))}}$", std::regex_constants::ECMAScript | std::regex_constants::icase);
  std::string in(cutstr.Data());
  std::smatch m;

  std::regex_search(in, m, cutregex);
  if (m.empty() or (m.size() < 3)) {
    Fatal("CutBrickThreshold<TValueToFilter>::ConstructCutFromString", "Wrong RE: %s, use pT{th{0.2}} for instance", cutstr.Data());
  } else {
    this->SetName(m[1].str().c_str());
    this->SetTitle(cutstr.Data());
    mThreshold = TValueToFilter(std::stod(m[2]));
  }
}

/// \brief Filter the passed value to update the brick status accordingly
/// \param value The value to filter
/// \return true if the value passed the cut false otherwise
template <typename TValueToFilter>
std::vector<bool> CutBrickThreshold<TValueToFilter>::Filter(const TValueToFilter& value)
{
  std::vector<bool> res;
  if (mThreshold <= value) {
    this->mState = this->kACTIVE;
    res.push_back(true);
  } else {
    this->mState = this->kPASSIVE;
    res.push_back(false);
  }
  return res;
}

///////////////////////////////////////////////////////////////////////////////////////
/// Default constructor
template <typename TValueToFilter>
CutBrickRange<TValueToFilter>::CutBrickRange()
  : CutBrick<TValueToFilter>(),
    mLow(0),
    mHigh(0)
{
}

/// Named constructor
/// \param name The name of the brick
/// \param low The low value for the cut range
/// \param high The high value for the cut range
template <typename TValueToFilter>
CutBrickRange<TValueToFilter>::CutBrickRange(const char* name, const TValueToFilter& low, const TValueToFilter& high)
  : CutBrick<TValueToFilter>(name, TString::Format("%s{rg{%f,%f}}", name, float(low), float(high))),
    mLow(low),
    mHigh(high)
{
}

/// \brief Cut string constructor
/// \param cutstr The cuts string
template <typename TValueToFilter>
CutBrickRange<TValueToFilter>::CutBrickRange(const TString& cutstr)
  : CutBrick<TValueToFilter>(),
    mLow(0),
    mHigh(0)
{
  ConstructCutFromString(cutstr);
}

/// \brief Construct the cut from a cut string
/// \param cutstr The cut string
/// The cut string should have the structure
///    name{rg{low,high}}
/// If the cut string is correctly parsed the cut is correctly built
/// if not a fatal exception is rised
template <typename TValueToFilter>
void CutBrickRange<TValueToFilter>::ConstructCutFromString(const TString& cutstr)
{
  std::regex cutregex("^(\\w+)\\{rg\\{((?:-?[\\d]+\\.?[\\d]*)|(?:-?[\\d]*\\.?[\\d]+)),((?:-?[\\d]+\\.?[\\d]*)|(?:-?[\\d]*\\.?[\\d]+))}}$", std::regex_constants::ECMAScript | std::regex_constants::icase);
  std::string in(cutstr.Data());
  std::smatch m;

  std::regex_search(in, m, cutregex);
  if (m.empty() or (m.size() < 4)) {
    Fatal("CutBrickRange<TValueToFilter>::ConstructCutFromString", "Wrong RE: %s, use pT{rg{0.2,2.0}} for instance", cutstr.Data());
  } else {
    this->SetName(m[1].str().c_str());
    this->SetTitle(cutstr.Data());
    mLow = TValueToFilter(std::stod(m[2]));
    mHigh = TValueToFilter(std::stod(m[3]));
  }
}

/// \brief Filter the passed value to update the brick status accordingly
/// \param value The value to filter
/// \return true if the value passed the cut false otherwise
template <typename TValueToFilter>
std::vector<bool> CutBrickRange<TValueToFilter>::Filter(const TValueToFilter& value)
{
  std::vector<bool> res;
  if ((mLow <= value) and (value < mHigh)) {
    this->mState = this->kACTIVE;
    res.push_back(true);
  } else {
    this->mState = this->kPASSIVE;
    res.push_back(false);
  }
  return res;
}

///////////////////////////////////////////////////////////////////////////////////////
/// Default constructor
template <typename TValueToFilter>
CutBrickExtToRange<TValueToFilter>::CutBrickExtToRange()
  : CutBrick<TValueToFilter>(),
    mLow(0),
    mHigh(0)
{
}

/// Named constructor
/// \param name The name of the brick
/// \param low The low value for the cut excluded range
/// \param high The high value for the cut excluded range
template <typename TValueToFilter>
CutBrickExtToRange<TValueToFilter>::CutBrickExtToRange(const char* name, const TValueToFilter& low, const TValueToFilter& high)
  : CutBrick<TValueToFilter>(name, TString::Format("%s{xrg{%f,%f}}", name, float(low), float(high))),
    mLow(low),
    mHigh(high)
{
}

/// \brief Cut string constructor
/// \param cutstr The cuts string
template <typename TValueToFilter>
CutBrickExtToRange<TValueToFilter>::CutBrickExtToRange(const TString& cutstr)
  : CutBrick<TValueToFilter>(),
    mLow(0),
    mHigh(0)
{
  ConstructCutFromString(cutstr);
}

/// \brief Construct the cut from a cut string
/// \param cutstr The cut string
/// The cut string should have the structure
///    name{xrg{low,high}}
/// If the cut string is correctly parsed the cut is correctly built
/// if not a fatal exception is rised
template <typename TValueToFilter>
void CutBrickExtToRange<TValueToFilter>::ConstructCutFromString(const TString& cutstr)
{
  std::regex cutregex("^(\\w+)\\{xrg\\{((?:-?[\\d]+\\.?[\\d]*)|(?:-?[\\d]*\\.?[\\d]+)),((?:-?[\\d]+\\.?[\\d]*)|(?:-?[\\d]*\\.?[\\d]+))}}$", std::regex_constants::ECMAScript | std::regex_constants::icase);
  std::string in(cutstr.Data());
  std::smatch m;

  std::regex_search(in, m, cutregex);
  if (m.empty() or (m.size() < 4)) {
    Fatal("CutBrickExtToRange<TValueToFilter>::ConstructCutFromString", "Wrong RE: %s, use minv{xrg{0.02,0.04}} for instance", cutstr.Data());
  } else {
    this->SetName(m[1].str().c_str());
    this->SetTitle(cutstr.Data());
    mLow = TValueToFilter(std::stod(m[2]));
    mHigh = TValueToFilter(std::stod(m[3]));
  }
}

/// \brief Filter the passed value to update the brick status accordingly
/// \param value The value to filter
/// \return true if the value passed the cut false otherwise
template <typename TValueToFilter>
std::vector<bool> CutBrickExtToRange<TValueToFilter>::Filter(const TValueToFilter& value)
{
  std::vector<bool> res;
  if ((value < mLow) or (mHigh <= value)) {
    this->mState = this->kACTIVE;
    res.push_back(true);
  } else {
    this->mState = this->kPASSIVE;
    res.push_back(false);
  }
  return res;
}

///////////////////////////////////////////////////////////////////////////////////////
/// Default constructor
template <typename TValueToFilter>
CutBrickSelectorMultipleRanges<TValueToFilter>::CutBrickSelectorMultipleRanges()
  : CutBrick<TValueToFilter>()
{
}

/// Named constructor
/// \param name The name of the brick
/// \param edges Vector with the ranges edges
template <typename TValueToFilter>
CutBrickSelectorMultipleRanges<TValueToFilter>::CutBrickSelectorMultipleRanges(const char* name, const std::vector<TValueToFilter>& edges)
  : CutBrick<TValueToFilter>(name, name)
{
  TString title = name;
  title += "{";
  bool first = true;
  for (auto edge : edges) {
    mEdges.push_back(edge);
    if (first) {
      title += TString::Format("%.2f", float(edge));
    } else {
      title += TString::Format(",%.2f", float(edge));
    }
  }
  title += "}";
  this->SetTitle(title.Data());
  for (int i = 1; i < mEdges.size(); ++i) {
    mActive.push_back(false);
  }
}

/// \brief Cut string constructor
/// \param cutstr The cuts string
template <typename TValueToFilter>
CutBrickSelectorMultipleRanges<TValueToFilter>::CutBrickSelectorMultipleRanges(const TString& cutstr)
  : CutBrick<TValueToFilter>()
{
  ConstructCutFromString(cutstr);
}

/// \brief Construct the cut from a cut string
/// \param cutstr The cut string
/// The cut string should have the structure
///    name{mrg{edge,edge, ...,edge}}
/// If the cut string is correctly parsed the cut is correctly built
/// if not a fatal exception is rised
template <typename TValueToFilter>
void CutBrickSelectorMultipleRanges<TValueToFilter>::ConstructCutFromString(const TString& cutstr)
{
  std::regex cutregex("^(\\w+)\\{mrg\\{((?:-?[\\d]+\\.?[\\d]*)|(?:-?[\\d]*\\.?[\\d]+))((?:,((?:-?[\\d]+\\.?[\\d]*)|(?:-?[\\d]*\\.?[\\d]+))){2,})}}$", std::regex_constants::ECMAScript | std::regex_constants::icase);
  std::string in(cutstr.Data());
  std::smatch m;

  std::regex_search(in, m, cutregex);
  if (m.empty() or (m.size() < 5)) {
    Fatal("CutBrickSelectorMultipleRanges<TValueToFilter>::ConstructCutFromString", "Wrong RE: %s, use V0M{mrg{0,5,10,20,30,40,50,60,70,80}} for instance", cutstr.Data());
  } else {
    this->SetName(m[1].str().c_str());
    this->SetTitle(cutstr.Data());
    /* the first edge on the list */
    mEdges.push_back(TValueToFilter(std::stod(m[2])));
    /* this now is a bit tricky due to the nature of the groups repetition within RE */
    TObjArray* tokens = TString(m[3]).Tokenize(",");
    for (int i = 0; i < tokens->GetEntries(); ++i) {
      mEdges.push_back(TValueToFilter(TString(tokens->At(i)->GetName()).Atof()));
    }
    for (int i = 1; i < mEdges.size(); ++i) {
      mActive.push_back(false);
    }
    delete tokens;
  }
}

/// \brief Filter the passed value to update the brick status accordingly
/// \param value The value to filter
/// \return true if the value passed the cut false otherwise
template <typename TValueToFilter>
std::vector<bool> CutBrickSelectorMultipleRanges<TValueToFilter>::Filter(const TValueToFilter& value)
{
  if ((mEdges.front() <= value) and (value < mEdges.back())) {
    this->mState = this->kACTIVE;
    for (int i = 0; i < mActive.size(); ++i) {
      if (value < mEdges[i + 1]) {
        mActive[i] = true;
      } else {
        mActive[i] = false;
      }
    }
  } else {
    this->mState = this->kPASSIVE;
    for (int i = 0; i < mActive.size(); ++i) {
      mActive[i] = false;
    }
  }
  return std::vector<bool>(mActive);
}

///////////////////////////////////////////////////////////////////////////////////////
/// Default constructor
template <typename TValueToFilter>
CutWithVariations<TValueToFilter>::CutWithVariations()
  : CutBrick<TValueToFilter>(),
    mAllowSeveralDefaults(false)
{
  mDefaultBricks.SetOwner(true);
  mVariationBricks.SetOwner(true);
}

/// Named constructor
/// \param name The name of the brick
/// \param cutstr The string associated with the cut
/// \param severaldefaults The cut should allow multiple defaults values or not
template <typename TValueToFilter>
CutWithVariations<TValueToFilter>::CutWithVariations(const char* name, const char* cutstr, bool severaldefaults)
  : CutBrick<TValueToFilter>(name, cutstr),
    mAllowSeveralDefaults(severaldefaults)
{
  mDefaultBricks.SetOwner(true);
  mVariationBricks.SetOwner(true);
}

/// \brief Cut string constructor
/// \param cutstr The cuts string
template <typename TValueToFilter>
CutWithVariations<TValueToFilter>::CutWithVariations(const TString& cutstr)
  : CutBrick<TValueToFilter>(),
    mAllowSeveralDefaults(false)
{
  mDefaultBricks.SetOwner(true);
  mVariationBricks.SetOwner(true);

  ConstructCutFromString(cutstr);
}

/// \brief Construct the cut from a cut string
/// \param cutstr The cuts string
/// The cut string should have the structure
///    name{def,def,..,def[;alt,alt,...,alt]}
/// where each of the def and alt are basic cut bricks
/// If the cut string is correctly parsed the cut is correctly built
/// if not a fatal exception is raised
template <typename TValueToFilter>
void CutWithVariations<TValueToFilter>::ConstructCutFromString(const TString& cutstr)
{
  /* let's catch the first level */
  std::regex cutregex("^(\\w+)\\{cwv\\{([\\w\\d.,;{}]+)}}$", std::regex_constants::ECMAScript | std::regex_constants::icase);
  std::string in(cutstr.Data());
  std::smatch m;

  std::regex_search(in, m, cutregex);
  if (m.empty() or (m.size() < 3)) {
    Fatal("CutWithVariations<TValueToFilter>::ConstructCutFromString", "Wrong RE: %s, use pT{cwv{rg{0.2,10.0}}} for instance", cutstr.Data());
  }
  this->SetName(m[1].str().c_str());
  this->SetTitle(cutstr.Data());

  /* let's split default and variations */
  TObjArray* lev1toks = TString(m[2]).Tokenize(";");
  if (lev1toks->GetEntries() > 2) {
    Fatal("CutWithVariations<TValueToFilter>::ConstructCutFromString", "Wrong RE: %s, use pT{cwv{rg{0.2,10.0}}} for instance", cutstr.Data());
  }
  /* let's handle the default values */
  {
    TObjArray* lev2toks = TString(lev1toks->At(0)->GetName()).Tokenize(",");
    if (lev2toks->GetEntries() > 1) {
      /* TODO: severa default options for track type and for track pid selection */
      Fatal("CutWithVariations<TValueToFilter>::ConstructCutFromString", "Wrong RE: %s, several defaults only for trktype or trkpid pending of implementation", cutstr.Data());
    }
    std::set<std::string> allowed = {"lim", "th", "rg", "xrg"};
    for (int i = 0; i < lev2toks->GetEntries(); ++i) {
      mDefaultBricks.Add(CutBrick<TValueToFilter>::constructBrick(m[1].str().c_str(), lev2toks->At(i)->GetName(), allowed));
    }
    delete lev2toks;
  }
  /* let's now handle the variations if any */
  if (lev1toks->GetEntries() > 1) {
    TObjArray* lev2toks = TString(lev1toks->At(1)->GetName()).Tokenize(",");
    std::set<std::string> allowed = {"lim", "th", "rg", "xrg"};
    for (int i = 0; i < lev2toks->GetEntries(); ++i) {
      mVariationBricks.Add(CutBrick<TValueToFilter>::constructBrick(m[1].str().c_str(), lev2toks->At(i)->GetName(), allowed));
    }
    delete lev2toks;
  }
  delete lev1toks;
}

/// \brief Stores the brick with a default value for the cut
/// \param brick pointer to the brick to incorporate
/// \returns true if the brick was successfully added
/// If several default values are allowed it is only required
/// that the name of the new default brick were unique
/// If only one default value is allowed it is required that
/// no previous default were stored
/// If any of the above conditions fails the brick is not
/// added and false is returned
template <typename TValueToFilter>
bool CutWithVariations<TValueToFilter>::AddDefaultBrick(CutBrick<TValueToFilter>* brick)
{
  if (mAllowSeveralDefaults) {
    if (mDefaultBricks.FindObject(brick->GetName())) {
      return false;
    } else {
      mDefaultBricks.Add(brick);
      return true;
    }
  } else {
    if (mDefaultBricks.GetEntries() > 0) {
      return false;
    } else {
      mDefaultBricks.Add(brick);
      return true;
    }
  }
}

/// \brief Stores the brick with the variation to the default value for the cut
/// \param brick pointer to the brick to incorporate
/// \returns true if the brick was successfully added
/// It is required that the brick name were unique in the list of variation values brick
template <typename TValueToFilter>
bool CutWithVariations<TValueToFilter>::AddVariationBrick(CutBrick<TValueToFilter>* brick)
{
  if (mVariationBricks.FindObject(brick->GetName())) {
    return false;
  } else {
    mVariationBricks.Add(brick);
    return true;
  }
}

/// Filters the passed value
/// The bricks on the default values list and in the variation
/// values list will change to active or passive accordingly to the passed value
/// \param value The value to filter
/// \returns true if the value activated any of the bricks

template <typename TValueToFilter>
std::vector<bool> CutWithVariations<TValueToFilter>::Filter(const TValueToFilter& value)
{
  std::vector<bool> res;
  res.reserve(Length());
  bool active = false;
  for (int i = 0; i < mDefaultBricks.GetEntries(); ++i) {
    std::vector<bool> tmp = ((CutBrick<TValueToFilter>*)mDefaultBricks.At(i))->Filter(value);
    res.insert(res.end(), tmp.begin(), tmp.end());
  }
  for (int i = 0; i < mVariationBricks.GetEntries(); ++i) {
    std::vector<bool> tmp = ((CutBrick<TValueToFilter>*)mVariationBricks.At(i))->Filter(value);
    res.insert(res.end(), tmp.begin(), tmp.end());
  }
  return res;
}

/// Return the length needed to code the cut
/// The length is in brick units. The actual length is implementation dependent
/// \returns Cut length in units of bricks
template <typename TValueToFilter>
int CutWithVariations<TValueToFilter>::Length()
{
  /* TODO: should a single default cut without variations return zero length? */
  int length = 0;
  for (int i = 0; i < mDefaultBricks.GetEntries(); ++i) {
    length += ((CutBrick<TValueToFilter>*)mDefaultBricks.At(i))->Length();
  }
  for (int i = 0; i < mVariationBricks.GetEntries(); ++i) {
    length += ((CutBrick<TValueToFilter>*)mVariationBricks.At(i))->Length();
  }
  return length;
}


template class o2::analysis::PWGCF::CutBrickLimit<int>;
template class o2::analysis::PWGCF::CutBrickLimit<float>;

template class o2::analysis::PWGCF::CutBrickThreshold<int>;
template class o2::analysis::PWGCF::CutBrickThreshold<float>;

template class o2::analysis::PWGCF::CutBrickRange<int>;
template class o2::analysis::PWGCF::CutBrickRange<float>;

template class o2::analysis::PWGCF::CutBrickExtToRange<int>;
template class o2::analysis::PWGCF::CutBrickExtToRange<float>;

template class o2::analysis::PWGCF::CutBrickSelectorMultipleRanges<int>;
template class o2::analysis::PWGCF::CutBrickSelectorMultipleRanges<float>;

template class o2::analysis::PWGCF::CutWithVariations<float>;
template class o2::analysis::PWGCF::CutWithVariations<int>;

} // namespace PWGCF
} // namespace analysis
} // namespace o2
#endif // ANALYSIS_CONFIGURABLE_CUTS_CLASSES_H
