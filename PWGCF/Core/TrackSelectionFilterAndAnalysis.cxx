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

#include <boost/regex.hpp>
#include <TObjArray.h>

#include "Framework/AnalysisTask.h"
#include "Framework/AnalysisDataModel.h"
#include "TrackSelectionFilterAndAnalysis.h"

using namespace o2::analysis::PWGCF;
using namespace boost;

ClassImp(TrackSelectionFilterAndAnalysis);

/// \brief Default constructor
TrackSelectionFilterAndAnalysis::TrackSelectionFilterAndAnalysis() : TNamed(),
                                                                     mNClustersTPC(nullptr),
                                                                     mNCrossedRowsTPC(nullptr),
                                                                     mNClustersITS(nullptr),
                                                                     mMaxChi2PerClusterTPC(nullptr),
                                                                     mMaxChi2PerClusterITS(nullptr),
                                                                     mMinNCrossedRowsOverFindableClustersTPC(nullptr),
                                                                     mMaxDcaXY(nullptr),
                                                                     mMaxDcaZ(nullptr),
                                                                     mMaskLength(0),
                                                                     mSelectedMask(0UL),
                                                                     mArmedMask(0UL)
{
  /* we own the track types cuts objects */
  mTrackTypes.SetOwner(true);
  /* at least we initialize by default pT and eta cuts */
  mPtRange = CutBrick<float>::constructBrick("pT", "rg{0.2,10}", std::set<std::string>{"rg"});
  mEtaRange = CutBrick<float>::constructBrick("eta", "rg{-0.8,0.8}", std::set<std::string>{"rg"});
}

/// \brief Constructor from regular expression
TrackSelectionFilterAndAnalysis::TrackSelectionFilterAndAnalysis(const TString& cutstr) : TNamed(),
                                                                                          mNClustersTPC(nullptr),
                                                                                          mNCrossedRowsTPC(nullptr),
                                                                                          mNClustersITS(nullptr),
                                                                                          mMaxChi2PerClusterTPC(nullptr),
                                                                                          mMaxChi2PerClusterITS(nullptr),
                                                                                          mMinNCrossedRowsOverFindableClustersTPC(nullptr),
                                                                                          mMaxDcaXY(nullptr),
                                                                                          mMaxDcaZ(nullptr),
                                                                                          mMaskLength(0),
                                                                                          mSelectedMask(0UL),
                                                                                          mArmedMask(0UL)
{
  /* we own the track types cuts objects */
  mTrackTypes.SetOwner(true);
  /* at least we initialize by default pT and eta cuts */
  mPtRange = CutBrick<float>::constructBrick("pT", "rg{0.2,10}", std::set<std::string>{"rg"});
  mEtaRange = CutBrick<float>::constructBrick("eta", "rg{-0.8,0.8}", std::set<std::string>{"rg"});

  ConstructCutFromString(cutstr);
}

void TrackSelectionFilterAndAnalysis::SetPtRange(const TString& regex)
{
  if (mPtRange != nullptr) {
    delete mPtRange;
  }
  mPtRange = CutBrick<float>::constructBrick("pT", regex.Data(), std::set<std::string>{"rg", "th", "lim", "xrg"});
}

void TrackSelectionFilterAndAnalysis::SetEtaRange(const TString& regex)
{
  if (mEtaRange != nullptr) {
    delete mEtaRange;
  }
  mEtaRange = CutBrick<float>::constructBrick("eta", regex.Data(), std::set<std::string>{"rg", "th", "lim", "xrg"});
}

