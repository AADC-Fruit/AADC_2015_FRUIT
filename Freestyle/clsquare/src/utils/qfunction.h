#ifndef _CLSQUARE_QFUNCTION_H_
#define _CLSQUARE_QFUNCTION_H_

#include "global.h"
#include "utils/BatchUtils/BatchData.h"
#include <cstring>

namespace CLS
{
namespace Util
{

/** Abstract interface for a Q-function.
  * @author Thomas Lampe */
class QFunction
{
 public:
  virtual ~QFunction () {};

  /** Initializes the Q-function for use.
    * It can be assumed that this function will be called explicitly by any client code.
    * Repeated calls should re-initialize the Q-function, unless QFunction::reinit() was implemented.
    * \param state_dim dimensionality of state space
    * \param action_dim dimensionality of action space
    * \param fname path to file containing configuration information
    * \param chapter section within configuration file contraining to get parameters from
    * \return false if initialization failed */
  virtual bool init (const int state_dim, const int action_dim, const char* fname=0, const char* chapter=0) = 0;

  /** Re-initializes the Q-function.
    * \return false if reinitialization failed */
  virtual bool reset () = 0;

  /** Notifies that a sequence has started. */
  virtual void notify_episode_starts (const long episode) {};

  /** Gets the Q-value for a given state in a given action.
    * \param state current state
    * \param action action chosen in state
    * \return Q-value */
  virtual double get_value (const double* state, const double* action) = 0;

  /** Updates the Q-function with a list of training patterns.
    * \param patterns list of training patterns
    * \return false if update failed */
  virtual bool train (std::vector<BatchUtils::XUT>&) = 0;

  /** Prepares the Q-function for deletion.
    * \return false if an error occurred during deinitialization */
  virtual bool deinit () = 0;

  /** Saves the current Q-function to file.
    * \param filename full path to file
    * \return false if error occurred during saving */
  virtual bool save (char* fname) = 0;

  /** Determines the dimensionality of the state space.
    * \return state dimensionality */
  virtual inline int get_state_dim () = 0;

  /** Determines the dimensionality of the action space.
    * \return action dimensionality */
  virtual inline int get_action_dim () = 0;
};

/** Extended Q-function interface that allows computing derivatives.
  * @author Thomas Lampe */
class DifferentiableQFunction : public QFunction
{
 public:
  virtual ~DifferentiableQFunction () {};

  /** Calculates the derivative of the Q-function.
    * \param pattern Q-target containing the state and action for which to compute the derivatives
    * \param derivative blank pattern that will store the derivatives */
  virtual void compute_derivative (const BatchUtils::XUT& pattern, BatchUtils::XUT& derivative) = 0;
};

};
};

#endif
