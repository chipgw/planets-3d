
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

        var curKey = 0;
        /* Will be the first key in the list. */
        var startKey = universe.nextKey(0);

        while (curKey !== startKey) {
            if (curKey === 0)
                curKey = universe.nextKey(0);

            var pos = universe.getPlanetPosition(curKey);
            var vel = universe.getPlanetVelocity(curKey);
            var mass = universe.getPlanetMass(curKey);

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

            curKey = universe.nextKey(curKey);
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