void TrackSelectionFilterAndAnalysis::ConstructCutFromString(const TString& cutstr)
{
  /* let's catch the first level */
  regex cutregex("^tracksel\\{ttype\\{([\\w\\d,]+)};?(\\w+\\{[\\w\\d.,;{}]+})*}$", regex_constants::ECMAScript | regex_constants::icase);
  std::string in(cutstr.Data());
  smatch m;

  regex_search(in, m, cutregex);
  if (m.empty() or (m.size() > 3)) {
    Fatal("TrackSelectionFilterAndAnalysis::::ConstructCutFromString", "Wrong RE: %s, try tracksel{ttype{FB32,FB96};ncls{th{70}},nxr{cwv{th{70},th{80}}}} for instance", cutstr.Data());
  }
  this->SetName("TrackSelectionFilterAndAnalysisCuts");
  this->SetTitle(cutstr.Data());

  /* let's split the handling of track types and of its characteristics */
  /* let's handle the track types */
  {
    TObjArray* lev2toks = TString(m[1].str()).Tokenize(",");
    LOGF(info, "Captured %s", m[1].str().c_str());
    for (int i = 0; i < lev2toks->GetEntries(); ++i) {
      mTrackTypes.Add(new TrackSelectionBrick(lev2toks->At(i)->GetName()));
    }
    delete lev2toks;
  }
  /* let's now handle the reco track characteristics */
  {
    TString lev2str = m[2].str();
    while (lev2str.Length() != 0) {
      std::set<std::string> allowed = {"lim", "th", "rg", "xrg", "cwv"};
      lev2str.Remove(TString::kLeading, ' ');
      lev2str.Remove(TString::kLeading, ',');
      lev2str.Remove(TString::kLeading, ' ');
      if (lev2str.Length() == 0) {
        break;
      }
      regex cutregex("^(\\w+)\\{((?:[^{}]++|\\{(?2)\\})*)\\}");
      std::string in(lev2str.Data());
      smatch m;
      regex_search(in, m, cutregex);

      if (m.empty() or (m.size() != 3)) {
        Fatal("TrackSelectionFilterAndAnalysis::::ConstructCutFromString", "Wrong RE: %s, try tracksel{ttype{FB32,FB96};nclstpc{th{70}},nxr{cwv{th{70},th{80}}}} for instance", cutstr.Data());
      }
      LOGF(info, "Captured %s", m[1].str().c_str());
      if (m[1].str() == "nclstpc") {
        if (mNClustersTPC != nullptr) {
          delete mNClustersTPC;
        }
        mNClustersTPC = CutBrick<float>::constructBrick(m[1].str().c_str(), m[2].str().c_str(), allowed);
        for (int i = 0; i < mTrackTypes.GetEntries(); ++i) {
          ((TrackSelectionBrick*)mTrackTypes.At(i))->DisableNClustersTPCCheck();
        }
      } else if (m[1].str() == "nclsits") {
        if (mNClustersITS != nullptr) {
          delete mNClustersITS;
        }
        mNClustersITS = CutBrick<float>::constructBrick(m[1].str().c_str(), m[2].str().c_str(), allowed);
        for (int i = 0; i < mTrackTypes.GetEntries(); ++i) {
          ((TrackSelectionBrick*)mTrackTypes.At(i))->DisableNClustersITSCheck();
        }
      } else if (m[1].str() == "nxrtpc") {
        if (mNCrossedRowsTPC != nullptr) {
          delete mNCrossedRowsTPC;
        }
        mNCrossedRowsTPC = CutBrick<float>::constructBrick(m[1].str().c_str(), m[2].str().c_str(), allowed);
        for (int i = 0; i < mTrackTypes.GetEntries(); ++i) {
          ((TrackSelectionBrick*)mTrackTypes.At(i))->DisableNCrossedRowsTPCCheck();
        }
      } else if (m[1].str() == "chi2clustpc") {
        if (mMaxChi2PerClusterTPC != nullptr) {
          delete mMaxChi2PerClusterTPC;
        }
        mMaxChi2PerClusterTPC = CutBrick<float>::constructBrick(m[1].str().c_str(), m[2].str().c_str(), allowed);
        for (int i = 0; i < mTrackTypes.GetEntries(); ++i) {
          ((TrackSelectionBrick*)mTrackTypes.At(i))->DisableMaxChi2PerClusterTPCCheck();
        }
      } else if (m[1].str() == "chi2clusits") {
        if (mMaxChi2PerClusterITS != nullptr) {
          delete mMaxChi2PerClusterITS;
        }
        mMaxChi2PerClusterITS = CutBrick<float>::constructBrick(m[1].str().c_str(), m[2].str().c_str(), allowed);
        for (int i = 0; i < mTrackTypes.GetEntries(); ++i) {
          ((TrackSelectionBrick*)mTrackTypes.At(i))->DisableMaxChi2PerClusterITSCheck();
        }
      } else if (m[1].str() == "xrofctpc") {
        if (mMinNCrossedRowsOverFindableClustersTPC != nullptr) {
          delete mMinNCrossedRowsOverFindableClustersTPC;
        }
        mMinNCrossedRowsOverFindableClustersTPC = CutBrick<float>::constructBrick(m[1].str().c_str(), m[2].str().c_str(), allowed);
        for (int i = 0; i < mTrackTypes.GetEntries(); ++i) {
          ((TrackSelectionBrick*)mTrackTypes.At(i))->DisableMinNCrossedRowsOverFindableClustersTPCCheck();
        }
      } else if (m[1].str() == "dcaxy") {
        if (mMaxDcaXY != nullptr) {
          delete mMaxDcaXY;
        }
        mMaxDcaXY = CutBrick<float>::constructBrick(m[1].str().c_str(), m[2].str().c_str(), allowed);
        for (int i = 0; i < mTrackTypes.GetEntries(); ++i) {
          ((TrackSelectionBrick*)mTrackTypes.At(i))->DisableMaxDcaXYCheck();
        }
      } else if (m[1].str() == "dcaz") {
        if (mMaxDcaZ != nullptr) {
          delete mMaxDcaZ;
        }
        mMaxDcaZ = CutBrick<float>::constructBrick(m[1].str().c_str(), m[2].str().c_str(), allowed);
        for (int i = 0; i < mTrackTypes.GetEntries(); ++i) {
          ((TrackSelectionBrick*)mTrackTypes.At(i))->DisableMaxDcaZCheck();
        }
      } else {
        Fatal("TrackSelectionFilterAndAnalysis::::ConstructCutFromString", "Wrong RE: %s, cut on variable %s not implemented", cutstr.Data(), m[1].str().c_str());
      }
      /* removing the already handled track characteristics cut */
      lev2str.Remove(0, m[0].length());
    }
  }
}
