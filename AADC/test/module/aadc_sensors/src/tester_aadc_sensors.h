/**
 *
 * AADC Arduino sensors filter tests
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved
 *
 * $Author: VG8D3AW $
 * $Date: 2013-02-06 16:30:41 +0100 (Mi, 06. Feb 2013) $
 * $Revision: 18162 $
 *
 * @remarks
 *
*/

#ifndef _TESTER_AADC_ARDUINO_SENSORS_HEADER_
#define _TESTER_AADC_ARDUINO_SENSORS_HEADER_

/**
*
* Tester class
*
*/
DECLARE_TESTER_CLASS(cTesterAADCSensors)
{
    BEGIN_TESTER_MAP(cTesterAADCSensors)
        DECLARE_TESTER_FUNCTION(TestSteeringAngle)
        DECLARE_TESTER_FUNCTION(TestRPM)
        DECLARE_TESTER_FUNCTION(TestIMU)
        DECLARE_TESTER_FUNCTION(TestIR)
        //DECLARE_TESTER_FUNCTION(TestPHOTO)
        DECLARE_TESTER_FUNCTION(TestUS)
        DECLARE_TESTER_FUNCTION(TestVoltage)
    END_TESTER_MAP()

    public:
        void setUp();
        void tearDown();

    public:
        tTestResult TestSteeringAngle();
        tTestResult TestRPM();
        tTestResult TestIMU();
        tTestResult TestIR();
        //tTestResult TestPHOTO();
        tTestResult TestUS();
        tTestResult TestVoltage();
};

#endif


