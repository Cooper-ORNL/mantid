#ifndef MANTID_ALGORITHMS_PDDETERMINECHARACTERIZATIONSTEST_H_
#define MANTID_ALGORITHMS_PDDETERMINECHARACTERIZATIONSTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidAlgorithms/PDDetermineCharacterizations.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidAPI/PropertyManagerDataService.h"
#include "MantidAPI/TableRow.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/PropertyWithValue.h"

using Mantid::Algorithms::PDDetermineCharacterizations;
using namespace Mantid::API;
using namespace Mantid::Kernel;

typedef boost::shared_ptr<Mantid::Kernel::PropertyManager> PropertyManager_sptr;

class PDDetermineCharacterizationsTest : public CxxTest::TestSuite {
private:
  std::string m_propertyManagerName = "__pd_reduction_properties";
  std::string m_logWSName;

public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static PDDetermineCharacterizationsTest *createSuite() {
    return new PDDetermineCharacterizationsTest();
  }
  static void destroySuite(PDDetermineCharacterizationsTest *suite) {
    delete suite;
  }

  void createLogWksp() {
    m_logWSName = "_det_char_log";

    {
      auto alg =
          FrameworkManager::Instance().createAlgorithm("CreateWorkspace");
      alg->setPropertyValue("DataX",
                            "-1.0,-0.8,-0.6,-0.4,-0.2,0.0,0.2,0.4,0.6,0.8,1.0");
      alg->setPropertyValue("DataY",
                            "-1.0,-0.8,-0.6,-0.4,-0.2,0.0,0.2,0.4,0.6,0.8");
      alg->setPropertyValue("OutputWorkspace", m_logWSName);
      TS_ASSERT(alg->execute());
    }
    {
      auto alg = FrameworkManager::Instance().createAlgorithm("AddSampleLog");
      alg->setPropertyValue("LogName", "frequency");
      alg->setPropertyValue("LogText", "60.");
      alg->setPropertyValue("LogUnit", "Hz");
      alg->setPropertyValue("LogType", "Number");
      alg->setPropertyValue("Workspace", m_logWSName);
      TS_ASSERT(alg->execute());
    }
    {
      auto alg = FrameworkManager::Instance().createAlgorithm("AddSampleLog");
      alg->setPropertyValue("LogName", "LambdaRequest");
      alg->setPropertyValue("LogText", "0.533");
      alg->setPropertyValue("LogUnit", "Angstrom");
      alg->setPropertyValue("LogType", "Number");
      alg->setPropertyValue("Workspace", m_logWSName);
      TS_ASSERT(alg->execute());
    }
  }

  void addRow(ITableWorkspace_sptr wksp, double freq, double wl, int bank,
              int van, int can, int empty, std::string dmin, std::string dmax,
              double tofmin, double tofmax) {
    Mantid::API::TableRow row = wksp->appendRow();
    row << freq;
    row << wl;
    row << bank;
    row << van;
    row << can;
    row << empty;
    row << dmin;
    row << dmax;
    row << tofmin;
    row << tofmax;
  }

  ITableWorkspace_sptr createTableWksp(bool full) {
    ITableWorkspace_sptr wksp = WorkspaceFactory::Instance().createTable();
    wksp->addColumn("double", "frequency");
    wksp->addColumn("double", "wavelength");
    wksp->addColumn("int", "bank");
    wksp->addColumn("int", "vanadium");
    wksp->addColumn("int", "container");
    wksp->addColumn("int", "empty");
    wksp->addColumn("str", "d_min"); // b/c it is an array for NOMAD
    wksp->addColumn("str", "d_max"); // b/c it is an array for NOMAD
    wksp->addColumn("double", "tof_min");
    wksp->addColumn("double", "tof_max");

    if (full) {
      addRow(wksp, 60., 0.533, 1, 17702, 17711, 0, "0.05", "2.20", 0000.00,
             16666.67);
      addRow(wksp, 60., 1.333, 3, 17703, 17712, 0, "0.43", "5.40", 12500.00,
             29166.67);
      addRow(wksp, 60., 2.665, 4, 17704, 17713, 0, "1.15", "9.20", 33333.33,
             50000.00);
      addRow(wksp, 60., 4.797, 5, 17705, 17714, 0, "2.00", "15.35", 66666.67,
             83333.67);
    }

    return wksp;
  }

  PropertyManager_sptr
  createExpectedInfo(const double freq, const double wl, const int bank,
                     const int van, const int can, const int empty,
                     const std::string &dmin, const std::string &dmax,
                     const double tofmin, const double tofmax) {

    PropertyManager_sptr expectedInfo = boost::make_shared<PropertyManager>();
    expectedInfo->declareProperty(
        new PropertyWithValue<double>("frequency", freq));
    expectedInfo->declareProperty(
        new PropertyWithValue<double>("wavelength", wl));
    expectedInfo->declareProperty(new PropertyWithValue<int>("bank", bank));
    expectedInfo->declareProperty(
        new PropertyWithValue<int32_t>("vanadium", van));
    expectedInfo->declareProperty(
        new PropertyWithValue<int32_t>("container", can));
    expectedInfo->declareProperty(
        new PropertyWithValue<int32_t>("empty", empty));
    expectedInfo->declareProperty(
        new ArrayProperty<std::vector<double>>("d_min"));
    expectedInfo->setPropertyValue("d_min", dmin);
    expectedInfo->declareProperty(
        new ArrayProperty<std::vector<double>>("d_max"));
    expectedInfo->setPropertyValue("d_max", dmax);
    expectedInfo->declareProperty(
        new PropertyWithValue<double>("tof_min", tofmin));
    expectedInfo->declareProperty(
        new PropertyWithValue<double>("tof_max", tofmax));

    return expectedInfo;
  }

