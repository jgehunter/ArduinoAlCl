#include "Notas.h";

#define bt1 3 // Pin correspondiente al boton 1
#define bt2 2 // Pin correspondiente al boton 2
#define spk 11 // Pin correspondiente al altavoz
#define ledR	6 // Pin correspondiente al LED rojo
#define ledG	9 // Pin correspondiente al LED verde
#define ledB	5 // Pin correspondiente al LED azul
#define digSel	13 // Pin correspondiente selector entre los dos displays de 7 digitos


int programMode = 0; // Esta variable indica en que modo esta el reloj:
					 // 0-inicio, 1-programacion, 2-cuenta atras, 3-alarma

					 // Creamos un array con los valores que corresponeden a
					 // las 7 salidas del display
int displayPin[7] = { A2,10,8,4,7,A3,12 };

int estadoBT2 = HIGH; // Lectura del estado actual del boton 2
int estadoBT1 = HIGH; // Lectura del estado actual del boton 1
bool bt2Pulsado = false; // Variable que indica que el boton 2 estaba pulsado anteriormente
bool bt1Pulsado = false; // Variable que indica que el boton 1 estaba pulsado anteriormente
unsigned long ultimaComprobacionBT2 = 0; // Tiempo en el que comprobamos por ultima vez BT2
unsigned long ultimaComprobacionBT1 = 0; // Tiempo en el que comprobamos por ultima vez BT1
int esperaComprobacion = 30; // Tiempo que esperamos para comprobar un bot?n
int numeroAlarma = 0; // Numero de segundos que dura la alarma
unsigned long ultimoCambioAlarma = 0; // Tiempo en el que disminiuyo el contador de la alarma por ultima vez
unsigned long ultimoParpadeo = 0; // Tiempo en el que parpadeo el display por ultima vez
int frecuenciaParapedo = 1000; // Frecuencia con la que parpadea el display 
int numeroMostrado = 0; // Numero que se muestra por el display
int unidadDisplay = 0; // Unidad que se muestra en el display
int decenaDisplay = 0; // Decena que se muestra en el display
int counter = 0;

int partitura[] = { NOTE_B4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_D5, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_B4, NOTE_A4, NOTE_A4, NOTE_B4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_D5, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_B4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5, NOTE_B4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_A4, NOTE_D4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_D5, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_G4, NOTE_D5, NOTE_C5, NOTE_B4, NOTE_B4, NOTE_G5, NOTE_F5, NOTE_E5, NOTE_E5, NOTE_C6, NOTE_A5, NOTE_G5, NOTE_FS5, NOTE_A5, NOTE_FS5, NOTE_E5, NOTE_D5, NOTE_E5, NOTE_FS5, NOTE_G5, NOTE_B5, NOTE_A5, NOTE_FS5, NOTE_G5 };
int longitud[] = { 4,4,4,4,4,4,4,4,4,4,4,4,4,8,2,4,4,4,4,4,4,4,4,4,4,4,4,4,8,4,4,4,4,4,4,4,8,4,4,4,4,8,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,8,4,4,4,8,4,4,4,8,4,4,8,8,8,8,8,8,8,8,8,8,8,8,8,4 };
int pausaEntreNotas = 0;
unsigned long ultimaNota = 0;
int nota = 0;

