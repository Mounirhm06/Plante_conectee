#include <BluetoothSerial.h>

//  Initialisation du Bluetooth
BluetoothSerial ESP_BT;

void setup() {
    Serial.begin(115200);  // Moniteur série (pour le débogage)
    
    //  Initialiser la communication série avec le STM32
    Serial2.begin(9600, SERIAL_8N1, 3, 1);  // RX = GPIO3, TX = GPIO1

    //  Démarrer le Bluetooth avec un nom identifiable
    if (!ESP_BT.begin("ESP32_Humidite")) {  // Nom visible sur le téléphone
        Serial.println("Erreur lors de l'initialisation du Bluetooth");
        while (1);
    }

    Serial.println(" Bluetooth prêt ! Connectez-vous avec votre téléphone.");
}

void loop() {
    //  Lire les données reçues du STM32 via USART et les envoyer au téléphone
    if (Serial2.available()) {
        String receivedFromSTM32 = Serial2.readStringUntil('\n');  // Lire jusqu'à la fin de ligne

        //  Afficher sur le moniteur série pour le débogage
        Serial.print("Données STM32 : ");
        Serial.println(receivedFromSTM32);

        //  Envoyer au téléphone via Bluetooth
        ESP_BT.println(receivedFromSTM32);
    }

    //  Lire les commandes reçues du téléphone et les transmettre au STM32
    if (ESP_BT.available()) {
        String receivedFromPhone = ESP_BT.readStringUntil('\n');  // Lire la commande reçue

        //  Afficher sur le moniteur série pour le débogage
        Serial.print("Commande reçue du téléphone : ");
        Serial.println(receivedFromPhone);

        //  Transmettre la commande au STM32 via USART
        Serial2.print(receivedFromPhone);
    }
}
