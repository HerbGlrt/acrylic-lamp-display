#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

// Defina os pinos conforme sua conexão
#define TFT_CS     10
#define TFT_RST    8
#define TFT_DC     9
#define SD_CS      4
#define NEOPIXEL_PIN 6
#define NUM_LEDS 10

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Inicializa todos os LEDs apagados
  strip.setBrightness(255); // Define a luminosidade máxima

  // Inicialize o display
  tft.initR(INITR_BLACKTAB);  
  tft.setRotation(1);  // Modo paisagem
  tft.fillScreen(ST77XX_BLACK);

  // Inicialize o cartão SD
  if (!SD.begin(SD_CS)) {
    Serial.println("Falha ao inicializar o cartão SD!");
    return;
  }
  Serial.println("Cartão SD inicializado com sucesso.");

  // Usar uma leitura analógica ruidosa para definir a semente aleatória
  randomSeed(analogRead(A0));

  // Outra opção é usar o millis() como seed, mas isso depende do tempo exato que o Arduino inicia
  // randomSeed(millis());
}


void loop() {
  File dir = SD.open("/");
  File file;
  unsigned long previousMillis = 0;
  const long interval = 3500; // Intervalo de exibição de imagens

  while (true) {
    // Calcular um salto aleatório baseado no tempo
    int skipFiles = random(1, 10); // Pule de 1 a 10 arquivos (ajuste conforme necessário)
    
    // Abrir o diretório de imagens
    dir.rewindDirectory(); // Reiniciar o diretório
    int currentFile = 0;
    
    // Pular arquivos até chegar no "aleatório"
    while (currentFile < skipFiles) {
      file = dir.openNextFile();
      if (!file) {
        dir.rewindDirectory();
        file = dir.openNextFile(); // Reiniciar o loop de arquivos
      }
      if (file && !file.isDirectory() && String(file.name()).endsWith(".BMP")) {
        currentFile++; // Contar o arquivo válido
      }
      file.close(); // Fechar o arquivo para liberar memória
    }
    
    // Abrir o arquivo selecionado e exibir
    file = dir.openNextFile();
    if (file && !file.isDirectory() && String(file.name()).endsWith(".BMP")) {
      Serial.println(file.name()); // Exibe o nome da imagem no Monitor Serial
      mostrarBMP(file.name());
      calcularMediaCor(file.name());
      file.close();

      // Esperar o intervalo antes de mostrar a próxima imagem
      unsigned long currentMillis = millis();
      while (millis() - currentMillis < interval) {
        // Aguarde aqui
      }
    } else {
      file.close(); // Garantir que o arquivo seja fechado
    }
  }
}


void mostrarBMP(const char *filename) {
  File bmpFile;
  int bmpWidth, bmpHeight;
  uint8_t bmpDepth;
  uint32_t bmpImageoffset;
  uint32_t rowSize;
  uint8_t sdbuffer[3*20];
  uint8_t buffidx = sizeof(sdbuffer);
  boolean goodBmp = false;
  boolean flip = true;
  int w, h, row, col;
  uint8_t r, g, b;
  uint32_t pos = 0;

  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print("Falha ao abrir o arquivo BMP: ");
    Serial.println(filename);
    return;
  }

  if(read16(bmpFile) == 0x4D42) {
    read32(bmpFile);
    read32(bmpFile);
    bmpImageoffset = read32(bmpFile);
    read32(bmpFile);
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) {
      bmpDepth = read16(bmpFile);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) {
        goodBmp = true;
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip = false;
        }

        w = bmpWidth;
        h = bmpHeight;
        if((w-1) >= tft.width())  w = tft.width();
        if((h-1) >= tft.height()) h = tft.height();

        tft.setAddrWindow(0, 0, w - 1, h - 1);
        rowSize = (bmpWidth * 3 + 3) & ~3;

        for (row=0; row<h; row++) {
          if(flip)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else
            pos = bmpImageoffset + row * rowSize;

          if(bmpFile.position() != pos) {
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer);
          }

          for (col=0; col<w; col++) {
            if (buffidx >= sizeof(sdbuffer)) {
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0;
            }

            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];

            tft.pushColor(tft.color565(r, g, b));
          }
        }
      }
    }
  }
  bmpFile.close();
  if(!goodBmp) Serial.println("Arquivo BMP inválido");
}