int melody[] = {
	NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
int noteDurations[] = {
	4, 8, 8, 4, 4, 4, 4, 4
};

// Este funcion, basada en la que aparece en la practica muestra en el display
// de 7 segmentos el numero decimal 0-9 introducido en el display que se decida
// 0(izquierda) o 1(derecha). En el caso de introducir 100 se apaga el display.
void writeDisplay(int val, int digit) {

	// Primero apagamos todos los segmentos del digito que estuviera activo
	for (int i = 0; i < 7; i++) {
		digitalWrite(displayPin[i], LOW);
	}

	// Ahora activamos el digito deseado
	if (digit == 0) {
		digitalWrite(digSel, LOW);
	}
	else {
		digitalWrite(digSel, HIGH);
	}
	// Calculamos los segmentos que toca activar para mostrar 'val'
	// Para ello, codificaremos los segmentos activos con un byte (8 bits,
	// del 7 al 0) donde los bits 6..0 corresponden con los segmentos a..g
	char segments = 0; // Byte con los segmentos a activar (inicialmente ninguno)
	switch (val) {
		// abcdefg
	case 0: segments = 0b01111110; break; // segmentos del 0
	case 1: segments = 0b00110000; break; // segmentos del 1
	case 2: segments = 0b01101101; break; // segmentos del 2
	case 3: segments = 0b01111001; break; // segmentos del 3
	case 4: segments = 0b00110011; break; // segmentos del 4
	case 5: segments = 0b01011011; break; // segmentos del 5
	case 6: segments = 0b01011111; break; // segmentos del 6
	case 7: segments = 0b01110010; break; // segmentos del 7
	case 8: segments = 0b01111111; break; // segmentos del 8
	case 9: segments = 0b01111011; break; // segmentos del 9
	case 10: segments = 0b00000000; break; // segmentos de apagado
	default: segments = 0b00000001; // si no es ninguno, ponemos un guion
	}

	// Y por ?ltimo activamos los segmentos previamente obtenidos
	char currSegment = 0b01000000; // empezaremos por mirar el segmento 'a'
	for (int i = 0; i < 7; i++) {
		if ((currSegment & segments) != 0) { // si el bit del segmento es 1
			digitalWrite(displayPin[i], HIGH);
		}
		currSegment = currSegment >> 1; // desplazamos un lugar a derecha
	}

}

// Funcion que muestra por el display las unidades que tienen que mostrarse en cada momento
void mostrarPantallaUnidades() {
	writeDisplay(unidadDisplay, 0);
}

// Funcion que muestra por el display las decenas que tienen que mostrarse en cada momento
void mostrarPantallaDecenas() {
	writeDisplay(decenaDisplay, 1);
}

// Esta funcion detecta cuando hemos pulsado el boton 2, evitando
// el efecto rebote e incrementa el modo del programa
void deteccionBoton2() {
	estadoBT2 = digitalRead(bt2); //Leemos el valor de bt2
	if ((millis() - ultimaComprobacionBT2) > esperaComprobacion) {
		ultimaComprobacionBT2 = millis();
		if (bt2Pulsado == false && estadoBT2 == LOW) {
			bt2Pulsado = true;
			programMode++;
			if (programMode == 4) programMode = 0;
		}
		else if (estadoBT2 == HIGH) bt2Pulsado = false;
	}
}

// Esta funcion detecta cuando hemos pulsado el boton 1, incrementando
// el numero de segundos que dura la alarma siempre que estemos en el
// modo de programacion
void deteccionBoton1() {
	if (programMode == 1) {
		estadoBT1 = digitalRead(bt1);  // Leemos el valor de bt1
									   // Comprobamos si ha pasado un tiempo determinado
									   // desde la ultima vez que hemos comprobado el boton
									   // y solo si es asi lo volvemos a comprobar
		if ((millis() - ultimaComprobacionBT1) > esperaComprobacion) {
			ultimaComprobacionBT1 = millis(); // Marcamos el tiempo en el que hemos hecho la comprobacion
											  // Si detectamos un flanco y es de bajada
											  // ya que nuestro boton es pull-down actuamos
											  // y cambiamos el modo del programa
			if (bt1Pulsado == false && estadoBT1 == LOW) {
				bt1Pulsado = true;
				numeroAlarma++;
			}
		}
		else if (estadoBT1 == HIGH) bt1Pulsado = false;
	}
}



void setup() {
	Serial.begin(9600);
	pinMode(bt1, INPUT);
	pinMode(bt2, INPUT);
	pinMode(spk, OUTPUT);
	pinMode(ledR, OUTPUT);
	pinMode(ledG, OUTPUT);
	pinMode(ledB, OUTPUT);
	for (int i = 0; i < 7; i++) pinMode(displayPin[i], OUTPUT);
	pinMode(digSel, OUTPUT);

	attachPeriodicInterrupt(mostrarPantallaUnidades, 1);
	attachPeriodicInterrupt(mostrarPantallaDecenas, 2);
	attachPeriodicInterrupt(deteccionBoton2, 3);
	attachPeriodicInterrupt(deteccionBoton1, 4);
}

// En este modo de funcionamiento est? apagado el LED
// y el display muestra "00"
void inicio() {
	digitalWrite(ledR, LOW); digitalWrite(ledG, LOW); digitalWrite(ledB, LOW); // Apagamos el LED
	decenaDisplay = 0;
	unidadDisplay = 0;
	numeroAlarma = 0;
	digitalWrite(spk, LOW);
	noTone(spk);
}

// En este modo cada pulsacion del boton 1 aumenta el numero que se muestra por el display
void programacion() {
	digitalWrite(ledR, LOW); digitalWrite(ledG, HIGH); digitalWrite(ledB, LOW); // Mostramos verde por el LED
	if (numeroAlarma == 100) numeroAlarma = 0; // SI hemos pasado de 99 volvemos al 0
	unidadDisplay = numeroAlarma % 10; // Mostramos las unidades del numero
	decenaDisplay = numeroAlarma / 10; // Mostramos las decenas del numero
}

// El reloj va disminuyendo en una unidad cada vez que pasa un segundo
// y muestra esa cantidad en el display
void cuentaAtras() {
	digitalWrite(ledR, LOW); digitalWrite(ledG, LOW); digitalWrite(ledB, HIGH); // Mostramos azul por el LED
																				// Si ha pasado 1 segundo bajamos el contador
	if ((millis() - ultimoCambioAlarma) > 1000) {
		ultimoCambioAlarma = millis();
		numeroAlarma--;
	}
	unidadDisplay = numeroAlarma % 10;
	decenaDisplay = numeroAlarma / 10;
	if (numeroAlarma == 0) programMode = 3;
}

// En este modo el display parpadea mostrando "00" y suena un tono
void alarma() {
	digitalWrite(ledR, HIGH); digitalWrite(ledG, LOW); digitalWrite(ledB, LOW); // Mostramos rojo por el LED
																				// Cada vez que pasa un tiempo determinado cambiamos el estado del display
	if ((millis() - ultimoParpadeo) > frecuenciaParapedo) {
		ultimoParpadeo = millis();
		if (numeroMostrado == 0) {
			numeroMostrado = 10;
		}
		else numeroMostrado = 0;
	}
	unidadDisplay = numeroMostrado;
	decenaDisplay = numeroMostrado;
	if (millis() - ultimaNota > ((1000 / longitud[nota])*1.5)) {
		ultimaNota = millis();
		tone(spk, partitura[nota], 1000 / longitud[nota]);
		nota++;
		if (nota > 86) nota = 0;
	}
}

void loop() {
	switch (programMode) {
	case 0: inicio(); break;
	case 1: programacion(); break;
	case 2: cuentaAtras(); break;
	case 3: alarma(); break;
	}
}
