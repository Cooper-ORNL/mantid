// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2007 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTID_KERNEL_TIMER_H_
#define MANTID_KERNEL_TIMER_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidKernel/DllConfig.h"
#include <chrono>
#include <iosfwd>
#include <string>

namespace Mantid {
namespace Kernel {

class MANTID_KERNEL_DLL NewTimer {
public:
  NewTimer();
  double elapsed(bool reset = true);
  int64_t elapsedNanoSec(bool reset = true);
// Commented functions are valid only in Linux due to existence of user space and system space
//  int64_t elapsedCPUNanoSec(bool reset = true);
//  double fraction();
  void reset();
  int64_t getStart() {return m_start;}
private:
  int64_t m_start; // total nanoseconds
//  int64)t m_CPUstart; // CPU nanoseconds
};

/** A simple class that provides a wall-clock (not processor time) timer.

    @author Russell Taylor, Tessella plc
    @date 29/04/2010
 */
class MANTID_KERNEL_DLL Timer {
public:
  Timer();
  virtual ~Timer() = default;

  float elapsed(bool reset = true);
  float elapsed_no_reset() const;
  std::string str() const;
  void reset();

private:
  std::chrono::time_point<std::chrono::high_resolution_clock>
      m_start; ///< The starting time
};

MANTID_KERNEL_DLL std::ostream &operator<<(std::ostream &, const Timer &);

} // namespace Kernel
} // namespace Mantid

#endif /* TIMER_H_ */
