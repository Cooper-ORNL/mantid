// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2011 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "Common/DllConfig.h"
#include "GUI/Runs/IRunsView.h"
#include "ISearcher.h"
#include "MantidAPI/AlgorithmObserver.h"
#include "MantidAPI/IAlgorithm_fwd.h"
#include "MantidAPI/ITableWorkspace_fwd.h"

namespace MantidQt {
namespace CustomInterfaces {
namespace ISISReflectometry {

/** @class QtCatalogSearcher

QtCatalogSearcher implements ISearcher to provide ICAT search
functionality.
*/
class MANTIDQT_ISISREFLECTOMETRY_DLL QtCatalogSearcher
    : public QObject,
      public ISearcher,
      public RunsViewSearchSubscriber,
      public Mantid::API::AlgorithmObserver {
  Q_OBJECT
public:
  explicit QtCatalogSearcher(IRunsView *view);
  virtual ~QtCatalogSearcher() = default;

  // ISearcher overrides
  void subscribe(SearcherSubscriber *notifyee) override;
  SearchResults search(const std::string &text, const std::string &instrument,
                       const std::string &cycle,
                       SearchType searchType) override;
  bool startSearchAsync(const std::string &text, const std::string &instrument,
                        const std::string &cycle,
                        SearchType searchType) override;
  bool searchInProgress() const override;
  SearchResult const &getSearchResult(int index) const override;
  void reset() override;
  bool searchSettingsChanged(const std::string &text,
                             const std::string &instrument,
                             const std::string &cycle,
                             SearchType searchType) const override;

  // RunsViewSearchSubscriber overrides
  void notifySearchComplete() override;

protected:
  void finishHandle(const Mantid::API::IAlgorithm *alg) override;
  void errorHandle(const Mantid::API::IAlgorithm *alg,
                   const std::string &what) override;
  virtual bool hasActiveCatalogSession() const;
  virtual Mantid::API::IAlgorithm_sptr createSearchAlgorithm();
  virtual Mantid::API::ITableWorkspace_sptr
  getSearchAlgorithmResultsTable(Mantid::API::IAlgorithm_sptr searchAlg);
  bool requiresICat() const;
  virtual void logInToCatalog();

private slots:
  void dialogClosed();

private:
  IRunsView *m_view;
  SearcherSubscriber *m_notifyee;
  std::string m_searchText;
  std::string m_instrument;
  std::string m_cycle;
  SearchType m_searchType;
  bool m_searchInProgress;

  void execLoginDialog(const Mantid::API::IAlgorithm_sptr &alg);
  std::string activeSessionId() const;
  ISearchModel &results() const;
  void searchAsync();
  SearchResults convertResultsTableToSearchResults(
      Mantid::API::ITableWorkspace_sptr resultsTable);
  SearchResults convertICatResultsTableToSearchResults(
      Mantid::API::ITableWorkspace_sptr tableWorkspace);
  SearchResults convertJournalResultsTableToSearchResults(
      Mantid::API::ITableWorkspace_sptr tableWorkspace);
};
} // namespace ISISReflectometry
} // namespace CustomInterfaces
} // namespace MantidQt
