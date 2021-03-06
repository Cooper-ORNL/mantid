// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidKernel/PropertyManager.h"

/**
 * Helper class to use IPropertyManager methods in the DataObjects tests
 */
class PropertyManagerHelper : public Mantid::Kernel::PropertyManager {
public:
  PropertyManagerHelper() : PropertyManager() {}

  using IPropertyManager::getValue;
  using IPropertyManager::TypedValue;
  using PropertyManager::declareProperty;
  using PropertyManager::setProperty;
};