void calcularMediaCor(const char *filename) {
  File bmpFile;
  int bmpWidth, bmpHeight;
  uint8_t bmpDepth;
  uint32_t bmpImageoffset;
  uint32_t rowSize;
  uint8_t sdbuffer[3*20];
  uint8_t buffidx = sizeof(sdbuffer);
  boolean goodBmp = false;
  boolean flip = true;
  int w, h, row, col;
  uint8_t r, g, b;
  uint32_t pos = 0;
  uint32_t rTotal[NUM_LEDS] = {0};
  uint32_t gTotal[NUM_LEDS] = {0};
  uint32_t bTotal[NUM_LEDS] = {0};
  uint32_t pixelCount[NUM_LEDS] = {0};

  // Defina as 10 cores predefinidas
  uint32_t cores[10] = {
    strip.Color(255, 0, 0),    // Vermelho
    strip.Color(0, 255, 0),    // Verde
    strip.Color(0, 0, 255),    // Azul
    strip.Color(255, 255, 0),  // Amarelo
    strip.Color(255, 0, 255),  // Magenta
    strip.Color(0, 255, 255),  // Ciano
    strip.Color(255, 128, 0),  // Laranja
    strip.Color(128, 0, 255),  // Roxo
    strip.Color(255, 255, 255),// Branco
    strip.Color(0, 0, 0)       // Desligado (Preto)
  };

  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print("Falha ao abrir o arquivo BMP: ");
    Serial.println(filename);
    return;
  }

  if (read16(bmpFile) == 0x4D42) {
    read32(bmpFile);
    read32(bmpFile);
    bmpImageoffset = read32(bmpFile);
    read32(bmpFile);
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) {
      bmpDepth = read16(bmpFile);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) {
        goodBmp = true;
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip = false;
        }

        w = bmpWidth;
        h = bmpHeight;
        rowSize = (bmpWidth * 3 + 3) & ~3;

        int sectionWidth = w / NUM_LEDS; // Divide a imagem em 10 seções verticais

        for (row = 0; row < h; row++) {
          if (flip)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else
            pos = bmpImageoffset + row * rowSize;

          if (bmpFile.position() != pos) {
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer);
          }

          for (col = 0; col < w; col++) {
            if (buffidx >= sizeof(sdbuffer)) {
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0;
            }

            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];

            int currentSection = col / sectionWidth;
            rTotal[currentSection] += r;
            gTotal[currentSection] += g;
            bTotal[currentSection] += b;
            pixelCount[currentSection]++;
          }
        }

        // Decidir qual cor usar para cada LED baseado no componente dominante
        for (int i = 0; i < NUM_LEDS; i++) {
          if (pixelCount[i] > 0) {
            uint8_t avgR = rTotal[i] / pixelCount[i];
            uint8_t avgG = gTotal[i] / pixelCount[i];
            uint8_t avgB = bTotal[i] / pixelCount[i];

            // Determinar o componente dominante
            if (avgR > avgG && avgR > avgB) {
              strip.setPixelColor(i, cores[0]);  // Vermelho
            } else if (avgG > avgR && avgG > avgB) {
              strip.setPixelColor(i, cores[1]);  // Verde
            } else if (avgB > avgR && avgB > avgG) {
              strip.setPixelColor(i, cores[2]);  // Azul
            } else if (avgR > 200 && avgG > 200) {
              strip.setPixelColor(i, cores[3]);  // Amarelo
            } else if (avgR > 200 && avgB > 200) {
              strip.setPixelColor(i, cores[4]);  // Magenta
            } else if (avgG > 200 && avgB > 200) {
              strip.setPixelColor(i, cores[5]);  // Ciano
            } else {
              strip.setPixelColor(i, cores[6]);  // Laranja (ou outra cor)
            }
          } else {
            strip.setPixelColor(i, cores[9]);  // Desligado (Preto)
          }
        }

        // Garantir que o brilho esteja no máximo
        strip.setBrightness(255);
        strip.show();  // Atualizar os LEDs após calcular as cores
        Serial.println("LEDs atualizados com sucesso.");
      }
    }
  }
  bmpFile.close();
}



uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read();
  ((uint8_t *)&result)[1] = f.read();
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read();
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read();
  return result;
}
