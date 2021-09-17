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
class TrackSelectionAndFilterAnalysis : public TNamed
{
 public:
  TrackSelectionAndFilterAnalysis();

 private:
  TList mTrackTypes;

  CutWithVariations<float>* mNClustersTPC;
  CutWithVariations<float>* mNCrossedRowsTPC;
  CutWithVariations<float>* mNClustersITS;
  CutWithVariations<float>* mMaxChi2PerClusterTPC;
  CutWithVariations<float>* mMaxChi2PerClusterITS;
  CutWithVariations<float>* mMinNCrossedRowsOverFindableClustersTPC;
  CutWithVariations<float>* mMaxDcaXY;
  CutWithVariations<float>* mMaxDcaZ;
  CutWithVariations<float>* mPtRange;
  CutWithVariations<float>* mEtaRange;
  ULong64_t mSelectedMask = 0UL;
  ULong64_t mArmedMask = 0UL;

  ClassDef(TrackSelectionAndFilterAnalysis, 1)
};

} // namespace PWGCF
} // namespace analysis
} // namespace o2

#endif // TRACKSELECTIONFILTERANDANALYSIS_H
