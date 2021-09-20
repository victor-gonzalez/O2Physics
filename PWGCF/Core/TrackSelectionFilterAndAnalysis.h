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
  bool Filter(TrackToFilter const& track);

 private:
  void ConstructCutFromString(const TString&);

  TList mTrackTypes;                                        /// the track types to select list
  CutBrick<float>* mNClustersTPC;                           //! the number of TPC clusters cuts
  CutBrick<float>* mNCrossedRowsTPC;                        //! the number of TPC crossed rows cuts
  CutBrick<float>* mNClustersITS;                           //! the number of ITS clusters cuts
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

} // namespace PWGCF
} // namespace analysis
} // namespace o2

#endif // TRACKSELECTIONFILTERANDANALYSIS_H
