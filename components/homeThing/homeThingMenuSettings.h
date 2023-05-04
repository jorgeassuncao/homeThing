#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace homething_menu_base {

enum MenuMode { MENU_MODE_ROTARY, MENU_MODE_3_BUTTON, MENU_MODE_2_BUTTON };

class HomeThingMenuSettings {
 public:
  MenuMode get_mode() { return mode_; }
  void set_mode(MenuMode mode) { mode_ = mode; }
  int get_display_timeout() { return display_timeout_; }
  void set_display_timeout(int display_timeout) {
    display_timeout_ = display_timeout;
  }
  int get_sleep_after() { return sleep_after_; }
  void set_sleep_after(int sleep_after) { sleep_after_ = sleep_after; }

  switch_::Switch* get_sleep_switch() { return sleep_switch_; }
  void set_sleep_switch(switch_::Switch* sleep_switch) {
    sleep_switch_ = sleep_switch;
  }

  bool get_menu_rollover() { return menu_rollover_; }
  void set_menu_rollover(bool menu_rollover) { menu_rollover_ = menu_rollover; }

 private:
  MenuMode mode_;
  int display_timeout_;
  int sleep_after_;
  bool menu_rollover_;
  switch_::Switch* sleep_switch_;
};

}  // namespace homething_menu_base
}  // namespace esphome