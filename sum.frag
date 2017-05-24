uniform sampler2D diff;
uniform int imageHeight;

void main() {
	float sum = 0.0;

	float imgH = float(imageHeight);

	for (int row = 0; row < imageHeight; ++row) {
		float crow = float(row) / imgH;
		sum += texture2D(diff, vec2(gl_TexCoord[0].s, crow)).r;
	}

	gl_FragColor.r = sum / imgH;
	gl_FragColor.g = 1.0;
	gl_FragColor.b = 0.0;
	gl_FragColor.a = 1.0;
}