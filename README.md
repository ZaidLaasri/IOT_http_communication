# IOT_http_communication

# Serveur Web ESP32 Asynchrone - Fonctionnalités

Ce document décrit les principales fonctionnalités du serveur web asynchrone ESP32, orienté autour de la lecture de capteurs et du contrôle de périphériques.

## Fonctionnalités

### Capteurs et Contrôles Automatiques

- **Gestion des Capteurs** : Le fichier `sensors.ino` contient la logique pour lire les valeurs des capteurs de température et de lumière de la carte ESP. Il inclut également les fonctions pour activer la LED appropriée et régler la vitesse du ventilateur en fonction des seuils de température et de lumière.
  
- **Format JSON** : Les valeurs des capteurs peuvent être retournées au format JSON, permettant une intégration facile avec d'autres systèmes ou des dashboards personnalisés.

### Routes HTTP et Traitement des URL

- **`GET /temperature`** : Retourne la valeur actuelle de la température captée par le capteur.
  
- **`GET /light`** : Retourne le niveau actuel de lumière mesuré.
  
- **`GET /upTime`** : Indique le temps d'opération du système en secondes depuis son dernier démarrage.
  
- **`GET /value`** : Retourne des données spécifiques des capteurs au format JSON en fonction des paramètres de la requête (ex : lumière, température, vitesse du ventilateur, détection de feu, etc.).

### Modes Automatique et Manuel

- Un **mode automatique** est intégré, permettant à l'ESP de déterminer automatiquement l'état des LEDs et la vitesse du ventilateur en fonction des lectures de température et de lumière. Ce mode peut être outrepassé par une configuration manuelle via la route `/set` ou le contrôle depuis le dashboard avec la route `/control`.

- **`GET /set`** : Permet la configuration manuelle des périphériques, comme le cooler (`/set?cool=on/off`), le heater (`/set?heat=on/off`), la vitesse du ventilateur (`/set?fanspeed=int`), la couleur du bandeau LED (`/set?ledStrip=red/blue/green`), l'activation du mode automatique (`/set?automatique=true/false`), et les seuils de température (`/set?HIGH_TEMP_THRESHOLD=int` ou `/set?LOW_TEMP_THRESHOLD=int`), en veillant à ce que le seuil haut ne soit pas inférieur au seuil bas et vice versa.

- **`GET /control`** : Route responsable du contrôle de l'ESP à partir du dashboard, permettant l'allumage et l'extinction des LEDs et du bandeau LED, la saisie de la vitesse du ventilateur, et le contrôle du mode automatique. Un avertissement de détection de feu est également affiché sur le dashboard et change d'état en fonction de la valeur de la variable `fire`, qui est liée aux seuils prédéfinis.

N.B : les fonctions request du shéma node-red contiennent pour le moment des urls avec des adresses ip statiques de cette forme "msg.url = "http://172.20.10.14/control?fanspeed=" + msg.payload.fanspeed;
return msg;" 
veuillez mettre votre adresse ip pour que ça marche.

### Dashboard 

N.B : les fonctions request du shéma node-red contiennent pour le moment des urls avec des adresses ip statiques de cette forme "msg.url = "http://172.20.10.14/control?fanspeed=" + msg.payload.fanspeed;
return msg;" 
veuillez mettre votre adresse ip pour que ça marche.

- Le dashboard offre une visualisation en temps réel des données des capteurs, ainsi que des contrôles interactifs pour les LEDs, le bandeau LED, la vitesse du ventilateur, et le mode automatique. Un indicateur d'alerte pour la détection de feu change dynamiquement en fonction de la situation.

### Periodic Reports

- le système intègre une fonctionnalité de reporting automatique qui permet d'envoyer périodiquement les données collectées vers un serveur externe, tel que Node-RED. Cela est réalisé par la fonction sendPeriodicReports() dans le code principal. Cette fonction envoie les données des capteurs, incluant la température, le niveau de lumière, l'état des LEDs, la vitesse du ventilateur, et l'état du détecteur de feu, à l'adresse IP et au port configurés via le formulaire de la page d'administration de l'ESP. La périodicité de ces envois peut être définie par l'utilisateur sur la page d'administration (data).


### Auteur
Ce projet a été développé par :

- **[Zaid LAASRI]**
#