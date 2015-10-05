#ifndef _MULTI_PLANT_H_
#define _MULTI_PLANT_H_

#include "plant.h"
#include <vector>

/** Allows combining multiple plants into one.
  * Sub-plants are specified by parameters of the form
  * \b plant_i <em> = <class> <chapter> </em>, with \b i
  * being a continuous index starting at 0, <class> being the
  * type of plant to use, and <chapter> being the configuration
  * chapter its parameters are described in.
  *
  * States, measurements and actions of sub-plants will be
  * appended to each other in the order they have been declared.
  *
  * \note While this module offers a convenient method for joining
  *       simpler plants into a larger scene without having
  *       redundant code, it should be used with caution. Plants
  *       that enforce fixed cycle times will not work as intended
  *       anymore, since pauses are executed one after another.
  * 
  * @ingroup PLANT */
class MultiPlant : public CLS::Plant {
 public:
  bool get_next_plant_state(const double *current_plant_state, const double *current_action, double *next_plant_state);
  bool get_measurement(const double *plant_state, double *measurement);
  bool init(int &plant_state_dim, int &measurement_dim, int &action_dim, double &delta_t, const char *fname=0, const char *chapter=0);
  bool check_initial_state(double *initial_plant_state);
  void deinit();
  void notify_episode_starts();
  void notify_episode_stops();
  void notify_command_string(const char* buf);
  void notify_suspend_for_aux_call_cmd();
  void notify_return_from_aux_call_cmd();
 protected:
  int _pdim, _i;
  std::vector<int> _pstart, _mstart, _astart;
  std::vector<Plant*> _plants;
};

#endif
