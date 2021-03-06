// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2020 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidDataHandling/LoadMuonNexusV2.h"
#include "MantidAPI/FileProperty.h"
#include "MantidAPI/RegisterFileLoader.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidDataHandling/LoadISISNexus2.h"
#include "MantidDataHandling/LoadMuonNexusV2Helper.h"
#include "MantidDataHandling/SinglePeriodLoadMuonStrategy.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/BoundedValidator.h"
#include "MantidKernel/ListValidator.h"

#include <vector>

namespace Mantid {
namespace DataHandling {

DECLARE_NEXUS_HDF5_FILELOADER_ALGORITHM(LoadMuonNexusV2)

using namespace Kernel;
using namespace API;
using namespace NeXus;
using namespace HistogramData;
using std::size_t;
using namespace DataObjects;

namespace NeXusEntry {
const std::string RAWDATA{"/raw_data_1"};
const std::string DEFINITION{"/raw_data_1/definition"};
const std::string PERIOD{"/periods"};
const std::string BEAMLINE{"/raw_data_1/beamline"};
} // namespace NeXusEntry

/// Empty default constructor
LoadMuonNexusV2::LoadMuonNexusV2()
    : m_filename(), m_entrynumber(0), m_isFileMultiPeriod(false),
      m_multiPeriodsLoaded(false) {}

/**
 * Return the confidence criteria for this algorithm can load the file
 * @param descriptor A descriptor for the file
 * @returns An integer specifying the confidence level. 0 indicates it will not
 * be used
 */
int LoadMuonNexusV2::confidence(NexusHDF5Descriptor &descriptor) const {
  // Without this entry we cannot use LoadISISNexus
  if (!descriptor.isEntry(NeXusEntry::RAWDATA, "NXentry")) {
    return 0;
  }

  // Check if beamline entry exists beneath raw_data_1 - /raw_data_1/beamline
  // Necessary to differentiate between ISIS and PSI nexus files.
  if (!descriptor.isEntry(NeXusEntry::BEAMLINE))
    return 0;

  // Check if Muon source in definition entry
  if (!descriptor.isEntry(NeXusEntry::DEFINITION))
    return 0;

  ::NeXus::File file(descriptor.getFilename());
  file.openPath(NeXusEntry::DEFINITION);
  std::string def = file.getStrData();
  if (def == "muonTD" || def == "pulsedTD") {
    return 82; // have to return 82 to "beat" the LoadMuonNexus2 algorithm,
               // which returns 81 for this file as well
  } else {
    return 0;
  }
}
/// Initialization method.
void LoadMuonNexusV2::init() {

  std::vector<std::string> extensions{".nxs", ".nxs_v2"};
  declareProperty(std::make_unique<FileProperty>(
                      "Filename", "", FileProperty::Load, extensions),
                  "The name of the Nexus file to load");

  declareProperty(
      std::make_unique<WorkspaceProperty<Workspace>>("OutputWorkspace", "",
                                                     Direction::Output),
      "The name of the workspace to be created as the output of the\n"
      "algorithm. For multiperiod files, one workspace will be\n"
      "generated for each period");

  auto mustBePositiveSpectra = std::make_shared<BoundedValidator<specnum_t>>();
  mustBePositiveSpectra->setLower(0);
  declareProperty("SpectrumMin", static_cast<specnum_t>(0),
                  mustBePositiveSpectra);
  declareProperty("SpectrumMax", static_cast<specnum_t>(EMPTY_INT()),
                  mustBePositiveSpectra);
  declareProperty(std::make_unique<ArrayProperty<specnum_t>>("SpectrumList"));
  auto mustBePositive = std::make_shared<BoundedValidator<int64_t>>();
  mustBePositive->setLower(0);
  declareProperty("EntryNumber", static_cast<int64_t>(0), mustBePositive,
                  "0 indicates that every entry is loaded, into a separate "
                  "workspace within a group. "
                  "A positive number identifies one entry to be loaded, into "
                  "one workspace");

  std::vector<std::string> FieldOptions{"Transverse", "Longitudinal"};
  declareProperty("MainFieldDirection", "Transverse",
                  std::make_shared<StringListValidator>(FieldOptions),
                  "Output the main field direction if specified in Nexus file "
                  "(default longitudinal).",
                  Direction::Output);

  declareProperty("TimeZero", 0.0,
                  "Time zero in units of micro-seconds (default to 0.0)",
                  Direction::Output);
  declareProperty("FirstGoodData", 0.0,
                  "First good data in units of micro-seconds (default to 0.0)",
                  Direction::Output);

  declareProperty(
      std::make_unique<WorkspaceProperty<Workspace>>(
          "DeadTimeTable", "", Direction::Output, PropertyMode::Optional),
      "Table or a group of tables containing detector dead times.");

  declareProperty(std::make_unique<WorkspaceProperty<Workspace>>(
                      "DetectorGroupingTable", "", Direction::Output,
                      PropertyMode::Optional),
                  "Table or a group of tables with information about the "
                  "detector grouping.");
}
void LoadMuonNexusV2::execLoader() {
  // prepare nexus entry
  m_entrynumber = getProperty("EntryNumber");
  m_filename = getPropertyValue("Filename");
  NXRoot root(m_filename);
  NXEntry entry = root.openEntry(NeXusEntry::RAWDATA);
  isEntryMultiPeriod(entry);
  if (m_isFileMultiPeriod) {
    throw std::invalid_argument(
        "Multiperiod nexus files not yet supported by LoadMuonNexusV2");
  }

  // Execute child algorithm LoadISISNexus2 and load Muon specific properties
  auto outWS = runLoadISISNexus();
  loadMuonProperties(entry);

  // Check if single or multi period file and create appropriate loading
  // strategy
  if (m_multiPeriodsLoaded) {
    // Currently not implemented
    throw std::invalid_argument(
        "Multiperiod nexus files not yet supported by LoadMuonNexusV2");
  } else {
    // we just have a single workspace
    Workspace2D_sptr workspace2D =
        std::dynamic_pointer_cast<Workspace2D>(outWS);
    m_loadMuonStrategy = std::make_unique<SinglePeriodLoadMuonStrategy>(
        g_log, m_filename, entry, workspace2D, static_cast<int>(m_entrynumber),
        m_isFileMultiPeriod);
  }
  m_loadMuonStrategy->loadMuonLogData();
  m_loadMuonStrategy->loadGoodFrames();
  m_loadMuonStrategy->applyTimeZeroCorrection();
  // Grouping info should be returned if user has set the property
  if (!getPropertyValue("DetectorGroupingTable").empty()) {
    auto loadedGrouping = m_loadMuonStrategy->loadDetectorGrouping();
    setProperty("DetectorGroupingTable", loadedGrouping);
  };
  // Deadtime table should be returned if user has set the property
  auto deadtimeTable = m_loadMuonStrategy->loadDeadTimeTable();
  if (!getPropertyValue("DeadTimeTable").empty()) {
    setProperty("DeadTimeTable", deadtimeTable);
  }
}

/**
 * Determines whether the file is multi period
 * If multi period the function determines whether multi periods are loaded
 */
void LoadMuonNexusV2::isEntryMultiPeriod(const NXEntry &entry) {
  NXClass periodClass = entry.openNXGroup(NeXusEntry::PERIOD);
  int numberOfPeriods = periodClass.getInt("number");
  if (numberOfPeriods > 1) {
    m_isFileMultiPeriod = true;
    if (m_entrynumber == 0) {
      m_multiPeriodsLoaded = true;
    }
  } else {
    m_isFileMultiPeriod = false;
    m_multiPeriodsLoaded = false;
  }
}
/**
 * Runs the child algorithm LoadISISNexus, which loads data into an output
 * workspace
 * @returns :: Workspace loaded by runLoadISISNexus
 */
Workspace_sptr LoadMuonNexusV2::runLoadISISNexus() {
  // Here we explicit set the number of OpenMP threads, as by default
  // LoadISISNexus spawns up a large number of threads,
  // which is unnecessary for the size (~100 spectra) of workspaces seen here.
  // Through profiling it was found that a single threaded call to LoadISISNexus
  // was quicker due to the overhead of setting up the threads, which outweighs
  // the cost of the resulting operations.
  // To prevent the omp_set_num_threads call having side effects, we use a RAII
  // pattern to restore the default behavior once runLoadISISNexus is complete.
  struct ScopedNumThreadsSetter {
    ScopedNumThreadsSetter(const int numThreads) {
      (void)numThreads; // Treat compiler warning in OSX
      globalNumberOfThreads = PARALLEL_GET_MAX_THREADS;
      PARALLEL_SET_NUM_THREADS(numThreads);
    }
    ~ScopedNumThreadsSetter() {
      PARALLEL_SET_NUM_THREADS(globalNumberOfThreads);
    }
    int globalNumberOfThreads;
  };
  ScopedNumThreadsSetter restoreDefaultThreadsOnExit(1);
  IAlgorithm_sptr childAlg =
      createChildAlgorithm("LoadISISNexus", 0, 1, true, 2);
  declareProperty("LoadMonitors", "Exclude"); // we need to set this property
  auto ISISLoader = std::dynamic_pointer_cast<API::Algorithm>(childAlg);
  ISISLoader->copyPropertiesFrom(*this);
  ISISLoader->execute();
  this->copyPropertiesFrom(*ISISLoader);
  Workspace_sptr outWS = getProperty("OutputWorkspace");
  return outWS;
}
/**
 * Loads Muon specific data from the nexus entry
 * and sets the appropriate output properties
 */
void LoadMuonNexusV2::loadMuonProperties(const NXEntry &entry) {

  std::string mainFieldDirection =
      LoadMuonNexusV2Helper::loadMainFieldDirectionFromNexus(entry);
  setProperty("MainFieldDirection", mainFieldDirection);

  double timeZero = LoadMuonNexusV2Helper::loadTimeZeroFromNexusFile(entry);
  setProperty("timeZero", timeZero);

  auto firstGoodData = LoadMuonNexusV2Helper::loadFirstGoodDataFromNexus(entry);
  setProperty("FirstGoodData", firstGoodData);
}
} // namespace DataHandling
} // namespace Mantid
