
function loadFile (filename) {
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
