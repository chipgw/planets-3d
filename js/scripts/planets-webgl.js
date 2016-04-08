var spheres, context;

var colorShader, colorCameraMat, colorModelMat, colorColor;
var textureShader, textureCameraMat, textureModelMat, planetTexture;

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
    GLctx.attachShader(program, vertex);
    GLctx.attachShader(program, fragment);
    GLctx.linkProgram(program);

    if (!GLctx.getProgramParameter(program, GLctx.LINK_STATUS)) {
        alert("Could not initialise shader");
    }

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

    var ident = Module.getIdentityMatrix();

    GLctx.useProgram(colorShader);

    GLctx.uniformMatrix4fv(colorCameraMat, false, cameraMat);
    GLctx.uniformMatrix4fv(colorModelMat, false, ident);
    GLctx.uniform4fv(colorColor, [1.0, 1.0, 1.0, 1.0]);

    spheres.bindWire()

    spheres.drawWire()

    GLctx.useProgram(textureShader);

    GLctx.uniformMatrix4fv(textureCameraMat, false, cameraMat);
    GLctx.uniformMatrix4fv(textureModelMat, false, ident);

    spheres.bindSolid()

    spheres.drawSolid()
}

var lastTime = null;

function animate(time) {
    if (lastTime === null)
        lastTime = time;

    var delta = Math.min(time - lastTime, 10.0);
    lastTime = time;

    universe.advance(delta);

    paint();

    requestAnimationFrame(animate);
}
