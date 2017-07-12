var context, grid;

var colorShader, colorCameraMat, colorModelMat, colorColor;
var textureShader, textureCameraMat, textureViewMat, textureModelMat, textureLightDir;
var planetTextureDiff, planetTextureNrm;
var highResVBO, highResTriIBO, highResTriCount;
var lowResVBO, lowResLineIBO, lowResLineCount;
var circleVBO, circleLineIBO, circleLineCount;
var arrowVBO, arrowIBO;
var gridBuf;

/* Convinience function to create a matrix with a position and scale value. */
function makeMat(pos, scale) {
    return [scale, 0.0, 0.0, 0.0,
            0.0, scale, 0.0, 0.0,
            0.0, 0.0, scale, 0.0,
            pos[0], pos[1], pos[2], 1.0 ]
}

/* Init an individual shader component from DOM element. */
function initShader(element) {
    if (!element)
        return null;

    var shader;
    if (element.type === "x-shader/x-vertex")
        shader = GLctx.createShader(GLctx.VERTEX_SHADER);
    else if (element.type === "x-shader/x-fragment")
        shader = GLctx.createShader(GLctx.FRAGMENT_SHADER);
    else
        return null;

    var source = "";
    var child = element.firstChild;

    /* Join al element children to get the full shader source code. */
    while (child) {
        if (child.nodeType === 3)
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

/* Load and compile a shader brogram based on vertex and fragment shader element IDs. */
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
    textureViewMat = GLctx.getUniformLocation(textureShader, "viewMatrix");
    textureModelMat = GLctx.getUniformLocation(textureShader, "modelMatrix");
    textureLightDir = GLctx.getUniformLocation(textureShader, "lightDir");

    var planetTextureDiffPromise = loadTexture("images/planet_diffuse.png");
    var planetTextureNrmPromise = loadTexture("images/planet_nrm.png");
    planetTextureDiffPromise.then(function(texture) {
        planetTextureDiff = texture;
    })
    planetTextureNrmPromise.then(function(texture) {
        planetTextureNrm = texture;
    })

    GLctx.uniform1i(GLctx.getUniformLocation(textureShader, "texture_diff"), 0);
    GLctx.uniform1i(GLctx.getUniformLocation(textureShader, "texture_nrm"), 1);

    grid = new Module.Grid();

    gridBuf = GLctx.createBuffer();
    GLctx.bindBuffer(GLctx.ARRAY_BUFFER, gridBuf);
    GLctx.bufferData(GLctx.ARRAY_BUFFER, 0, GLctx.DYNAMIC_DRAW);

    highResVBO = GLctx.createBuffer();
    highResTriIBO = GLctx.createBuffer();
    GLctx.bindBuffer(GLctx.ARRAY_BUFFER, highResVBO);
    GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, highResTriIBO);

    highResTriCount = Module.genSolidSphere();

    lowResVBO = GLctx.createBuffer();
    lowResLineIBO = GLctx.createBuffer();
    GLctx.bindBuffer(GLctx.ARRAY_BUFFER, lowResVBO);
    GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, lowResLineIBO);

    lowResLineCount = Module.genWireSphere();

    circleVBO = GLctx.createBuffer();
    circleLineIBO = GLctx.createBuffer();
    GLctx.bindBuffer(GLctx.ARRAY_BUFFER, circleVBO);
    GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, circleLineIBO);

    circleLineCount = Module.genCircle();

    arrowVBO = GLctx.createBuffer();
    arrowIBO = GLctx.createBuffer();
    GLctx.bindBuffer(GLctx.ARRAY_BUFFER, arrowVBO);
    GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, arrowIBO);

    GLctx.bufferData(GLctx.ARRAY_BUFFER, new Float32Array([0.1, 0.1, 0.0,
                                                           0.1,-0.1, 0.0,
                                                          -0.1,-0.1, 0.0,
                                                          -0.1, 0.1, 0.0,

                                                           0.1, 0.1, 1.0,
                                                           0.1,-0.1, 1.0,
                                                          -0.1,-0.1, 1.0,
                                                          -0.1, 0.1, 1.0,

                                                           0.2, 0.2, 1.0,
                                                           0.2,-0.2, 1.0,
                                                          -0.2,-0.2, 1.0,
                                                          -0.2, 0.2, 1.0,

                                                           0.0, 0.0, 1.4 ]), GLctx.DYNAMIC_DRAW);

    GLctx.bufferData(GLctx.ELEMENT_ARRAY_BUFFER, new Uint8Array([0,  1,  2,       2,  3,  0,

                                                                 1,  0,  5,       4,  5,  0,
                                                                 2,  1,  6,       5,  6,  1,
                                                                 3,  2,  7,       6,  7,  2,
                                                                 0,  3,  4,       7,  4,  3,

                                                                 5,  4,  9,       8,  9,  4,
                                                                 6,  5, 10,       9, 10,  5,
                                                                 7,  6, 11,      10, 11,  6,
                                                                 4,  7,  8,      11,  8,  7,

                                                                 9,  8, 12,
                                                                10,  9, 12,
                                                                11, 10, 12,
                                                                 8, 11, 12 ]), GLctx.STATIC_DRAW);

    return Promise.all([planetTextureDiffPromise, planetTextureNrmPromise]);
}

