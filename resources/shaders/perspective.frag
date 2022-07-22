#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D sTexture;

void main() {
    vec2 coords;
    float xDist;
    float yDist;
    float offset;
    float dir;

    xDist = distance(gl_TexCoord[0].x, 0.5);
    yDist = distance(gl_TexCoord[0].y, 0.5);
    offset = (xDist * 0.2) * yDist;

    dir = (int(gl_TexCoord[0].y <= 0.5) - 0.5) * 2;

    coords = vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + xDist * (offset * 8.0 * dir));

    gl_FragColor = gl_Color * texture2D(sTexture, coords);
}