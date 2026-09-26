#ifndef PLAN_HPP
#define PLAN_HPP

#include <vector>
#include <string>

struct Actividad { 

    std::string id;
    std::string nombre;
    long tiempo_ms;
    std::vector<std::string> deps;

};

#endif