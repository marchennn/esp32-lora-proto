#include <Arduino.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "LoRa.h"

#include "test.pb.h"

#define SS 5
#define RST 25
#define DIO0 2

unsigned long lastSendTime = 0;
// Encoder callback
bool encode_message(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
  const char *str = (const char *)(*arg);
  return pb_encode_tag_for_field(stream, field) &&
         pb_encode_string(stream, (const uint8_t *)str, strlen(str));
}

// Decoder callback
bool decode_message(pb_istream_t *stream, const pb_field_t *field, void **arg) {
  char *buffer = (char *)*arg;
  size_t len = stream->bytes_left;

  if (len >= 32) len = 31; // Hindari overflow
  if (!pb_read(stream, (uint8_t *)buffer, len)) return false;

  buffer[len] = '\0';
  return true;
}

void sendProtobuf() {
  Test msg = Test_init_default;
  const char *text = "Hello from ESP32!";
  msg.message.funcs.encode = &encode_message;
  msg.message.arg = (void *)text;

  uint8_t buffer[64];
  pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  if (pb_encode_delimited(&ostream, Test_fields, &msg)) {
    LoRa.beginPacket();
    LoRa.write(buffer, ostream.bytes_written);
    LoRa.endPacket();
    Serial.println("Message sent");
  } else {
    Serial.println("Failed to encode message");
  }
}

void receiveProtobuf() {
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    uint8_t buffer[64];
    int len = 0;
    Serial.print("Received packet: ");

    while (LoRa.available() && len < sizeof(buffer)) {
      buffer[len++] = LoRa.read();
    }
    
    pb_istream_t istream = pb_istream_from_buffer(buffer, len);

    Test recv = Test_init_default;
    char textBuffer[64];
    recv.message.funcs.decode = &decode_message;
    recv.message.arg = textBuffer;

    if (pb_decode_delimited(&istream, Test_fields, &recv)) {
      Serial.print("Decoded message: ");
      Serial.println(textBuffer);
    } else {
      Serial.println("Failed to decode message");
    }
    
  }
  }

void setup() {
  Serial.begin(115200);
  Serial1.begin(57600, SERIAL_8N1, 16, 17); // RX=16, TX=17
  LoRa.setPins(SS, RST, DIO0);

  while (!LoRa.begin(868E6)) {
    Serial.println(".");
    delay(500);
  }

  LoRa.setSyncWord(0xF3);
  LoRa.setSpreadingFactor(8);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
}

void loop() {
  receiveProtobuf();

  if (millis() - lastSendTime > 1000) {
    lastSendTime = millis();
    sendProtobuf();
  }
}
