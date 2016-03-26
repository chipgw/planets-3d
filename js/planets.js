mergeInto(LibraryManager.library, {
    loadTextureJS: function (filename, texture) {
        var image = new Image();
        image.onload = function() {
            GLctx.bindTexture(GLctx.TEXTURE_2D, GL.textures[texture]);
            GLctx.texImage2D(GLctx.TEXTURE_2D, 0, GLctx.RGBA, GLctx.RGBA, GLctx.UNSIGNED_BYTE, image);

            GLctx.generateMipmap(GLctx.TEXTURE_2D);
            GLctx.texParameteri(GLctx.TEXTURE_2D, GLctx.TEXTURE_MAG_FILTER, GLctx.LINEAR);
            GLctx.texParameteri(GLctx.TEXTURE_2D, GLctx.TEXTURE_MIN_FILTER, GLctx.LINEAR_MIPMAP_LINEAR);
        }
        image.src = Pointer_stringify(filename);
    }
});
