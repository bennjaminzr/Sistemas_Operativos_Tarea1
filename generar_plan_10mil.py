import random
import sys
 
NOMBRES_ASADO = [
    "prender_carbon", "comprar_carne", "comprar_pan", "asar_longaniza",
    "armar_choripan", "servir_mesa", "poner_musica", "pelar_papas",
    "hacer_ensalada", "enfriar_bebidas", "buscar_sillas", "poner_mantel",
    "cortar_limones", "preparar_pebre", "sacar_hielo", "asar_vacuno",
    "voltear_carbon", "limpiar_parrilla", "traer_hielo", "abrir_vino",
]
 
def generar_plan(n, archivo_salida, max_deps=3, prob_tiempo_vacio=0.1):
    with open(archivo_salida, "w", encoding="utf-8") as f:
        for i in range(1, n + 1):
            nombre_base = random.choice(NOMBRES_ASADO)
            nombre = f"{nombre_base}_{i}"
 
            if random.random() < prob_tiempo_vacio:
                tiempo = ""
            else:
                tiempo = str(random.randint(50, 2000))
 
            deps = []
            if i > 1:
                cantidad = random.randint(0, min(max_deps, i - 1))
                if cantidad > 0:
                    deps = random.sample(range(1, i), cantidad)
 
            deps_str = ", ".join(str(d) for d in deps)
 
            f.write(f"{i} : {nombre} : {tiempo} : {deps_str}\n")
 
    print(f"Generado '{archivo_salida}' con {n} actividades.")
 
 
if __name__ == "__main__":
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 10000
    salida = sys.argv[2] if len(sys.argv) > 2 else "plan_10mil.txt"
    generar_plan(n, salida)
