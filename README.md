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

*   📅 **Gestão de Agenda e Organização:** Lembretes automáticos sobre horários de consultas e tarefas do consultório.
*   🔔 **Alertas de Hábitos (Lembretes):** Avisos visuais e sonoros programáveis (ex: hora de beber água, pausas ergonômicas).
*   📱 **Painel de Controle Web (PWA):** Interface amigável para a nutricionista configurar horários, alertas e interagir com o robô de forma remota.
*   👀 **Interação Procedural:** Olhinhos animados renderizados no display OLED, garantindo "personalidade" e feedback visual imediato.
*   ⌨️ **Interação Tátil:** Botões com switches mecânicos de teclado para confirmar tarefas ou silenciar alarmes com um clique satisfatório.

---

## 💻 **Tecnologias Utilizadas**

O projeto integra microeletrônica com desenvolvimento web para entregar a melhor experiência:

*   **Hardware / Microeletrônica:** `ESP32` | `C++ (Arduino IDE)`
*   **Interface Física:** Display `OLED 0.96" SSD1306` | Switches Mecânicos
*   **Controle / Backend:** `Node.js` | `Express` | `PWA` (Painel Web)
*   **Banco de Dados:** `SQLite` (Armazenamento local de rotinas e lembretes)

---

## ⚙️ **Pré-requisitos**

Para replicar, testar ou compilar o código deste projeto, você precisará de:

*   [**Arduino IDE**](https://www.arduino.cc/en/software) configurado com as placas da família **ESP32**.
*   Bibliotecas do display OLED (ex: `Adafruit_SSD1306`).
*   [**Node.js**](https://nodejs.org/en/) (Para rodar o painel web PWA localmente).

---

## 🚀 **Instalação e Execução**

**1. Configurando o Hardware (ESP32):**
*   Abra o arquivo `.ino` na Arduino IDE.
*   Certifique-se de instalar as bibliotecas necessárias.
*   Selecione sua placa ESP32 e a porta COM correspondente, depois clique em "Carregar".

**2. Rodando o Painel Web Local:**
```bash
# Clone o repositório
git clone [https://github.com/seu-usuario/nutr-e.git](https://github.com/seu-usuario/nutr-e.git)

# Acesse a pasta do painel web
cd nutr-e/painel-web

# Instale as dependências
npm install

# Execute a aplicação do painel
npm start
