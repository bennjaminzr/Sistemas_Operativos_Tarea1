#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

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
    int n = 0;
    while (std::getline(archivo, linea)) {
        n++;
        std::vector<std::string> campos = dividir(linea, ':');
        std::printf("linea %d tiene %zu campos:", n, campos.size());
        for (const std::string &c : campos) {
            std::printf(" [%s]", recortar(c).c_str());
        }


    std::printf("\n");

   

}
 return 0;
}