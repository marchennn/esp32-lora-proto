#include <Arduino.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "LoRa.h"

#include "telemetry.pb.h"

#define SS 5
#define RST 25
#define DIO0 2

uint8_t buffer[1024];
unsigned long lastSendTime = 0;

void sendProtobuf()
{
  String line = Serial.readStringUntil('\n');
  line.trim();

  if (line.length() == 0)
  {
    return;
  }

  char buf[1024];
  line.toCharArray(buf, sizeof(buf));

  UAVTelemetry msg = UAVTelemetry_init_default;

  char *token = strtok(buf, ",");
  while (token != NULL)
  {
    char *sep = strchr(token, ':');
    if (sep != NULL)
    {
      *sep = '\0';
      char *key = token;
      char *value = sep + 1;

      if (strcmp(key, "roll") == 0)
        msg.roll = atof(value);
      else if (strcmp(key, "pitch") == 0)
        msg.pitch = atof(value);
      else if (strcmp(key, "yaw") == 0)
        msg.yaw = atof(value);
      else if (strcmp(key, "compass") == 0)
        msg.compass = atof(value);
      else if (strcmp(key, "gps_sat") == 0)
        msg.gps_sat = atoi(value);
      else if (strcmp(key, "esc1") == 0)
        msg.esc1 = atoi(value);
      else if (strcmp(key, "esc2") == 0)
        msg.esc2 = atoi(value);
      else if (strcmp(key, "esc3") == 0)
        msg.esc3 = atoi(value);
      else if (strcmp(key, "esc4") == 0)
        msg.esc4 = atoi(value);
      else if (strcmp(key, "lat") == 0)
        msg.lat = atof(value);
      else if (strcmp(key, "lon") == 0)
        msg.lon = atof(value);
      else if (strcmp(key, "altitude") == 0)
        msg.height = atof(value);
      else if (strcmp(key, "speed") == 0)
        msg.speed = atof(value);
      else if (strcmp(key, "airspeed") == 0)
        msg.airspeed = atof(value);
      else if (strcmp(key, "preflight_check") == 0)
        msg.preflight_check = atoi(value);
      else if (strcmp(key, "modestatus") == 0)
        msg.modestatus = atoi(value);
      else if (strcmp(key, "armstatus") == 0)
        msg.armstatus = atoi(value);
      else if (strcmp(key, "altholdstatus") == 0)
        msg.altholdstatus = atoi(value);
      else if (strcmp(key, "possholdstatus") == 0)
        msg.possholdstatus = atoi(value);
      else if (strcmp(key, "gainroll") == 0)
        msg.gainroll = atof(value);
      else if (strcmp(key, "gainpitch") == 0)
        msg.gainpitch = atof(value);
      else if (strcmp(key, "gainyaw") == 0)
        msg.gainyaw = atof(value);
      else if (strcmp(key, "gainrollvel") == 0)
        msg.gainrollvel = atof(value);
      else if (strcmp(key, "gainpitchvel") == 0)
        msg.gainpitchvel = atof(value);
      else if (strcmp(key, "gainyawvel") == 0)
        msg.gainyawvel = atof(value);
      else if (strcmp(key, "gainlatitude") == 0)
        msg.gainlatitude = atof(value);
      else if (strcmp(key, "gainlatitudevel") == 0)
        msg.gainlatitudevel = atof(value);
      else if (strcmp(key, "gainlongitude") == 0)
        msg.gainlongitude = atof(value);
      else if (strcmp(key, "gainlongitudevel") == 0)
        msg.gainlongitudevel = atof(value);
      else if (strcmp(key, "gainalthold") == 0)
        msg.gainalthold = atof(value);
      else if (strcmp(key, "gainaltholdvel") == 0)
        msg.gainaltholdvel = atof(value);
      else if (strcmp(key, "remthrottle") == 0)
        msg.remthrottle = atoi(value);
      else if (strcmp(key, "remyaw") == 0)
        msg.remyaw = atof(value);
      else if (strcmp(key, "rempitch") == 0)
        msg.rempitch = atof(value);
      else if (strcmp(key, "remroll") == 0)
        msg.remroll = atof(value);
      else if (strcmp(key, "batteryvolt") == 0)
        msg.batteryvolt = atof(value);
      else if (strcmp(key, "battery") == 0)
        msg.battery = atoi(value);
      else if (strcmp(key, "trimroll") == 0)
        msg.trimroll = atof(value);
      else if (strcmp(key, "trimpitch") == 0)
        msg.trimpitch = atof(value);
      else if (strcmp(key, "yawtrim") == 0)
        msg.yawtrim = atof(value);
    }
    token = strtok(NULL, ",");
  }

  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  if (pb_encode_delimited(&stream, UAVTelemetry_fields, &msg))
  {
    LoRa.beginPacket();
    LoRa.write(buffer, stream.bytes_written);
    bool result = LoRa.endPacket();

    if (result)
    {
      Serial.println("Telemetry packet sent");
    }
    else
    {
      Serial.println("Failed to send packet");
    }
  }
  else
  {
    Serial.print("Failed to encode message");
    Serial.println(PB_GET_ERROR(&stream));
  }
}

