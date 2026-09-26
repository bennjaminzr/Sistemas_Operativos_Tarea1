#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <random>
#include "plan.hpp"

std::string recortar(const std::string &s) {

    const char *ws = " \t\r\n";
    size_t ini = s.find_first_not_of(ws);
    if (ini == std::string::npos) return "";
    size_t fin = s.find_last_not_of(ws);
    return s.substr(ini, fin - ini + 1);
}

std::vector<std::string> dividir(const std::string &s, char sep) {

    std::vector<std::string> partes;
    size_t ini = 0;
    while (true) {
        size_t pos = s.find(sep, ini);
         if (pos == std::string::npos) {
            partes.push_back(s.substr(ini));
            break; 

    }
    partes.push_back(s.substr(ini, pos - ini));
    ini = pos + 1;


}

    return partes;
}




int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::fprintf(stderr, "Uso: %s plan.txt K\n", argv[0]);
        return 1;

    }

    std::ifstream archivo(argv[1]);


    if (!archivo)   {
        std::fprintf(stderr, "No se pudo abrir '%s'\n", argv[1]);
        return 1;

    }

    std::string linea;
    std::vector<Actividad> actividades;
    int n = 0;
    std:: mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<long> sorteo(100, 5000);

    while (std::getline(archivo, linea)) {
        n++;
        if (recortar(linea).empty()) continue;
        std::vector<std::string> campos = dividir(linea, ':');

        if (campos.size() != 4) {
            std::fprintf(stderr, "linea %d: formato invalido\n", n);
            return 1;
        }

        Actividad a;
        a.id = recortar(campos[0]);
        a.nombre = recortar(campos[1]);
        std::string t = recortar(campos[2]);
        if(t.empty()) {
            a.tiempo_ms = sorteo(rng);
        } else {
            a.tiempo_ms = std::strtol(t.c_str(), nullptr, 10);
        }

    
        for(const std::string &d : dividir(campos[3], ',')) {
            std::string dep = recortar(d);
            if (!dep.empty()) a.deps.push_back(dep);
        }

        
        actividades.push_back(a);


     



    }
    if (actividades.empty()) {
        std::fprintf(stderr, "El plan no contiene actividades\n");
        return 1;
    }

       std::printf("%zu actividades leidas\n", actividades.size());
       
    for (const Actividad &a : actividades) {
        std::printf("[%s] %s (%ld ms) deps:", a.id.c_str(), a.nombre.c_str(),
                    a.tiempo_ms);
        for (const std::string &d : a.deps) {
            std::printf(" [%s]", d.c_str());
        }
        std::printf("\n");
    }

    return 0;
}