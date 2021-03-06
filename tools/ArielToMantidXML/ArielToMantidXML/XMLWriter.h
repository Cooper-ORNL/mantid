// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include <string>
#include <fstream>

class Component;

class XMLWriter
{
public:
  XMLWriter(const std::string& name, Component* const startPoint);
  virtual ~XMLWriter(void);

  void writeDetectors();

private:
  void writeDefaults();
  void writeSourceSample();

  std::ofstream m_outputFile;
  Component* const m_startPoint;
};
