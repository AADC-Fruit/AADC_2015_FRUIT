// SKELETON CODE! :)

#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <emokit/emokit.h>
#include <fcntl.h>
#include "emotiv/global.h"

using std::string;

using namespace Emotiv;

struct emokit_device* device;
int emokit_pipe;

void create_and_open_emokit_device()
{
    // Open emokit_device
    device = emokit_create();
    IOUT("Current epoc devices connected: " << emokit_get_count(device, EMOKIT_VID, EMOKIT_PID));
  int deviceNumber = 1;
  int r = emokit_open(device, EMOKIT_VID, EMOKIT_PID, deviceNumber);  // changed to 1 because we have 2 devices 
  if(r != 0)
  {
    EOUT("CANNOT CONNECT: " << r);
    return;
  }
  IOUT("Connected to Epoc Device.");
}

bool open_pipe(string pipeName)
{
  emokit_pipe  = open(pipeName.c_str(), O_WRONLY);
  bool couldBeOpened = emokit_pipe != -1;
  return couldBeOpened;
}
    
struct emokit_frame read_frame_from_emokit()
{
    bool read_data_from_emokit = false;
    while (!read_data_from_emokit) {
        read_data_from_emokit = emokit_read_data(device) > 0;
    }
     return emokit_get_next_frame(device);
}

void write_emokit_frame_to_pipe(struct emokit_frame frame)
{
    const int frameStringSize = 200;
    char frameString[frameStringSize];
    sprintf(&(frameString[0]),
        "%d %d %d %d %d %hd %d %hd %d %hd %d %hd %d %hd %d %hd %d %hd %d %hd %d %hd %d %hd %d %hd %d %hd %d %hd %d %hd\n",
        frame.counter, frame.battery, frame.gyroX, frame.gyroY,
        frame.F3, frame.cq.F3, frame.FC6, frame.cq.FC6, frame.P7, frame.cq.P7,
        frame.T8, frame.cq.T8, frame.F7, frame.cq.F7, frame.F8, frame.cq.F8,
        frame.T7, frame.cq.T7, frame.P8, frame.cq.P8, frame.AF4, frame.cq.AF4,
        frame.F4, frame.cq.F4, frame.AF3, frame.cq.AF3, frame.O2, frame.cq.O2,
        frame.O1, frame.cq.O1, frame.FC5, frame.cq.FC5);
    string trimmedFrameString(frameString);
    int i = write(emokit_pipe, trimmedFrameString.c_str(), trimmedFrameString.size());
    i = 0; // pointless nonsense to suppress warnings
    i = i;
}

void continouslyReadFramesAndWriteToPipe()
{
    while(true) {
        struct emokit_frame one_emokit_frame = read_frame_from_emokit();
        write_emokit_frame_to_pipe(one_emokit_frame);
    }
}

void closeEpocDevice()
{
    emokit_close(device);
    emokit_delete(device);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
      fprintf(stderr, "Usage: ./epocToPipe pipeName\n");
      return -1;
    }
    string pipeName = argv[1];
    create_and_open_emokit_device();
    bool couldOpenPipe = open_pipe(pipeName);
    if (!couldOpenPipe) {
      std::cout << "Pipe " << pipeName << " could not be opened." << std::endl;
      return -1;
    }
    continouslyReadFramesAndWriteToPipe();
    closeEpocDevice();
   return 0;
}

