// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidKernel/Chainable.h"
#include "MantidMatrixTabExtension.h"
#include "boost/shared_ptr.hpp"

class IMantidMatrixExtensionHandler
    : public Mantid::Kernel::Chainable<IMantidMatrixExtensionHandler> {
public:
  ~IMantidMatrixExtensionHandler() override {}
  virtual void setNumberFormat(MantidMatrixTabExtension &extension,
                               const QChar &format, int precision) = 0;
  virtual void recordFormat(MantidMatrixTabExtension &extension,
                            const QChar &format, int precision) = 0;
  virtual QChar getFormat(MantidMatrixTabExtension &extension) = 0;
  virtual int getPrecision(MantidMatrixTabExtension &extension) = 0;
  virtual void setColumnWidth(MantidMatrixTabExtension &extension, int width,
                              int numberOfColumns) = 0;
  virtual int getColumnWidth(MantidMatrixTabExtension &extension) = 0;
  virtual QTableView *getTableView(MantidMatrixTabExtension &extension) = 0;
  virtual void setColumnWidthPreference(MantidMatrixTabExtension &extension,
                                        int width) = 0;
  virtual int getColumnWidthPreference(MantidMatrixTabExtension &extension) = 0;
};