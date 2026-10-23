#include <LiquidCrystal.h>

// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(13, 14, 27, 26, 32, 12);

// Pines
#define FSR 34
#define VENTILADOR 33

// Datos de calibracion de la balanza
const float adcCal[] = {0, 2882, 3136, 3287};
const float pesoCal[] = {0, 520, 2000, 3000};

// Peso de la tabla en gramos
const float pesoTabla = 370.0;

// Filtro de peso
float pesoFiltrado = 0;
const float alpha = 0.15;

// Temporizadores
unsigned long tiempoADC = 0;
unsigned long tiempoLCD = 0;
unsigned long tiempoVentilador = 0;

// Control del ventilador
bool ventiladorEncendido = true;

const unsigned long tiempoEncendido = 6000;
const unsigned long tiempoApagado = 4000;

// Convertir ADC a gramos
float calcularPeso(float adc) {

  if (adc <= adcCal[0]) return 0;
  if (adc >= adcCal[3]) return 3400;

  for (int i = 0; i < 3; i++) {
    if (adc <= adcCal[i + 1]) {

      return pesoCal[i] +
             (adc - adcCal[i]) *
             (pesoCal[i + 1] - pesoCal[i]) /
             (adcCal[i + 1] - adcCal[i]);
    }
  }

  return 0;
}

void setup() {

  Serial.begin(115200);

  // Configurar balanza
  analogReadResolution(12);
  analogSetPinAttenuation(FSR, ADC_11db);

  // Configurar ventilador
  pinMode(VENTILADOR, OUTPUT);
  digitalWrite(VENTILADOR, LOW);

  // Inicializar LCD
  lcd.begin(16, 2);

  lcd.setCursor(0, 0);
  lcd.print("BALANZA DIGITAL");

  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");

  delay(1500);
  lcd.clear();

  // Iniciar ventilador
  digitalWrite(VENTILADOR, HIGH);
  ventiladorEncendido = true;
  tiempoVentilador = millis();

  Serial.println("Ventilador ENCENDIDO");
}

void loop() {

  unsigned long tiempoActual = millis();

  // =====================================
  // CONTROL DEL VENTILADOR
  // =====================================

  if (ventiladorEncendido) {

    if (tiempoActual - tiempoVentilador >= tiempoEncendido) {

      digitalWrite(VENTILADOR, LOW);
      ventiladorEncendido = false;
      tiempoVentilador = tiempoActual;

      Serial.println("Ventilador APAGADO");
    }

  } else {

    if (tiempoActual - tiempoVentilador >= tiempoApagado) {

      digitalWrite(VENTILADOR, HIGH);
      ventiladorEncendido = true;
      tiempoVentilador = tiempoActual;

      Serial.println("Ventilador ENCENDIDO");
    }
  }

  // =====================================
  // LECTURA DE LA BALANZA
  // =====================================

  if (tiempoActual - tiempoADC >= 20) {

    tiempoADC = tiempoActual;

    int adc = analogRead(FSR);

    // Descontar los 370 g de la tabla
    float peso = calcularPeso(adc) - pesoTabla;

    // Evitar pesos negativos
    if (peso < 0) {
      peso = 0;
    }

    // Filtro para suavizar las lecturas
    pesoFiltrado = alpha * peso +
                   (1.0 - alpha) * pesoFiltrado;

    // Mostrar datos en monitor serial
    Serial.print("ADC: ");
    Serial.print(adc);

    Serial.print(" | Peso: ");
    Serial.print(pesoFiltrado, 1);
    Serial.print(" g");

    Serial.print(" | Ventilador: ");

    if (ventiladorEncendido) {
      Serial.println("ENCENDIDO");
    } else {
      Serial.println("APAGADO");
    }
  }

  // =====================================
  // ACTUALIZAR PANTALLA LCD
  // =====================================

  if (tiempoActual - tiempoLCD >= 200) {

    tiempoLCD = tiempoActual;

    lcd.setCursor(0, 0);
    lcd.print("BALANZA DIGITAL ");

    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);

    if (pesoFiltrado >= 1000) {

      lcd.print("Peso: ");
      lcd.print(pesoFiltrado / 1000.0, 3);
      lcd.print(" kg");

    } else {

      lcd.print("Peso: ");
      lcd.print(pesoFiltrado, 1);
      lcd.print(" g");
    }
  }
}
