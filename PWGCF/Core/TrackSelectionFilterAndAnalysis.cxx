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

#include "TrackSelectionFilterAndAnalysis.h"

using namespace o2::analysis::PWGCF;

ClassImp(TrackSelectionAndFilterAnalysis);

/// \brief Default constructor
TrackSelectionAndFilterAnalysis::TrackSelectionAndFilterAnalysis() : TNamed(),
                                                                     mNClustersTPC(nullptr),
                                                                     mNCrossedRowsTPC(nullptr),
                                                                     mNClustersITS(nullptr),
                                                                     mMaxChi2PerClusterTPC(nullptr),
                                                                     mMaxChi2PerClusterITS(nullptr),
                                                                     mMinNCrossedRowsOverFindableClustersTPC(nullptr),
                                                                     mMaxDcaXY(nullptr),
                                                                     mMaxDcaZ(nullptr),
                                                                     mSelectedMask(0UL),
                                                                     mArmedMask(0UL)
{
  /* at least we initialize by default pT and eta cuts */
  mPtRange = new CutWithVariations<float>(TString("pT{rg{0.2,10}}"));
  mEtaRange = new CutWithVariations<float>(TString("eta{rg{-0.8,0.8}}"));
}