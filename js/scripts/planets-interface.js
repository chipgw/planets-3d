var universe, camera, placing;

var gamepad;

function inFrame () {
    try {
        return window.top !== window.self;
    } catch (e) {
        return true;
    }
}

function initFileUI(dropTarget) {
    document.getElementById("loadFile").addEventListener("change", function(e) {
        loadFile(e.target.files[0]);
        document.getElementById("loadFileForm").reset();
    }, false);

    document.getElementById("menuOpenFile").addEventListener("click", function(e) {
        document.getElementById("loadFile").click();
    }, false);

    document.getElementById("menuSaveFile").addEventListener("click", function(e) {
        downloadDOM(generateDOM());
    }, false);

    var urlButton = document.getElementById("menuGetURL");

    /* If we are in a frame the "Get URL" functionality doesn't work right. */
    if (inFrame())
        urlButton.parentElement.removeChild(urlButton)
    else
        urlButton.addEventListener("click", function(e) {
            window.prompt("Sharable URL:", location.protocol + '//' + location.host + location.pathname + "?" + getBase64());
        }, false);

    dropTarget.addEventListener("dragenter", function(e) {
        e.stopPropagation();
        e.preventDefault();
    }, false);

    dropTarget.addEventListener("dragover", function(e) {
        e.stopPropagation();
        e.preventDefault();
    }, false);

    dropTarget.addEventListener("drop", function(e) {
        e.stopPropagation();
        e.preventDefault();
        if (e.dataTransfer.files.length > 0)
            loadFile(e.dataTransfer.files[0]);
    }, false);
}

function initMenu() {
    document.getElementById("menuClear").addEventListener("click", function(e) {
        if (confirm("Are you sure you want to destroy the universe?"))
            universe.deleteAll();
    }, false);

    document.getElementById("menuCenter").addEventListener("click", function(e) {
        universe.centerAll();
    }, false);

    document.getElementById("menuDelete").addEventListener("click", function(e) {
        universe.remove(universe.selected);
    }, false);

    document.getElementById("menuStop").addEventListener("click", function(e) {
        universe.setPlanetVelocity(universe.selected, [0,0,0]);
    }, false);

    document.getElementById("menuFullscreen").addEventListener("click", function(e) {
        if (!document.fullscreenElement && !document.mozFullScreenElement && !document.webkitFullscreenElement) {
            if (document.documentElement.requestFullscreen)
                document.documentElement.requestFullscreen();
            else if (document.documentElement.mozRequestFullScreen)
                document.documentElement.mozRequestFullScreen();
            else if (document.documentElement.webkitRequestFullscreen)
                document.documentElement.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT);
        } else {
            if (document.exitFullscreen)
                document.exitFullscreen();
            else if (document.mozCancelFullScreen)
                document.mozCancelFullScreen();
            else if (document.webkitExitFullscreen)
                document.webkitExitFullscreen();
        }
    }, false);

    document.getElementById("menuInteractive").addEventListener("click", function(e) {
        placing.beginInteractiveCreation();
    }, false);

    document.getElementById("menuOrbital").addEventListener("click", function(e) {
        placing.beginOrbitalCreation();
    }, false);
}

function initPopup(name, visible) {
    var popup = document.getElementById(name);
    var style = popup.children[1].style;

    var toggleButton = document.createElement("input");
    toggleButton.setAttribute("type", "button");
    toggleButton.setAttribute("class", "popupToggle");
    popup.appendChild(toggleButton);

    var toggle = function(e) {
        if (style.display === "table") {
            style.display = "none";
            toggleButton.setAttribute("value", "\u25BC");
        } else {
            style.display = "table";
            toggleButton.setAttribute("value", "\u25B2");
        }
    }

    popup.children[0].addEventListener("dblclick", toggle, false);
    toggleButton.addEventListener("click", toggle, false);

    if (visible) {
        style.display = "table";
        toggleButton.setAttribute("value", "\u25B2");
    } else {
        style.display = "none";
        toggleButton.setAttribute("value", "\u25BC");
    }
}

function initSpeedPopup() {
    initPopup("speedPopup", true);

    document.getElementById("speedRange").addEventListener("input", function(e) {
        universe.speed = e.target.valueAsNumber;
    }, false);

    document.getElementById("speedPauseResume").addEventListener("click", function(e) {
        var range = document.getElementById("speedRange");
        if (range.valueAsNumber === 0.0) {
            /* TODO - store previous value to resume to. */
            range.value = 1;
            e.target.value = "Pause";
        } else {
            range.value = 0;
            e.target.value = "Resume";
        }
        universe.speed = range.valueAsNumber;
    }, false);

    document.getElementById("speedFastForward").addEventListener("click", function(e) {
        var range = document.getElementById("speedRange");
        var value = range.valueAsNumber;
        if (value === 0.0 || value === parseFloat(range.max)) {
            range.value = 1;
        } else {
            range.value = range.value * 2;
        }
        document.getElementById("speedPauseResume").value = "Pause";
        universe.speed = range.valueAsNumber;
    }, false);
}

