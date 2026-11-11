<h1 align="center"> 
  🤖 NUTR-E (NutriBot)
</h1>

<p align="center">
  <img src="https://img.shields.io/badge/Status-Em%20Desenvolvimento-green" alt="Status">
  <img src="https://img.shields.io/badge/Hardware-ESP32-blue" alt="Hardware">
  <img src="https://img.shields.io/badge/Vers%C3%A3o-1.0.0-blue" alt="Versão">
</p>

<p align="center">
  <img src="icon.png" alt="Logo" width="200">
</p>

> **Resumo:** Um simpático e interativo robô assistente de mesa projetado para o escritório. Ele une hardware e software para auxiliar na organização diária, gestão de agendas, controle de hábitos e emissão de lembretes, tornando o ambiente de trabalho mais produtivo e tecnológico.

---

## 🎯 **Principais Funcionalidades**

*   📅 **Gestão de Agenda e Organização:** Lembretes automáticos sobre horários marcados e tarefas do escritório.
*   🔔 **Alertas de Hábitos (Lembretes):** Avisos visuais e sonoros programáveis (ex: hora de beber água, hora de se alimentar).
*   📱 **Painel de Controle Web (PWA):** Interface amigável para configurar horários, alertas e interagir com o robô de forma remota.
*   👀 **Interação Procedural:** Olhinhos animados renderizados no display OLED, garantindo "personalidade" e feedback visual imediato.
*   ⌨️ **Interação Tátil:** Botões com switches mecânicos de teclado para confirmar tarefas ou silenciar alarmes com um clique satisfatório.

---

## 💻 **Tecnologias Utilizadas**

O projeto integra microeletrônica com desenvolvimento web para entregar a melhor experiência:

*   **Hardware / Microeletrônica:** `ESP32` | `C++ (Arduino IDE)`
*   **Interface Física:** Display `OLED 0.96" SSD1306` | Switches Mecânicos

---

## ⚙️ **Pré-requisitos**

Para replicar, testar ou compilar o código deste projeto, você precisará de:

*   [**Arduino IDE**](https://www.arduino.cc/en/software) configurado com as placas da família **ESP32**.
*   Bibliotecas ( `Adafruit_SSD1306, Async TCP, ESP Async WebServer, QRCode `).

---

## 🚀 **Instalação e Execução**

**1. Configurando o Hardware (ESP32):**
*   Abra o arquivo `.ino` na Arduino IDE.
*   Certifique-se de instalar as bibliotecas necessárias.
*   Selecione sua placa ESP32 e a porta COM correspondente, depois clique em "Carregar".


## 📸 **Telas do Painel (Screenshots)**

> *Confira abaixo a interface do Painel Web PWA utilizado para gerenciar e configurar o NUTR-E.*

<p align="center">
  <!-- Substitua os nomes 'print-dashboard.png' e 'print-agenda.png' pelo caminho dos seus prints -->
  <img width="628" height="843" alt="print1" src="https://github.com/user-attachments/assets/20d71429-08c5-4a89-b0a7-682aaaec422d" />
  <img src="print-agenda.png" alt="Tela de Agenda e Lembretes" width="400">
</p>
<p align="center">
  <i>Esquerda: Dashboard principal de controle. Direita: Configuração de agenda e disparo de alertas.</i>
</p>
