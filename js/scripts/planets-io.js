
function loadFile(filename) {
    var reader = new FileReader();

    reader.onload = function() {
        loadDOM(new DOMParser().parseFromString(this.result, "text/xml"));
    };
    reader.readAsText(filename);
}

function loadUrl(url) {
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open("GET", url, false);
    xmlHttp.send(null);
    loadDOM(xmlHttp.responseXML);
}

function loadDOM(parsed) {
    var planetsXML = parsed.getElementsByTagName("planet");

    if(planetsXML.length === 0){
        alert("Error loading simulation: no planets found!\nIs the file a valid universe file?");
        return;
    }

    universe.deleteAll();

    for(var i = 0; i < planetsXML.length; ++i) {
        var planet = planetsXML[i];

        var position = [parseFloat(planet.getElementsByTagName("position")[0].getAttribute("x")),
                        parseFloat(planet.getElementsByTagName("position")[0].getAttribute("y")),
                        parseFloat(planet.getElementsByTagName("position")[0].getAttribute("z"))];

        var velocity = [parseFloat(planet.getElementsByTagName("velocity")[0].getAttribute("x")) * universe.velocityfac,
                        parseFloat(planet.getElementsByTagName("velocity")[0].getAttribute("y")) * universe.velocityfac,
                        parseFloat(planet.getElementsByTagName("velocity")[0].getAttribute("z")) * universe.velocityfac];

        var mass = parseFloat(planet.getAttribute("mass"));

        universe.addPlanet(position, velocity, mass);
    }

    console.log("loaded " + planetsXML.length + " planets.");
}

function generateDOM() {
    if (document.implementation && document.implementation.createDocument) {
        var rootTagName = "";
        var namespaceURL = "";

        var doc = document.implementation.createDocument(namespaceURL, rootTagName, null);

        var root = doc.createElement("planets-3d-universe");

        for (var i = 0; i < universe.size(); ++i) {
            var pos = universe.getPlanetPosition(i);
            var vel = universe.getPlanetVelocity(i);
            var mass = universe.getPlanetMass(i);

            var p = doc.createElement("planet");

            var posXML = doc.createElement("position");
            posXML.setAttribute("x", pos[0]);
            posXML.setAttribute("y", pos[1]);
            posXML.setAttribute("z", pos[2]);
            p.appendChild(posXML);
            var velXML = doc.createElement("velocity");
            velXML.setAttribute("x", vel[0] / universe.velocityfac);
            velXML.setAttribute("y", vel[1] / universe.velocityfac);
            velXML.setAttribute("z", vel[2] / universe.velocityfac);
            p.setAttribute("mass", mass);
            p.appendChild(velXML);

            root.appendChild(p);
        }

        doc.appendChild(root);

        return doc;
    }
}

function downloadDOM(doc) {
    var x = new XMLSerializer();

    var blob = new Blob([x.serializeToString(doc)], { type: "text/xml" })

    saveAs(blob, "universe.xml");
}

function saveLocalStorage(name) {
    if (typeof(Storage) !== "undefined") {
        var x = new XMLSerializer();

        localStorage.setItem("planets-" + name, x.serializeToString(generateDOM()));
    }
}

function loadLocalStorage(name) {
    if (typeof(Storage) !== "undefined")
        loadDOM(new DOMParser().parseFromString(localStorage.getItem("planets-" + name), "text/xml"));
}

function getBase64() {
    var json = [];

    for (var i = 0; i < universe.size(); ++i) {
        json.push([universe.getPlanetPosition(i),
                   universe.getPlanetVelocity(i),
                   universe.getPlanetMass(i)])

        curKey = universe.nextKey(i);
    }

    return LZString.compressToEncodedURIComponent(JSON.stringify(json));
}

function loadBase64(enc) {
    var arr = JSON.parse(LZString.decompressFromEncodedURIComponent(enc));

    universe.deleteAll();

    for (var i = 0; i < arr.length; i++) {
        universe.addPlanet(arr[i][0], arr[i][1], arr[i][2]);
    }
}
