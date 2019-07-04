// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "CatalogSearcher.h"
#include "GUI/Runs/IRunsView.h"

#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/CatalogManager.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidQtWidgets/Common/AlgorithmDialog.h"
#include "MantidQtWidgets/Common/AlgorithmRunner.h"
#include "MantidQtWidgets/Common/InterfaceManager.h"

using namespace Mantid::API;

namespace MantidQt {
namespace CustomInterfaces {

namespace { // unnamed
void removeResultsWithoutFilenameExtension(ITableWorkspace_sptr results) {
  std::set<size_t> toRemove;
  for (size_t i = 0; i < results->rowCount(); ++i) {
    std::string &run = results->String(i, 0);

    // Too short to be more than ".raw or .nxs"
    if (run.size() < 5) {
      toRemove.insert(i);
    }
  }

  // Sets are sorted so if we go from back to front we won't trip over ourselves
  for (auto row = toRemove.rbegin(); row != toRemove.rend(); ++row)
    results->removeRow(*row);
}
} // unnamed namespace

CatalogSearcher::CatalogSearcher(IRunsView *view)
    : m_view(view), m_notifyee(nullptr), m_searchText(), m_instrument(),
      m_searchType(SearchType::NONE), m_searchInProgress(false) {
  m_view->subscribeSearch(this);
}

void CatalogSearcher::subscribe(SearcherSubscriber *notifyee) {
  m_notifyee = notifyee;
}

ITableWorkspace_sptr CatalogSearcher::search(const std::string &text,
                                             const std::string &instrument,
                                             ISearcher::SearchType searchType) {
  m_searchText = text;
  m_instrument = instrument;
  m_searchType = searchType;
  auto algSearch = createSearchAlgorithm(text);
  algSearch->execute();
  ITableWorkspace_sptr results = algSearch->getProperty("OutputWorkspace");
  // Now, tidy up the data
  removeResultsWithoutFilenameExtension(results);
  return results;
}

void CatalogSearcher::searchAsync() {
  auto algSearch = createSearchAlgorithm(m_searchText);
  auto algRunner = m_view->getAlgorithmRunner();
  algRunner->startAlgorithm(algSearch);
  m_searchInProgress = true;
}

bool CatalogSearcher::startSearchAsync(const std::string &text,
                                       const std::string &instrument,
                                       SearchType searchType) {
  m_searchText = text;
  m_instrument = instrument;
  m_searchType = searchType;

  // Already logged in
  if (hasActiveCatalogSession()) {
    searchAsync();
  } else {
    // Else attempt to login, once login is complete finishHandle will be
    // called.
    logInToCatalog();
  }

  return true;
}

void CatalogSearcher::finishHandle(const Mantid::API::IAlgorithm *alg) {
  stopObserving(alg);
  if (!hasActiveCatalogSession()) {
    m_notifyee->notifySearchFailed();
  } else {
    searchAsync();
  }
}

void CatalogSearcher::errorHandle(const Mantid::API::IAlgorithm *alg,
                                  const std::string &what) {
  UNUSED_ARG(what)
  stopObserving(alg);
  if (!hasActiveCatalogSession()) {
    m_notifyee->notifySearchFailed();
  }
}

void CatalogSearcher::notifySearchComplete() {
  m_searchInProgress = false;
  auto algRunner = m_view->getAlgorithmRunner();
  IAlgorithm_sptr searchAlg = algRunner->getAlgorithm();

  if (searchAlg->isExecuted()) {
    ITableWorkspace_sptr table = searchAlg->getProperty("OutputWorkspace");
    results().addDataFromTable(table, m_instrument);
  }

  m_notifyee->notifySearchComplete();
  if (m_searchType == SearchType::AUTO) {
    m_notifyee->notifyAutoProcessSearchComplete();
  }
}

bool CatalogSearcher::searchInProgress() const { return m_searchInProgress; }

SearchResult const &CatalogSearcher::getSearchResult(int index) const {
  return results().getRowData(index);
}

void CatalogSearcher::setSearchResultError(int index,
                                           const std::string &errorMessage) {
  results().setError(index, errorMessage);
}

void CatalogSearcher::reset() {
  m_searchText.clear();
  m_instrument.clear();
  m_searchType = SearchType::NONE;
  results().clear();
}

bool CatalogSearcher::searchSettingsChanged(const std::string &text,
                                            const std::string &instrument,
                                            SearchType searchType) const {
  return m_searchText != text || m_instrument != instrument ||
         searchType != m_searchType;
}

bool hasActiveCatalogSession() {
  auto sessions = CatalogManager::Instance().getActiveSessions();
  return !sessions.empty();
}

void CatalogSearcher::execLoginDialog(const IAlgorithm_sptr &alg) {
  API::InterfaceManager interfaceMgr;
  auto dlg = dynamic_cast<MantidQt::API::AlgorithmDialog *>(
      interfaceMgr.createDialog(alg));
  QObject::connect(dlg, SIGNAL(closeEventCalled()), this, SLOT(dialogClosed()));
  dlg->setModal(true);
  dlg->show();
  dlg->raise();
  dlg->activateWindow();
}

void CatalogSearcher::dialogClosed() {
  if (!hasActiveCatalogSession()) {
    m_notifyee->notifySearchFailed();
  }
}

/** Log in to the catalog
 * @returns : true if login succeeded
 */
void CatalogSearcher::logInToCatalog() {
  IAlgorithm_sptr alg = AlgorithmManager::Instance().create("CatalogLogin");
  alg->initialize();
  alg->setProperty("KeepSessionAlive", true);
  this->observeFinish(alg);
  this->observeError(alg);
  execLoginDialog(alg);
}

std::string CatalogSearcher::activeSessionId() const {
  auto sessions = CatalogManager::Instance().getActiveSessions();
  if (sessions.empty())
    throw std::runtime_error("You are not logged into any catalogs.");

  return sessions.front()->getSessionId();
}

IAlgorithm_sptr
CatalogSearcher::createSearchAlgorithm(const std::string &text) {
  auto sessionId = activeSessionId();

  auto algSearch = AlgorithmManager::Instance().create("CatalogGetDataFiles");
  algSearch->initialize();
  algSearch->setChild(true);
  algSearch->setLogging(false);
  algSearch->setProperty("Session", sessionId);
  algSearch->setProperty("InvestigationId", text);
  algSearch->setProperty("OutputWorkspace", "_ReflSearchResults");

  return algSearch;
}

ISearchModel &CatalogSearcher::results() const {
  return m_view->mutableSearchResults();
}
} // namespace CustomInterfaces
} // namespace MantidQt
