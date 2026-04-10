#include <ioCC2530.h>
#include "hal_types.h"
#include "hal_mcu.h"
#include "DHT11.h"

#define DHT11_DATA      P2_0
#define DHT11_PIN_MASK  0x01

uint8 DHT11_LastErr = 0;
uint8 DHT11_LastRaw[5] = {0, 0, 0, 0, 0};

static uint8 dht11ByteError = 0;
static uint8 dht11Ready = 0;

static void delay_us(void)
{
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
}

static void delay_10us(void)
{
  delay_us();
  delay_us();
}

static void delay_ms(uint16 time)
{
  uint16 i;
  uint8 j;

  for(i = 0; i < time; i++)
  {
    for(j = 0; j < 100; j++)
    {
      delay_10us();
    }
  }
}

static uint8 dht11_wait_for_level(uint8 level, uint16 timeout)
{
  while(timeout > 0)
  {
    if((uint8)DHT11_DATA == level)
    {
      return 1;
    }

    delay_us();
    timeout--;
  }

  return 0;
}

static uint8 dht11_read_byte(uint8 index)
{
  uint8 i;
  uint8 value = 0;

  dht11ByteError = 0;

  for(i = 0; i < 8; i++)
  {
    if(!dht11_wait_for_level(1, 100))
    {
      DHT11_LastErr = 0x10 + index;
      dht11ByteError = 1;
      return 0;
    }

    delay_10us();
    delay_10us();
    delay_10us();

    value <<= 1;
    if(DHT11_DATA)
    {
      value |= 0x01;
    }

    if(!dht11_wait_for_level(0, 100))
    {
      DHT11_LastErr = 0x20 + index;
      dht11ByteError = 1;
      return 0;
    }
  }

  return value;
}

void DHT11_Init(void)
{
  P2SEL &= ~DHT11_PIN_MASK;
  P2DIR |= DHT11_PIN_MASK;
  DHT11_DATA = 1;

  /* Per the documentation, allow 1s to stabilize after power-on. */
  delay_ms(1000);
  dht11Ready = 1;
}

uint8 DHT11_Read(uint8 *temperature, uint8 *humidity)
{
  uint8 humidityHigh;
  uint8 humidityLow;
  uint8 temperatureHigh;
  uint8 temperatureLow;
  uint8 checkData;
  uint8 sum;
  halIntState_t intState;

  DHT11_LastErr = 0;
  DHT11_LastRaw[0] = 0;
  DHT11_LastRaw[1] = 0;
  DHT11_LastRaw[2] = 0;
  DHT11_LastRaw[3] = 0;
  DHT11_LastRaw[4] = 0;

  if ((temperature == 0) || (humidity == 0))
  {
    DHT11_LastErr = 0x01;
    return 0;
  }

  if (!dht11Ready)
  {
    DHT11_LastErr = 0x31;
    return 0;
  }

  P2SEL &= ~DHT11_PIN_MASK;

  HAL_ENTER_CRITICAL_SECTION(intState);

  P2DIR |= DHT11_PIN_MASK;
  DHT11_DATA = 0;
  delay_ms(20);

  DHT11_DATA = 1;
  P2DIR &= ~DHT11_PIN_MASK;

  delay_10us();
  delay_10us();
  delay_10us();

  if(!dht11_wait_for_level(0, 20))
  {
    DHT11_LastErr = 0x02;
    HAL_EXIT_CRITICAL_SECTION(intState);
    P2DIR |= DHT11_PIN_MASK;
    DHT11_DATA = 1;
    return 0;
  }

  if(!dht11_wait_for_level(1, 100))
  {
    DHT11_LastErr = 0x03;
    HAL_EXIT_CRITICAL_SECTION(intState);
    P2DIR |= DHT11_PIN_MASK;
    DHT11_DATA = 1;
    return 0;
  }

  if(!dht11_wait_for_level(0, 100))
  {
    DHT11_LastErr = 0x04;
    HAL_EXIT_CRITICAL_SECTION(intState);
    P2DIR |= DHT11_PIN_MASK;
    DHT11_DATA = 1;
    return 0;
  }

  humidityHigh = dht11_read_byte(0);
  if(dht11ByteError)
  {
    HAL_EXIT_CRITICAL_SECTION(intState);
    P2DIR |= DHT11_PIN_MASK;
    DHT11_DATA = 1;
    return 0;
  }
  DHT11_LastRaw[0] = humidityHigh;

  humidityLow = dht11_read_byte(1);
  if(dht11ByteError)
  {
    HAL_EXIT_CRITICAL_SECTION(intState);
    P2DIR |= DHT11_PIN_MASK;
    DHT11_DATA = 1;
    return 0;
  }
  DHT11_LastRaw[1] = humidityLow;

  temperatureHigh = dht11_read_byte(2);
  if(dht11ByteError)
  {
    HAL_EXIT_CRITICAL_SECTION(intState);
    P2DIR |= DHT11_PIN_MASK;
    DHT11_DATA = 1;
    return 0;
  }
  DHT11_LastRaw[2] = temperatureHigh;

  temperatureLow = dht11_read_byte(3);
  if(dht11ByteError)
  {
    HAL_EXIT_CRITICAL_SECTION(intState);
    P2DIR |= DHT11_PIN_MASK;
    DHT11_DATA = 1;
    return 0;
  }
  DHT11_LastRaw[3] = temperatureLow;

  checkData = dht11_read_byte(4);
  if(dht11ByteError)
  {
    HAL_EXIT_CRITICAL_SECTION(intState);
    P2DIR |= DHT11_PIN_MASK;
    DHT11_DATA = 1;
    return 0;
  }
  DHT11_LastRaw[4] = checkData;

  HAL_EXIT_CRITICAL_SECTION(intState);

  P2DIR |= DHT11_PIN_MASK;
  DHT11_DATA = 1;

  sum = humidityHigh + humidityLow + temperatureHigh + temperatureLow;
  if(sum != checkData)
  {
    DHT11_LastErr = 0x30;
    return 0;
  }

  *humidity = humidityHigh;
  *temperature = temperatureHigh;
  return 1;
}
