/*
  Copyright (C) 2017 Sven Willner <sven.willner@gmail.com>

  This file is part of DICE++.

  DICE++ is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  DICE++ is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with DICE++.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DICE_H
#define DICE_H

#include <autodiff.h>
#include <memory>
#include <vector>
#include "Climate.h"
#include "Control.h"
#include "Damage.h"
#include "Economy.h"
#include "Global.h"

namespace settings {
class SettingsNode;
}

namespace netCDF {
class NcGroup;
class NcDim;
}

namespace dice {

template<typename Value, typename Time, typename Constant, typename Variable>
class Emissions;

template<typename Value, typename Time>
class DICE {
  protected:
    const settings::SettingsNode& settings;
    const Global<Value, Time> global;

  public:
    Control<autodiff::Value<Value>, Time, Value, autodiff::Variable<Value>> control;

  protected:
    std::vector<Economy<autodiff::Value<Value>, Time, Value, autodiff::Variable<Value>>> economies;
    std::unique_ptr<climate::Climate<autodiff::Value<Value>, Time, Value, autodiff::Variable<Value>>> climate;
    std::unique_ptr<damage::Damage<autodiff::Value<Value>, Time, Value, autodiff::Variable<Value>>> damage;

    void write_netcdf_output(const settings::SettingsNode& output_node);
    void write_csv_output(const settings::SettingsNode& output_node);

  public:
    DICE(const settings::SettingsNode& settings_p);
    inline autodiff::Value<Value> calc_single_utility();
    Emissions<autodiff::Value<Value>, Time, Value, autodiff::Variable<Value>> emissions;
    void reset();
    void initialize();
    void output();
    void run();
};
}

#endif
