#include "MantidKernel/RebinParamsValidator.h"
#include <boost/python/class.hpp>
#include <boost/python/default_call_policies.hpp>
#include <boost/python/make_constructor.hpp>

using Mantid::Kernel::IValidator;
using Mantid::Kernel::RebinParamsValidator;
using namespace boost::python;

namespace {
RebinParamsValidator *createRebinParamsValidator(bool allowEmpty,
                                                 bool allowRange) {
  return new RebinParamsValidator(allowEmpty, allowRange);
}
} // namespace

void export_RebinParamsValidator() {
  class_<RebinParamsValidator, bases<IValidator>, boost::noncopyable>(
      "RebinParamsValidator")
      .def("__init__",
           make_constructor(
               &createRebinParamsValidator, default_call_policies(),
               (arg("AllowEmpty") = false, arg("AllowRange") = false)));
}