function loadTexture(filename) {
    return new Promise(function(resolve, reject) {
        var image = new Image();
        var texture = GLctx.createTexture();

        image.onload = function() {
            GLctx.bindTexture(GLctx.TEXTURE_2D, texture);
            GLctx.texImage2D(GLctx.TEXTURE_2D, 0, GLctx.RGBA, GLctx.RGBA, GLctx.UNSIGNED_BYTE, image);

            GLctx.generateMipmap(GLctx.TEXTURE_2D);
            GLctx.texParameteri(GLctx.TEXTURE_2D, GLctx.TEXTURE_MAG_FILTER, GLctx.LINEAR);
            GLctx.texParameteri(GLctx.TEXTURE_2D, GLctx.TEXTURE_MIN_FILTER, GLctx.LINEAR_MIPMAP_LINEAR);
            resolve(texture)
        }
        image.onerror = function() {
            alert("Image \"" + filename + "\" failed to load! Planets may not render correctly.");
            reject()
        }

        image.src = filename;
    })
}

function paint() {
    GLctx.clear(GLctx.COLOR_BUFFER_BIT | GLctx.DEPTH_BUFFER_BIT);

    var cameraMat = camera.setup();

    /* Use the light & texture shader for drawing the planets. */
    GLctx.useProgram(textureShader);

    /* Bind the view and camera matricies. */
    GLctx.uniformMatrix4fv(textureViewMat, false, camera.viewMat);
    GLctx.uniformMatrix4fv(textureCameraMat, false, cameraMat);

    GLctx.activeTexture(GLctx.TEXTURE0);
    GLctx.bindTexture(GLctx.TEXTURE_2D, planetTextureDiff)
    GLctx.activeTexture(GLctx.TEXTURE1);
    GLctx.bindTexture(GLctx.TEXTURE_2D, planetTextureNrm)

    GLctx.bindBuffer(GLctx.ARRAY_BUFFER, highResVBO);
    GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, highResTriIBO);

    GLctx.enableVertexAttribArray(0);
    GLctx.enableVertexAttribArray(1);
    GLctx.enableVertexAttribArray(2);
    GLctx.enableVertexAttribArray(3);
    GLctx.vertexAttribPointer(0, 3, GLctx.FLOAT, GLctx.FALSE, 48, 0);
    GLctx.vertexAttribPointer(1, 2, GLctx.FLOAT, GLctx.FALSE, 48, 36);
    GLctx.vertexAttribPointer(2, 3, GLctx.FLOAT, GLctx.FALSE, 48, 12);
    GLctx.vertexAttribPointer(3, 3, GLctx.FLOAT, GLctx.FALSE, 48, 24);

    /* Update the view-space light vector. */
    GLctx.uniform3fv(textureLightDir, camera.getLightDir());

    for (var i = 0; i < universe.size(); ++i) {
        GLctx.uniformMatrix4fv(textureModelMat, false, makeMat(universe.getPlanetPosition(i), universe.getPlanetRadius(i)));

        GLctx.drawElements(GLctx.TRIANGLES, highResTriCount, GLctx.UNSIGNED_INT, 0);
    }

    GLctx.disableVertexAttribArray(1);
    GLctx.disableVertexAttribArray(2);
    GLctx.disableVertexAttribArray(3);

    GLctx.useProgram(colorShader);

    GLctx.uniformMatrix4fv(colorCameraMat, false, cameraMat);

    GLctx.bindBuffer(GLctx.ARRAY_BUFFER, lowResVBO);
    GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, lowResLineIBO);
    GLctx.vertexAttribPointer(0, 3, GLctx.FLOAT, GLctx.FALSE, 48, 0);

    GLctx.uniform4fv(colorColor, [0.0, 1.0, 0.0, 1.0]);

    if (placing.step !== Module.PlacingStep.NotPlacing && placing.step !== Module.PlacingStep.Firing) {
        GLctx.uniformMatrix4fv(colorModelMat, false, makeMat(placing.getPosition(),  placing.getRadius()));

        GLctx.drawElements(GLctx.LINES, lowResLineCount, GLctx.UNSIGNED_INT, 0);

        GLctx.uniform4fv(colorColor, [1.0, 1.0, 1.0, 1.0]);

        /* Draw two circles to show the relative orbits of both the planets. */
        if ((placing.step === Module.PlacingStep.OrbitalPlane || placing.step === Module.PlacingStep.OrbitalPlanet) && universe.isSelectedValid()) {
            GLctx.bindBuffer(GLctx.ARRAY_BUFFER, circleVBO);
            GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, circleLineIBO);

            GLctx.vertexAttribPointer(0, 3, GLctx.FLOAT, 0, 12, 0);

            GLctx.uniformMatrix4fv(colorModelMat, false, placing.getOrbitalCircleMat());
            GLctx.drawElements(GLctx.LINES, circleLineCount, GLctx.UNSIGNED_INT, 0);

            GLctx.uniformMatrix4fv(colorModelMat, false, placing.getOrbitedCircleMat());
            GLctx.drawElements(GLctx.LINES, circleLineCount, GLctx.UNSIGNED_INT, 0);
        }

        if (placing.step === Module.PlacingStep.FreeVelocity) {
            var length = placing.getArrowLength() / universe.velocityfac();

            GLctx.uniformMatrix4fv(colorModelMat, false, placing.getArrowMat());

            GLctx.bindBuffer(GLctx.ARRAY_BUFFER, arrowVBO);
            GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, arrowIBO);

            /* Keep the tip of the arrow from stretching. */
            GLctx.bufferSubData(GLctx.ARRAY_BUFFER, 152, new Float32Array([ 1.0 + 0.4 / length]));

            GLctx.vertexAttribPointer(0, 3, GLctx.FLOAT, 0, 0, 0);
            GLctx.drawElements(GLctx.TRIANGLES, 66, GLctx.UNSIGNED_BYTE, 0);
        }
    } else if (universe.isSelectedValid()) {
        GLctx.uniformMatrix4fv(colorModelMat, false, makeMat(universe.getPlanetPosition(universe.selected),
                                                             universe.getPlanetRadius(universe.selected) * 1.02));

        GLctx.drawElements(GLctx.LINES, lowResLineCount, GLctx.UNSIGNED_INT, 0);
    }

    /* A circle in the center of the view to show where the gamepad is focused.
     * Not needed when placing or following as the target planet serves the same function. */
    if (gamepad.attached && placing.step === Module.PlacingStep.NotPlacing && camera.followingState === Module.FollowingState.FollowNone) {
        /* Draw it over everything. */
        GLctx.disable(GLctx.DEPTH_TEST);
        GLctx.uniform4fv(colorColor, [0.0, 1.0, 1.0, 1.0]);

        GLctx.uniformMatrix4fv(colorModelMat, false, makeMat(camera.position, camera.distance * 4.0e-3));

        GLctx.drawElements(GLctx.LINES, circleLineCount, GLctx.UNSIGNED_INT, 0);
        GLctx.enable(GLctx.DEPTH_TEST);
    }

    if (grid.enabled) {
        GLctx.bindBuffer(GLctx.ARRAY_BUFFER, gridBuf);
        grid.update(camera);

        GLctx.vertexAttribPointer(0, 2, GLctx.FLOAT, 0, 0, 0);

        /* Don't write to depth buffer. */
        GLctx.depthMask(false);

        var color = grid.color;
        color[3] *= grid.alphafac;

        var gridMat = makeMat([0.0, 0.0, 0.0], grid.scale);

        GLctx.uniformMatrix4fv(colorModelMat, false, gridMat);

        GLctx.uniform4fv(colorColor, color);

        /* Draw the large grid. */
        GLctx.drawArrays(GLctx.LINES, 0, grid.numPoints());

        /* Modify the matrix to be half the size. */
        gridMat[0] *= 0.5; gridMat[5] *= 0.5; gridMat[10] *= 0.5;
        GLctx.uniformMatrix4fv(colorModelMat, false, gridMat);

        color[3] = grid.color[3] - color[3];
        GLctx.uniform4fv(colorColor, color);

        /* Draw the small grid. */
        GLctx.drawArrays(GLctx.LINES, 0, grid.numPoints());

        GLctx.depthMask(true);
    }

    if (document.getElementById("drawTrails").checked) {
        /* Trails are in world space. */
        GLctx.uniformMatrix4fv(colorModelMat, false, IDENTITY_MATRIX);
        GLctx.uniform4fv(colorColor, [1.0, 1.0, 1.0, 1.0]);

        /* The actual drawing of the trails is handled in C++. */
        universe.drawTrails();
    }
}

var lastTime = null;

function animate(time) {
    if (lastTime === null)
        lastTime = time;

    /* Limit advance time to 10 seconds. */
    var delta = Math.min(time - lastTime, 10.0) * 1000;
    lastTime = time;

    /* We don't advance when placing. */
    if (placing.step === Module.PlacingStep.NotPlacing || placing.step === Module.PlacingStep.Firing)
        universe.advance(delta);

    gamepad.pollInput();
    gamepad.doAxisInput(delta);

    paint();

    requestAnimationFrame(animate);

    /* Keep the speed slider updated. */
    document.getElementById("speedRange").value = universe.speed
}

const IDENTITY_MATRIX = [1.0, 0.0, 0.0, 0.0,
                         0.0, 1.0, 0.0, 0.0,
                         0.0, 0.0, 1.0, 0.0,
                         0.0, 0.0, 0.0, 1.0];
