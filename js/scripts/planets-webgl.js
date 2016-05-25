var spheres, context, grid;

var colorShader, colorCameraMat, colorModelMat, colorColor;
var textureShader, textureCameraMat, textureModelMat, planetTexture;

const IDENTITY_MATRIX = [1.0, 0.0, 0.0, 0.0,
                         0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0,
                         0.0, 0.0, 0.0, 1.0];

function initShader(element) {
    if (!element)
        return null;

    var shader;
    if (element.type == "x-shader/x-vertex")
        shader = GLctx.createShader(GLctx.VERTEX_SHADER);
    else if (element.type == "x-shader/x-fragment")
        shader = GLctx.createShader(GLctx.FRAGMENT_SHADER);
    else
        return null;

    var source = "";
    var child = element.firstChild;

    while (child) {
        if (child.nodeType == 3)
            source += child.textContent;
        child = child.nextSibling;
    }

    GLctx.shaderSource(shader, source);
    GLctx.compileShader(shader);

    if (!GLctx.getShaderParameter(shader, GLctx.COMPILE_STATUS)) {
        alert(GLctx.getShaderInfoLog(shader));
        return null;
    }

    return shader;
}

function initShaderProgram(vsh, fsh) {
    var vertex = initShader(document.getElementById(vsh));
    var fragment = initShader(document.getElementById(fsh));

    var program = GLctx.createProgram();

    /* Make sure the attributes are bound to the same values used in C++. */
    GLctx.bindAttribLocation(program, 0, "vertex");
    GLctx.bindAttribLocation(program, 1, "uv");

    GLctx.attachShader(program, vertex);
    GLctx.attachShader(program, fragment);
    GLctx.linkProgram(program);

    if (!GLctx.getProgramParameter(program, GLctx.LINK_STATUS))
        alert("Could not initialise shader");

    GLctx.useProgram(program);

    return program;
}

function initGL() {
    var canvas = document.getElementById("canvas");

    context = GL.createContext(canvas, {});

    GL.makeContextCurrent(context);

    GLctx.clearColor(0.0, 0.0, 0.0, 1.0);
    GLctx.enable(GLctx.DEPTH_TEST);
    GLctx.depthFunc(GLctx.LEQUAL);

    GLctx.enable(GLctx.CULL_FACE);
    GLctx.cullFace(GLctx.BACK);

    GLctx.enable(GLctx.BLEND);
    GLctx.blendFunc(GLctx.SRC_ALPHA, GLctx.ONE_MINUS_SRC_ALPHA);

    GLctx.clear(GLctx.COLOR_BUFFER_BIT | GLctx.DEPTH_BUFFER_BIT);

    colorShader = initShaderProgram("color-vertex", "color-fragment");

    colorCameraMat = GLctx.getUniformLocation(colorShader, "cameraMatrix");
    colorModelMat = GLctx.getUniformLocation(colorShader, "modelMatrix");
    colorColor = GLctx.getUniformLocation(colorShader, "color");

    textureShader = initShaderProgram("texture-vertex", "texture-fragment");

    textureCameraMat = GLctx.getUniformLocation(textureShader, "cameraMatrix");
    textureModelMat = GLctx.getUniformLocation(textureShader, "modelMatrix");

    planetTexture = loadTexture("images/planet.png");

    spheres = new Module.Spheres();

    grid = new Module.Grid();
}

function loadTexture(filename) {
    var image = new Image();
    var texture = GLctx.createTexture();

    image.onload = function() {
        GLctx.bindTexture(GLctx.TEXTURE_2D, texture);
        GLctx.texImage2D(GLctx.TEXTURE_2D, 0, GLctx.RGBA, GLctx.RGBA, GLctx.UNSIGNED_BYTE, image);

        GLctx.generateMipmap(GLctx.TEXTURE_2D);
        GLctx.texParameteri(GLctx.TEXTURE_2D, GLctx.TEXTURE_MAG_FILTER, GLctx.LINEAR);
        GLctx.texParameteri(GLctx.TEXTURE_2D, GLctx.TEXTURE_MIN_FILTER, GLctx.LINEAR_MIPMAP_LINEAR);
    }
    image.src = filename;

    return texture;
}

