#define USE_MICRO_WIRE
#include "microWire.h"
#include "NanoOLED.h"
#include "GyverINA.h"
#include "TimerFF.h"
#include "ButtonFF.h"

#define FW_VER "1.0"
#define NO_LOAD 11111

#define BUZZER 14
#define MOSFET 0

#define DO		262  // C4 - до
#define	DOd		277  // до диез
#define RE		294  // D4 - ре
#define	REd		311  // ре диез
#define MI		330  // E4 - ми
#define FA		349  // F4 - фа
#define	FAd		370  // фа диез
#define SOL		392  // G4 - соль
#define	SOLd	415  // соль диез
#define LA		440  // A4 - ля
#define	LAd		466  // ля диез
#define SI		494  // B4 - си

NanoOLED oled;
INA219 ina;
ButtonFF start_rst(15);
ButtonFF U_inc(PIN_A6);
ButtonFF U_dec(16);

TimerFF task100ms(100);
TimerFF task50ms(50);
TimerFF task1000ms(1000);

typedef enum {
    pause,
    work,
} mods;

// Time
struct {
uint8_t sec = 0;
uint8_t min = 0;
uint8_t hour = 0;
} time;

// Battery
struct {
float capacity = 0;
float voltage = 0;
float end_voltage = 3.1;
float current = 0;
float power = 0;
float energy = 0;
uint16_t load = 0;
} battery;

// Other
mods mode = pause;
uint8_t cnt_work = 0;
bool oled_off = false;
uint8_t oled_off_cnt = 0;

void oled_turn_on() {
    oled.setBrightness(100);
    oled_off = false;
    oled_off_cnt = 0;
}

void oled_turn_off() {
    oled.setBrightness(0);
    oled_off = true;
    oled_off_cnt = 0;
}

void load_off() {
    mode = pause;
    digitalWrite(MOSFET, LOW);
}

void load_on() {
    mode = work;
    digitalWrite(MOSFET, HIGH);
}

void working_animation() {
    oled.setCursor(120, 0);
    switch (cnt_work) {
        case 0: oled.print(F("/")); break; 
        case 1: oled.print(F("-")); break;
        case 2: oled.print(F("\\")); break;  
        case 3: oled.print(F("|")); break;
    }
    if(++cnt_work > 3) cnt_work = 0;
}

void time_handler() {
    if(mode == pause) return;
    time.sec++;
    if(time.sec >= 60) {
        time.sec = 0;
        time.min++;
    }
    if(time.min >= 60) {
        time.min = 0;
        time.hour++;
    }
}

void button_handler() {
    start_rst.tick();
    U_dec.tick();
    U_inc.tick();

    if(oled_off == true && (start_rst.release() || U_inc.release() || U_dec.release())) {
        oled_turn_on();
        return;
    }
    
    if(start_rst.release()) {
        if(mode == pause) load_on();
        else load_off();
        oled_off_cnt = 0;
    }
    if(start_rst.hold()) {
        time.sec = time.min = time.hour = 0;
        battery.capacity = 0;
        battery.energy = 0;
        load_off();
    }
    if(U_inc.release()) {
        battery.end_voltage += 0.05;
        oled_off_cnt = 0;
    }
    if(U_inc.hold()) battery.end_voltage += 0.5;
    if(U_dec.release()) {
        battery.end_voltage -= 0.05;
    }
    if(U_dec.hold()) battery.end_voltage -= 0.5;
    battery.end_voltage = constrain(battery.end_voltage, 0, 4.2);
}

void draw_text() {
    //string 0
    oled.setCursor(0, 0);
    oled.print(F("Time:"));
    //string 1
    oled.setCursor(0, 1);
    oled.print(F("Capacity:"));
    //string 2
    oled.setCursor(0, 2);
    oled.print(F("Energy:"));
    //string 3
    oled.setCursor(0, 3);
    oled.print(F("Voltage:"));
    //string 4
    oled.setCursor(0, 4);
    oled.print(F("Cut-off:"));
    //string 5
    oled.setCursor(0, 5);
    oled.print(F("Current:"));
    //string 6
    oled.setCursor(0, 6);
    oled.print(F("Load:"));
    //string 7
    oled.setCursor(0, 7);
    oled.print(F("     /RESET | U- | U+"));
}

