#include <stdio.h>
#include <stdlib.h>
#define FOSC 16000000U // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC / 16 / BAUD - 1


#define BOTAO PD2



char msg_tx[20];
char msg_rx[32];
int pos_msg_rx = 0;
int tamanho_msg_rx = 3;



unsigned int x = 0;
int valor_volume,valor_tempo = 0;
int conta_gotas = 0;

//Prototipos das funcoes
void UART_Init(unsigned int ubrr);
void UART_Transmit(char *dados);

ISR(INT0_vect){
  conta_gotas += 0.02;
}


int getValues(){
  int value;
  while(x == 0){
    value = (msg_rx[0] - 48) * 100 + (msg_rx[1] - 48) * 10 + (msg_rx[2] - 48) * 1;

    if(msg_rx[2] != NULL){
      x=1;
    }
  }

  msg_rx[0,1,2] = NULL;
  x=0;
  return value;

}

void setValues(){

  UART_Transmit("Entre com o volume:\n");
  valor_volume = getValues();


  UART_Transmit("Entre com o Tempo de Infusão em minutos:\n");
  valor_tempo = getValues();

  calcula();

}
float fluxo_real = 0.0;
float fluxo_definido = 0.0;
float error = 0.0;
float potencia = 0.0;


float calcula(){
  fluxo_real = (conta_gotas/valor_tempo)*0.05;
  fluxo_definido = valor_volume/valor_tempo;
  error = ((fluxo_real-fluxo_definido)/fluxo_definido) * 100;

}


void message(){
  UART_Transmit("Deseja alterar valores? (sim/nao): \n");
}

int main(void){

  PORTD |= (1<<BOTAO);
  EICRA |= (1 << ISC10);
  EIMSK |= ( 1<< INT0);
  UART_Init(MYUBRR);
  sei();


  setValues();
  message();

  while (1) {

    while(x == 0){
      if((msg_rx[0]=='s') && (msg_rx[1]=='i') && (msg_rx[2]=='m')){
        x = 1;
        msg_rx[0,1,2] = NULL;
        setValues();
      }
    }
    x = 0;


    while(x == 0){
      if((msg_rx[0]=='n') && (msg_rx[1]=='a') && (msg_rx[2]=='o')){
        x = 1;
        msg_rx[0,1,2] = NULL;
        itoa(error,msg_tx,10);
        UART_Transmit(msg_tx);
        UART_Transmit("");
        message();

      }
    }
    x = 0;
  }

}















ISR(USART_RX_vect)
{
  // Escreve o valor recebido pela UART na posição pos_msg_rx do buffer msg_rx
  msg_rx[pos_msg_rx++] = UDR0;
  if (pos_msg_rx == tamanho_msg_rx)
    pos_msg_rx = 0;
}

void UART_Transmit(char *dados)
{
  // Envia todos os caracteres do buffer dados ate chegar um final de linha
  while (*dados != 0)
  {
    while (!(UCSR0A & (1 << UDRE0))); // Aguarda a transmissão acabar
    // Escreve o caractere no registro de tranmissão
    UDR0 = *dados;
    // Passa para o próximo caractere do buffer dados
    dados++;
  }
}

void UART_Init(unsigned int ubrr)
{
  // Configura a baud rate */
  UBRR0H = (unsigned char)(ubrr >> 8);
  UBRR0L = (unsigned char)ubrr;
  // Habilita a recepcao, tranmissao e interrupcao na recepcao */
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
  // Configura o formato da mensagem: 8 bits de dados e 1 bits de stop */
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}
