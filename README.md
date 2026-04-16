# Actividad: Manejo de sensores y actuadores en IoT

## Descripción de la actividad

La plataforma Wokwi es una herramienta que permite simular componentes de sistemas IoT, lo que incluye sensores, dispositivos y módulos de comunicación.

En esta práctica crearemos un sistema de alarma con botones y leds que se activará cuando la temperatura o la humedad alcancen un umbral determinado. Para ello, accede a https://wokwi.com/ y crea una cuenta:

1. Selecciona el ESP32 como microcontrolador.
1. Agrega un sensor DHT22 a tu proyecto. 
1. Agrega dos botones y dos leds a tu proyecto.
1. Conecta los botones a los pines GPIO 26 y 27 del ESP32.
1. Conecta los leds a los pines GPIO 14 y 15 del ESP32.
1. En el código, define un umbral de temperatura y humedad que activará la alarma.
1. El led de alerta de temperatura se encenderá si la temperatura es superior a 45 grados.
1. El led de alerta de humedad se encenderá si la humedad no está entre el 25 % y el 60 %.
1. En el código, lee los valores del sensor DHT22.
1. Si la temperatura o la humedad superan el umbral definido, enciende los leds.
1. Si se presiona uno de los botones, apaga los leds.
1. El código debe verificar estas condiciones y actuar en consecuencia cada segundo.

## Arquitectura del proyecto

```sh
.
├── LICENSE
├── README.md
├── src
│   └── dht22-alarm
│       ├── CMakeLists.txt
│       ├── dependencies.lock
│       ├── sdkconfig
│       ├── main
│       │   ├── CMakeLists.txt
│       │   ├── dht22-alarm.c
│       │   └── idf_component.yml
│       └── managed_components
│           ├── esp-idf-lib__dht
│           └── esp-idf-lib__esp_idf_lib_helpers
└── wowki
    ├── diagram.json
    └── wokwi.toml
```

> **Notas:**
> - Se excluye `build/` → es generado automáticamente por `idf.py build`
> - `managed_components/` → dependencias manejadas por el sistema de componentes de ESP-IDF
> - `sdkconfig` → configuración específica del proyecto (puede o no versionarse dependiendo del flujo de trabajo)
> - `wokwi/` → contiene la simulación (Wokwi), separada del firmware real

## Guías de instalación de herramientas y drivers

- Proyecto base de Wokwi: [Wokwi Project Online](https://wokwi.com/projects/461468898633518081)
- Extensión para Visual Studio Code: [Wokwi Extension for VSCode](https://marketplace.visualstudio.com/items?itemName=Wokwi.wokwi-vscode)
- Instalación de ESPRESSIF en Linux: [EIM Installation on Linux](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-setup.html)
- DHT Driver: [ESP-IDF DHT Driver Component](https://components.espressif.com/components/esp-idf-lib/dht/versions/1.2.0/readme)

