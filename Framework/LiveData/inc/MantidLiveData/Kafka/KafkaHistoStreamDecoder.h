#ifndef MANTID_LIVEDATA_ISISKAFKAHISTOSTREAMDECODER_H_
#define MANTID_LIVEDATA_ISISKAFKAHISTOSTREAMDECODER_H_

//#include "MantidAPI/SpectraDetectorTypes.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidLiveData/Kafka/IKafkaBroker.h"
#include "MantidLiveData/Kafka/IKafkaStreamSubscriber.h"

#include <atomic>
//#include <condition_variable>
//#include <mutex>
#include <thread>

namespace Mantid {
namespace LiveData {

/**
  High-level interface to ISIS Kafka histo system.

  A call to startCapture() starts the process of capturing the stream on a
  separate thread.

  Copyright &copy; 2017 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
  National Laboratory & European Spallation Source

  This file is part of Mantid.

  Mantid is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Mantid is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  File change history is stored at: <https://github.com/mantidproject/mantid>
  Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
class DLLExport KafkaHistoStreamDecoder {
public:
  KafkaHistoStreamDecoder(std::shared_ptr<IKafkaBroker> broker,
                          const std::string &histoTopic,
                          const std::string &instrumentName);
  ~KafkaHistoStreamDecoder();
  KafkaHistoStreamDecoder(const KafkaHistoStreamDecoder &) = delete;
  KafkaHistoStreamDecoder &operator=(const KafkaHistoStreamDecoder &) = delete;

public:
  ///@name Start/stop
  ///@{
  void startCapture(bool startNow = true);
  void stopCapture();
  ///@}

  ///@name Querying
  ///@{
  bool isCapturing() const { return m_capturing; }
  bool hasData() const;
  int runNumber() const { return 1; }
  bool hasReachedEndOfRun() { return !m_capturing; }
  ///@}

  ///@name Modifying
  ///@{
  API::Workspace_sptr extractData();
  ///@}

private:
  void captureImpl();
  void captureImplExcept();

  /*
  DataObjects::Workspace2D_sptr createBufferWorkspace(const size_t nspectra,
                                                         const int32_t *spec,
                                                         const int32_t *udet,
                                                         const uint32_t length);
  //*/
  DataObjects::Workspace2D_sptr copyBufferWorkspace(
      const DataObjects::Workspace2D_sptr &workspace);
  DataObjects::Workspace2D_sptr createBufferWorkspace();
//  void loadInstrument(const std::string &name,
//                      DataObjects::Workspace2D_sptr workspace);

  API::Workspace_sptr extractDataImpl();

  /// Broker to use to subscribe to topics
  std::shared_ptr<IKafkaBroker> m_broker;
  /// Topic name
  const std::string m_histoTopic;
  /// Instrument name
  const std::string m_instrumentName;
  /// Subscriber for the histo stream
  std::unique_ptr<IKafkaStreamSubscriber> m_histoStream;
  /// Local buffer workspace
  DataObjects::Workspace2D_sptr m_workspace;
  /// Detector ID to Workspace Index mapping
  std::vector<size_t> m_indexMap;
  detid_t m_indexOffset;

  /// Associated thread running the capture process
  std::thread m_thread;
  /// Flag indicating if user interruption has been requested
  std::atomic<bool> m_interrupt;

//  /// Mapping of spectrum number to workspace index.
//  spec2index_map m_specToIdx; // ?

  /// Mutex protecting histo buffers
  mutable std::mutex m_workspace_mutex;


//  /// Mutex protecting the wait flag
//  mutable std::mutex m_waitMutex;
//  /// Mutex protecting the runStatusSeen flag
//  mutable std::mutex m_runStatusMutex;
  /// Flag indicating that the decoder is capturing
  std::atomic<bool> m_capturing;
  /// Exception object indicating there was an error
  boost::shared_ptr<std::runtime_error> m_exception;

//  /// For notifying other threads of changes to conditions (the following bools)
//  std::condition_variable m_cv;
//  std::condition_variable m_cvRunStatus;
//  /// Indicate that decoder has reached the last message in a run
//  std::atomic<bool> m_endRun;
//  /// Indicate that LoadLiveData is waiting for access to the buffer workspace
//  std::atomic<bool> m_extractWaiting;
//  /// Indicate that MonitorLiveData has seen the runStatus since it was set to
//  /// EndRun
//  bool m_runStatusSeen;
//  std::atomic<bool> m_extractedEndRunData;
};

} // namespace LiveData
} // namespace Mantid

#endif /* MANTID_LIVEDATA_ISISKAFKAHISTOSTREAMDECODER_H_ */

