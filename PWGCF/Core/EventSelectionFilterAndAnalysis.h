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
#ifndef EVENTSELECTIONFILTERANDANALYSIS_H
#define EVENTSELECTIONFILTERANDANALYSIS_H

#include <Rtypes.h>
#include <TString.h>
#include <TObject.h>
#include <TNamed.h>
#include <TList.h>

#include "AnalysisConfigurableCuts.h"
#include "SelectionFilterAndAnalysis.h"

namespace o2
{
namespace analysis
{
namespace PWGCF
{
/* forward declaration */
class EventSelectionFilterAndAnalysis;

///\brief Convenince class for configurable access
class EventSelectionConfigurable
{
  friend class EventSelectionFilterAndAnalysis;

 public:
  EventSelectionConfigurable(std::string multsel = "",
                             std::string trigsel = "",
                             std::string zvtxsel = "",
                             std::string pileuprej = "")
    : mMultSel{multsel}, mTriggerSel{trigsel}, mZVertexSel{zvtxsel}, mPileUpRejection{pileuprej}
  {
  }

 private:
  std::string mMultSel = "";         //! the multiplicity selection cuts
  std::string mTriggerSel = "";      //! the trigger selection cuts
  std::string mZVertexSel = "";      //! the z vertex selection cuts
  std::string mPileUpRejection = ""; //! the pile-up rejection criteria

 private:
  ClassDefNV(EventSelectionConfigurable, 1);
};

/// \brief Filter of tracks and track selection once filetered
class EventSelectionFilterAndAnalysis : public SelectionFilterAndAnalysis
{
 public:
  EventSelectionFilterAndAnalysis();
  EventSelectionFilterAndAnalysis(const TString&, selmodes);
  EventSelectionFilterAndAnalysis(const EventSelectionConfigurable&, selmodes);

  template <typename CollisionToFilter>
  uint64_t Filter(CollisionToFilter const& col);

 private:
  void ConstructCutFromString(const TString&);
  int CalculateMaskLength();

  CutBrick<float>* mMultiplicityClasses; //! the multiplicity classes cuts
  CutBrick<int>* mTriggerSelection;      //! the trigger selection cuts
  CutBrick<float>* mZVertex;             //! the z vertex selection cuts
  CutBrick<int>* mPileUpRejection;       //! the pile-up rejection criteria

  ClassDef(EventSelectionFilterAndAnalysis, 1)
};

/// \brief Fills the filter cuts mask
template <typename CollisionToFilter>
uint64_t EventSelectionFilterAndAnalysis::Filter(CollisionToFilter const& col)
{
  /* limit for the current implementation */
  int length = CalculateMaskLength();
  if (length > 64) {
    LOGF(fatal, "EventSelectionFilterAndAnalysis not ready for filter mask of %d bits. Just 64 available for the time being");
  }

  uint64_t selectedMask = 0UL;
  int bit = 0;

  auto filterBrickValue = [&](auto brick, auto value) {
    std::vector<bool> res = brick->Filter(value);
    for (auto b : res) {
      if (b) {
        SETBIT(selectedMask, bit);
      }
      bit++;
    }
  };

  if (mZVertex != nullptr) {
    filterBrickValue(mZVertex, col.posZ());
  }
  return mSelectedMask = selectedMask;
}

} // namespace PWGCF
} // namespace analysis
} // namespace o2

#endif // EVENTSELECTIONFILTERANDANALYSIS_H
