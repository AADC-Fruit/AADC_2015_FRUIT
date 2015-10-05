/**
 *
 * ADTF Moose Filter.
 *
 * @file
 * Copyright &copy; FRUIT. All rights reserved
 *
 * $Author: FRUIT $
 * $Date: 2011-06-30 16:51:21 +0200 (Thu, 30 Jun 2011) $
 * $Revision: 26514 $
 *
 * @remarks
 *
 */
#ifndef _PRINT_FILTER_H_
#define _PRINT_FILTER_H_

#define OID_ADTF_PRINT_FILTER "fruit.dev.print_filter"

#include <fstream>


//*************************************************************************************************
class PrintFilter : public adtf::cFilter {
  ADTF_FILTER(OID_ADTF_PRINT_FILTER, "Print Filter", adtf::OBJCAT_DataFilter);
  public:
    PrintFilter(const tChar* __info);
    virtual ~PrintFilter();

  protected:
    // Input pins
    cInputPin   data_;

    // Filter methods
    tResult Init(tInitStage eStage, __exception);
    tResult Shutdown(tInitStage eStage, __exception);

    // Coder Descriptor for the input pins
	cObjectPtr<IMediaTypeDescription> m_pCoderDescSignalInput;

    // Implements IPinEventSink
    tResult OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample);

  private:
    float acc_value_;
    int datapoint_counter_;
    float last_value_;
    std::ofstream file_;
};

//*************************************************************************************************
#endif // _PRINT_FILTER_H_
