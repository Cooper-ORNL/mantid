#ifndef FRAMEWORKMANAGERTEST_H_
#define FRAMEWORKMANAGERTEST_H_

#include <cxxtest/TestSuite.h>

#include "../inc/FrameworkManager.h"
#include "../inc/Algorithm.h"
#include <stdexcept>

namespace Mantid
{

class ToyAlgorithm2 : public Algorithm
{
public:
  ToyAlgorithm2() {}
  virtual ~ToyAlgorithm2() {}
  StatusCode init() { return StatusCode::SUCCESS; }
  StatusCode exec() { return StatusCode::SUCCESS; }
  StatusCode final() { return StatusCode::SUCCESS; }
};

}

DECLARE_ALGORITHM(ToyAlgorithm2)

using namespace Mantid;

class FrameworkManagerTest : public CxxTest::TestSuite
{
public:

	void testInitialize()
	{
	  // Not really much to test
		TS_ASSERT_THROWS_NOTHING( manager.initialize() )
	}

	void test_global_Mantid_FrameworkManager_createAlgorithm()
	{
	  TS_ASSERT_THROWS_NOTHING( manager.createAlgorithm("ToyAlgorithm2") )
	  TS_ASSERT_THROWS( manager.createAlgorithm("aaaaaa"), std::runtime_error )
	}

	void test_global_Mantid_FrameworkManager_createAlgorithmWithProps()
	{
	  IAlgorithm *alg = manager.createAlgorithm("ToyAlgorithm2","Prop:Val,P2:V2");
	  std::string prop;
	  StatusCode status = alg->getProperty("Prop",prop);
	  TS_ASSERT ( ! status.isFailure() )
	  TS_ASSERT ( ! prop.compare("Val") )
    status = alg->getProperty("P2",prop);
    TS_ASSERT ( ! status.isFailure() )
    TS_ASSERT ( ! prop.compare("V2") )
	  
    TS_ASSERT_THROWS_NOTHING( manager.createAlgorithm("ToyAlgorithm2","") )
    TS_ASSERT_THROWS_NOTHING( manager.createAlgorithm("ToyAlgorithm2","noValProp") )
    TS_ASSERT_THROWS( manager.createAlgorithm("ToyAlgorithm2","p1:p2:p3"), std::runtime_error )
	}

	void testExec()
	{
	  // Switch to using a dummy algorithm when auto-registration implemented
	  IAlgorithm *alg = manager.exec("ToyAlgorithm2","Filename:../../../../Test/HET15869.RAW,OutputWorkspace:outer");
	  TS_ASSERT( alg->isExecuted() )
	}

	void testGetWorkspace()
	{
		TS_ASSERT_THROWS( manager.getWorkspace("wrongname"), std::runtime_error )
	}

private:
  FrameworkManager manager;
	
};

#endif /*FRAMEWORKMANAGERTEST_H_*/
