
#include <FastLED.h>

#pragma region DEFINES
#define COLOR_ORDER GRB      // if colors are mismatched; change this
#define LED_TYPE    WS2812B
#define NUM_LEDS 60
#define DATA_PIN 5

#define BLEND_REPLACE 0
#define BLEND_ADD 1
#define BLEND_ALPHA 2

#define FPS 25

#pragma endregion

#pragma region GLOBAL DATA
struct BGLED
{
	CRGB COLOR;
};
struct FGLED
{
	CRGB COLOR;
	int ALPHA;
	uint8_t BLEND;
};

unsigned long currentTime;
unsigned int frameTime;
unsigned long Timer1;
int param[NUM_LEDS];
CRGB STRIP_LEDs[NUM_LEDS];

BGLED BGLEDS[NUM_LEDS];
FGLED FGLEDS[NUM_LEDS];

CRGB BG_COLOR = CRGB(2, 0, 7);
CRGB BG_COLOR2 = CRGB(0, 0, 0);

CRGB FG_COLOR = CRGB(164, 255, 0);
CRGB FG_COLOR2 = CRGB(164, 255, 0);
#pragma endregion



void(*FOREGROUND)();
void(*BACKGROUND)();

void setup()
{
	frameTime = 1000 / FPS;
	FOREGROUND = FG_FIREFLIES;
	BACKGROUND = BG_FLAT;
	FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(STRIP_LEDs, NUM_LEDS);
}
unsigned long long Index;

void loop()
{
	currentTime = millis();
	BACKGROUND();
	FOREGROUND();
	FlattenAndShow();
	while (millis() - currentTime < frameTime); //TEST: See if this helps the flickering problem.
}
bool Roll(unsigned int chance)
{
	return (random16() < chance);
}

void nop() {};
void FlattenAndShow()
{
	for (int i = 0; i < NUM_LEDS; i++)
	{
		switch (FGLEDS[i].BLEND)
		{
		case BLEND_ALPHA:
			STRIP_LEDs[i] = CRGB(lerp8by8(BGLEDS[i].COLOR.r, FGLEDS[i].COLOR.r, FGLEDS[i].ALPHA),
				lerp8by8(BGLEDS[i].COLOR.g, FGLEDS[i].COLOR.g, FGLEDS[i].ALPHA),
				lerp8by8(BGLEDS[i].COLOR.b, FGLEDS[i].COLOR.b, FGLEDS[i].ALPHA));
			break;
		case BLEND_ADD:
			STRIP_LEDs[i] = CRGB(qadd8(BGLEDS[i].COLOR.r, FGLEDS[i].COLOR.r),
				qadd8(BGLEDS[i].COLOR.g, FGLEDS[i].COLOR.g),
				qadd8(BGLEDS[i].COLOR.b, FGLEDS[i].COLOR.b));
			break;
		default:
			STRIP_LEDs[i] = FGLEDS[i].COLOR;
		}

	}
	FastLED.show();
}

#pragma region BACKGROUNDS


void BG_FLAT()
{
	for (int i = 0; i < NUM_LEDS; i++)
	{
		BGLEDS[i].COLOR = BG_COLOR;
	}
}

void BG_SLOW_SINE()
{

	for (int i = 0; i < NUM_LEDS; i++)
	{
		float offset = currentTime / 2000.0;
		offset += 0.3 * i;
		BGLEDS[i].COLOR = BG_COLOR.lerp16(BG_COLOR2, (sin(offset) * 16384 + 16384));
	}
}

#pragma endregion

#pragma region FOREGROUNDS



void FG_NONE()
{
	for (int i = 0; i < NUM_LEDS; i++)
	{
		FGLEDS[i].BLEND = BLEND_ADD;
		FGLEDS[i].COLOR = CRGB::Black;
	}
}



void FG_FIREFLIES()
{
	//TODO: Consider reversing the function of param so it increases instead of decreases. This may allow use of an unsigned variable.

	for (int i = 0; i < NUM_LEDS; i++)
	{
		FGLEDS[i].COLOR = FG_COLOR;
		FGLEDS[i].BLEND = BLEND_ALPHA;

		if (param[i] <= 0)
		{
			if (Roll(15))
			{
				param[i] = 255; //Create firefly
			}
			else
			{
				FGLEDS[i].ALPHA = 0; //Don't create firefly
			}
		}
		else
		{
			FGLEDS[i].ALPHA = 255 - cos8(param[i]); //We are using param to control the alpha channel via a function. (No idea why cosine instead of sine)
		}
		param[i] -= 8;
		if (param[i] < 0) { param[i] = 0; } //param must currently be signed in order to do sub-zero checks.
	}

}






#pragma endregion