#include <FastLED.h>

#define LED_PIN             6
#define BUTTON_PIN          12    // Switch connected between Pin 12 and Ground
#define NUM_LEDS            30    // Adjust to the exact number of LEDs on your forearm strip
#define BRIGHTNESS          50   // Overall max brightness (0 to 255)
#define LED_TYPE            WS2812B
#define COLOR_ORDER         GRB   // BTF WS2812B strips use GRB order

// ============================================================================
// CONFIGURABLE SETTINGS (Adjust your animation behaviors and colors here)
// ============================================================================
#define TRAIL_LENGTH        5     // Active charge-up block size & trail length
#define CHARGE_CYCLES       3     // Number of times the charge-up sequence repeats
#define CHARGE_SPEED_DELAY  60    // Starting slow speed (ms) for the charge-up effect
#define FLOW_SPEED_DELAY    10    // Fast speed (ms) for the electric flow effect 
#define BREATH_BPM          15     // Breath rate (Beats Per Minute)
#define BREATH_MIN          100   // Lowest brightness floor
#define BREATH_MAX          255   // Peak brightness ceiling

#define COLOR_BACKGROUND    CRGB(0, 40, 80)       // Base color for breathing
#define COLOR_TRAIL         CRGB(0, 80, 180)      // Uniform color for the trail
#define COLOR_LEAD_BRIGHT   CRGB(255, 255, 255)   // Brightest head / Final charge color

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  // 1. Idle / Standby State: Continuously breathe until switch is closed (pulled LOW)
  while (digitalRead(BUTTON_PIN) == HIGH) {
    animationIdleBreath();
  }

  // 2. Triggered! Run the accelerating middle-out charge-up sequence 
  animationChargeUp(TRAIL_LENGTH, CHARGE_CYCLES);
  
  // 3. Transition seamlessly into the flowing electric animation
  for (int i = 0; i < 3; i++) {
    animationElectricFlow(FLOW_SPEED_DELAY);
  }
}

// ============================================================================
// HELPER: Ultra-smooth independent breathing frame generator
// ============================================================================
CRGB getBreathColor() {
  // Using millis() directly guarantees a mathematically fluid wave 
  // completely decoupled from animation loop speeds or delays.
  uint16_t rawBeat = beatsin16(BREATH_BPM, 0, 65535);
  uint8_t breathVal = map(rawBeat, 0, 65535, BREATH_MIN, BREATH_MAX);
  
  CRGB breathColor = COLOR_BACKGROUND;
  breathColor.nscale8(breathVal);
  return breathColor;
}

// ============================================================================
// ANIMATION 0: Standby Breathing Effect
// ============================================================================
void animationIdleBreath() {
  fill_solid(leds, NUM_LEDS, getBreathColor());
  FastLED.show();
}

// ============================================================================
// ANIMATION 1: Accelerating Middle-Out Charge-Up Effect
// ============================================================================
void animationChargeUp(int activeLEDs, int cycles) {
  int midRight = activeLEDs / 2;
  int midLeft  = (activeLEDs - 1) / 2;
  int maxDistance = max(midRight, (activeLEDs - 1) - midRight);

  for (int c = 0; c < cycles; c++) {
    for (int p = 0; p <= 255; p += 8) {
      
      // Fill background with the live, continuous breathing color
      fill_solid(leds, NUM_LEDS, getBreathColor());
      
      // Overlay the expanding charge-up sequence on top
      for (int i = 0; i < activeLEDs; i++) {
        int dist = (i <= midLeft) ? (midLeft - i) : (i - midRight);
        int startP = dist * (255 / (maxDistance + 1));
        
        if (p >= startP) {
          int localBrightness = map(p, startP, 255, 0, 255);
          localBrightness = constrain(localBrightness, 0, 255);
          
          leds[i] = CRGB(
            (COLOR_LEAD_BRIGHT.r * localBrightness) / 255,
            (COLOR_LEAD_BRIGHT.g * localBrightness) / 255,
            (COLOR_LEAD_BRIGHT.b * localBrightness) / 255
          );
        }
      }
      
      float progress = (float)p / 255.0;
      int currentDelay = CHARGE_SPEED_DELAY - (progress * (CHARGE_SPEED_DELAY - FLOW_SPEED_DELAY));
      
      FastLED.show();
      delay(currentDelay);
    }
    delay(150);
  }
}

// ============================================================================
// ANIMATION 2: Flowing "Electricity" Chasing Wave (Continuous Breathing Background)
// ============================================================================
void animationElectricFlow(uint8_t speedDelay) {
  for (int i = TRAIL_LENGTH; i < NUM_LEDS; i++) {
    // 1. Keep background breathing continuously instead of hard-resetting
    fill_solid(leds, NUM_LEDS, getBreathColor()); 
    
    // 2. Draw the solid, uniform trail behind the lead pixel
    for (int t = 1; t <= TRAIL_LENGTH; t++) {
      int trailIndex = i - t;
      if (trailIndex >= 0) {
        leds[trailIndex] = COLOR_TRAIL;
      }
    }
    
    // 3. Lead pulse (brightest point)
    if (i < NUM_LEDS) {
      leds[i] = COLOR_LEAD_BRIGHT; 
    }
    
    FastLED.show();
    delay(speedDelay);
  }
}