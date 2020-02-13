/*
  Copyright (C) 2017-2020 Sven Willner <sven.willner@pik-potsdam.de>

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

#ifndef GLOBAL_H
#define GLOBAL_H

#include "settingsnode.h"

namespace dice {
template<typename Constant, typename Time>
class Global {
  protected:
    const settings::SettingsNode& settings;

  public:
    const Constant dK{settings["dK"].template as<Constant>()};              // Depreciation rate on capital (per year)
    const Constant elasmu{settings["elasmu"].template as<Constant>()};      // Elasticity of marginal utility of consumption
    const Constant expcost2{settings["expcost2"].template as<Constant>()};  // Exponent of control cost function
    const Constant fosslim{settings["fosslim"].template as<Constant>()};    // Maximum cumulative extraction fossil fuels (GtC)
    const Constant gamma{settings["gamma"].template as<Constant>()};        // Capital elasticity in production function
    const Constant prstp{settings["prstp"].template as<Constant>()};        // Initial rate of social time preference per year
    const Constant scale1{settings["scale1"].template as<Constant>()};      // Multiplicative scaling coefficient
    const Constant scale2{settings["scale2"].template as<Constant>()};      // Additive scaling coefficient

    const Time timestep_length{settings["timestep_length"].template as<Time>()};
    const Time start_year{settings["start_year"].template as<Time>()};
    const Time timestep_num{settings["timestep_num"].template as<Time>()};

    const Constant optlrsav = (dK + 0.004) / (dK + 0.004 * elasmu + prstp) * gamma;  // Optimal long-run savings rate used for transversality

    explicit Global(const settings::SettingsNode& settings_p) : settings(settings_p) {}
};
}  // namespace dice

#endif
