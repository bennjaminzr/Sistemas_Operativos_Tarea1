#include <cstdio>
#include <fstream>
#include <string>

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
    int n = 0;
    while (std::getline(archivo, linea)) {
        n++;
        std::printf("%d: [%s]\n", n, linea.c_str());


    }

    return 0;

}