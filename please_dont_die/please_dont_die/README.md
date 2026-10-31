# PLEASE, DON'T DIE v0.1

Juego de texto tipo terminal de supervivencia biológica, en C++17 con Raylib.
Controlas a S.O.R.A., un organismo consciente que vive dentro de Anastasia Volkova.
Tienes 72 horas de tiempo del juego para decidir si ambas sobreviven.

> Three days. One body. Two organisms. One chance to survive.

## Estructura del proyecto

```
please_dont_die/
  CMakeLists.txt
  README.md
  data/                         Contenido editable sin recompilar
    events.json                 Eventos narrativos y sus condiciones
    memories.json               Fragmentos de memoria en ruso
    upgrades.json               Ramas evolutivas, costos y textos
    dialogue.json               Líneas de S.O.R.A., decisiones y finales
  audio/
    gm5.ogg                     Música de fondo (en bucle)
  saves/                        Partidas guardadas (se crea sola)
  src/
    main.cpp                    Punto de entrada: ventana, bucle, música
    simulation/
      simulation.h / .cpp       Estado del juego y biología (sin Raylib)
    systems/
      time_system.*             Reloj de 72 horas, pausa de la decisión final
      event_system.*            Dispara eventos según condiciones de data/
      ending_system.*           Decide uno de los 5 finales
      evolution_system.*        Ramas evolutivas y sus efectos numéricos
      condition.*               Mini lenguaje de condiciones ("minutes>=720 and flag:x")
    narrative/
      json.*                    Lector JSON propio, sin dependencias
      content.*                 Carga de los archivos de data/
    terminal/
      commands.h                Interfaz de la capa de comandos
      command_parser.cpp        Parser y comandos informativos
      actions.*                 Acciones que gastan tiempo, DECIDE
      command_util.h            Ayudas compartidas
      text_wrap.*               Ajuste de texto (sin Raylib)
      terminal_view.*           Pantalla tipo terminal con Raylib
    save/
      save_system.*             SAVE y LOAD en texto legible
    audio/
      music_player.*            Reproducción en bucle y silenciar
  tests/
    test_logic.cpp              Pruebas headless de la lógica
```

## Comandos

| Comando | Tiempo | Qué hace |
|---|---:|---|
| STATUS, INVENTORY, HELP | 0 min | Información. No avanza el reloj. |
| DIAGNOSE [HEART/BRAIN/LUNGS/IMMUNE/ALL] | 30 min | Lee los valores del organismo. |
| HEAL ORGAN / NEURAL / OXYGEN | 60 min | Repara consumiendo biomasa y energía. |
| ABSORB NUTRIENTS | 45 min | Gasta una bolsa de nutrientes (hay 4). Sin daño. |
| ABSORB TISSUE | 45 min | Más biomasa, pero daña a Anastasia. |
| STABILIZE IMMUNE / NEURAL / ORGAN | 45 min | Reduce el rechazo o estabiliza al huésped. |
| EVOLVE REGENERATION / CAMOUFLAGE / NEURAL INTERFACE | 120 min | Sube una rama (máximo nivel 2). |
| COMMUNICATE | 30 min | Busca fragmentos de memoria en ruso, con traducción y % de confianza. |
| WAIT [minutos] | Libre | Deja pasar el tiempo (10 a 1440). Por defecto 60. |
| DECIDE PROTECT / CLAIM / RELEASE | Hasta el final | Decisión final. Solo disponible a la hora 66. |
| SAVE / LOAD | 0 min | Guarda o carga `saves/savegame.txt`. |
| CLEAR / EXIT | 0 min | Limpia la pantalla o cierra. |

Atajo: **F2** activa o silencia la música.

## Decisión final (hora 66)

Al llegar a la hora 66 el reloj se detiene y el juego pide una decisión:

- **PROTECT**: S.O.R.A. entrega toda su biomasa al corazón de Anastasia. Pierde todo su material, pero el órgano se recupera.
- **CLAIM**: S.O.R.A. se compromete por completo y toma el cuerpo como suyo.
- **RELEASE**: S.O.R.A. retira su apoyo y deja que los dos cuerpos decidan qué son.

Mientras la decisión esté pendiente, las acciones que gastan tiempo se bloquean. Después de elegir, el reloj avanza hasta el final.

## Finales

| Final | Cómo se alcanza |
|---|---|
| **RECOVERY** | Ambos vivos al final, vitalidad ≥ 60, neural ≥ 70, 3 fragmentos y NEURAL INTERFACE nivel 1 o más. No elegir RELEASE. |
| **SYMBIOSIS** | Ambos vivos al final, sin cumplir las condiciones anteriores. |
| **TAKEOVER** | Anastasia muere con S.O.R.A. vivo, o elegir CLAIM con integridad ≥ 40 al final. |
| **SACRIFICE** | S.O.R.A. muere con Anastasia viva. |
| **TERMINATION** | Mueren ambos. |

