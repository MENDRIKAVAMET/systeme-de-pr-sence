const WebSocket = require('ws');
const net = require('net');
const https = require('https');
const fs = require('fs');
const path = require('path');

const isPkg = typeof process.pkg !== 'undefined';
const basePath = isPkg
  ? path.dirname(process.execPath)
  : __dirname;

// Configuration
const WS_PORT = 8080;
const TCP_PORT = 12345;
const QT_HOST = '127.0.0.1';

// Certificat SSL
const options = {
    key: fs.readFileSync(path.join(basePath, 'key.pem')),
    cert: fs.readFileSync(path.join(basePath, 'cert.pem'))
};

// Serveur HTTPS
const httpsServer = https.createServer(options, (req, res) => {
    let filePath = path.join(basePath, req.url === '/' ? 'index.html' : req.url);

    const extname = path.extname(filePath);
    const contentTypes = {
        '.html': 'text/html',
        '.css': 'text/css',
        '.js': 'application/javascript',
        '.png': 'image/png',
        '.jpg': 'image/jpeg',
        '.woff': 'font/woff',
        '.woff2': 'font/woff2',
        '.ttf': 'font/ttf',
        '.eot': 'application/vnd.ms-fontobject'
    };

    const contentType = contentTypes[extname] || 'text/plain';

    fs.readFile(filePath, (err, data) => {
        if (err) {
            res.writeHead(404);
            res.end('Page non trouvée');
            return;
        }
        res.writeHead(200, { 'Content-Type': contentType });
        res.end(data);
    });
});

// Serveur WebSocket sur HTTPS
const wss = new WebSocket.Server({ server: httpsServer });

wss.on('connection', (ws) => {
    console.log('Téléphone connecté');

    ws.on('message', (message) => {
        const numeroInscription = message.toString().trim();
        console.log('QR Code reçu :', numeroInscription);

        envoyerAuQt(numeroInscription, (reponse) => {
            ws.send(reponse);
        });
    });

    ws.on('close', () => {
        console.log('Téléphone déconnecté');
    });
});

// Fonction pour envoyer le numéro au serveur Qt via TCP
function envoyerAuQt(numero, callback) {
    const client = new net.Socket();

    client.connect(TCP_PORT, QT_HOST, () => {
        console.log('Connecté au serveur Qt');
        client.write(numero);
    });

    client.on('data', (data) => {
        const reponse = data.toString().trim();
        console.log('Réponse Qt :', reponse);
        callback(reponse);
        client.destroy();
    });

    client.on('error', (err) => {
        console.log('Erreur TCP :', err.message);
        callback('ERREUR_CONNEXION');
    });

    client.setTimeout(3000, () => {
        client.destroy();
        callback('TIMEOUT');
    });
}

// Démarrer le serveur
httpsServer.listen(WS_PORT, () => {
    console.log('Serveur HTTPS démarré sur https://localhost:' + WS_PORT);
    console.log('En attente de connexions...');
});
