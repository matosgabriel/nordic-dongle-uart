#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <stdio.h>
#include <regex.h>

#define SLEEP_TIME_MS   10
#define RECEIVE_BUFF_SIZE 1024
#define RECEIVE_TIMEOUT 100

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

static uint8_t tx_buf[] =   {"nRF52840 Dongle - ESP8266 Uart communication\n\r"
                             "The main objective of this work is to use the best of both devices to establish a link between a fire detector circuit and a Thread network\n\r"};

static char rx_buf[RECEIVE_BUFF_SIZE] = {0};  // Buffer de recepção
static char rx_index = 0;  // Índice do buffer de recepção

#include <regex.h>

void extract_value(const char *string_to_match, const char *pattern, char *value, int value_size) {
    regex_t regex;
    regmatch_t matches[2];  // O primeiro índice é a correspondência total, o segundo é o grupo capturado
    int result = regcomp(&regex, pattern, REG_EXTENDED);
    
    if (result) {
        // printf("Não foi possível compilar o regex\n");
        return;
    }

    result = regexec(&regex, string_to_match, 2, matches, 0);
    if (!result) {
        // Extrai o valor usando a posição do grupo capturado
        int start = matches[1].rm_so; // Início do grupo do valor
        int end = matches[1].rm_eo;   // Fim do grupo do valor

        // Cria uma string para armazenar o valor capturado
        snprintf(value, value_size, "%.*s", end - start, string_to_match + start);  // Copia o valor
    }
	// else if (result == REG_NOMATCH) {
    //     printf("Nenhuma correspondência para o padrão: %s\n", pattern);
    // }
	else {
        char errbuf[100];
        regerror(result, &regex, errbuf, sizeof(errbuf));
        // printf("Erro na correspondência de regex: %s\n", errbuf);
    }

    // Libera a memória usada pela regex
    regfree(&regex);
}

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data) {
	char temperature[10]; // Para armazenar a temperatura
    char humidity[10];    // Para armazenar a umidade
    char microphone[10];  // Para armazenar o valor do microfone
	char message[50];

    switch (evt->type) {
        case UART_RX_RDY:
            for (int i = 0; i < evt->data.rx.len; i++) {
                char received_char = evt->data.rx.buf[evt->data.rx.offset + i];
                
                // Verifica se o caractere recebido é um delimitador (ex: '\n')
                if (received_char == '\n' || received_char == '\0') {
                    rx_buf[rx_index] = '\0';  // Finaliza a string recebida
                    printk("Mensagem recebida: %s\n", rx_buf);

					// Limpa as variáveis antes de usar
					memset(temperature, 0, sizeof(temperature));
					memset(humidity, 0, sizeof(humidity));
					memset(microphone, 0, sizeof(microphone));
					memset(message, 0, sizeof(message));

					strcpy(message, rx_buf);

					// Padrões das mensagens
					char *temperature_pattern = "^\\(HUT2X\\) Temperature: (-?[0-9]+(\\.[0-9]+)?) °C";
					char *humidity_pattern = "^\\(HUT2X\\) Humidity: (-?[0-9]+(\\.[0-9]+)?) %";
					char *microphone_pattern = "^\\(WPSE309\\) Microphone sensor value: (-?[0-9]+)";

					// Verifica se a mensagem corresponde ao padrão de temperatura
					extract_value(message, temperature_pattern, temperature, sizeof(temperature));
					if (strlen(temperature) > 0) {
						printf("Valor do sensor de Temperatura: %s\n", temperature);
					}

					// Verifica se a mensagem corresponde ao padrão de umidade
					extract_value(message, humidity_pattern, humidity, sizeof(humidity));
					if (strlen(humidity) > 0) {
						printf("Valor do sensor de Umidade: %s\n", humidity);
					}

					// Verifica se a mensagem corresponde ao padrão do microfone
					extract_value(message, microphone_pattern, microphone, sizeof(microphone));
					if (strlen(microphone) > 0) {
						printf("Valor do sensor de Microfone: %s\n", microphone);
					}

                    // Limpar o buffer após o processamento
                    // memset(rx_buf, 0, RECEIVE_BUFF_SIZE);
                    rx_index = 0;  // Resetar o índice do buffer
                } else {
                    // Armazena o caractere no buffer de recepção
                    rx_buf[rx_index++] = received_char;
                }
            }
            break;

        case UART_RX_DISABLED:
            uart_rx_enable(dev, rx_buf, sizeof(rx_buf), RECEIVE_TIMEOUT);
            break;

        default:
            break;
    }
}

int main(void)
{
    int ret;

    if (!device_is_ready(uart)) {
        printk("UART device not ready\r\n");
        return 1;
    }

    ret = uart_callback_set(uart, uart_cb, NULL);
    if (ret) {
        return 1;
    }

    ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_MS);
    if (ret) {
        return 1;
    }

    ret = uart_rx_enable(uart, rx_buf, sizeof(rx_buf), RECEIVE_TIMEOUT);
    if (ret) {
        return 1;
    }
}
