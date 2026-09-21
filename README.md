# Control de Motor DC con DRV8833 y Joystick Analógico

Este proyecto controla la velocidad y dirección de un motor DC (5V–9V) usando el driver **DRV8833** y un **joystick analógico**, leído desde un Arduino.

---

## 1. ¿Qué es el DRV8833?

El **DRV8833** es un driver (puente H doble) para motores DC, fabricado por Texas Instruments. Permite controlar hasta **2 motores DC** de forma independiente, o **1 motor paso a paso (stepper)**, usando señales lógicas de bajo voltaje (3.3V–5V) provenientes de un microcontrolador como Arduino.

### ¿Por qué se necesita un driver y no se conecta el motor directo al Arduino?

- Los pines del Arduino solo pueden entregar corrientes muy bajas (~20-40 mA máx), insuficientes para mover un motor.
- Los motores DC suelen requerir voltajes más altos (5V–9V o más) que los 5V lógicos del Arduino.
- El DRV8833 actúa como "interruptor de potencia": el Arduino le dice *qué hacer* con señales pequeñas, y el driver entrega la corriente/voltaje real al motor desde una fuente externa.

### Características principales

- 2 puentes H independientes → controla 2 motores DC (o 1 stepper).
- Voltaje de alimentación de motor: **2.7V a 10.8V** (por eso funciona perfecto con motores de 5V–9V).
- Corriente de salida: hasta ~1.5A por canal (picos más altos).
- Control por PWM para variar la velocidad.
- Protecciones integradas: sobrecorriente, sobretemperatura y bajo voltaje.

---

## 2. Distribución de pines (Pinout) del DRV8833

El módulo usado en este proyecto tiene **12 pines en total**: 4 entradas, 4 salidas, GND, VCC, EEP y ULT. **No tiene un pin VM separado**: la alimentación del motor y la lógica comparten el mismo pin **VCC**.

| Pin del módulo | Tipo | Función | Descripción |
|---|---|---|---|
| **IN1** | Entrada | Control motor A | Señal lógica/PWM desde el Arduino. |
| **IN2** | Entrada | Control motor A | Señal lógica/PWM desde el Arduino. |
| **IN3** | Entrada | Control motor B | Señal lógica/PWM desde el Arduino (si usas un segundo motor). |
| **IN4** | Entrada | Control motor B | Señal lógica/PWM desde el Arduino (si usas un segundo motor). |
| **OUT1** | Salida | Salida motor A | Se conecta a uno de los cables del motor A. |
| **OUT2** | Salida | Salida motor A | Se conecta al otro cable del motor A. |
| **OUT3** | Salida | Salida motor B | Se conecta a uno de los cables del motor B (si usas un segundo motor). |
| **OUT4** | Salida | Salida motor B | Se conecta al otro cable del motor B. |
| **GND** | Alimentación | Tierra | Tierra común para el motor, la fuente externa y el Arduino. |
| **VCC** | Alimentación | Alimentación general (lógica + motor) | En este módulo **no hay VM separado**: aquí se conecta la fuente que alimentará tanto el chip como el motor (6V–9V). |
| **EEP** (nSLEEP) | Entrada | Habilitación / Sleep | Pone al chip en modo activo o en reposo. Normalmente el módulo trae un puente (jumper) en la parte de abajo que lo conecta internamente a VCC. **Si el módulo no trae ese puente** (como en este caso), hay que conectarlo manualmente a 5V; de lo contrario el driver se queda dormido y el motor no responde. |
| **ULT** (nFAULT) | Salida | Reporte de fallas | Salida tipo *open-drain* que normalmente está en HIGH. Cae a LOW cuando el driver detecta una falla (sobrecorriente, sobretemperatura, bajo voltaje), permitiendo que el microcontrolador (uC) se entere del error. Es opcional conectarlo a una entrada digital del Arduino para detectar fallas por software; si no se usa, se puede dejar sin conectar. |

> ⚠️ Nota: la serigrafía puede variar según el fabricante del módulo, pero en breakouts DRV8833 sin VM separado, es común que **VCC alimente tanto la lógica como el puente H**, por lo que no debe superarse el voltaje máximo del motor (10.8V) al alimentar este pin.

---

## 3. Lógica de control del puente H (por canal)

Cada motor se controla con **2 pines** (por ejemplo AIN1 y AIN2 para el motor A). La combinación de estados determina el comportamiento:

| IN1 | IN2 | Comportamiento del motor |
|---|---|---|
| LOW | LOW | Motor libre (coast / sin frenado activo) |
| HIGH | LOW | Motor gira en un sentido (velocidad = HIGH lógico) |
| LOW | HIGH | Motor gira en sentido contrario |
| HIGH | HIGH | Freno activo (brake) |
| PWM | LOW | Gira en un sentido, con velocidad variable según el duty cycle del PWM |
| LOW | PWM | Gira en sentido contrario, con velocidad variable |

Esto es exactamente lo que hace el código: usa `analogWrite()` (PWM) en un pin mientras el otro está en `LOW`, para variar la velocidad en cada sentido.

---

## 4. Cómo conectar todo (diagrama de conexión)

