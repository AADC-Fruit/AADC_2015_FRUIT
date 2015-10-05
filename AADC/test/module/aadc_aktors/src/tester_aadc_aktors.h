/**
 *
 * AADC Arduino Aktors filter tests
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

#ifndef _TESTER_AADC_ARDUINO_AKTORS_HEADER_
#define _TESTER_AADC_ARDUINO_AKTORS_HEADER_

/**
*
* Tester class
*
*/
DECLARE_TESTER_CLASS(cTesterAADCAktors)
{
    BEGIN_TESTER_MAP(cTesterAADCAktors)
        DECLARE_TESTER_FUNCTION(TestAcceleration)
        DECLARE_TESTER_FUNCTION(TestSteeringAngle)
        DECLARE_TESTER_FUNCTION(TestLights)
        DECLARE_TESTER_FUNCTION(TestWatchdog)
        DECLARE_TESTER_FUNCTION(TestEmergencyStop)
    END_TESTER_MAP()

    public:
        void setUp();
        void tearDown();

    public:
        tTestResult TestAcceleration();
        tTestResult TestSteeringAngle();
        tTestResult TestLights();
        tTestResult TestWatchdog();
        tTestResult TestEmergencyStop();
};

#endif