function initViewSettings() {
    initPopup("viewPopup", true);

    document.getElementById("pathLength").addEventListener("change", function(e) {
        universe.pathLength = e.target.valueAsNumber;
    }, false);

    document.getElementById("pathDistance").addEventListener("change", function(e) {
        universe.pathRecordDistance = e.target.valueAsNumber;
        universe.pathRecordDistance *= universe.pathRecordDistance;
    }, false);

    document.getElementById("drawGrid").addEventListener("change", function(e) {
        grid.enabled = e.target.checked;
    }, false);
}

function initCameraControls() {
    initPopup("cameraPopup");

    document.getElementById("followPrevious").addEventListener("click", function(e) {
        camera.followPrevious();
    }, false);

    document.getElementById("followNext").addEventListener("click", function(e) {
        camera.followNext();
    }, false);

    document.getElementById("followPlain").addEventListener("click", function(e) {
        camera.followPlainAverage();
    }, false);

    document.getElementById("followWeighted").addEventListener("click", function(e) {
        camera.followWeightedAverage();
    }, false);

    document.getElementById("followSelected").addEventListener("click", function(e) {
        camera.followSelection();
    }, false);

    document.getElementById("clearFollow").addEventListener("click", function(e) {
        camera.clearFollow();
    }, false);
}

function initCreatePlanetPopup() {
    initPopup("createPlanetPopup");

    document.getElementById("createPlanetButton").addEventListener("click", function(e) {
        var position = [document.getElementById("createPositionX").valueAsNumber,
                        document.getElementById("createPositionY").valueAsNumber,
                        document.getElementById("createPositionZ").valueAsNumber];
        var velocity = [document.getElementById("createVelocityX").valueAsNumber * universe.velocityfac,
                        document.getElementById("createVelocityY").valueAsNumber * universe.velocityfac,
                        document.getElementById("createVelocityZ").valueAsNumber * universe.velocityfac];
        var mass = document.getElementById("createMass").valueAsNumber;

        universe.selected = universe.addPlanet(position, velocity, mass);
    }, false);
}

function initRandomPopup() {
    initPopup("randomPopup");

    document.getElementById("randomGenerateButton").addEventListener("click", function(e) {
        var amount = Math.min(document.getElementById("randomAmount").valueAsNumber, 50);

        if (document.getElementById("randomOrbital").checked) {
            universe.generateRandomOrbital(amount, universe.selected);
        } else {
            var range = document.getElementById("randomRange").valueAsNumber;
            var maxSpeed = document.getElementById("randomSpeed").valueAsNumber * universe.velocityfac;
            var maxMass = document.getElementById("randomMass").valueAsNumber;

            universe.generateRandom(amount, range, maxSpeed, maxMass);
        }
    }, false);

    document.getElementById("randomOrbital").addEventListener("change", function(e) {
        document.getElementById("randomRange").disabled = e.target.checked;
        document.getElementById("randomSpeed").disabled = e.target.checked;
        document.getElementById("randomMass").disabled = e.target.checked;
    }, false);
}

function initFiringPopup() {
    initPopup("firingPopup");

    document.getElementById("toggleFiringButton").addEventListener("click", function(e) {
        placing.enableFiringMode(placing.step !== Module.PlacingStep.Firing);
    }, false);

    document.getElementById("firingMass").addEventListener("change", function(e) {
        placing.firingMass = e.target.valueAsNumber;
    }, false);

    document.getElementById("firingSpeed").addEventListener("change", function(e) {
        placing.firingSpeed = e.target.valueAsNumber / universe.velocityfac;
    }, false);
}