void draw_values() {
    constexpr uint8_t x_start = 6 * 11; // ширина_символа * позиция
    //string 0
    oled.setCursor(x_start, 0);
    if(time.hour < 10) oled.print("0");
    oled.print(time.hour);
    oled.print(F(":"));
    if(time.min < 10) oled.print("0");
    oled.print(time.min);
    oled.print(F(":"));
    if(time.sec < 10) oled.print("0");
    oled.print(time.sec);
    //string 1
    oled.setCursor(x_start, 1);
    if(battery.capacity < 1000) oled.print(" ");
    if(battery.capacity < 100) oled.print(" ");
    if(battery.capacity < 10) oled.print(" ");
    oled.print((uint16_t)battery.capacity);
    oled.print(F(" mAh "));
    //string 2
    oled.setCursor(x_start, 2);
    oled.print(battery.energy, 2);
    oled.print(F(" Wh "));
    //string 3
    oled.setCursor(x_start, 3);
    oled.print(battery.voltage, 2);
    oled.print(F(" V "));
    //string 4
    oled.setCursor(x_start, 4);
    oled.print(battery.end_voltage, 2);
    oled.print(F(" V "));
    //string 5
    oled.setCursor(x_start, 5);
    oled.print(battery.current, 2);
    oled.print(F(" A "));
    //string 6
    oled.setCursor(x_start, 6);
    if(battery.load >= NO_LOAD) oled.print(F("NO LOAD  "));
    else {
        if(battery.load < 1000) oled.print(" ");
        if(battery.load < 100) oled.print(" ");
        if(battery.load < 10) oled.print(" ");
        oled.print(battery.load);
        oled.print(F(" Ohm "));
    }
    //string 7
    oled.setCursor(0, 7);
    if(mode == pause) oled.print(F("START"));
    if(mode == work) oled.print(F("PAUSE"));
}

void bootup_screen() {
    oled.clearDisplay();
    //oled.setScale(2);
    oled.setCursor((127 - (6 * 1 * 6)) / 2, 2); // (ширина_дисплея - (ширина_символа * масштаб * длина_строки)) / 2
    oled.print(F("Attiny"));
    oled.setCursor((127 - (6 * 1 * 14)) / 2, 3);
    oled.print(F("Battery Tester"));
    //oled.setScale(1);
    oled.setCursor(0, 7);
    oled.print(F("BetaFF           v"));
    oled.print(F(FW_VER));
    delay(2000);
    oled.clearDisplay();
}

void play_note(uint8_t pin, uint16_t frequency, uint16_t duration_ms) {
    if (frequency == 0) {
		delay(duration_ms);  // Пауза
		return;
	}
	uint16_t half_period_us = 1000000 / (frequency * 2);  // С учетом HIGH/LOW для одного шага
	uint32_t total_steps = (frequency * (uint32_t)duration_ms) / 1000;
	for (uint32_t i = 0; i < total_steps; i++) {
		digitalWrite(pin, HIGH);
		delayMicroseconds(half_period_us);
		digitalWrite(pin, LOW);
		delayMicroseconds(half_period_us);
	}
}

void play_song(uint8_t octave) {
    play_note(BUZZER, DO * octave, 200);
    play_note(BUZZER, MI * octave, 200);
    play_note(BUZZER, SOL * octave, 200);
    play_note(BUZZER, MI * octave, 200);
    play_note(BUZZER, DO * octave, 200);
}

void setup() {
    pinMode(MOSFET, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    load_off();

    Wire.begin();
    oled.init();
    oled_turn_on();
    bootup_screen();

    ina.begin();
    ina.setResolution(INA219_VBUS, INA219_RES_12BIT_X8);      // Напряжение в 12ти битном режиме + 8х кратное усреднение
    ina.setResolution(INA219_VSHUNT, INA219_RES_12BIT_X64);  // Ток в 12ти битном режиме + 64х кратное усреднение
    Wire.setClock(400000);

    draw_text();
}

void loop() {
    if(task50ms.ready()) {
        if (mode == work) working_animation();
        button_handler();
    }
    if(task100ms.ready()) {
        battery.voltage = ina.getVoltage() + 0.04; // + 0.04 - моя INA врет
        battery.current = constrain(ina.getCurrent() * 0.95, 0, 100); // * 0.95 - моя INA врет 
        //battery.power = ina.getPower();
        battery.power = battery.voltage * battery.current;
        if(battery.current != 0.0) battery.load = (uint16_t)(battery.voltage / battery.current);
        else battery.load = NO_LOAD;
        draw_values();
        if(mode == work && (battery.voltage < battery.end_voltage)) {
            load_off();
            play_song(4);
        }
    }
    if(task1000ms.ready()) {
        time_handler();
        if(mode == work){
            battery.capacity += (battery.current * task1000ms.getPeriod() / 3600); //расчет емкости АКБ в мАч
            battery.energy += battery.power * task1000ms.getPeriod() / 3600000; //расчет емкости АКБ в ВтЧ
        }
        if(oled_off == false) oled_off_cnt++;
        if(oled_off_cnt > 60 * 2) oled_turn_off(); // 2 минуты
    }
}