function paint() {
    GLctx.clear(GLctx.COLOR_BUFFER_BIT | GLctx.DEPTH_BUFFER_BIT);

    var cameraMat = camera.setup();

    GLctx.useProgram(textureShader);

    GLctx.uniformMatrix4fv(textureCameraMat, false, cameraMat);

    var curKey = 0;
    /* Will be the first key in the list. */
    var startKey = universe.nextKey(0);

    spheres.bindSolid()

    while (curKey !== startKey) {
        if (curKey === 0)
            curKey = universe.nextKey(0);

        var pos = universe.getPlanetPosition(curKey);
        var radius = universe.getPlanetRadius(curKey);

        var planetMat = [radius, 0.0, 0.0, 0.0,
                         0.0, radius, 0.0, 0.0,
                         0.0, 0.0, radius, 0.0,
                         pos[0], pos[1], pos[2], 1.0 ]

        GLctx.uniformMatrix4fv(textureModelMat, false, planetMat);

        spheres.drawSolid()

        curKey = universe.nextKey(curKey);
    }

    GLctx.useProgram(colorShader);

    GLctx.uniformMatrix4fv(colorCameraMat, false, cameraMat);

    spheres.bindWire()

    GLctx.uniform4fv(colorColor, [0.0, 1.0, 0.0, 1.0]);

    if (placing.step !== Module.PlacingStep.NotPlacing && placing.step !== Module.PlacingStep.Firing) {
        var pos = placing.getPosition();
        var radius = placing.getRadius();

        var planetMat = [radius, 0.0, 0.0, 0.0,
                         0.0, radius, 0.0, 0.0,
                         0.0, 0.0, radius, 0.0,
                         pos[0], pos[1], pos[2], 1.0 ]

        GLctx.uniformMatrix4fv(colorModelMat, false, planetMat);

        spheres.drawWire()

        if ((placing.step === Module.PlacingStep.OrbitalPlane || placing.step === Module.PlacingStep.OrbitalPlanet) && universe.isSelectedValid()) {
            spheres.bindCircle();
            GLctx.uniform4fv(colorColor, [1.0, 1.0, 1.0, 1.0]);

            GLctx.uniformMatrix4fv(colorModelMat, false, placing.getOrbitalCircleMat());
            spheres.drawCircle();

            GLctx.uniformMatrix4fv(colorModelMat, false, placing.getOrbitedCircleMat());
            spheres.drawCircle();
        }

        if (placing.step === Module.PlacingStep.FreeVelocity) {
            var length = placing.getArrowLength() / universe.velocityfac;

            GLctx.uniformMatrix4fv(colorModelMat, false, placing.getArrowMat());

            GLctx.uniform4fv(colorColor, [1.0, 1.0, 1.0, 1.0]);

            spheres.drawArrow(length);
        }
    } else if (universe.isSelectedValid()) {
        var pos = universe.getPlanetPosition(universe.selected);
        var radius = universe.getPlanetRadius(universe.selected) * 1.02;

        var planetMat = [radius, 0.0, 0.0, 0.0,
                         0.0, radius, 0.0, 0.0,
                         0.0, 0.0, radius, 0.0,
                         pos[0], pos[1], pos[2], 1.0 ]

        GLctx.uniformMatrix4fv(colorModelMat, false, planetMat);

        spheres.drawWire();
    }

    if (grid.enabled) {
        grid.update(camera);

        grid.bind();

        GLctx.depthMask(false);

        var color = grid.color;
        color[3] *= grid.alphafac;

        var gridMat = [grid.scale, 0.0, 0.0, 0.0,
                       0.0, grid.scale, 0.0, 0.0,
                       0.0, 0.0, grid.scale, 0.0,
                       0.0, 0.0, 0.0, 1.0 ]

        GLctx.uniformMatrix4fv(colorModelMat, false, gridMat);

        GLctx.uniform4fv(colorColor, color);

        grid.draw();

        gridMat[0] *= 0.5; gridMat[5] *= 0.5; gridMat[10] *= 0.5;
        GLctx.uniformMatrix4fv(colorModelMat, false, gridMat);

        color[3] = grid.color[3] - color[3];
        GLctx.uniform4fv(colorColor, color);

        grid.draw();

        GLctx.depthMask(true);
    }

    if (document.getElementById("drawTrails").checked) {
        GLctx.uniformMatrix4fv(colorModelMat, false, IDENTITY_MATRIX);
        GLctx.uniform4fv(colorColor, [1.0, 1.0, 1.0, 1.0]);

        universe.drawTrails();
    }
}

var lastTime = null;

function animate(time) {
    if (lastTime === null)
        lastTime = time;

    var delta = Math.min(time - lastTime, 10.0) * 1000;
    lastTime = time;

    if (placing.step === Module.PlacingStep.NotPlacing || placing.step === Module.PlacingStep.Firing)
        universe.advance(delta);

    gamepad.pollInput();
    gamepad.doAxisInput(delta);

    paint();

    requestAnimationFrame(animate);
}