function initLocalStoragePopup() {
    initPopup("localStoragePopup");

    var updateList = function() {
        var list = document.getElementById("savesList");

        while (list.firstChild)
            list.removeChild(list.firstChild);

        var tr = document.createElement('tr');
        var th1 = document.createElement('th');
        var th2 = document.createElement('th');
        var text = document.createElement('input');
        var button = document.createElement('input');

        text.type = "text";
        text.className = "tableInput";

        button.type = "button";
        button.value = "Save";
        button.className = "tableInput";
        button.addEventListener("click", function(e) {
            if (localStorage.getItem("planets-" + text.value) === null || confirm("Overwrite existing save?"))
                saveLocalStorage(text.value);

            updateList();
        }, false);

        th1.appendChild(text);
        th2.appendChild(button);
        tr.appendChild(th1);
        tr.appendChild(th2);
        list.appendChild(tr);

        for (var i = 0; i < localStorage.length; i++) {
            var name = localStorage.key(i);

            if (name.startsWith("planets-")) {
                var tr = document.createElement('tr');
                var th1 = document.createElement('th');
                var th2 = document.createElement('th');
                var loadButton = document.createElement('input');
                var delButton = document.createElement('input');

                loadButton.type = "button";
                loadButton.value = name.slice(8);
                loadButton.className = "tableInput";
                loadButton.addEventListener("click", function(e) {
                    if (universe.size() === 0 || confirm("Load selected universe? (The current universe will not be saved)"))
                        loadLocalStorage(e.target.value);
                }, false);

                delButton.type = "button";
                delButton.value = "X";
                delButton.className = "tableInput";
                delButton.fileName = name;
                delButton.addEventListener("click", function(e) {
                    if (confirm("Are you sure you want to delete the saved universe?"))
                        localStorage.removeItem(e.target.fileName);

                    updateList();
                }, false);

                th1.appendChild(loadButton);
                th2.appendChild(delButton);
                tr.appendChild(th1);
                tr.appendChild(th2);
                list.appendChild(tr);
            }
        }
    }

    updateList();
}

var Module = {
    onRuntimeInitialized: function() {
        var canvas = document.getElementById("canvas");

        initGL();

        window.onresize = function(e) {
            canvas.width = window.innerWidth;
            canvas.height = window.innerHeight;
            camera.resizeViewport(window.innerWidth, window.innerHeight);
            GLctx.viewport(0, 0, window.innerWidth, window.innerHeight);
        };

        universe = new Module.PlanetsUniverse();

        camera = new Module.Camera(universe);

        placing = new Module.PlacingInterface(universe);

        gamepad = new Module.Gamepad(universe, camera, placing);
        gamepad.init();

        /* To track the button being pressed during mousemove. */
        var button = -1;

        /* Variables for tracking the mouse and getting motion delta. */
        var lastMouseX, lastMouseY;

        canvas.addEventListener("mousedown", function(e) {
            button = e.button;
        }, false);
        canvas.addEventListener("mousemove", function(e) {
            var deltaX = lastMouseX - e.clientX;
            var deltaY = lastMouseY - e.clientY;

            var placingBools = placing.handleMouseMove([e.clientX, e.clientY], [deltaX, deltaY], camera);

            if (!placingBools[0]) {
                switch (button) {
                case 1:
                    camera.distance += deltaY;
                    break;
                case 2:
                    camera.zrotation -= deltaX * 0.05;
                    camera.xrotation -= deltaY * 0.02;
                    break;
                }

                camera.bound();
            } else if (placingBools[1]) {
                /* TODO - Implement. */
            }

            /* Keep track of these values for the next event. */
            lastMouseX = e.clientX;
            lastMouseY = e.clientY;

            e.preventDefault();
        }, false);

        /* Letting go of mouse button anywhere needs to reset the button for dragging. */
        document.addEventListener("mouseup", function(e) {
            button = -1;
        }, false);

        canvas.addEventListener("click", function(e) {
            if (e.button === 0 && !placing.handleMouseClick([e.clientX, e.clientY], camera))
                camera.selectUnder([e.clientX, e.clientY], 1);
        }, false);

        canvas.addEventListener("dblclick", function(e) {
            if (e.button === 0 && placing.step === Module.PlacingStep.NotPlacing)
                camera.followSelection();
        }, false);

        canvas.addEventListener("wheel", function(e) {
            if (!placing.handleMouseWheel(e.deltaY * -0.01)) {
                camera.distance += camera.distance * e.deltaY * 0.01;

                camera.bound();
            }
        }, false);

        document.addEventListener("keydown", function(e) {
            /* CTRL+S */
            if (e.keyCode === 83 && (e.ctrlKey)) {
                downloadDOM(generateDOM());
                e.preventDefault();
            }
            /* CTRL+O */
            if (e.keyCode === 79 && (e.ctrlKey)) {
                document.getElementById("loadFile").click();
                e.preventDefault();
            }
            /* DEL */
            if (e.keyCode === 46) {
                universe.remove(universe.selected);
                e.preventDefault();
            }
        }, false);

        initFileUI(canvas);
        initMenu();
        initSpeedPopup();
        initViewSettings();
        initCameraControls();
        initCreatePlanetPopup();
        initRandomPopup();
        initFiringPopup();
        initLocalStoragePopup();

        /* Call this once to make sure it's the right size. */
        window.onresize();

        try {
            /* Attempt to load from URL. */
            loadBase64(window.location.search.substring(1));
        } catch (e) {
            try {
                loadUrl("systems/default.xml");
            } catch (e) { }
        }

        document.body.removeChild(document.getElementById("loading"));

        requestAnimationFrame(animate);
    }
}
