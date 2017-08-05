#ifndef DICECLIMATE_H
#define DICECLIMATE_H

#include <math.h>
#include "Climate.h"
#include "Emissions.h"
#include "settingsnode.h"

namespace dice {
namespace climate {

template<typename Value, typename Time, typename Constant = Value, typename Variable = TimeSeries<Value>>
class DICEClimate : public Climate<Value, Time, Constant, Variable> {
  protected:
    using Climate<Value, Time, Constant, Variable>::global;
    using Climate<Value, Time, Constant, Variable>::control;
    using Climate<Value, Time, Constant, Variable>::E;  // Total CO2 emissions (GtCO2 per year)
    const settings::SettingsNode& settings;

    const Constant b12{settings["b12"].as<Constant>()};            // Carbon cycle transition matrix
    const Constant b23{settings["b23"].as<Constant>()};            // Carbon cycle transition matrix
    const Constant c1{settings["c1"].as<Constant>()};              // Climate equation coefficient for upper level
    const Constant c3{settings["c3"].as<Constant>()};              // Transfer coefficient upper to lower stratum
    const Constant c4{settings["c4"].as<Constant>()};              // Transfer coefficient for lower level
    const Constant fco22x{settings["fco22x"].as<Constant>()};      // Forcings of equilibrium CO2 doubling (Wm-2)
    const Constant fex0{settings["fex0"].as<Constant>()};          // 2010 forcings of non-CO2 GHG (Wm-2)
    const Constant fex1{settings["fex1"].as<Constant>()};          // 2100 forcings of non-CO2 GHG (Wm-2)
    const Constant M_atm_eq{settings["M_atm_eq"].as<Constant>()};  // Equilibrium concentration atmosphere  (GtC)
    const Constant M_l_eq{settings["M_l_eq"].as<Constant>()};      // Equilibrium concentration in lower strata (GtC)
    const Constant M_u_eq{settings["M_u_eq"].as<Constant>()};      // Equilibrium concentration in upper strata (GtC)
    const Constant t2xco2{settings["t2xco2"].as<Constant>()};      // Equilibrium temp impact (oC per doubling CO2)
    const Value T_atm_upper{control.variables_num, settings["T_atm_upper"].as<Constant>()};

    // Carbon cycle transition matrix
    Constant b11 = 1 - b12;
    Constant b21 = b12 * M_atm_eq / M_u_eq;
    Constant b22 = 1 - b21 - b23;
    Constant b32 = b23 * M_u_eq / M_l_eq;
    Constant b33 = 1 - b32;

    StepwiseBackwardLookingTimeSeries<LowerBounded<Value>, Time> M_atm_series{
        global.timestep_num, {{control.variables_num, settings["M_atm0"].as<Constant>()}, {control.variables_num, settings["M_atm_lower"].as<Constant>()}}};
    StepwiseBackwardLookingTimeSeries<LowerBounded<Value>, Time> M_l_series{
        global.timestep_num, {{control.variables_num, settings["M_l0"].as<Constant>()}, {control.variables_num, settings["M_l_lower"].as<Constant>()}}};
    StepwiseBackwardLookingTimeSeries<LowerBounded<Value>, Time> M_u_series{
        global.timestep_num, {{control.variables_num, settings["M_u0"].as<Constant>()}, {control.variables_num, settings["M_u_lower"].as<Constant>()}}};
    StepwiseBackwardLookingTimeSeries<Bounded<Value>, Time> T_ocean_series{global.timestep_num,
                                                                           {{control.variables_num, settings["T_ocean0"].as<Constant>()},
                                                                            {control.variables_num, settings["T_ocean_lower"].as<Constant>()},
                                                                            {control.variables_num, settings["T_ocean_upper"].as<Constant>()}}};
    StepwiseBackwardLookingTimeSeries<Value, Time> T_atm_series{global.timestep_num, {control.variables_num, settings["T_atm0"].as<Constant>()}};

  public:
    DICEClimate(const settings::SettingsNode& settings_p,
                const Global<Constant, Time>& global_p,
                const Control<Value, Time, Constant, Variable>& control_p,
                Emissions<Value, Time, Constant, Variable>& E_p)
        : Climate<Value, Time, Constant, Variable>(global_p, control_p, E_p), settings(settings_p){};

    // Concentration in atmosphere 2010 (GtC)
    Value M_atm(Time t) {
        return M_atm_series.get(t, [this](Time t, Value M_atm_last) {

            return M_atm_last * b11 + M_u(t - 1) * b21 + E(t - 1) * global.timestep_length / 3.666;

        });
    }

    // Carbon concentration increase in lower oceans (GtC from 1750)
    Value M_l(Time t) {
        return M_l_series.get(t, [this](Time t, Value M_l_last) {

            return M_l_last * b33 + M_u(t - 1) * b23;

        });
    }

    // Carbon concentration increase in shallow oceans (GtC from 1750)
    Value M_u(Time t) {
        return M_u_series.get(t, [this](Time t, Value M_u_last) {

            return M_atm(t - 1) * b12 + M_u_last * b22 + M_l(t - 1) * b32;

        });
    }

    // Increase temperatureof lower oceans (degrees C from 1900)
    Value T_ocean(Time t) {
        return T_ocean_series.get(t, [this](Time t, Value T_ocean_last) {

            return T_ocean_last + c4 * (T_atm(t - 1) - T_ocean_last);

        });
    }

    // Exogenous forcing for other greenhouse gases
    Constant forcoth(Time t) {
        const Time year = global.start_year + global.timestep_length * t;
        if (year > 2100) {
            return fex1;
        } else {
            return fex0 + (fex1 - fex0) * (global.timestep_length * 0.2 * t) / 18;
        }
    }

    // Increase in radiative forcing (watts per m2 from 1900)
    Value force(Time t) {
        return fco22x * std::log2(M_atm(t) / 588) + forcoth(t);  // TODO 588 == M_atm_eq ??
    }

    // Increase temperature of atmosphere (degrees C from 1900)
    Value T_atm(Time t) override {
        return T_atm_series.get(t, [this](Time t, Value T_atm_last) {

            const Value T_atm_t = T_atm_last + c1 * (force(t) - (fco22x / t2xco2) * T_atm_last - c3 * (T_atm_last - T_ocean(t - 1)));
            if (T_atm_t > T_atm_upper) {
                return T_atm_upper;
            } else {
                return T_atm_t;
            }

        });
    }

    bool observe(Observer<Value, Time, Constant>& observer) override {
        OBSERVE_VAR(M_atm);
        OBSERVE_VAR(M_l);
        OBSERVE_VAR(M_u);
        OBSERVE_VAR(T_ocean);
        OBSERVE_VAR(T_atm);
        OBSERVE_VAR(force);
        OBSERVE_VAR(forcoth);
        OBSERVE_VAR(T_atm);
        return true;
    }

    void reset() override {
        M_atm_series.reset();
        M_l_series.reset();
        M_u_series.reset();
        T_ocean_series.reset();
        T_atm_series.reset();
    }
};
}
}

#endif