void receiveProtobuf()
{
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    int len = 0;
    Serial.print("Received packet: ");

    while (LoRa.available() && len < sizeof(buffer))
    {
      buffer[len++] = LoRa.read();
    }

    pb_istream_t istream = pb_istream_from_buffer(buffer, len);

    UAVTelemetry recv = UAVTelemetry_init_default;
    if (pb_decode_delimited(&istream, UAVTelemetry_fields, &recv))
    {
      Serial.print(recv.lat, 6);
      Serial.print(", ");
      Serial.print(recv.lon, 6);
      Serial.print(", ");
      Serial.print(recv.height, 2);
      Serial.print(", ");
      Serial.print(recv.roll, 2);
      Serial.print(", ");
      Serial.print(recv.yaw);
      Serial.print(", ");
      Serial.print(recv.pitch);
      Serial.print(", ");
      Serial.print(recv.remthrottle);
      Serial.print(", ");
      Serial.print(recv.modestatus);
      Serial.print(", ");
      Serial.print(recv.batteryvolt);
      Serial.print(", ");
      Serial.print(recv.armstatus);
      Serial.print(", ");
      Serial.print(recv.speed);
      Serial.print(", ");
      Serial.print(recv.compass);
      Serial.print(", ");
      Serial.print(recv.esc1);
      Serial.print(", ");
      Serial.print(recv.esc2);
      Serial.print(", ");
      Serial.print(recv.esc3);
      Serial.print(", ");
      Serial.print(recv.esc4);
      Serial.print(", ");
      Serial.print(recv.altholdstatus);
      Serial.print(", ");
      Serial.print(recv.possholdstatus);
      Serial.print(", ");
      Serial.print(recv.gainroll);
      Serial.print(", ");
      Serial.print(recv.gainpitch);
      Serial.print(", ");
      Serial.print(recv.gainyaw);
      Serial.print(", ");
      Serial.print(recv.gainrollvel);
      Serial.print(", ");
      Serial.print(recv.gainpitchvel);
      Serial.print(", ");
      Serial.print(recv.gainyawvel);
      Serial.print(", ");
      Serial.print(recv.gainlatitude);
      Serial.print(", ");
      Serial.print(recv.gainlatitudevel);
      Serial.print(", ");
      Serial.print(recv.gainlongitude);
      Serial.print(", ");
      Serial.print(recv.gainlongitudevel);
      Serial.print(", ");
      Serial.print(recv.gainalthold);
      Serial.print(", ");
      Serial.print(recv.gainaltholdvel);
      Serial.print(", ");
      Serial.print(recv.remyaw);
      Serial.print(", ");
      Serial.print(recv.rempitch);
      Serial.print(", ");
      Serial.print(recv.remroll);
      Serial.print(", ");
      Serial.print(recv.trimroll);
      Serial.print(", ");
      Serial.print(recv.trimpitch);
      Serial.print(", ");
      Serial.print(recv.yawtrim);
      Serial.print(", ");
      Serial.print(recv.batteryvolt);
      Serial.print(", ");
      Serial.print(recv.battery);
      Serial.println();
      Serial.println("Telemetry packet received");
    }
    else
    {
      Serial.println("Failed to decode message");
    }
  }
}

void setup()
{
  Serial.begin(57600);
  LoRa.setPins(SS, RST, DIO0);

  while (!LoRa.begin(433E6))
  {
    Serial.println(".");
    delay(500);
  }

  LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);
  LoRa.setSyncWord(0xF3);
  LoRa.setSpreadingFactor(9);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(6);
  LoRa.enableCrc();

  Serial.println("Telemetry ready!");
}

void loop()
{
  receiveProtobuf();
  sendProtobuf();
}
