/*
 * Copyright (C) 2020  Edward LEI <edward_lei72@hotmail.com>
 *
 * The code is licensed under the MIT license
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"


static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

static char *decoding_table = NULL;
static unsigned int mod_table[] = {0, 2, 1};


static void
_build_decoding_table()
{
  int i = 0;
  decoding_table = malloc(256);
  do {
    decoding_table[(unsigned char)encoding_table[i]] = i;
    i++;
  } while (i < 64);
}

void
b64_cleanup()
{
  free(decoding_table);
}

char *
b64_encode(const unsigned char *data, size_t len_in, size_t *len_out)
{
  unsigned int i, j;
  uint32_t octet_a;
  uint32_t octet_b;
  uint32_t octet_c;
  uint32_t triple;

  *len_out = 4 * ((len_in + 2) / 3);

  char *encoded_data = malloc(*len_out + 1);
  if (encoded_data == 0) return 0;

  for (i = 0, j = 0; i < len_in;) {
    octet_a = i < len_in ? (unsigned char)data[i++] : 0;
    octet_b = i < len_in ? (unsigned char)data[i++] : 0;
    octet_c = i < len_in ? (unsigned char)data[i++] : 0;

    triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

    encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
  }

  for (i = 0; i < mod_table[len_in % 3]; i++) {
    encoded_data[*len_out - 1 - i] = '=';
  }

  return encoded_data;
}

unsigned char *
b64_decode(const char *data, size_t len_in, size_t *len_out)
{
  unsigned int i, j;
  uint32_t sextet_a;
  uint32_t sextet_b;
  uint32_t sextet_c;
  uint32_t sextet_d;
  uint32_t triple;
  unsigned char *decoded_data;

  if (decoding_table == 0) _build_decoding_table();

  if (len_in % 4 != 0) return 0;

  *len_out = len_in / 4 * 3;
  if (data[len_in - 1] == '=') (*len_out)--;
  if (data[len_in - 2] == '=') (*len_out)--;

  decoded_data = malloc(*len_out + 1);
  if (decoded_data == 0) return 0;

  for (i = 0, j = 0; i < len_in;) {
    sextet_a = data[i] == '=' ?
               0 & i++ : (unsigned int)decoding_table[(unsigned char)data[i++]];
    sextet_b = data[i] == '=' ?
               0 & i++ : (unsigned int)decoding_table[(unsigned char)data[i++]];
    sextet_c = data[i] == '=' ?
               0 & i++ : (unsigned int)decoding_table[(unsigned char)data[i++]];
    sextet_d = data[i] == '=' ?
               0 & i++ : (unsigned int)decoding_table[(unsigned char)data[i++]];

    triple = (sextet_a << 3 * 6) +
             (sextet_b << 2 * 6) +
             (sextet_c << 1 * 6) +
             (sextet_d << 0 * 6);

    if (j < *len_out) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
    if (j < *len_out) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
    if (j < *len_out) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
  }

  return decoded_data;
}

/*
int main()
{
  char* data = "Hello World!";
  long input_size = strlen(data);
  char* encoded_data = b64_encode(data, input_size, &input_size);
  printf("Encoded Data is: %s \n",encoded_data);

  long decode_size = strlen(encoded_data);
  char* decoded_data = b64_decode(encoded_data, decode_size, &decode_size);
  printf("Decoded Data is: %s \n",decoded_data);
  exit(0);
}
*/
