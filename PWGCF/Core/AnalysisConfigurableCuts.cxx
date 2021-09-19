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
template <typename TValueToFilter>
const char* CutBrick<TValueToFilter>::mgImplementedbricks[] = {"lim", "th", "rg", "xrg", "mrg", "cwv"};

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
  TString brickregex = "{" + TString(name) + TString(regex) + "}";

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
  } else {
    ::Fatal("CutBrick<TValueToFilter>* CutBrick<TValueToFilter>::constructBrick", "Wrong RE: %s, trying to construct na unknown basic cut brick", regex);
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

templateClassImp(CutBrick);

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
bool CutBrickLimit<TValueToFilter>::Filter(const TValueToFilter& value)
{
  if (value < mLimit) {
    this->mState = this->kACTIVE;
    return true;
  } else {
    this->mState = this->kPASSIVE;
    return false;
  }
}

templateClassImp(CutBrickLimit);

template class o2::analysis::PWGCF::CutBrickLimit<int>;
template class o2::analysis::PWGCF::CutBrickLimit<float>;

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
bool CutBrickThreshold<TValueToFilter>::Filter(const TValueToFilter& value)
{
  if (mThreshold <= value) {
    this->mState = this->kACTIVE;
    return true;
  } else {
    this->mState = this->kPASSIVE;
    return false;
  }
}

templateClassImp(CutBrickThreshold);

template class o2::analysis::PWGCF::CutBrickThreshold<int>;
template class o2::analysis::PWGCF::CutBrickThreshold<float>;

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
bool CutBrickRange<TValueToFilter>::Filter(const TValueToFilter& value)
{
  if ((mLow <= value) and (value < mHigh)) {
    this->mState = this->kACTIVE;
    return true;
  } else {
    this->mState = this->kPASSIVE;
    return false;
  }
}

templateClassImp(CutBrickRange);

template class o2::analysis::PWGCF::CutBrickRange<int>;
template class o2::analysis::PWGCF::CutBrickRange<float>;

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
bool CutBrickExtToRange<TValueToFilter>::Filter(const TValueToFilter& value)
{
  if ((value < mLow) or (mHigh <= value)) {
    this->mState = this->kACTIVE;
    return true;
  } else {
    this->mState = this->kPASSIVE;
    return false;
  }
}

templateClassImp(CutBrickExtToRange);

template class o2::analysis::PWGCF::CutBrickExtToRange<int>;
template class o2::analysis::PWGCF::CutBrickExtToRange<float>;

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
bool CutBrickSelectorMultipleRanges<TValueToFilter>::Filter(const TValueToFilter& value)
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
    return true;
  } else {
    this->mState = this->kPASSIVE;
    for (int i = 0; i < mActive.size(); ++i) {
      mActive[i] = false;
    }
    return false;
  }
}

templateClassImp(CutBrickSelectorMultipleRanges);

template class o2::analysis::PWGCF::CutBrickSelectorMultipleRanges<int>;
template class o2::analysis::PWGCF::CutBrickSelectorMultipleRanges<float>;

/// Default constructor
template <typename TValueToFilter>
CutWithVariations<TValueToFilter>::CutWithVariations() : TNamed(),
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
  : TNamed(name, cutstr),
    mAllowSeveralDefaults(severaldefaults)
{
  mDefaultBricks.SetOwner(true);
  mVariationBricks.SetOwner(true);
}

/// \brief Cut string constructor
/// \param cutstr The cuts string
template <typename TValueToFilter>
CutWithVariations<TValueToFilter>::CutWithVariations(const TString& cutstr)
  : TNamed(),
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
  std::regex cutregex("^(\\w+)\\{cwv\\{([\\w\\d.,{}]+)}}$", std::regex_constants::ECMAScript | std::regex_constants::icase);
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

/// Copy constructor
/// To be used only by the configurable framework
/// \param
template <typename TValueToFilter>
CutWithVariations<TValueToFilter>::CutWithVariations(const CutWithVariations<TValueToFilter>& cwv)
  : TNamed(cwv),
    mAllowSeveralDefaults(cwv.mAllowSeveralDefaults)
{
  mDefaultBricks.SetOwner(true);
  mVariationBricks.SetOwner(true);

  if (cwv.mDefaultBricks.GetEntries() != 0) {
    Error("CutWithVariations<TValueToFilter>::CutWithVariations", "Copying an already fully created instance");
  }
}

/// Assignment operator
/// To be used only by the configurable framework
/// \param
template <typename TValueToFilter>
CutWithVariations<TValueToFilter>& CutWithVariations<TValueToFilter>::operator=(const CutWithVariations<TValueToFilter>& cwv)
{
  mAllowSeveralDefaults = cwv.mAllowSeveralDefaults;
  mDefaultBricks.SetOwner(true);
  mVariationBricks.SetOwner(true);

  if (cwv.mDefaultBricks.GetEntries() != 0) {
    Error("CutWithVariations<TValueToFilter>::CutWithVariations", "Copying an already fully created instance");
  }
  return *this;
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
bool CutWithVariations<TValueToFilter>::Filter(const TValueToFilter& value)
{
  bool active = false;
  for (int i = 0; i < mDefaultBricks.GetEntries(); ++i) {
    active = active or ((CutBrick<TValueToFilter>*)mDefaultBricks.At(i))->Filter(value);
  }
  for (int i = 0; i < mVariationBricks.GetEntries(); ++i) {
    active = active or ((CutBrick<TValueToFilter>*)mVariationBricks.At(i))->Filter(value);
  }
  return active;
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

templateClassImp(o2::analysis::CutWithVariations);

template class o2::analysis::PWGCF::CutWithVariations<float>;
template class o2::analysis::PWGCF::CutWithVariations<int>;

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

templateClassImp(SpecialCutBrick);

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
void TrackSelectionBrick::constructFB36LHC2010()
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
  constructFB36LHC2010();
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
void TrackSelectionBrick::constructFB36LHC2011()
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
  constructFB36LHC2011();
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
