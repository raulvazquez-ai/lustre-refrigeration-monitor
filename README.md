# Monitor de Refrigeración Industrial: Lustre + C (GTK3) ❄️🖥️

Este repositorio contiene un sistema de monitorización reactivo para cámaras frigoríficas industriales, desarrollado mediante programación síncrona en Lustre e integrado con una Interfaz Gráfica de Usuario (GUI) en C. 

## ⚙️ Arquitectura del Sistema

El proyecto se divide en dos capas principales: el núcleo lógico reactivo y la interfaz de interacción visual.

### 1. Núcleo Lógico en Lustre (`nevera.lus`)
El procesamiento de las telemetrías se gestiona mediante nodos síncronos estructurados:
* **`TempCamera`**: Procesa arrays de temperaturas utilizando iteradores espaciales (`red` y `fillred`) para calcular la media térmica y activar alarmas si los valores exceden los rangos seguros (0°C - 6°C) o si la desviación térmica interna supera los 2°C.
* **`CameraHistory`**: Detecta tendencias anómalas comparando la temperatura actual con una media móvil estimada a partir de instantes temporales previos (utilizando el operador `pre`), disparando una alerta si la fluctuación supera ±1.0°C.
* **`SistemaFrio`**: Nodo orquestador que consolida las lecturas de tres cámaras independientes y calcula el estado global del sistema.

### 2. Interfaz Gráfica en C (`main.c`)
La interacción humana se realiza mediante una aplicación de escritorio compilada nativamente:
* Interfaz desarrollada con la librería **GTK3** y estilizada dinámicamente mediante **CSS** para una experiencia de usuario moderna.
* Permite la entrada manual de sensores y avanza el estado del sistema mediante un botón de simulación iterativa que se comunica directamente con el código Lustre transpilado a C (`nevera_SistemaFrio_step`).
* Presenta un panel de resultados con formato enriquecido (Pango Markup) para la rápida visualización de diagnósticos y alertas.

## 🚀 Compilación y Ejecución

Para levantar la interfaz gráfica y conectar el motor lógico, se requiere el compilador de Lustre (v6) y las librerías de desarrollo de GTK3.

```bash
# 1. Instalar dependencias de GTK3 (Ubuntu/Debian)
sudo apt update
sudo apt install pkg-config libgtk-3-dev

# 2. Transpilar el código Lustre a C
lv6 nevera.lus -n SistemaFrio -2c

# 3. Compilar el ejecutable final
gcc main.c nevera_SistemaFrio.c -o panel_grafico `pkg-config --cflags --libs gtk+-3.0`

# 4. Ejecutar el panel de control
./panel_grafico
```

## 📊 Escenarios de Simulación

El simulador lógico (Nodo Main) y la interfaz están diseñados para auditar la robustez del sistema frente a cinco estados progresivos:

Instante 1: Operación normal de los sensores entre 2°C y 4°C.  

Instante 2: Deriva térmica lenta y continuada en la Cámara 1.  

Instante 3: Pico de calor en la Cámara 2 (>6°C) provocando una alarma instantánea de rango.  

Instante 4: Desplome térmico abrupto en la Cámara 3 (-2°C).  

Instante 5: Consolidación de la tendencia ascendente en la Cámara 1, activando exitosamente la alarma de desviación temporal.

## 🧑‍💻 Interfaz gráfica

<img width="827" height="517" alt="interfaz lustre" src="https://github.com/user-attachments/assets/487c036a-9823-49cd-bc32-b3ebfb932975" />

