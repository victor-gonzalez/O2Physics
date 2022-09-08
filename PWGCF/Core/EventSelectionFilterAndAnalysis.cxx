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
#include "Framework/ASoAHelpers.h"
#include "EventSelectionFilterAndAnalysis.h"

using namespace o2;
using namespace o2::framework;
using namespace o2::soa;
using namespace o2::framework::expressions;
using namespace o2::analysis::PWGCF;
using namespace boost;

ClassImp(EventSelectionFilterAndAnalysis);

/// \brief Default constructor
EventSelectionFilterAndAnalysis::EventSelectionFilterAndAnalysis()
  : SelectionFilterAndAnalysis(),
    mMultiplicityClasses(nullptr),
    mTriggerSelection(nullptr),
    mZVertex(nullptr),
    mPileUpRejection(nullptr)
{
}

/// \brief Constructor from regular expression
EventSelectionFilterAndAnalysis::EventSelectionFilterAndAnalysis(const TString& cutstr, selmodes mode)
  : SelectionFilterAndAnalysis("", mode),
    mMultiplicityClasses(nullptr),
    mTriggerSelection(nullptr),
    mZVertex(nullptr),
    mPileUpRejection(nullptr)
{
  ConstructCutFromString(cutstr);
}

/// \brief Constructor from the track selection configurable
EventSelectionFilterAndAnalysis::EventSelectionFilterAndAnalysis(const EventSelectionConfigurable& evtsel, selmodes mode)
  : SelectionFilterAndAnalysis("", mode),
    mMultiplicityClasses(nullptr),
    mTriggerSelection(nullptr),
    mZVertex(nullptr),
    mPileUpRejection(nullptr)
{
  TString cutString = "eventsel{";
  bool first = true;

  auto appendCut = [&cutString, &first](std::string str) {
    if (str.size() > 0) {
      if (first) {
        cutString += str;
        first = false;
      } else {
        cutString += "," + str;
      }
    }
  };
  appendCut(evtsel.mMultSel);
  appendCut(evtsel.mTriggerSel);
  appendCut(evtsel.mZVertexSel);
  appendCut(evtsel.mPileUpRejection);

  cutString += "}";
  ConstructCutFromString(cutString);
}

/// \brief Calculates the length of the mask needed to store the selection cuts
int EventSelectionFilterAndAnalysis::CalculateMaskLength()
{
  int length = 0;
  auto getLength = [&length](auto& brick) {
    if (brick != nullptr) {
      length += brick->Length();
    }
  };

  getLength(mMultiplicityClasses);
  getLength(mTriggerSelection);
  getLength(mZVertex);
  getLength(mPileUpRejection);

  return length;
}

void EventSelectionFilterAndAnalysis::ConstructCutFromString(const TString& cutstr)
{
  LOGF(info, "Cut string: %s", cutstr);
  /* let's catch the first level */
  regex cutregex("^eventsel\\{?(\\w+\\{[\\w\\d.,:{}-]+})*}$", regex_constants::ECMAScript | regex_constants::icase);
  std::string in(cutstr.Data());
  smatch m;

  bool res = regex_search(in, m, cutregex);
  if (not res or m.empty() or (m.size() > 2)) {
    Fatal("EventSelectionFilterAndAnalysis::::ConstructCutFromString", "Wrong RE: %s, try tracksel{ttype{FB32,FB96};ncls{th{70}},nxr{cwv{th{70},th{80}}}} for instance", cutstr.Data());
  }
  this->SetName("EventSelectionFilterAndAnalysisCuts");
  this->SetTitle(cutstr.Data());

  /* let's now handle the event characteristics */
  {
    LOGF(info, "Captured %s", m[1].str().c_str());
    TString lev2str = m[1].str();
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
      bool res = regex_search(in, m, cutregex);

      if (not res or m.empty() or (m.size() != 3)) {
        Fatal("EventSelectionFilterAndAnalysis::::ConstructCutFromString", "Wrong RE: %s, try tracksel{ttype{FB32,FB96};nclstpc{th{70}},nxr{cwv{th{70},th{80}}}} for instance", cutstr.Data());
      }
      LOGF(info, "Captured %s", m[1].str().c_str());
      auto storeIntCut = [&m, &allowed](CutBrick<int>*& brickvar) {
        if (brickvar != nullptr) {
          delete brickvar;
        }
        brickvar = CutBrick<int>::constructBrick(m[1].str().c_str(), m[2].str().c_str(), allowed);
      };
      auto storeFloatCut = [&m, &allowed](CutBrick<float>*& brickvar) {
        if (brickvar != nullptr) {
          delete brickvar;
        }
        brickvar = CutBrick<float>::constructBrick(m[1].str().c_str(), m[2].str().c_str(), allowed);
      };
      if (m[1].str() == "zvtx") {
        storeFloatCut(mZVertex);
      } else if (m[1].str() == "mult") {
        storeFloatCut(mMultiplicityClasses);
      } else if (m[1].str() == "mtrigg") {
        storeIntCut(mTriggerSelection);
      } else if (m[1].str() == "pileup") {
        storeIntCut(mPileUpRejection);
      }
      /* removing the already handled track characteristics cut */
      lev2str.Remove(0, m[0].length());
    }
  }
  mMaskLength = CalculateMaskLength();
}

