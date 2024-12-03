//Se connecter au serveur
var ws = new WebSocket(`ws://${window.location.hostname}/ws`);
//var ws = new WebSocket('ws://192.168.1.2/ws'); //Mettre adresse IP du serveur WEBSOCKET
//Détecter l'ouverture d'une connexion avec le serveur
ws.onopen = function (event) {
    ws.send("Connection avec le serveur Etablie");
    console.log("Connection avec le serveur OK");
};
//Recevoir les données
ws.onmessage = function (event) {
    document.getElementById("VALEUR").innerHTML = event.data; //Remplace l'id "VALEUR" par la donnée reçue
};
function FonctionON() {
    ws.send("ON");
    console.log("ON");
}
function FonctionOFF() {
    ws.send("OFF");
    console.log("OFF");
}