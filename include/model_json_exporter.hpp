/**
 * BSD 2-Clause License
 *
 * Copyright (c) 2017, Laouen Mayal Louan Belloli
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#ifndef PMGBP_PDEVS_MODEL_JSON_EXPORTER
#define PMGBP_PDEVS_MODEL_JSON_EXPORTER

#include <iostream>
#include <string>

#include <boost/type_index.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <cadmium/modeling/message_bag.hpp>

#include "json_exporter_helper.hpp"

using namespace std;

using cadmium::make_message_bags;
using boost::property_tree::ptree;
using boost::property_tree::write_json;

template<typename TIME, template<typename T> class MODEL>
class Atomic_cadmiun_to_json {
    using output_ports=typename MODEL<TIME>::output_ports;
    using input_ports=typename MODEL<TIME>::input_ports;

public:
    Atomic_cadmiun_to_json() = default;

    void export_to_json(ptree& json_model) {

        json_model.put("id", boost::typeindex::type_id<MODEL<TIME>>().pretty_name());
        json_model.put("type", "atomic");

        ptree json_ports_model;
        ports_to_json<TIME, output_ports>(json_ports_model, cadmium::port_kind::out);
        ports_to_json<TIME, input_ports>(json_ports_model, cadmium::port_kind::in);

        if (!json_ports_model.empty()) {
            json_model.add_child("ports", json_ports_model);
        }
    }

    // just to allow the method call with the depth without asking if is an atomic exporter
    void export_to_json(ptree& json_model, int deph) {
        this->export_to_json(json_model);
    }

    std::ostream& print_to_json(std::ostream& os) {
        ptree json_model;
        this->export_to_json(json_model);

        //std::ostringstream buf;

        write_json (os, json_model, true);
        //os << buf.str();
        return os;
    }

    // just to allow the method call with the depth without asking if is an atomic exporter
    void print_to_json(const char* json_file_path, int depth) {
        this->print_to_json(json_file_path);
    }
};

template<typename TIME, template<typename T> class MODEL>
class Cadmium_to_JSON {

    template<typename P>
    using submodels_type=typename MODEL<TIME>::template models<P>;
    using output_ports=typename MODEL<TIME>::output_ports;
    using input_ports=typename MODEL<TIME>::input_ports;
    using eic=typename MODEL<TIME>::external_input_couplings;
    using eoc=typename MODEL<TIME>::external_output_couplings;
    using ic=typename MODEL<TIME>::internal_couplings;

    template<template<typename> class M>
    using coupled_submodel_exporter=Cadmium_to_JSON<TIME, M>;

    template<template<typename> class M>
    using atomic_submodel_exporter=Atomic_cadmiun_to_json<TIME, M>;

public:
    Cadmium_to_JSON() = default;

    void export_to_json(ptree& json_model) {

        json_model.put("id", boost::typeindex::type_id<MODEL<TIME>>().pretty_name());
        json_model.put("type", "coupled");

        IC_to_json<TIME, ic>(json_model);
        EIC_to_json<TIME, eic>(json_model);
        EOC_to_json<TIME, eoc>(json_model);

        ptree json_ports_model;
        ports_to_json<TIME, output_ports>(json_ports_model, cadmium::port_kind::out);
        ports_to_json<TIME, input_ports>(json_ports_model, cadmium::port_kind::in);

        if (!json_ports_model.empty()) {
            json_model.add_child("ports", json_ports_model);
        }

        submodels_to_json<TIME, submodels_type, coupled_submodel_exporter, atomic_submodel_exporter>(json_model);
    }

    void export_to_json(ptree& json_model, int depth) {

        json_model.put("id", boost::typeindex::type_id<MODEL<TIME>>().pretty_name());
        json_model.put("type", "coupled");

        IC_to_json<TIME, ic>(json_model);
        EIC_to_json<TIME, eic>(json_model);
        EOC_to_json<TIME, eoc>(json_model);

        ptree json_ports_model;
        ports_to_json<TIME, output_ports>(json_ports_model, cadmium::port_kind::out);
        ports_to_json<TIME, input_ports>(json_ports_model, cadmium::port_kind::in);

        if (!json_ports_model.empty()) {
            json_model.add_child("ports", json_ports_model);
        }

        submodels_to_json<TIME, submodels_type, coupled_submodel_exporter, atomic_submodel_exporter>(json_model, depth);
    }

    std::ostream& print_to_json(std::ostream& os) {
        ptree json_model;
        this->export_to_json(json_model);

        //std::ostringstream buf;

        write_json (os, json_model, true);
        //os << buf.str();
        return os;
    }

    std::ostream& print_to_json(std::ostream& os, int depth) {
        ptree json_model;
        this->export_to_json(json_model, depth);

        //std::ostringstream buf;

        write_json (os, json_model, true);
        //os << buf.str();
        return os;
    }
};

template<typename TIME, template<typename T> class MODEL>
void export_model_to_json(std::ostream& os) {
    using json_exporter=typename std::conditional<cadmium::concept::is_atomic<MODEL>::value(), Atomic_cadmiun_to_json<TIME, MODEL>, Cadmium_to_JSON<TIME, MODEL>>::type;

    json_exporter exporter;
    exporter.print_to_json(os);
}

template<typename TIME, template<typename T> class MODEL>
void export_model_to_json(std::ostream& os, int depth) {
    using json_exporter=typename std::conditional<cadmium::concept::is_atomic<MODEL>::value(), Atomic_cadmiun_to_json<TIME, MODEL>, Cadmium_to_JSON<TIME, MODEL>>::type;

    json_exporter exporter;
    exporter.print_to_json(os, depth);
}


#endif //PMGBP_PDEVS_MODEL_JSON_EXPORTER
