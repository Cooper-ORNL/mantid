#ifndef MANTID_API_IPEAKFUNCTION_H_
#define MANTID_API_IPEAKFUNCTION_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/ParamFunction.h"
#include "MantidAPI/IFunction1D.h"

using namespace std;

namespace Mantid
{
namespace API
{
/** An interface to a peak function, which extend the interface of 
    IFunctionWithLocation by adding methods to set and get peak width.

    @author Roman Tolchenov, Tessella Support Services plc
    @date 16/10/2009

    Copyright &copy; 2009 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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

    File change history is stored at: <https://github.com/mantidproject/mantid>.
    Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
class MANTID_API_DLL IPowderDiffPeakFunction : virtual public API::ParamFunction,public virtual API::IFunction1D
{
public:

  /// Constructor and Destructor
  IPowderDiffPeakFunction();

  virtual ~IPowderDiffPeakFunction();

  /// Overwrite IFunction base class methods
  // virtual const std::string name() = 0;
  /// Category of function
  // virtual const std::string category(){ return "General"; }

  /// Get peak's centre
  double centre() const;
  /// Get peak's intensity
  double height() const;
  /// Get peakl's FWHM
  double fwhm()const;
  /// Set peak's height
  void setHeight(const double h);
  /// Set peak's radius
  void setPeakRadius(const int& r);

  //--------------- ThermalNeutron peak function special ---------------------------------------
  /// Set Miller Indicies
  void setMillerIndex(int h, int k, int l);

  /// Get Miller Index from this peak
  void getMillerIndex(int& h, int &k, int &l);

  /// Get peak parameters
  double getPeakParameter(std::string);

  /// Calculate peak parameters (alpha, beta, sigma2..)
  void calculateParameters(bool explicitoutput) const;

  /// Set up the flag to show whether (from client) cell parameter value changed
  void setUnitCellParameterValueChangeFlag(bool changed)
  {
    m_cellParamValueChanged = changed;
  }

  /// Override setting a new value to the i-th parameter
  void setParameter(size_t i, const double& value, bool explicitlySet=true);

  /// Override setting a new value to a parameter by name
  void setParameter(const std::string& name, const double& value, bool explicitlySe=true);

  void function1D(double* out, const double* xValues, const size_t nData)const;

protected:
  /// Local function for GSL minimizer
  virtual void functionLocal(double*, const double*, int&) const;

  /// Local function for calculation in Mantid
  virtual void functionLocal(vector<double> &out, const vector<double> &xValues) const;

  /// General implementation of the method for all peaks. Calculates derivatives only
  /// void functionDeriv1D(Jacobian* out, const double* xValues, const size_t nData) const;

  /// General implemenation of derivative
  void functionDerivLocal(Jacobian* out, const double* xValues, const size_t nData) const;

  /// Defines the area around the centre where the peak values are to be calculated (in FWHM).
  static int s_peakRadius;

  /// An indicator to re-calculate peak d-space position
  bool m_cellParamValueChanged;

  /// Centre of the peak
  mutable bool m_centre;
  /// Peak intensity
  bool m_intensity;
  /// Peak's FWHM
  mutable bool m_fwhm;
  /// Flag if any parameter value changed
  bool m_hasNewParameterValue;

};

typedef boost::shared_ptr<IPowderDiffPeakFunction> IPowderDiffPeakFunction_sptr;

} // namespace API
} // namespace Mantid

#endif /*MANTID_API_IPEAKFUNCTION_H_*/
