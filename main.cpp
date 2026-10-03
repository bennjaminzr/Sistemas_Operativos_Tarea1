#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <random>
#include "plan.hpp"
#include <unordered_map>
#include <queue>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>   
#include <fcntl.h>       
#include <cstring>
#include <ctime>

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

    int K = std::atoi(argv[2]);
    if (K <= 0) {
        std::fprintf(stderr, "K debe ser un numero mayor a 0\n");
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

    std::unordered_map<std::string, int> indice;
    for (size_t i = 0; i < actividades.size(); i++) {
        if (!indice.insert({actividades[i].id, static_cast<int>(i)}).second) {
            std::fprintf(stderr, "ID duplicado: %s\n", actividades[i].id.c_str());
            return 1;
        }
    }


    std::vector<std::vector<int>> dependientes(actividades.size());
    std::vector<int> pendientes(actividades.size(), 0);
    for (size_t i = 0; i < actividades.size(); i++) {
        for(const std::string &d : actividades[i].deps) {
            int j = indice.at(d);
            dependientes[j].push_back(static_cast<int>(i));
            pendientes[i]++;
        }

    }

    std::vector<int> restantes = pendientes;
    std::queue<int> listos;
    for (size_t i = 0; i < actividades.size(); i++) {
        if (restantes[i] == 0) listos.push(static_cast<int>(i));
    }

    size_t procesadas = 0;
    while (!listos.empty()) {
        int i = listos.front();
        listos.pop();
        procesadas++;
        for (int j : dependientes[i]) {
            restantes[j]--;
            if (restantes[j] == 0) listos.push(j);
        }
    }

    if (procesadas != actividades.size()) {
        std::fprintf(stderr, "El plan tiene un ciclo de dependencias. Bloqueadas:");
        for (size_t i = 0; i < actividades.size(); i++) {
            if (restantes[i] > 0) {
                std::fprintf(stderr, " %s", actividades[i].id.c_str());
            }
        }
        std::fprintf(stderr, "\n");
        return 1;
    }

        

    std::vector<int> restantes_exec = pendientes; 
    std::queue<int> listos_exec;
    for (size_t i = 0; i < actividades.size(); i++) {
        if (restantes_exec[i] == 0) listos_exec.push(static_cast<int>(i));
    }

    int procesos_activos = 0;
    std::unordered_map<pid_t, int> pid_a_indice;
    std::unordered_map<pid_t, int> pid_a_fd;

    size_t terminadas = 0;


    int fallo_prob = 0;
    if (const char *env = std::getenv("FALLO_PROB")) {
        fallo_prob = std::atoi(env);
        if (fallo_prob < 0) fallo_prob = 0;
        if (fallo_prob > 100) fallo_prob = 100;
    }
    std::vector<bool> abortada(actividades.size(), false);


    while (terminadas < actividades.size()) {

        
        while (procesos_activos < K && !listos_exec.empty()) {
            int i = listos_exec.front();
            listos_exec.pop();

            if (abortada[i]) {
                std::printf("[%s] abortada (dependencia fallida)\n",
                            actividades[i].id.c_str());
                terminadas++;
                for (int j : dependientes[i]) {
                    abortada[j] = true;
                    restantes_exec[j]--;
                    if (restantes_exec[j] == 0) listos_exec.push(j);
                }
                continue;
            }

            pid_t pid = fork();

            if (pid < 0) {
                std::fprintf(stderr, "Error al crear proceso para %s\n",
                             actividades[i].id.c_str());
                return 1;
            }

            
            if (pid == 0) {

                std::printf("[%s] iniciando (%ld ms)\n",
                            actividades[i].id.c_str(), actividades[i].tiempo_ms);
                std::fflush(stdout);

                usleep(actividades[i].tiempo_ms * 1000);

                unsigned semilla = static_cast<unsigned>(getpid()) ^
                                   static_cast<unsigned>(time(nullptr));
                srand(semilla);
                bool exito = !(fallo_prob > 0 && (rand() % 100) < fallo_prob);

                if (exito) {
                    std::printf("[%s] terminado\n", actividades[i].id.c_str());
                } else {
                    std::fprintf(stderr, "[%s] fallo simulado\n",
                                 actividades[i].id.c_str());
                }
                std::fflush(stdout);

                std::string ruta = "/tmp/fifo_" + std::to_string(getpid()) + "_" +
                                    actividades[i].id;
                int fd = open(ruta.c_str(), O_WRONLY);
                const char *msg = exito ? "OK" : "FALLO";
                if (fd != -1) {
                    write(fd, msg, strlen(msg));
                    close(fd);
                }

                _exit(exito ? 0 : 1);
            }

            
            std::string ruta = "/tmp/fifo_" + std::to_string(pid) + "_" +
            actividades[i].id;
            unlink(ruta.c_str());
            mkfifo(ruta.c_str(), 0666);

            pid_a_indice[pid] = i;
            procesos_activos++;

            int fd_lectura = open(ruta.c_str(), O_RDONLY | O_NONBLOCK);
            pid_a_fd[pid] = fd_lectura;

        }

        
         if (procesos_activos > 0) {
            int estado;
            pid_t pid_terminado = waitpid(-1, &estado, 0);

            int i = pid_a_indice[pid_terminado];
            int fd = pid_a_fd[pid_terminado];

            char buffer[64] = {0};
            read(fd, buffer, sizeof(buffer) - 1);
            close(fd);

            std::string ruta = "/tmp/fifo_" + std::to_string(pid_terminado) + "_" +
                                actividades[i].id;
            unlink(ruta.c_str());

            bool exito = WIFEXITED(estado) && WEXITSTATUS(estado) == 0;

            if (exito) {
                std::printf("[%s] mensaje recibido: %s\n",
                            actividades[i].id.c_str(), buffer);
            } else {
                std::fprintf(stderr,
                             "[%s] fallo detectado, se abortan sus dependientes\n",
                             actividades[i].id.c_str());
            }

            procesos_activos--;
            terminadas++;

            for (int j : dependientes[i]) {
                if (!exito) abortada[j] = true;
                restantes_exec[j]--;
                if (restantes_exec[j] == 0) listos_exec.push(j);
            }
        }
    }

    std::printf("Todas las actividades terminaron.\n");
    return 0;
}