  void compareResult(PropertyManager_sptr expected,
                     PropertyManager_sptr observed) {
    TS_ASSERT_EQUALS(expected->propertyCount(), observed->propertyCount());

    const std::vector<Property *> &expectedProps = expected->getProperties();

    for (std::size_t i = 0; i < expectedProps.size(); ++i) {
      const std::string name = expectedProps[i]->name();
      TS_ASSERT_EQUALS(expected->getPropertyValue(name),
                       observed->getPropertyValue(name));
    }
  }

  void test_Init() {
    PDDetermineCharacterizations alg;
    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT(alg.isInitialized());
  }

  void testNoChar() {
    createLogWksp();
    // don't create characterization table

    PDDetermineCharacterizations alg;
    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT_THROWS_NOTHING(
        alg.setPropertyValue("InputWorkspace", m_logWSName));
    TS_ASSERT_THROWS_NOTHING(
        alg.setPropertyValue("ReductionProperties", m_propertyManagerName));
    TS_ASSERT_THROWS_NOTHING(alg.execute(););
    TS_ASSERT(alg.isExecuted());

    PropertyManager_sptr expectedInfo = createExpectedInfo(0., 0., 1, 0, 0, 0, "", "", 0., 0.);

    compareResult(expectedInfo, PropertyManagerDataService::Instance().retrieve(
                                    m_propertyManagerName));
  }

  void testEmptyChar() {
    createLogWksp();
    auto tableWS = createTableWksp(false);

    PDDetermineCharacterizations alg;
    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT_THROWS_NOTHING(
        alg.setPropertyValue("InputWorkspace", m_logWSName));
    TS_ASSERT_THROWS_NOTHING(alg.setProperty("Characterizations", tableWS));
    TS_ASSERT_THROWS_NOTHING(
        alg.setPropertyValue("ReductionProperties", m_propertyManagerName));
    TS_ASSERT_THROWS_NOTHING(alg.execute(););
    TS_ASSERT(alg.isExecuted());

    PropertyManager_sptr expectedInfo = createExpectedInfo(0., 0., 1, 0, 0, 0, "", "", 0., 0.);

    compareResult(expectedInfo, PropertyManagerDataService::Instance().retrieve(
                                    m_propertyManagerName));
  }

  void testFullChar() {
    createLogWksp();
    auto tableWS = createTableWksp(true);

    PDDetermineCharacterizations alg;
    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT_THROWS_NOTHING(
        alg.setPropertyValue("InputWorkspace", m_logWSName));
    TS_ASSERT_THROWS_NOTHING(alg.setProperty("Characterizations", tableWS));
    TS_ASSERT_THROWS_NOTHING(
        alg.setPropertyValue("ReductionProperties", m_propertyManagerName));
    TS_ASSERT_THROWS_NOTHING(alg.execute(););
    TS_ASSERT(alg.isExecuted());

    PropertyManager_sptr expectedInfo = createExpectedInfo(60., 0.533, 1, 17702, 17711, 0,
                                           "0.05", "2.20", 0000.00, 16666.67);

    compareResult(expectedInfo, PropertyManagerDataService::Instance().retrieve(
                                    m_propertyManagerName));
  }

  void testFullCharDisableChar() {
    createLogWksp();
    auto tableWS = createTableWksp(true);

    PDDetermineCharacterizations alg;
    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT_THROWS_NOTHING(
        alg.setPropertyValue("InputWorkspace", m_logWSName));
    TS_ASSERT_THROWS_NOTHING(alg.setProperty("Characterizations", tableWS));
    TS_ASSERT_THROWS_NOTHING(alg.setProperty("BackRun", -1));
    TS_ASSERT_THROWS_NOTHING(alg.setProperty("NormRun", -1));
    TS_ASSERT_THROWS_NOTHING(alg.setProperty("NormBackRun", -1));
    TS_ASSERT_THROWS_NOTHING(
        alg.setPropertyValue("ReductionProperties", m_propertyManagerName));
    TS_ASSERT_THROWS_NOTHING(alg.execute(););
    TS_ASSERT(alg.isExecuted());

    PropertyManager_sptr expectedInfo = createExpectedInfo(60., 0.533, 1, 0, 0, 0, "0.05",
                                           "2.20", 0000.00, 16666.67);

    compareResult(expectedInfo, PropertyManagerDataService::Instance().retrieve(
                                    m_propertyManagerName));
  }
};

#endif /* MANTID_ALGORITHMS_PDDETERMINECHARACTERIZATIONS2TEST_H_ */
