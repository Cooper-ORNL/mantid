#ifndef MANTID_ALGORITHMS_FINDPEAKS_H_
#define MANTID_ALGORITHMS_FINDPEAKS_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/Algorithm.h"
#include "MantidDataObjects/TableWorkspace.h"

namespace Mantid
{
namespace Algorithms
{
/** This algorithm searches for peaks in a dataset.
    The method used is detailed in: M.A.Mariscotti, NIM 50 (1967) 309.

    Required Properties:
    <UL>
    <LI> InputWorkspace - The name of the Workspace to search for peaks. </LI>
    <LI> PeaksList      - The name of the TableWorkspace in which to store the list of peaks found. </LI>
    </UL>

    Optional Properties:
    <UL>
    <LI> fwhm - The number of points covered on average by the fwhm of a peak (default 7) </LI>
    <LI> Tolerance - Sets the strictness desired in meeting the conditions on peak candidates (default 4, Mariscotti recommended 2) </LI>
    <LI> WorkspaceIndex - The spectrum to search for peaks. Will search all spectra if absent. </LI>
    </UL>
    
    @author Russell Taylor, Tessella Support Services plc
    @date 25/11/2008

    Copyright &copy; 2008-9 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>
    Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
class DLLExport FindPeaks : public API::Algorithm
{
public:
  /// Constructor
  FindPeaks();
  /// Virtual destructor
  virtual ~FindPeaks() {if(m_progress) delete m_progress; m_progress=NULL;}
  /// Algorithm's name
  virtual const std::string name() const { return "FindPeaks"; }
  /// Algorithm's version
  virtual int version() const { return (1); }
  /// Algorithm's category for identification
  virtual const std::string category() const { return "Optimization\\PeakFinding"; }

private:
  /// Sets documentation strings for this algorithm
  virtual void initDocs();
  void init();
  void exec();

  API::MatrixWorkspace_sptr calculateSecondDifference(const API::MatrixWorkspace_const_sptr &input);
  void smoothData(API::MatrixWorkspace_sptr &WS, const int &w);
  void calculateStandardDeviation(const API::MatrixWorkspace_const_sptr &input, const API::MatrixWorkspace_sptr &smoothed, const int &w);
  long long computePhi(const int& w) const;

  void fitPeak(const API::MatrixWorkspace_sptr &input, const int spectrum, const int i0, const int i2, const int i4, std::string backgroundtype);
  void fitPeak(const API::MatrixWorkspace_sptr &input, const int spectrum, const double center_guess, const int FWHM_guess, std::string backgroundtype);
  void findPeaksUsingMariscotti(std::string backgroundtype);
  void findPeaksGivenStartingPoints(std::vector<double> peakCenters, std::string backgroundtype);

  // void fitPeakHighBackground(MantidVec& X, MantidVec& Y, MantidVec& E, int i0, int i2, int i4, int i_min, int i_max,
  //    double in_bg0, double in_bg1, double in_bg2, std::string backgroundtype);

  void fitPeakHighBackground(const API::MatrixWorkspace_sptr &input, const int spectrum, const int& i0, const int& i2, const int& i4,
      const unsigned int& i_min, const unsigned int& i_max,
      const double& in_bg0, const double& in_bg1, const double& in_bg2, std::string& backgroundtype);

  void fitPeakOneStep(const API::MatrixWorkspace_sptr &input, const int spectrum, const int& i0, const int& i2, const int& i4,
      const double& in_bg0, const double& in_bg1, const double& in_bg2, std::string& backgroundtype);

  bool checkFitResultParameterNames(std::vector<std::string> paramnames, std::string backgroundtype, std::string &errormessage);

  /// The number of smoothing iterations. Set to 5, the optimum value according to Mariscotti.
  static const int g_z = 5;
  
  /// Storage of the peak data
  API::ITableWorkspace_sptr m_peaks;
  /// Progress reporting
  API::Progress* m_progress;

  //Properties saved in the algo.
  API::MatrixWorkspace_sptr inputWS; ///<workspace to check for peaks
  int fwhm; ///<holder for the requested peak FWHM
  int index; ///<list of workspace indicies to check
  bool singleSpectrum; ///<flag for if only a single spectrum is present
  bool mHighBackground; ///<flag for find relatively weak peak in high background

  unsigned int minGuessedPeakWidth;
  unsigned int maxGuessedPeakWidth;
  unsigned int stepGuessedPeakWidth;

};

} // namespace Algorithms
} // namespace Mantid

#endif /*MANTID_ALGORITHMS_FINDPEAKS_H_*/
