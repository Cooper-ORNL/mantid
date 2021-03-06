// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidMatrixModel.h"

#include <QPointer>
#include <QString>
#include <QTableView>
/**
 * Holds the information for a new tab.
 */
struct MantidMatrixTabExtension {
  MantidMatrixTabExtension(const QString &&label, QTableView *tableView,
                           MantidMatrixModel *model,
                           MantidMatrixModel::Type type)
      : label(label), tableView(tableView), model(model), type(type) {}
  MantidMatrixTabExtension()
      : label(""), tableView(), model(nullptr),
        type(MantidMatrixModel::Type::DX) {}
  QString label;
  std::unique_ptr<QTableView> tableView;
  QPointer<MantidMatrixModel> model;
  MantidMatrixModel::Type type;
};

using MantidMatrixTabExtensionMap =
    std::map<MantidMatrixModel::Type, MantidMatrixTabExtension>;