```
                 ┌───────────────────┐
   Fuente 5-9V ──┤ VCC           AIN2├── Pin 6 (I1) Arduino
   (Batería/     │                   │
    fuente ext.) │ DRV8833       AIN2├── Pin 3 (I2) Arduino
                 │                   │
        GND ─────┤ GND          AOUT1├──┐
                 │                   │  │
   Arduino 5V ───┤ EEP          AOUT2├──┤  Motor DC (5-9V)
                 │                   │  │
   Arduino pin ──┤ ULT (opcional)    │──┘
   digital (in)  └───────────────────┘

   Joystick:
     VCC  -> 5V Arduino
     GND  -> GND Arduino
     VRx  -> A0 (Arduino)  [eje usado en el código]
```

### Pasos de conexión

1. **Alimentación (motor + lógica):**
   - Como este módulo no tiene VM separado, conecta el **positivo** de tu fuente externa (5V–9V) a **VCC** del DRV8833.
   - Conecta el **negativo/GND** de esa fuente a **GND** del DRV8833, y une ese mismo GND al **GND del Arduino** (tierra común obligatoria).
   - Ten en cuenta que este mismo VCC alimentará también la parte lógica del chip, así que no debe superar los 10.8V.

2. **Habilitación del chip (EEP / nSLEEP):**
   - Si tu módulo trae el puente (jumper) soldado en la parte inferior, este pin ya está conectado internamente a VCC y puedes ignorarlo.
   - Si **no** trae el puente (como en este proyecto), conecta **EEP** al **5V del Arduino** para sacar al chip del modo sleep. Si lo dejas flotando o en LOW, el driver no va a responder aunque el código esté enviando señales PWM.

3. **Reporte de fallas (ULT / nFAULT) — opcional:**
   - Puedes conectar **ULT** a un pin digital de entrada del Arduino para detectar sobrecorriente, sobretemperatura o bajo voltaje por software (leyendo si el pin cae a LOW).
   - Si no te interesa monitorear fallas, puedes dejarlo sin conectar; el driver funciona igual.

4. **Señales de control (según el código):**
   - `IN1` → Pin digital **6** del Arduino (`I1`).
   - `IN2` → Pin digital **3** del Arduino (`I2`).
   - Ambos pines (6 y 3) deben soportar PWM (en Arduino Uno/Nano, los pines PWM llevan el símbolo `~`).

5. **Motor:**
   - Conecta los dos cables del motor a **AOUT1** y **AOUT2**.
   - No importa cuál cable va a cuál salida; si el motor gira al revés de lo esperado, simplemente invierte los dos cables.

6. **Joystick:**
   - `VCC` del joystick → 5V del Arduino.
   - `GND` del joystick → GND del Arduino.
   - `VRx` (o el eje que uses) → **A0** del Arduino (`JS` en el código).

> ⚠️ **Importante:** El Arduino NO debe alimentar el motor directamente desde su pin 5V. La fuente de VCC del driver debe ser una fuente externa capaz de entregar la corriente que el motor necesita. Si usas una sola fuente para todo (por ejemplo un power bank o batería de 9V), asegúrate de que las tierras (GND) de todo el sistema estén unidas.

---

## 5. Usando un motor de 5V–9V

El DRV8833 acepta un rango de voltaje de motor (**VM**) de aproximadamente **2.7V a 10.8V**, así que un motor de 5V–9V trabaja perfectamente dentro de su rango seguro.

Recomendaciones:

- **Verifica la corriente nominal del motor.** El DRV8833 entrega hasta ~1.5A por canal de forma continua (según el módulo y disipación de calor). Si tu motor consume más que eso en arranque o carga, el driver puede entrar en protección por sobrecorriente o dañarse.
- **Este módulo no separa VM de la lógica**, así que la fuente que conectes a VCC alimenta tanto al motor como al chip. Evita usar directamente el 5V del Arduino como fuente de VCC si el motor consume corriente considerable; usa una fuente externa (batería o fuente regulada) capaz de entregar esa corriente.
- **Añade un capacitor** (por ejemplo 100–470 µF) cerca de VCC y GND del driver si notas reinicios del Arduino o ruido eléctrico al arrancar el motor.
- **No excedas 10.8V** en VCC; si tu fuente entrega más voltaje que el máximo soportado por el motor o el chip, usa un regulador de voltaje.
- Aprovecha el pin **ULT (nFAULT)** si quieres que tu código detecte automáticamente cuando el driver entra en protección, por ejemplo por exceso de corriente al frenar bruscamente.

---

## 6. Explicación rápida del código

- El programa lee el joystick (`analogRead(JS)`) **10 veces** (`LECTURAS`) y promedia el valor para reducir el ruido de la lectura analógica.
- El joystick entrega valores de **0 a 1023**:
  - **Valor cercano a 0-400:** el joystick está inclinado hacia un lado → el motor gira en un sentido, con velocidad proporcional a qué tan lejos está del centro.
  - **Valor cercano a 600-1023:** el joystick está inclinado hacia el otro lado → el motor gira en sentido contrario.
  - **Entre 400 y 600 (zona muerta):** se considera "centro/reposo" → el motor se detiene.
- La velocidad (0–255) se calcula con `map()`, que convierte el rango de lectura del joystick al rango de PWM que entiende `analogWrite()`.
- `constrain()` asegura que el valor de PWM nunca se salga del rango válido (0–255), evitando comportamientos erráticos.

---

## 7. Resumen de pines usados en este proyecto

| Función | Pin Arduino | Constante en código |
|---|---|---|
| Control motor - dirección 1 (PWM) | Pin 6 | `I1` |
| Control motor - dirección 2 (PWM) | Pin 3 | `I2` |
| Lectura joystick (eje X o Y) | A0 | `JS` |

---


