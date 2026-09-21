#define izquierda 8
#define arriba 9
#define abajo 10
#define derecha 11

// Variables para almacenar el estado actual
int izq_actual = 0;
int arr_actual = 0;
int aba_actual = 0;
int dere_actual = 0;

void setup() {
  Serial.begin(115200);
  pinMode(izquierda, OUTPUT);
  pinMode(arriba, OUTPUT);
  pinMode(abajo, OUTPUT);
  pinMode(derecha, OUTPUT);

  // Inicializar todos los LEDs apagados
  apagarTodosLEDs();
  Serial.println("Arduino listo - Esperando datos...");
}

void loop() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    data.trim();
    
    // Debug: mostrar datos recibidos
    Serial.print("Recibido: ");
    Serial.println(data);
    
    int comma1 = data.indexOf(',');
    int comma2 = data.indexOf(',', comma1 + 1);
    int comma3 = data.indexOf(',', comma2 + 1);
    int comma4 = data.indexOf(',', comma3 + 1);

    if (comma1 != -1 && comma2 != -1 && comma3 != -1 && comma4 != -1) {
      int izq = data.substring(0, comma1).toInt();
      int arr = data.substring(comma1 + 1, comma2).toInt();
      int aba = data.substring(comma2 + 1, comma3).toInt();
      int dere = data.substring(comma3 + 1, comma4).toInt();
      int presionado = data.substring(comma4 + 1).toInt();

      // Solo actualizar si hay cambios (optimización)
      if (izq != izq_actual || arr != arr_actual || aba != aba_actual || dere != dere_actual) {
        
        // Aplicar estados a los LEDs
        digitalWrite(izquierda, izq ? HIGH : LOW);
        digitalWrite(arriba, arr ? HIGH : LOW);
        digitalWrite(abajo, aba ? HIGH : LOW);
        digitalWrite(derecha, dere ? HIGH : LOW);
        
        // Actualizar estado actual
        izq_actual = izq;
        arr_actual = arr;
        aba_actual = aba;
        dere_actual = dere;
        
        // Mostrar estado en monitor serial
        Serial.print("LEDs: ");
        Serial.print(izq ? "IZQ " : "");
        Serial.print(arr ? "ARR " : "");
        Serial.print(aba ? "ABA " : "");
        Serial.print(dere ? "DER " : "");
        Serial.println(presionado ? "| BOTON" : "| ---");
      }
    } else {
      Serial.println("Error: Formato de datos incorrecto");
    }
  }
}

void apagarTodosLEDs() {
  digitalWrite(izquierda, LOW);
  digitalWrite(arriba, LOW);
  digitalWrite(abajo, LOW);
  digitalWrite(derecha, LOW);
  
  // Resetear estados actuales
  izq_actual = 0;
  arr_actual = 0;
  aba_actual = 0;
  dere_actual = 0;
}