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
#ifndef TRACKSELECTIONFILTERANDANALYSIS_H
#define TRACKSELECTIONFILTERANDANALYSIS_H

#include <Rtypes.h>
#include <TString.h>
#include <TObject.h>
#include <TNamed.h>
#include <TList.h>

#include "AnalysisConfigurableCuts.h"

namespace o2
{
namespace analysis
{
namespace PWGCF
{

/// \brief Filter of tracks and track selection once filetered
class TrackSelectionFilterAndAnalysis : public TNamed
{
 public:
  TrackSelectionFilterAndAnalysis();
  TrackSelectionFilterAndAnalysis(const TString&);

  void SetPtRange(const TString&);
  void SetEtaRange(const TString&);

  template <typename TrackToFilter>
  uint64_t Filter(TrackToFilter const& track);

 private:
  void ConstructCutFromString(const TString&);
  int CalculateMaskLength();

  TList mTrackTypes;                                        /// the track types to select list
  CutBrick<int>* mNClustersTPC;                             //! the number of TPC clusters cuts
  CutBrick<int>* mNCrossedRowsTPC;                          //! the number of TPC crossed rows cuts
  CutBrick<int>* mNClustersITS;                             //! the number of ITS clusters cuts
  CutBrick<float>* mMaxChi2PerClusterTPC;                   //! the max Chi2 per TPC cluster cuts
  CutBrick<float>* mMaxChi2PerClusterITS;                   //! the max Chi2 per ITS cluster cuts
  CutBrick<float>* mMinNCrossedRowsOverFindableClustersTPC; //! the min ration crossed TPC rows over findable TPC clusters cuts
  CutBrick<float>* mMaxDcaXY;                               //! the DCAxy cuts
  CutBrick<float>* mMaxDcaZ;                                //! the DCAz cuts
  CutBrick<float>* mPtRange;                                //! the pT range cuts
  CutBrick<float>* mEtaRange;                               //! the eta range cuts
  int mMaskLength;                                          /// the length of the mask needed to filter the selection cuts
  ULong64_t mSelectedMask = 0UL;                            /// the selection mask for the current passed track
  ULong64_t mArmedMask = 0UL;                               /// the armed mask identifying the significative selection cuts

  ClassDef(TrackSelectionFilterAndAnalysis, 1)
};

/// \brief Fills the filter cuts mask
template <typename TrackToFilter>
uint64_t TrackSelectionFilterAndAnalysis::Filter(TrackToFilter const& track)
{
  /* limit for the current implementation */
  int length = CalculateMaskLength();
  if (length > 64) {
    LOGF(fatal, "TrackSelectionFilterAndAnalysis not ready for filter mask of %d bits. Just 64 available for the time being");
  }

  uint64_t selectedMask = 0UL;
  int bit = 0;

  auto filterTrackType = [&](TrackSelectionBrick* ttype, auto trk) {
    if (ttype->Filter(trk)) {
      SETBIT(selectedMask, bit++);
    }
  };

  auto filterBrickValue = [&](auto brick, auto value) {
    std::vector<bool> res = brick->Filter(value);
    for (auto b : res) {
      if (b) {
        SETBIT(selectedMask, bit++);
      }
    }
  };

  for (int i = 0; i < mTrackTypes.GetEntries(); ++i) {
    filterTrackType((TrackSelectionBrick*)mTrackTypes.At(i), track);
  }
  if (mNClustersTPC != nullptr) {
    filterBrickValue(mNClustersTPC, track.tpcNClsFound());
  }
  if (mNCrossedRowsTPC != nullptr) {
    filterBrickValue(mNCrossedRowsTPC, track.tpcNClsCrossedRows());
  }
  if (mNClustersITS != nullptr) {
    filterBrickValue(mNClustersITS, track.itsNCls());
  }
  if (mMaxChi2PerClusterTPC != nullptr) {
    filterBrickValue(mMaxChi2PerClusterTPC, track.tpcChi2NCl());
  }
  if (mMaxChi2PerClusterITS != nullptr) {
    filterBrickValue(mMaxChi2PerClusterITS, track.itsChi2NCl());
  }
  if (mMinNCrossedRowsOverFindableClustersTPC != nullptr) {
    filterBrickValue(mMinNCrossedRowsOverFindableClustersTPC, track.tpcCrossedRowsOverFindableCls());
  }
  if (mMaxDcaXY != nullptr) {
    filterBrickValue(mMaxDcaXY, track.dcaXY());
  }
  if (mMaxDcaZ != nullptr) {
    filterBrickValue(mMaxDcaZ, track.dcaZ());
  }
  if (mPtRange != nullptr) {
    filterBrickValue(mPtRange, track.pt());
  }
  if (mEtaRange != nullptr) {
    filterBrickValue(mEtaRange, track.eta());
  }
  return mSelectedMask = selectedMask;
}

} // namespace PWGCF
} // namespace analysis
} // namespace o2

#endif // TRACKSELECTIONFILTERANDANALYSIS_H
