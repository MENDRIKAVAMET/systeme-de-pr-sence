// ===== VARIABLES GLOBALES =====
let ws = null;
let html5QrCode = null;
let scanning = false;
let dernierScan = '';
let scanEnPause = false;

// ===== CONNEXION AU SERVEUR WEBSOCKET =====
function connecterServeur() {
    const ip = document.getElementById('input_ip').value.trim();
    const port = document.getElementById('input_port').value.trim();

    if (!ip || !port) {
        alert('Veuillez entrer l\'IP et le port !');
        return;
    }

    const url = `wss://${ip}:${port}`;
    ws = new WebSocket(url);

    ws.onopen = () => {
        console.log('Connecté au serveur !');
        setBadgeConnecte();
    };

    ws.onmessage = (event) => {
        const reponse = event.data.trim();
        console.log('Réponse serveur :', reponse);
        afficherReponse(reponse);
    };

    ws.onclose = () => {
        console.log('Déconnecté du serveur');
        setBadgeDeconnecte();
        // Tenter de se reconnecter automatiquement après 3 secondes
        setTimeout(() => {
            if (ws.readyState === WebSocket.CLOSED) {
                connecterServeur();
            }
        }, 3000);
    };

    ws.onerror = () => {
        console.log('Erreur WebSocket !');
        setBadgeDeconnecte();
    };
}
// ===== BADGE CONNEXION =====
function setBadgeConnecte() {
    const badge = document.getElementById('badge_connexion');
    badge.className = 'badge bg-success connexion-badge';
    badge.innerHTML = '<i class="bi bi-wifi me-1"></i>Connecté';
}

function setBadgeDeconnecte() {
    const badge = document.getElementById('badge_connexion');
    badge.className = 'badge bg-danger connexion-badge';
    badge.innerHTML = '<i class="bi bi-wifi-off me-1"></i>Déconnecté';
}

// ===== SCANNER =====
function demarrerScan() {
    if (!ws || ws.readyState !== WebSocket.OPEN) {
        alert('Veuillez vous connecter au serveur d\'abord !');
        return;
    }

    html5QrCode = new Html5Qrcode("reader");

    const config = {
        fps: 10,
        qrbox: { width: 250, height: 250 },
        aspectRatio: 1.0,
        experimentalFeatures: {
            useBarCodeDetectorIfSupported: true
        }
    };

    html5QrCode.start(
        { facingMode: "environment" },
        config,
        (decodedText) => {
            console.log('QR détecté : ', decodedText);
            // Éviter de rescanner le même QR code ou si en pause
            if (scanEnPause || decodedText === dernierScan) return;

            // Mettre en pause 3 secondes
            scanEnPause = true;
            dernierScan = decodedText;

            console.log('QR Code scanné :', decodedText);
            ws.send(decodedText);
            afficherAttente(decodedText);

            // Reprendre le scan après 3 secondes
            setTimeout(() => {
                scanEnPause = false;
                dernierScan = '';
            }, 3000);
        },
        (error) => { }
    ).catch((err) => {
        console.log('Erreur caméra :', err);
        alert('Erreur caméra : ' + err);
    });

    scanning = true;
    document.getElementById('btn_scanner').style.display = 'none';
    document.getElementById('btn_arreter').style.display = 'block';
}

function arreterScan() {
    if (html5QrCode && scanning) {
        html5QrCode.stop().then(() => {
            scanning = false;
            document.getElementById('btn_scanner').style.display = 'block';
            document.getElementById('btn_arreter').style.display = 'none';
            document.getElementById('reader').innerHTML = '';
        });
    }
}

// ===== AFFICHAGE RESULTATS =====
function afficherAttente(numero) {
    const container = document.getElementById('resultat_container');
    container.className = 'resultat-container';
    container.style.display = 'block';
    container.innerHTML = `
        <div class="resultat-icone">⏳</div>
        <div class="resultat-titre">Vérification...</div>
        <div class="resultat-matricule">${numero}</div>
    `;
}

function afficherReponse(reponse) {
    const container = document.getElementById('resultat_container');
    const parties = reponse.split(':');
    const code = parties[0].trim();
    const nom = parties[1] ? parties[1].trim() : '';

    container.style.display = 'block';

    if (code === 'PRESENT') {
        container.className = 'resultat-container resultat-present';
        container.innerHTML = `
            <div class="resultat-icone">✅</div>
            <div class="resultat-titre" style="color:#28A745;">Présent !</div>
            <div class="resultat-nom">${nom}</div>
        `;
    } else if (code === 'DEJA_PRESENT') {
        container.className = 'resultat-container resultat-deja';
        container.innerHTML = `
            <div class="resultat-icone">⚠️</div>
            <div class="resultat-titre" style="color:#F0A500;">Déjà présent !</div>
            <div class="resultat-nom">${nom}</div>
        `;
    } else if (code === 'INCONNU') {
        container.className = 'resultat-container resultat-absent';
        container.innerHTML = `
            <div class="resultat-icone">❌</div>
            <div class="resultat-titre" style="color:#DC3545;">Non reconnu !</div>
            <div class="resultat-matricule">Matricule inconnu</div>
        `;
    } else if (code === 'ERREUR_CONNEXION' || code === 'TIMEOUT') {
        container.className = 'resultat-container resultat-absent';
        container.innerHTML = `
            <div class="resultat-icone">🔌</div>
            <div class="resultat-titre" style="color:#DC3545;">Erreur connexion !</div>
        `;
    }
}
