/*! \mainpage Tapir Vision Toolkit
  *
  * Author: Thomas Lampe \n
  * Machine Learning Lab \n
  * University of Freiburg \n
  *
  * \section intro_sec Introduction
  *
  * Tapir is intended as a versatile, easily configurable tool for object detection.
  * It divides the common steps during detection into several classes of modules,
  * which can then be exchanged and mixed to fit any given situation. Tapir
  * differentiates between:
  * \li the ImageSource, which acquires an image from a camera
  * \li the Detector, which finds objects in an image
  * \li the Tracker, which selects a subset of objects from detected candidates
  * \li the Output, which delivers the selected objects to their final purpose
  * \li the Display, which shows the original image or intermediate steps
  *
  * \subsection features Features:
  *
  * \li Provides various object detection methods for use in machine learning.
  * \li Modular design for easy exchanging of cameras, detection methods or output.
  * \li Lightweight enough implementation for use in robotics, based on OpenCV
  *     (http://opencv.org)
  * \li Provides default interfaces for use with \f$CLS^2\f$
  *     (http://ml.informatik.uni-freiburg.de/research/clsquare).
  *
  * \section install_sec Installation
  *
  * \subsection step1 Step1: Get a fresh copy of Tapir
  *
  * At the moment we do not provide precompiled versions. So you have to get the newest sources from:
  * \li http://ml.informatik.uni-freiburg.de/research/tapir/
  *
  * \subsection step2 Step2: Unpack the source code
  *
  * \li tar xzf tapir.tgz
  *
  * \subsection step3 Step3: Compile the main tree
  *
  * \li mkdir build
  * \li cd build
  * \li cmake ..
  * \li make install
  *
  * or, for short:
  * \li ./install
  *
  * Tapir will automatically install a suitable version of OpenCV, preferably from an archive
  * hosted on our own servers. You can also use either the newest version from the OpenCV
  * repositories or whichever version you have installed globally on your system; options
  * to do so will be presented when invoking cmake.
  *
  * \section usage Usage
  *
  * To run tapir, you need to create a configuration file that describes which modules you want to
  * use and how they are to be parametrized. Examples of such configs are included in the folder
  * \e demo/. More available modules and their parameters are described in this documentation
  * in the "Modules" section.
  *
  * Once you are done, simply run the program using
  * \li tapir <config_file>
  *
  * You can terminate the program at any time by pressing "q" on the keyboard (while the focus is
  * on the terminal you started Tapir from). Some modules may offer additional hotkeys; to list
  * them, you can press "?" at any time.
  */