## Cómo añadir contenido

- **Nuevo evento**: agrega una entrada en `data/events.json`. El campo `when` acepta condiciones como `minutes>=720`, `organ<25`, `flag:evolved`, `!flag:mem_mom` o `memories>=2 and immunity>=40`. Cada evento se dispara una sola vez.
- **Nuevo fragmento de memoria**: agrega una entrada en `data/memories.json`. El campo `requires` usa la misma sintaxis de condiciones. El id genera la bandera `mem_<id>`.
- **Nuevo texto de S.O.R.A.**: edita las claves de `data/dialogue.json`.

Los nombres de estadísticas válidos en las condiciones son: `vitality`, `oxygenation`, `immunity`, `neural`, `organ`, `biomass`, `energy`, `integrity`, `adaptation`, `minutes`, `day`, `memories`, `nutrients`, `regen`, `camo` y `neural_interface`.

## Compilar en Windows

### Opción A: CMake + Visual Studio (recomendada)

1. Instala [CMake](https://cmake.org/) y Visual Studio con la carga de trabajo "Desarrollo para escritorio con C++".
2. Desde la carpeta del proyecto:

```bat
cmake -S . -B build
cmake --build build --config Release
```

CMake descarga Raylib 5.0 si no la tienes. Si ya la tienes instalada, `find_package` la usa. Después de compilar, copia `data/`, `audio/` y crea `saves/` junto al `.exe`.

3. Ejecuta `build\Release\please_dont_die.exe`.

### Opción B: MinGW (g++)

Necesitas Raylib compilada para MinGW:

```bat
g++ -std=c++17 -Isrc src\main.cpp src\terminal\terminal_view.cpp src\audio\music_player.cpp ^
    src\narrative\json.cpp src\narrative\content.cpp src\simulation\simulation.cpp ^
    src\systems\condition.cpp src\systems\time_system.cpp src\systems\event_system.cpp ^
    src\systems\ending_system.cpp src\systems\evolution_system.cpp src\terminal\text_wrap.cpp ^
    src\terminal\actions.cpp src\terminal\command_parser.cpp src\save\save_system.cpp ^
    -o please_dont_die.exe -lraylib -lopengl32 -lgdi32 -lwinmm
```

### Pruebas de lógica

Desde la carpeta del proyecto, con g++:

```bat
g++ -std=c++17 -Isrc tests\test_logic.cpp src\narrative\json.cpp src\narrative\content.cpp src\simulation\simulation.cpp src\systems\condition.cpp src\systems\time_system.cpp src\systems\event_system.cpp src\systems\ending_system.cpp src\systems\evolution_system.cpp src\terminal\text_wrap.cpp src\terminal\actions.cpp src\terminal\command_parser.cpp src\save\save_system.cpp -o test_logic.exe
test_logic.exe
```

O con CMake: `ctest --test-dir build -C Release`.

## Notas

- **Fuente**: se usa `assets/terminal.ttf` si existe, si no `consola.ttf` de Windows (incluye cirílico). Si no encuentra ninguna, usa la fuente por defecto de Raylib, que no dibuja cirílico.
- **Música**: se reproduce `audio/gm5.ogg` en bucle. Si falta o no hay dispositivo de audio, el juego sigue sin sonido y lo indica en pantalla.
- **Datos**: si `data/` tiene un error, el juego lo muestra en pantalla y continúa con el contenido disponible.

## Estado de verificación

- **Lógica (verificada)**: compila sin advertencias con `-Wall -Wextra` y pasa todas las pruebas de `tests/test_logic.cpp`. Cubre el parser JSON, las condiciones, el ajuste de texto, los costos de tiempo, los límites de evolución y de bolsas de nutrientes, la pausa y decisión final, guardado y carga, y los cinco finales.
- **Balance (simulado)**: RECOVERY, SYMBIOSIS y TAKEOVER se alcanzan con partidas completas simuladas. SACRIFICE y TERMINATION se comprobaron con estados forzados, pero aún no con una partida natural.
- **Raylib (no verificado de extremo a extremo)**: `main.cpp`, `terminal_view.cpp` y `music_player.cpp` pasan la verificación de sintaxis contra una cabecera de prueba, pero no se han compilado contra Raylib real ni ejecutado en una ventana. La primera compilación en tu PC puede revelar detalles de la versión de Raylib que uses.
- **Equilibrio**: es una primera versión. Ajusta las constantes de `tickBiology` en `src/simulation/simulation.cpp` si una partida resulta demasiado fácil o difícil.
